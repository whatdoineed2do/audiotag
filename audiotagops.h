#ifndef AUDIO_TAG_OPS_H
#define AUDIO_TAG_OPS_H

#include <assert.h>
#include <list>

#include "audiotag.h"
#include "audiotagmeta.h"
#include "audiotagfile.h"


namespace AudioTag
{

class Op {
  public:
    virtual ~Op()  { }
    Op(const Op& rhs_) : readonly(rhs_.readonly) { }

    const bool  readonly;

    bool  execute(File& f_, bool verbose_) const
    {
        if (verbose_)  MP3_TAG_NOTICE_VERBOSE(_descr);
        _execute(f_, verbose_);
    }


  protected:
    Op(bool  b_, const char* descr_) : readonly(b_), _descr(descr_) { }

    virtual bool  _execute(File&, bool verbose_) const = 0;

  private:
    Op&  operator=(const Op&);
    const char*  _descr;
};

struct _OpRO : public Op
{
    virtual ~_OpRO() { }
    _OpRO(const char* descr_) : Op(true, descr_) { }
};

struct _OpWR : public Op
{
    virtual ~_OpWR() { }
    _OpWR(const char* descr_) : Op(false, descr_) { }
};


class Ops
{
  public:
    typedef std::list<const Op*>  _Ops;

    Ops() : _readonly(true) { }
    ~Ops()
    {
        for (_Ops::iterator i=_ops.begin(); i!=_ops.end(); ++i) {
            delete *i;
        }
    }

    bool  empty() const
    { return _ops.empty(); }

    _Ops::size_type  size() const
    { return _ops.size(); }


    bool  readonly() const
    { return _readonly; }

    void  add(const Op*  op_)
    {
        assert(op_);
        if (!op_->readonly) {
            _readonly = false;
        }
       _ops.push_back(op_);
    }

    void  execute(File& f_) const
    {
        for (_Ops::const_iterator i=_ops.begin(); i!=_ops.end(); ++i) {
            (*i)->execute(f_, _verbose);
        }
    }

  private:
    Ops(const Ops&);
    Ops&  operator=(const Ops&);

    bool  _readonly;

    _Ops  _ops;
};


////////////////////////////////////////////////////////////////////////////////

struct OpListTags : public _OpRO
{
    OpListTags() : _OpRO("listing")  { }

    bool  _execute(File& f_, bool verbose_) const
    {
        std::cout << f_;
    }
};


struct OpRemoveTags : public _OpWR
{
    OpRemoveTags(const AudioTag::MetaTOI& toi_) : _OpWR("removing tags"), toi(toi_)  { }

    const AudioTag::MetaTOI  toi;

    bool  _execute(File& f_, bool verbose_) const
    {
        f_.meta().remove(toi);
    }
};

struct OpRemoveArt : public _OpWR
{
    OpRemoveArt() : _OpWR("removing artwork")  { }

    bool  _execute(File& f_, bool verbose_) const
    {
        f_.meta().removeart();
    }
};

struct OpCleanTags: public _OpWR
{
    OpCleanTags() : _OpWR("cleaning tags")  { }

    bool  _execute(File& f_, bool verbose_) const
    {
        f_.meta().sanitize();
    }
};


struct OpCloneTags: public _OpWR
{
    OpCloneTags(const AudioTag::MetaTOI& to_, const AudioTag::MetaTOI& from_)
        : _OpWR("cloning tags"),
          _to(to_), _from(from_)
    { }

    const AudioTag::MetaTOI&  _to;
    const AudioTag::MetaTOI&  _from;

    bool  _execute(File& f_, bool verbose_) const
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


    bool  _execute(File& f_, bool verbose_) const
    {
        f_.meta().sanitize();
        if (!toi) {
            // nothing spec, use the default
            f_.meta() = input;
        }
        else {
            f_.meta().assign(toi, input);
        }
    }
};




};

#endif
