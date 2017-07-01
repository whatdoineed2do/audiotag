#include "audiotagmetaout.h"

#include <iostream>
#include <iomanip>

#include "audiotagmeta.h"


namespace AudioTag
{
MetaOut*  MetaOut::create(const char* optarg_)
{
    if (optarg_ == NULL)                 return new MetaOut();
    if (strcmp(optarg_, "basic") == 0)   return new MetaOutBasic();
    if (strcmp(optarg_, "json")  == 0)   return new MetaOutJson();

    return new MetaOut();
}


std::ostream&  MetaOut::out(std::ostream& os_, const Meta& m_, const TagLib::Tag& tag_, const char* tagtype_)
{
    os_ 
        << "  Artist: " << std::left << std::setw(15) << AudioTag::_strrep(tag_.artist()) 
        << "  Title: " << std::left << std::setw(15) << AudioTag::_strrep(tag_.title()) 
        << "  Album: " << std::left << std::setw(15) << AudioTag::_strrep(tag_.album()) 
        << "  Yr: " << tag_.year()
        << "  Genre: " << AudioTag::_strrep(tag_.genre())
        << "  Comment: " << AudioTag::_strrep(tag_.comment()) 
        << "  Artwork: " << m_.coverart();
    return os_;
}


std::ostream&  MetaOutBasic::out(std::ostream& os_, const Meta& m_, const TagLib::Tag& tag_, const char* tagtype_)
{
    os_ 
         << "    Artist  : " << _strrep(tag_.artist()) << '\n'
         << "    Title   : " << _strrep(tag_.title()) << "      Track : " << tag_.track() << '\n'
         << "    Album   : " << _strrep(tag_.album()) << '\n'
         << "    Comment : " << _strrep(tag_.comment()) << '\n'
         << "    Year    : " << tag_.year()
         << "    Genre   : " << _strrep(tag_.genre()) 
        << "  Artwork: " << m_.coverart();
    return os_;
}

std::ostream&  MetaOutJson::out(std::ostream& os_, const Meta& m_, const TagLib::Tag& tag_, const char* tagtype_)
{
    const char*  p = NULL;
    int  i = 0;

    os_ << "\n{\n"
        << "  \"file\": \"" << m_.file().name() << "\",\n"
        << "  \"tag\": \"" << tagtype_ << "\",\n"
        << "  \"meta\": {\n";

    os_ << "    \"Artist\": ";
    p = AudioTag::_strrep(tag_.artist());
    if (p) { os_ << "\"" << p << "\""; } else { os_ << "null"; }
    os_ << ",\n";

    os_ << "    \"Title\": ";
    p = AudioTag::_strrep(tag_.title());
    if (p) { os_ << "\"" << p << "\""; } else { os_ << "null"; }
    os_ << ",\n";

    os_ << "    \"Album\": ";
    p = AudioTag::_strrep(tag_.album());
    if (p) { os_ << "\"" << p << "\""; } else { os_ << "null"; }
    os_ << ",\n";

    os_ << "    \"Track\": ";
    i = tag_.track();
    if (i > 0) { os_ << "\"" << i << "\""; } else { os_ << "null"; }
    os_ << ",\n";

    os_ << "    \"Yr\": ";
    i = tag_.year();
    if (i > 0) { os_ << "\"" << i << "\""; } else { os_ << "null"; }
    os_ << ",\n";

    os_ << "    \"Genre\": ";
    p = AudioTag::_strrep(tag_.genre());
    if (p) { os_ << "\"" << p << "\""; } else { os_ << "null"; }
    os_ << ",\n";

    os_ << "    \"Comment\": ";
    p = AudioTag::_strrep(tag_.comment()) ;
    if (p) { os_ << "\"" << p << "\""; } else { os_ << "null"; }
    os_ << ",\n";

    os_ << "    \"Artwork\": ";
    bool  b = m_.coverart();
    os_ << "\"" << (b ? "yes" : "no") << "\"";

    os_ << "\n  }";
    os_ << "\n}";
    return os_;
}


};
