#ifndef AUDIOTAG_FILE_H
#define AUDIOTAG_FILE_H

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include <iostream>
#include <string>
#include <algorithm>

// taglib hrs
#include <taglib/tfile.h>
#include <taglib/fileref.h>
#include <taglib/mpegfile.h>
#include <taglib/flacfile.h>
#include <taglib/oggflacfile.h>
#include <taglib/mp4file.h>

#include "audiotagmeta.h"

namespace AudioTag
{
class MetaOut;


class File
{
  public:
    virtual ~File()
    { }

    Meta&  meta()
    { return _m; }


    const Meta&  meta() const
    { return _m; }

    TagLib::File&  taglibfile() const
    { return _taglibfile; }

    const struct stat&  st() const
    { return _st; }

    bool  writable() const
    { return !_taglibfile.readOnly(); } 


  protected:
    File(Meta& m_, const char* fn_, TagLib::File& f_) : _m(m_), _taglibfile(f_)
    {
        stat(fn_, &_st);
    }

  private:
    File(const File&);
    void operator=(const File&);

    Meta&  _m;
    TagLib::File&  _taglibfile;

    struct stat  _st;  // original file values;
};
std::ostream& operator<<(std::ostream& os_, const File& f_);


class FileMP3 : public File
{
  public:
    typedef TagLib::MPEG::File  value_type;
    static const char*  sffx[];

    FileMP3(const char* f_, MetaOut& mo_) 
	: _f(f_), 
	  _m(*this, mo_), 
	  File(_m, f_, _f)
    { }

    ~FileMP3()
    { }


  private:
    FileMP3(const FileMP3&);
    void operator=(const FileMP3&);

    FileMP3::value_type  _f;
    MetaMP3  _m;
};


class FileOGGFlac : public File
{
  public:
    static const char*  sffx[];

    FileOGGFlac(const char* f_, MetaOut& mo_)
        : _f(f_),
          _m(*this, mo_),
          File(_m, f_, _f)
    { }

    ~FileOGGFlac() 
    { }

  private:
    FileOGGFlac(const FileOGGFlac&);
    void operator=(const FileOGGFlac&);

    TagLib::Ogg::FLAC::File  _f;
    MetaOGGFlac  _m;
};



class FileFlac : public File
{
  public:
    static const char*  sffx[];

    FileFlac(const char* f_, MetaOut& mo_)
        : _f(f_),
          _m(*this, mo_),
          File(_m, f_, _f)
    { }

    ~FileFlac() 
    { }

  private:
    FileFlac(const FileFlac&);
    void operator=(const FileFlac&);

    TagLib::FLAC::File  _f;
    MetaFlac  _m;
};


class FileM4a : public File
{
  public:
    static const char*  sffx[];

    FileM4a(const char* f_, MetaOut& mo_)
        : _f(f_),
          _m(*this, mo_),
          File(_m, f_, _f)
    { }

    ~FileM4a() 
    { }

  private:
    FileM4a(const FileM4a&);
    void operator=(const FileM4a&);

    TagLib::MP4::File  _f;
    MetaM4a  _m;
};



class FileFactory
{
  public:
    static File*  create(const char* f_, int& errno_, MetaOut&);
    static const char*  what(int);

  private:
    FileFactory();
    FileFactory(const FileFactory&);
    void operator=(const FileFactory&);

    static bool  _match(const char* sffx_, const char* t_[])
    {
        const char**  p = t_;
        while (*p) {
            if (strcmp(sffx_, *p++) == 0) {
                return true;
            }
        }
        return false;
    }
};

};

#endif
