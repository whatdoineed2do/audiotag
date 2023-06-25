#include "Ops.h"

namespace AudioTag
{
    void  Ops::add(const Op*  op_)
    {
        if (!op_->readonly) {
            _readonly = false;
        }
       _ops.emplace_back(op_);
    }

    void  Ops::execute(File& f_) const
    {
        for (const auto& i : _ops) {
            i->execute(f_, _verbose);
        }
    }

    void  OpNested::_execute(File& f_, bool verbose_) const
    {
        for (const auto& i : _ops) {
            i->execute(f_, _verbose);
        }
    }

    void  OpListTags::_execute(File& f_, bool verbose_) const
    {
        std::cout << f_;
    }

    void  OpRemoveTags::_execute(File& f_, bool verbose_) const
    {
        f_.meta().remove(toi);
    }

    void  OpRemoveArt::_execute(File& f_, bool verbose_) const
    {
        f_.meta().removeart();
    }

    void  OpCleanTags::_execute(File& f_, bool verbose_) const
    {
        f_.meta().sanitize();
    }

    void  OpCloneIntnlTags::_execute(File& f_, bool verbose_) const
    {
        f_.meta().clone(_to, _from);
    }

    void  OpUpdateTags::_execute(File& f_, bool verbose_) const
    {
        if (!toi) {
            // nothing spec, use the default
            f_.meta() = input;
        }
        else {
            f_.meta().assign(toi, input);
        }
    }


    OpPropertyTags::OpPropertyTags(const AudioTag::MetaTOI& toi_, AudioTag::Input& input_, const OpPropertyTags::Map& m_)
        : _OpWR("merge property tags"), impl(toi_, input_), m(m_)
    {
        std::for_each(m.begin(), m.end(), [this, &input_](Map::value_type e)
        {
            const char*  prop  = e.first;
            const char*  value = e.second;

	    AUDIOTAG_DEBUG("prop='" << prop << "'  value='" << value << "'");

            TagLib::PropertyMap&  urgh = input_.properties;

            int  vl;
            if ( (vl = strlen(value)) == 0 || (vl == 2 && ( (value[0] == '\'' && value[1] == '\'') || (value[0] == '"' && value[1] == '"')) ) ) {
		AUDIOTAG_DEBUG("clearing prop='" << prop << "'");
                urgh[prop].clear();
            }
            else
            {
                const TagLib::String  converted = AudioTag::_cnvrt(value);
		AUDIOTAG_DEBUG("prop='" << prop << "'  converted value='" << value << "'");
                auto  where = urgh.find(prop);
                if (where == urgh.end()) {
		    AUDIOTAG_DEBUG("inserting prop='" << prop << "'");
                    urgh.insert(prop, TagLib::StringList(converted) );
                }
                else {
		    AUDIOTAG_DEBUG("replacing prop='" << prop << "'");
                    where->second.prepend(converted);
                }
            }

#ifdef DEBUG
	    AUDIOTAG_DEBUG("properties:");
	    for (const auto& i : urgh)
	    {
		std::stringstream  s;
		for (const auto& j : i.second)
		{
		    s << " \"" << AudioTag::_strrep(j) << "\"";
		}
		AUDIOTAG_DEBUG("  [" << i.first.toCString() << "] = {" << s.str() << " }");
	    }
#endif
        });
    }

    void  OpPropertyTags::_execute(File& f_, bool verbose_) const
    {
	impl._execute(f_, verbose_);
    }

    void  OpCloneFileMeta::_execute(File& f_, bool verbose_) const
    {
        f_.meta() = _meta;
    }

    void  OpAddArt::_execute(File& f_, bool verbose_) const
    {
	f_.meta().artwork(artwork);
    }
}
