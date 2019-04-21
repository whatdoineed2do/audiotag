#include "audiotagops.h"

namespace AudioTag
{
    Ops::~Ops()
    {
        for (auto i : _ops) {
            delete i;
        }
    }

    void  Ops::add(const Op*  op_)
    {
        if (!op_->readonly) {
            _readonly = false;
        }
       _ops.push_back(op_);
    }

    void  Ops::execute(File& f_) const
    {
        for (const auto i : _ops) {
            i->execute(f_, _verbose);
        }
    }


    void  OpPropertyTags::_execute(File& f_, bool verbose_) const
    {
        std::for_each(m.begin(), m.end(), [this](Map::value_type e)
        {
            const char*  prop  = e.first;
            const char*  value = e.second;

            TagLib::PropertyMap&  urgh = const_cast<TagLib::PropertyMap&>(impl.input.properties);

            int  vl;
            if ( (vl = strlen(value)) == 0 || (vl == 2 && (value[0] == '\'' && value[1] == '\'' || value[0] == '"' && value[1] == '"')) ) {
                urgh[prop].clear();
            }
            else
            {
                const TagLib::String  converted = AudioTag::_cnvrt(value);
                auto  where = urgh.find(prop);
                if (where == urgh.end()) {
                    urgh.insert(prop, TagLib::StringList(converted) );
                }
                else {
                    where->second.prepend(converted);
                }
            }
        });

	impl._execute(f_, verbose_);
    }

    void  OpAddArt::_execute(File& f_, bool verbose_) const
    {
	f_.meta().artwork(artwork);
    }
}
