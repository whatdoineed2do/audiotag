#ifndef AUDIOTAG_META_OUT_H
#define AUDIOTAG_META_OUT_H


#include "../config.h"
#include <iosfwd>

namespace TagLib
{ 
    class Tag; 
    class String;
};


namespace  AudioTag
{
class Meta;
class File;


/* different output formats
 */

class MetaOut
{
  public:
    MetaOut()  { }
    ~MetaOut() { }

    virtual std::ostream&  out(std::ostream&, const File&);

    static MetaOut*  create(const char*);


  private:
    MetaOut(const MetaOut&);
    void  operator=(const MetaOut&);
};

struct MetaOutBasic : public MetaOut
{
    std::ostream&  out(std::ostream&, const File&);
};

struct MetaOutJson: public MetaOut
{
    std::ostream&  out(std::ostream&, const File&);
    static const char*  out(std::string& buf_, const TagLib::String& src_);
};

#ifdef HAVE_JSONC
struct MetaOutJsonC: public MetaOut
{
    std::ostream&  out(std::ostream&, const File&);
};
#endif

};

#endif
