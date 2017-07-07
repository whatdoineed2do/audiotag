#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <utime.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <libgen.h>

#include <iostream>
#include <iomanip>
#include <list>
#include <string>
#include <utility>
#include <algorithm>

// taglib hrs
#include <taglib/tfile.h>
#include <taglib/fileref.h>
#include <taglib/mpegfile.h>
#include <taglib/flacfile.h>
#include <taglib/oggflacfile.h>

#include <taglib/tag.h>
#include <taglib/id3v1tag.h>
#include <taglib/id3v2header.h>
#include <taglib/id3v2frame.h>
#include <taglib/id3v2tag.h>
#include <taglib/apetag.h>
#include <taglib/tlist.h>
#include <taglib/textidentificationframe.h>
#include <taglib/commentsframe.h>

#include <taglib/xiphcomment.h>

#include "audiotag.h"
#include "audiotagfile.h"
#include "audiotagmeta.h"
#include "audiotagmetaout.h"
#include "audiotagops.h"

using namespace std;


namespace AudioTag
{

const char*  _argv0 = NULL;
bool  _verbose = false;

void  _usage()
{
    cout << "usage: " << _argv0 << " [OPTION]... [mp3 FILES]" << endl
	 << endl
	 << "  [tag encoding options]" << endl
         << "      [-e  encoding= utf16be [latin1|utf8|utf16|utf16be|utf18le]" << endl
	 << "      [-u  locale]    locale to use for multibyte conversion" << endl
	 << endl
	 << "  [tagging options]" << endl
	 << "       -t  title" << endl
	 << "       -a  artist" << endl
	 << "       -A  album" << endl
	 << "       -c  comment" << endl
	 << "       -g  genre" << endl
	 << "       -y  year" << endl
	 << "       -T  track" << endl
	 << endl
	 << "  [maintainence options]" << endl
	 << "       -l             list tags (exclusive maintanence option" << endl
	 << "       -i [1|2|a|f|A] add meta to tags types; defaults to ID3v2/mp3 " << endl
	 << "       -d [1|2|a|f|A] delete tags" << endl
	 << "       -n X:Y         clone tag from X to Y only if X exists" << endl
	 << "       -r             remove art from main tags" << endl
	 << "       -C             clean tags, leaving only basic info" << endl
	 << "       -M encoding    parse current tags and convert from -M <E> -e <E'>" << endl
	 << "               [warn] will damage tags if you get the E wrong!" << endl
         << endl
	 << "  [out options]" << endl
	 << "      [-O {base,basic,json}]  format for tag output" << endl
	 << endl
	 << "  [misc options]" << endl
	 << "       -V             verbose" << endl;

    exit(1);
}


/* returns the name of the succesful locale
 * if arg is NULL, attempt to use existing locale/utf'd version of it
 * if that fails, attempt the hardcoded fallbacks
 */
const char*  _setlocale(const char*& locale_)
{
    const char*  attempts[] = {
        NULL, // placeholder
        "en_US.UTF-8",
        "en_GB.UTF-8",
        "C.utf8",
        "C.UTF-8"  // debian specific version
    };
    const char*  l = NULL;

    static const char* const  mbdata = "香港hōtiあいうえお";
    const size_t  n = strlen(mbdata)*sizeof(wchar_t);
    wchar_t*  w = (wchar_t*)malloc(n+1);


    std::string  tmp;
    if (locale_ == NULL)
    {
        const char*  env = getenv("LANG");
        l = setlocale(LC_ALL, NULL);
        const std::string  tmpl = l;
        MP3_TAG_NOTICE_VERBOSE("user default LANG=" << (env ? env : "") << " locale=" << (l ? l : ""));

        memset(w, 0, n+1);
        size_t  res = mbstowcs(w, mbdata, n);
        if (res != (size_t)-1) {
            return l;
        }
#if 0
        MP3_TAG_WARN("multibye conversions failed using user default LANG=" << (env ? env : "") << " locale=" << (l ? l : "") << " - attempting fallbacks");

        bool  good = true;
        if (env == NULL)
        {
            // no LANG set, lets see if we can make sense of the LC_ALL
            if (l == NULL) {
                good = false;
            }
        }
        else
        {
            if (strlen(env) > 7 && strcmp(env+1, ".UTF-8") == 0) {
                // hmm, LANG == "x.UTF-8" -- this failed, something wrong
                // internally, ignore this in fallbacks
                good = false;
            }
        }

        if (good) {
            tmp = (env ? env : l);
            //MP3_TAG_NOTICE("env=" << env << "  l=" << l << " tmp=" << tmp);  // ??? tmp never gets set??
            tmp += ".UTF-8";
            attempts[0] = tmp.c_str();
            MP3_TAG_NOTICE_VERBOSE("attempting to use UTF8 version of user LANG/locale=" << tmp);
        }
#endif
    }


    const char*  what = NULL;
    for (int i=0; i<sizeof(attempts)/sizeof(const char*); ++i)
    {
        what = attempts[i];
        if (what == NULL) {
            continue;
        }

        if ( (l = setlocale(LC_ALL, what)) == NULL) {
            MP3_TAG_WARN("failed setting locale=" << what);
        }
        else
        {
            // validate mb conversion possible with this locale

            memset(w, 0, n+1);
            size_t  res = mbstowcs(w, mbdata, n);
            if (res != (size_t)-1) {
                // all done, found it
                break;
            }
            MP3_TAG_WARN("failed for requested locale=" << what << "  mb conv res=" << res);
        }
        what = NULL;
    }
    free(w);

    return what;
}


void  _addupdop(AudioTag::OpUpdateTags*&  op_, const AudioTag::MetaTOI& toi_, const AudioTag::Input& input_, AudioTag::Ops&  ops_)
{
    if (op_ == NULL) {
        ops_.add( (op_ = new AudioTag::OpUpdateTags(toi_, input_)) );
    }
}

};


