#include "File.h"

#include <errno.h>

namespace AudioTag
{
const char*  FileMP3::sffx[]     = { ".mp3", NULL };
const char*  FileOGGFlac::sffx[] = { ".ogg", NULL };
const char*  FileFlac::sffx[]    = { ".flac", NULL };
/* drop .aac - for some reason ffmpeg writing to an .aac container causes this 
 * crash; same opts to .m4a is fine
 */
const char*  FileM4a::sffx[]     = { ".m4a", ".mp4", ".alac", NULL };  // ffmpeg likes to force alac into a .m4a

std::ostream& operator<<(std::ostream& os_, const File& f_)
{
    return f_.meta().out(os_, f_);
}


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

    return f;
}

const char*  FileFactory::what(int err_)
{
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
