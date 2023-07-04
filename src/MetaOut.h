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
    MetaOut() = default;
    virtual ~MetaOut() = default;

    MetaOut(const MetaOut&) = default;
    MetaOut& operator=(const MetaOut&) = default;
    MetaOut(MetaOut&&) = default;
    MetaOut& operator=(MetaOut&&) = default;

    virtual std::ostream&  out(std::ostream&, const File&);

    static MetaOut*  create(const char*);
};

struct MetaOutBasic : public MetaOut
{
    std::ostream&  out(std::ostream&, const File&)  override;
};

struct MetaOutJson: public MetaOut
{
    std::ostream&  out(std::ostream&, const File&)  override;
    static const char*  out(std::string& buf_, const TagLib::String& src_);
};

#ifdef HAVE_JSONC
struct MetaOutJsonC: public MetaOut
{
    std::ostream&  out(std::ostream&, const File&)  override;
};
#endif

};

#endif