extern int    optind;
extern char*  optarg;


int main(int argc, char *argv[])
{
    AudioTag::_argv0 = basename(argv[0]);

    struct {
        bool  list;
        bool  listP;
        bool  clean;
        bool  mbconvert;
        bool  preserve;
        bool  removeart;

        AudioTag::MetaTOI  toi;

        AudioTag::MetaTOI  from;
        AudioTag::MetaTOI  to;


        AudioTag::Input  iflds;
        AudioTag::OpUpdateTags*  iop;  // not owned

        AudioTag::MetaOut*  mout;

        const char*  locale;
    } opts;
    opts.list = false;
    opts.listP = false;
    opts.clean = false;
    opts.mbconvert = false;
    opts.preserve = false;
    opts.removeart = false;
    opts.mout = NULL;
    opts.locale = NULL;
    opts.iop = NULL;

    /* what we're encoding from */
    TagLib::String::Type  mbenc = TagLib::String::UTF8;


    /* this is encoding for the frames, default it to utf8 */
    TagLib::String::Type  enc = TagLib::String::UTF8;

    AudioTag::Ops  ops;

    int c;
    while ( (c = getopt(argc, argv, "e:hla:pt:A:y:c:T:g:Dd:n:VM:Ci:O:u:r")) != EOF)
    {
	switch (c) {
	    case 'e':
	    {
		enc = AudioTag::parseEnc(optarg, TagLib::String::UTF8);
	    } break;

	    case 'l':
            {
                ops.add(new AudioTag::OpListTags());
            } break;

	    case 'L':  opts.listP = true;  break;

            case 't':  AudioTag::_addupdop(opts.iop, opts.toi, opts.iflds, ops);  opts.iflds.title = optarg;  break;
            case 'a':  AudioTag::_addupdop(opts.iop, opts.toi, opts.iflds, ops);  opts.iflds.artist = optarg;  break;
            case 'A':  AudioTag::_addupdop(opts.iop, opts.toi, opts.iflds, ops);  opts.iflds.album = optarg;  break;
            case 'y':  AudioTag::_addupdop(opts.iop, opts.toi, opts.iflds, ops);  opts.iflds.yr = optarg;  break;
            case 'c':  AudioTag::_addupdop(opts.iop, opts.toi, opts.iflds, ops);  opts.iflds.comment = optarg;  break;
            case 'T':  AudioTag::_addupdop(opts.iop, opts.toi, opts.iflds, ops);  opts.iflds.trackno = optarg;  break;
            case 'g':  AudioTag::_addupdop(opts.iop, opts.toi, opts.iflds, ops);  opts.iflds.genre = optarg;  break;

            // clone from tag X to Y if X exists
            case 'n':
            {
                // x:y
                if (strlen(optarg) == 3 && optarg[1] == ':') {
                    opts.from = optarg[0];
                    opts.to   = optarg[2];
                    if (opts.to && opts.from && opts.to.operator!=(opts.from) && 
                        opts.to.all == false && opts.from.all == false) 
                    {
                        ops.add(new AudioTag::OpCloneTags(opts.to, opts.from));
                    }
                    else
                    {
                        MP3_TAG_ERR("cloning from/to must be exclusive tags");
                        AudioTag::_usage();
                    }
               }
            } break;

            case 'i':
            {
                opts.toi = optarg;
            } break;

	    case 'd':
	    {
                ops.add(new AudioTag::OpRemoveTags(AudioTag::MetaTOI(optarg)));
	    } break;

            case 'V':
            {
                AudioTag::_verbose = true;
            } break;

	    case 'C':
            {
                opts.clean = true;
                ops.add(new AudioTag::OpCleanTags());
            } break;

	    case 'M':
	    {
		opts.mbconvert = true;
		mbenc = AudioTag::parseEnc(optarg, TagLib::String::Latin1);
	    } break;

            case 'r':
            {
                opts.removeart = true;
                ops.add(new AudioTag::OpRemoveArt());
            } break;

            case 'p':
                opts.preserve = true;
                break;

            case 'O':
                opts.mout = AudioTag::MetaOut::create(optarg);
                break;

            case 'u':
                opts.locale = optarg;
                break;

	    case 'h':
            default: AudioTag::_usage();
	}
    }

    if ( !(optind < argc) ) {
        MP3_TAG_ERR("no files specified");
        AudioTag::_usage();
    }

    const char*  l;
    if ( (l = AudioTag::_setlocale(opts.locale)) == NULL) {
        MP3_TAG_ERR("failed to set valid UTF8 locale - tried all fallbacks; verify locales (locale -a)");
        return -1;
    }
    if (opts.locale && l != opts.locale) {
        MP3_TAG_WARN("requested locale=" << opts.locale << " not suitable for mbyte conversions, using local=" << l);
    }
    MP3_TAG_NOTICE_VERBOSE("using locale=" << l);



    if (ops.empty()) {
        ops.add(new AudioTag::OpListTags());
    }

    opts.iflds.strip();

    if (opts.mout == NULL) {
        opts.mout = new AudioTag::MetaOut();
    }


    const char*  f;

    int  i = optind;
    while (i < argc)
    {
	f = argv[i++];

        AudioTag::File*  ff = NULL;
	TagLib::Tag*  tag = NULL;

        int  err;
        try
        {
            if ( !(ff = AudioTag::FileFactory::create(f, err, *opts.mout))) {
                MP3_TAG_WARN("failed to open - " << f << " - " << AudioTag::FileFactory::what(err) << " - ignored");
            }
        }
        catch (const std::exception& ex)
        {
            MP3_TAG_WARN("failed to open - " << f << " - " << ex.what() << " - ignored");
        }


        if (ff == NULL) {
            continue;
        }

        if ( !ops.readonly() && access(ff->taglibfile().name(), W_OK) < 0) {
            MP3_TAG_ERR(ff->taglibfile().name() << ": no write permissions, all operations skipped");
        }
        else
        {
            // execute in order they appeared on command line

            ops.execute(*ff);

            ff->meta().save();

            if (opts.preserve)
            {
                /* try to reset the timestamps on the file
                 */
                struct utimbuf  ub;
                ub.actime = ff->st().st_atime;
                ub.modtime =  ff->st().st_mtime;

                struct timeval  tv[2];
                memset(&tv[0], 0, sizeof(struct timeval));
                tv[1].tv_sec  = ff->st().st_mtim.tv_sec;
                tv[1].tv_usec = ff->st().st_mtim.tv_nsec/1000;

                //if ( utime(f, &ub) < 0) {
                if ( utimes(f, tv) < 0) {
                    MP3_TAG_WARN_VERBOSE("'" << f << "' unable to revert to original access times - " << strerror(errno));
                }
            }
        }
        delete ff;
    }
    delete opts.mout;
    return 0;
}
