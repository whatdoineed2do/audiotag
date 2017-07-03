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
#define DUMP(x)  if ( (p = AudioTag::_strrep( x )) && strlen(p) > 0) os_ << std::left << std::setw(15) << p;

    const char*  p;
    os_ << "  Artist: "; DUMP( tag_.artist() );
    os_ << "  Title: ";  DUMP( tag_.title() );
    os_ << "  Album: ";  DUMP( tag_.album() );
    os_ << "  Track: "; if ( tag_.track() > 0) os_ << tag_.track();
    os_ << "  Yr: "; if ( tag_.year() > 0) os_ << tag_.year();
    os_ << "  Genre: ";  DUMP( tag_.genre() );
    os_ << "  Comment: ";  DUMP( tag_.comment() );
    os_ << "  Artwork: " << m_.coverart();
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
        << "  \"file\": {\n"
        << "    \"name\": \"" << m_.file().name() << "\",\n"
        << "    \"tag\": \"" << tagtype_ << "\",\n"
        << "    \"meta\": {\n";

    os_ << "      \"Artist\": ";
    p = AudioTag::_strrep(tag_.artist());
    if (p) { os_ << "\"" << p << "\""; } else { os_ << "null"; }
    os_ << ",\n";

    os_ << "      \"Title\": ";
    p = AudioTag::_strrep(tag_.title());
    if (p) { os_ << "\"" << p << "\""; } else { os_ << "null"; }
    os_ << ",\n";

    os_ << "      \"Album\": ";
    p = AudioTag::_strrep(tag_.album());
    if (p) { os_ << "\"" << p << "\""; } else { os_ << "null"; }
    os_ << ",\n";

    os_ << "      \"Track\": ";
    i = tag_.track();
    if (i > 0) { os_ << "\"" << i << "\""; } else { os_ << "null"; }
    os_ << ",\n";

    os_ << "      \"Yr\": ";
    i = tag_.year();
    if (i > 0) { os_ << "\"" << i << "\""; } else { os_ << "null"; }
    os_ << ",\n";

    os_ << "      \"Genre\": ";
    p = AudioTag::_strrep(tag_.genre());
    if (p) { os_ << "\"" << p << "\""; } else { os_ << "null"; }
    os_ << ",\n";

    os_ << "      \"Comment\": ";
    p = AudioTag::_strrep(tag_.comment()) ;
    if (p) { os_ << "\"" << p << "\""; } else { os_ << "null"; }
    os_ << ",\n";

    os_ << "      \"Artwork\": ";
    bool  b = m_.coverart();
    os_ << "\"" << (b ? "yes" : "no") << "\"";

    os_ << "\n    }"
        << "\n  }"
        << "\n}";
    return os_;
}


};
