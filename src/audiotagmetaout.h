#ifndef AUDIOTAG_META_OUT_H
#define AUDIOTAG_META_OUT_H


#include <iosfwd>

namespace TagLib
{ class Tag; };


namespace  AudioTag
{
class Meta;


/* different output formats
 */

class MetaOut
{
  public:
    MetaOut()  { }
    ~MetaOut() { }

    virtual std::ostream&  out(std::ostream&, const Meta&, const TagLib::Tag&, const char*);

    static MetaOut*  create(const char*);


  private:
    MetaOut(const MetaOut&);
    void  operator=(const MetaOut&);
};

struct MetaOutBasic : public MetaOut
{
    std::ostream&  out(std::ostream&, const Meta&, const TagLib::Tag&, const char*);
};

struct MetaOutJson: public MetaOut
{
    std::ostream&  out(std::ostream&, const Meta&, const TagLib::Tag&, const char*);
};

};

#endif
