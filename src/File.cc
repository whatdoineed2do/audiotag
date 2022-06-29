#include "File.h"

#include "../config.h"

#include <errno.h>

#ifdef HAVE_FFMPEG
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavutil/log.h>

static void  _avlog_callback_null(void *ptr, int level, const char *fmt, va_list vl)
{ }
}
static int  _ffmpeg_err = 0;

#include <list>
static std::list<const char*>  ffmpeg_supported_fmts;
#endif


namespace AudioTag
{
const char*  FileMP3::sffx[]     = { ".mp3", NULL };
const char*  FileOGGFlac::sffx[] = { ".ogg", NULL };
const char*  FileFlac::sffx[]    = { ".flac", NULL };
/* drop .aac - this is a raw stream with no container and thus no meta 
 */
const char*  FileM4a::sffx[]     = { ".m4a", ".mp4", ".alac", NULL };  // ffmpeg likes to force alac into a .m4a

std::ostream& operator<<(std::ostream& os_, const File& f_)
{
    return f_.meta().out(os_, f_);
}

#ifdef HAVE_FFMPEG
File*  _ffmpeg_create(const char* file_, MetaOut& mo_)
{
    av_log_set_flags(AV_LOG_SKIP_REPEATED);
    av_log_set_callback(_avlog_callback_null);

    if (ffmpeg_supported_fmts.empty()) {
	const char**  fmts[] = {
	    FileFlac::sffx,
	    FileM4a::sffx,
	    FileMP3::sffx,
	    nullptr
	};
	const char*** pf = fmts;
	while (*pf)
	{
	    const char** p = *pf++;
	    while (*p) {
		ffmpeg_supported_fmts.push_back(**p == '.' ? *p+1 : *p);
		++p;
	    }
	}
    }

    int ret;

    AVFormatContext*  ctx = NULL;
    if ( (ret = avformat_open_input(&ctx, file_, NULL, NULL)) < 0) {
	_ffmpeg_err = ret;
	return NULL;
    }

    if ( (ret = avformat_find_stream_info(ctx, NULL)) < 0) {
        avformat_close_input(&ctx);
	_ffmpeg_err = ret;
        return NULL;
    }

    bool  hasvideo = true;

    File*  f = NULL;
 
    // find the container is supported and then worry about the codec
   auto  supported_container =  std::find_if(ffmpeg_supported_fmts.cbegin(), ffmpeg_supported_fmts.cend(), 
	[&ctx](const char* val_) {
	    return strstr(ctx->iformat->name, val_);
	});

    if (supported_container == ffmpeg_supported_fmts.cend()) {
	// container not supported
	_ffmpeg_err = AVERROR_DECODER_NOT_FOUND;
    }
    else if (ctx->video_codec_id == AV_CODEC_ID_NONE && ctx-> audio_codec_id == AV_CODEC_ID_NONE && ctx->nb_streams == 0) {
	// file is junk
	_ffmpeg_err = AVERROR_INVALIDDATA;
    }
    else
    {
	for (int i=0; i<ctx->nb_streams; ++i)
	{
	    switch (ctx->streams[i]->codecpar->codec_type)
	    {
		case AVMEDIA_TYPE_VIDEO:
		{
		    switch (ctx->streams[i]->codecpar->codec_id) {
			case AV_CODEC_ID_MJPEG:
			case AV_CODEC_ID_MJPEGB:
			    // embedded artwork, not video
			    hasvideo = false;
			    break;

			default:
			    hasvideo = true;
			    break;
		    }
		} break;

		/* WARN -- only consider the file's FIRST audio stream - if normal 
		 * siutations this is fine
		 */
		case AVMEDIA_TYPE_AUDIO:
		{
		    switch (ctx->streams[i]->codecpar->codec_id)
		    {
			case AV_CODEC_ID_MP3:
			    f = new FileMP3(file_, mo_);
			    break;

			case AV_CODEC_ID_AAC:
			case AV_CODEC_ID_ALAC:
			    f = new FileM4a(file_, mo_);
			    break;

			case AV_CODEC_ID_FLAC:
			    f = new FileFlac(file_, mo_);

			default:
			    break;
		    }
		} break;

		default:
		    break;
	    }

	    if (f) {
		break;
	    }
	}
    }
    avformat_close_input(&ctx);
    return f;
}
#endif

File*  FileFactory::create(const char* f_, int& errno_, MetaOut& mo_)
{
    errno_ = 0;
    if (f_ == NULL) {
        errno_ = EINVAL;
        return NULL;
    }

    struct stat  st;
    memset(&st, 0, sizeof(st));

    if (stat(f_, &st) < 0) {
        errno_ = errno == 0 ? ENOENT : errno;
        return NULL;
    }

    if ( ! (st.st_mode & (S_IFREG | S_IFLNK) ) ) {
        errno_ = ENOENT;
        return NULL;
    }

    //if ( ! (st.st_mode & S_IRUSR && st.st_mode & S_IWUSR) ) {
    if (access(f_, R_OK) < 0) { 
        errno_ = EPERM;
        return NULL;
    }


    File*  f = NULL;

#ifdef HAVE_FFMPEG
    f = _ffmpeg_create(f_, mo_);
#else
    const char*  p;
    if ( (p = strrchr(f_, '.')) == NULL) {
        return NULL;
    }

    std::string  tmp = p;
    std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);
    p = tmp.c_str();
    if (f == NULL && _match(p,  FileMP3::sffx))  f = new FileMP3(f_, mo_);
    if (f == NULL && _match(p, FileFlac::sffx))  f = new FileFlac(f_, mo_);
    if (f == NULL && _match(p,  FileM4a::sffx))  f = new FileM4a(f_, mo_);
    //if (f == NULL && _match(p, FileOGGFlac::sffx))  f = new FileOGGFlac(f_, mo_);
#endif

    return f;
}

const char*  FileFactory::what(int err_)
{
#ifdef HAVE_FFMPEG
    if (_ffmpeg_err) {
	static char  ffmpeg_errstr[1024] = { 0 };
	av_strerror(_ffmpeg_err, ffmpeg_errstr, 1024);
	return ffmpeg_errstr;
    }
#endif
    switch (err_)
    {
        case 0:  return "unknown/unhandled file format";
        case ENOENT:  return "Not a file";

        case EPERM:
        case EINVAL:
        default:
            return strerror(err_);
    }
}

};
