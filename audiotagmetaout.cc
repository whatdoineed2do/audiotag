#include "audiotagmetaout.h"

#include <iostream>
#include <iomanip>

#include <tpropertymap.h>
#include <tstringlist.h>

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
        << "    \"meta\": {\n"
        << "      \"tag\": \"" << tagtype_ << "\",\n"
        << "      \"data\": {\n";

    os_ << "        \"Artist\": ";
    p = AudioTag::_strrep(tag_.artist());
    if (p) { os_ << "\"" << p << "\""; } else { os_ << "null"; }
    os_ << ",\n";

    os_ << "        \"Title\": ";
    p = AudioTag::_strrep(tag_.title());
    if (p) { os_ << "\"" << p << "\""; } else { os_ << "null"; }
    os_ << ",\n";

    os_ << "        \"Album\": ";
    p = AudioTag::_strrep(tag_.album());
    if (p) { os_ << "\"" << p << "\""; } else { os_ << "null"; }
    os_ << ",\n";


    // stuff from the properties not available on the 'std' i/f

    /* this is a horrid hack - however, if you try to use the base class ref
     * to call properties() you can't seem to find the properties!
     */
    const TagLib::ID3v1::Tag*  id3v1tag = NULL;
    const TagLib::ID3v2::Tag*  id3v2tag = NULL;
    const TagLib::Ogg::XiphComment* flactag = NULL;
    const TagLib::MP4::Tag*  mp4tag = NULL;

    const TagLib::PropertyMap  m = m_.properties(tag_);

    const struct _NVP {
        const char*  tn;  // tag name
        const char*  on;  // output name
    }  ptags[] = {
        { "ALBUMARTIST",    "AlbumArtist"     },
        { "ALBUMSORT",      "AlbumSort"       },
        { "ARTISTSORT",     "ArtistSort"      },
        { "TITLESORT",      "TitleSort"       },
        { "ALBUMARTISTSORT","AlbumArtistSort" },

        { "COMPOSER",       "Composer"        },
        { "COPYRIGHT",      "Copyright"       },
        { "ENCODEDBY",      "Encoded"         },

        { NULL, NULL }
    };

    {
        const _NVP* p = ptags;
        while (p->tn)
        {
            const TagLib::StringList&  sl = m[p->tn];
            if ( !sl.isEmpty()) {
                os_ << "        \"" << p->on << "\": "
                    << "\"" << AudioTag::_strrep(sl.front().toCString()) << "\""
                    << ",\n";
            }
            ++p;
        }
    }


    os_ << "        \"Track\": ";
    i = tag_.track();
    if (i > 0) { os_ << "\"" << i << "\""; } else { os_ << "null"; }
    os_ << ",\n";

    os_ << "        \"Yr\": ";
    i = tag_.year();
    if (i > 0) { os_ << "\"" << i << "\""; } else { os_ << "null"; }
    os_ << ",\n";

    os_ << "        \"Genre\": ";
    p = AudioTag::_strrep(tag_.genre());
    if (p) { os_ << "\"" << p << "\""; } else { os_ << "null"; }
    os_ << ",\n";

    os_ << "        \"Comment\": ";
    p = AudioTag::_strrep(tag_.comment()) ;
    if (p) { os_ << "\"" << p << "\""; } else { os_ << "null"; }
    os_ << ",\n";

    os_ << "        \"Artwork\": ";
    bool  b = m_.coverart();
    os_ << "\"" << (b ? "yes" : "no") << "\"";

    os_ << "\n      }"
        << "\n    }"
        << "\n  }"
        << "\n}";
    return os_;
}


};
