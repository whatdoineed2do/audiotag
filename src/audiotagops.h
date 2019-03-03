#ifndef AUDIO_TAG_OPS_H
#define AUDIO_TAG_OPS_H

#include <assert.h>
#include <list>
#include <map>
#include <memory>

#include "audiotag.h"
#include "audiotagmeta.h"
#include "audiotagfile.h"


namespace AudioTag
{

class Op {
  public:
    virtual ~Op() = default;
    Op(const Op& rhs_) : readonly(rhs_.readonly) { }

    Op&  operator=(const Op&) = delete;

    const bool  readonly;

    void  execute(File& f_, bool verbose_) const
    {
        if (verbose_)  MP3_TAG_NOTICE_VERBOSE(_descr);
        _execute(f_, verbose_);
        if (!readonly) {
            f_.meta().save();
        }
    }


  protected:
    Op(bool  b_, const char* descr_) : readonly(b_), _descr(descr_) { }

    virtual void  _execute(File&, bool verbose_) const = 0;

  private:
    const char*  _descr;
};

struct _OpRO : public Op
{
    virtual ~_OpRO() = default;
    _OpRO(const char* descr_) : Op(true, descr_) { }
};

struct _OpWR : public Op
{
    virtual ~_OpWR() = default;
    _OpWR(const char* descr_) : Op(false, descr_) { }
};


class Ops
{
  public:
    typedef std::list<const Op*>  _Ops;

    Ops() : _readonly(true) { }
    ~Ops();

    Ops(const Ops&)  = delete;
    Ops(const Ops&&) = delete;
    Ops&  operator=(const Ops&)  = delete;
    Ops&  operator=(const Ops&&) = delete;

    bool  empty() const
    { return _ops.empty(); }

    _Ops::size_type  size() const
    { return _ops.size(); }


    bool  readonly() const
    { return _readonly; }

    void  add(const Op*  op_);

    void  execute(File& f_) const;

  private:
    bool  _readonly;
    _Ops  _ops;
};


////////////////////////////////////////////////////////////////////////////////

struct OpListTags : public _OpRO
{
    OpListTags() : _OpRO("listing")  { }

    void  _execute(File& f_, bool verbose_) const
    {
        std::cout << f_;
    }
};


struct OpRemoveTags : public _OpWR
{
    OpRemoveTags(const AudioTag::MetaTOI& toi_) : _OpWR("removing tags"), toi(toi_)  { }

    const AudioTag::MetaTOI  toi;

    void  _execute(File& f_, bool verbose_) const
    {
        f_.meta().remove(toi);
    }
};

struct OpRemoveArt : public _OpWR
{
    OpRemoveArt() : _OpWR("removing artwork")  { }

    void  _execute(File& f_, bool verbose_) const
    {
        f_.meta().removeart();
    }
};

class Artwork;
struct OpAddArt : public _OpWR
{
    OpAddArt(AudioTag::Artwork& artwork_) : _OpWR("add artwork"), artwork(artwork_)  { }

    void  _execute(File& f_, bool verbose_) const;

    AudioTag::Artwork&  artwork;
};

struct OpCleanTags: public _OpWR
{
    OpCleanTags() : _OpWR("cleaning tags")  { }

    void  _execute(File& f_, bool verbose_) const
    {
        f_.meta().sanitize();
    }
};


struct OpCloneIntnlTags: public _OpWR
{
    OpCloneIntnlTags(const AudioTag::MetaTOI& to_, const AudioTag::MetaTOI& from_)
        : _OpWR("cloning tags"),
          _to(to_), _from(from_)
    { }

    const AudioTag::MetaTOI&  _to;
    const AudioTag::MetaTOI&  _from;

    void  _execute(File& f_, bool verbose_) const
    {
        f_.meta().clone(_to, _from);
    }
};


struct OpUpdateTags: public _OpWR
{
    OpUpdateTags(const AudioTag::MetaTOI& toi_, const AudioTag::Input& input_)
        : _OpWR("updating tags"),
          toi(toi_), input(input_)
    { }

    // both these have to be a ref since the inputs are created in stages
    const AudioTag::MetaTOI&  toi;
    const AudioTag::Input&    input;


    void  _execute(File& f_, bool verbose_) const
    {
        if (!toi) {
            // nothing spec, use the default
            f_.meta() = input;
        }
        else {
            f_.meta().assign(toi, input);
        }
    }
};

struct OpPropertyTags: public _OpWR
{
    using  Map = std::multimap<const char*, const char*>;

    OpPropertyTags(const AudioTag::MetaTOI& toi_, const AudioTag::Input& input_, const OpPropertyTags::Map& m_)
        : _OpWR("merge property tags"), impl(toi_, input_), m(m_)
    { }

    Map  m;
    OpUpdateTags  impl;

    void  _execute(File& f_, bool verbose_) const;
};

class OpCloneFileMeta: public _OpWR
{
  public:
    OpCloneFileMeta(AudioTag::File* src_)
        : _OpWR("cloning tags from file"),
          _src(src_), _meta(_src->meta().tag())
    { }

    ~OpCloneFileMeta()
    { delete _src; }

  protected:
    void  _execute(File& f_, bool verbose_) const
    {
        f_.meta() = _meta;
    }

  private:
    AudioTag::File*  _src;
    const AudioTag::Input  _meta;
};

};

#endif
