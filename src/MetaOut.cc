#include "MetaOut.h"

#include <iostream>
#include <iomanip>

#include <taglib/tpropertymap.h>
#include <taglib/tstringlist.h>


#include "audiotag.h"
#include "Meta.h"
#include "File.h"


namespace AudioTag
{
MetaOut*  MetaOut::create(const char* optarg_)
{
    if (optarg_ == NULL)                 return new MetaOut();
    if (strcmp(optarg_, "basic") == 0)   return new MetaOutBasic();
    if (strcmp(optarg_, "json")  == 0)   return new MetaOutJson();
#ifdef HAVE_JSONC
    if (strcmp(optarg_, "json-c")  == 0)   return new MetaOutJsonC();
#endif
    AUDIOTAG_WARN("unknown output handler '" << optarg_ << "'\n");

    return new MetaOut();
}



std::ostream&  MetaOut::out(std::ostream& os_, const File& file_)
{ return os_ << "MetaOut::out(ostream,File) TODO - " << file_.taglibfile().name() << '\n'; }


std::ostream&  MetaOutBasic::out(std::ostream& os_, const File& f_)
{
    const Meta::Tags&  tags = f_.meta().tags();
    for (const auto& t : tags)
    {
	os_ << f_.taglibfile().name() << "  [" << t.first << "]\n"
	     << "    Artist  : " << _strrep(t.second->artist()) << '\n'
	     << "    Title   : " << _strrep(t.second->title()) << "      Track : " << t.second->track() << '\n'
	     << "    Album   : " << _strrep(t.second->album()) << '\n'
	     << "    Comment : " << _strrep(t.second->comment()) << '\n'
	     << "    Year    : " << t.second->year()
	     << "    Genre   : " << _strrep(t.second->genre()) 
	    << "  Artwork: " << f_.meta().coverart() << '\n';
    }
    return os_;
}


const char*  MetaOutJson::out(std::string& tmp_, const TagLib::String& s_)
{
    static TagLib::String  taglib_string_null;
    if (s_ == taglib_string_null) {
        return "null";
    } 
    else { 
        tmp_ = "\"";
        tmp_ += _strrep(s_);
        tmp_ += "\"";
        return tmp_.c_str();
    }
}

std::ostream&  MetaOutJson::out(std::ostream& os_, const File& f_)
{
    const Meta&  m_ = f_.meta();

    std::string  s;
    int  i = 0;

    char  mtime[50];
    strftime(mtime, sizeof(mtime)-1, "%c", localtime(&f_.st().st_mtime));

    const TagLib::AudioProperties*  ap = m_.file().audioProperties();

os_ << "{\n"
    << "  \"file\": {\n"
    << "    \"name\": \"" << m_.file().name() << "\",\n"
    << "    \"size\": " << f_.st().st_size << ",\n"
    << "    \"mod_time\": \"" << mtime << "\",\n"
    << "    \"audio_properties\": {\n";
    if (ap) {
os_ << "      \"length\": " << ap->lengthInMilliseconds()/1000.0 << ",\n"
    << "      \"bitrate\": " << ap->bitrate() << ",\n"
    << "      \"samplerate\": " << ap->sampleRate() << ",\n"
    << "      \"channels\": " << ap->channels() << ",\n"
    << "      \"hash\": \"" << f_.hash() << "\"\n";
    }
os_ << "    }\n"
    << "  },\n"
    << "  \"meta\": [";

    bool  any = false;
    bool  first = true;
    const Meta::Tags  tags = f_.meta().tags();
    for (const auto& t : tags)
    {
	if (!any) {
	    os_ << "\n";
	}

	any = true;
	const char*  tagtype_ = t.first;
	const auto&  tag = t.second;
	if (!first) {
os_ << ",\n";
	}
os_ << "    {\n"
    << "      \"tag_type\": \"" << tagtype_ << "\",\n"
    << "      \"artist\": " << MetaOutJson::out(s, tag->artist()) << ",\n"
    << "      \"title\": " << MetaOutJson::out(s, tag->title()) << ",\n"
    << "      \"album\": " << MetaOutJson::out(s, tag->album()) << ",\n";

	// stuff from the properties not available on the 'std' i/f

	/* this is a horrid hack - however, if you try to use the base class ref
	 * to call properties() you can't seem to find the properties!
	 */
	const TagLib::PropertyMap  m = m_.properties();
os_ << "      \"track\": ";
	i = tag->track();
	if (i > 0) { os_ << i; } else { os_ << "null"; }
	os_ << ",\n";

os_ << "      \"year\": ";
	i = tag->year();
	if (i > 0) { os_ << i; } else { os_ << "null"; }
	os_ << ",\n";

os_ << "      \"genre\": " << MetaOutJson::out(s, tag->genre()) << ",\n";

        if (m_.rating() > 0) {
os_ << "      \"rating\": " << m_.rating() << ",\n";
        }
os_ << "      \"comment\": " << MetaOutJson::out(s, tag->comment()) << ",\n"
    << "      \"artwork\": " << (m_.coverart() ? "true" : "false") << ",\n"
    << "      \"properties\": {\n";
	bool  fi = true;
	for (const auto& i : m)
	{
	    if (fi) {
		fi = false;
	    }
	    else {
		os_ << ",\n";
	    }
	    os_ << "        \"" << i.first.toCString() << "\": ";
	    if (i.second.isEmpty()) {
		os_ << "[ ]";
	    }
	    else 
	    {
		bool  fj = true;
		os_ << "[ ";
		for (const auto& j : i.second)
		{
		    if (fj) { 
			fj = false;
		    }
		    else { 
			os_ << ", "; 
		    }
		    os_ << "\"" << AudioTag::_strrep(j) << "\"";
		}
		os_ << " ]";
	    }
	}
os_ << "\n"
    << "      }\n"
    << "    }";
	first = false;
    }
os_ << (any ? "\n" : "")
    << "  ]\n"
    << "}\n";
    return os_;
}


#ifdef HAVE_JSONC
#include <json-c/json.h>

std::ostream&  MetaOutJsonC::out(std::ostream& os_, const File& f_)
{
    const char*  p = NULL;
    int  i;

    json_object*  jroot = json_object_new_object();

    json_object*  jfile = json_object_new_object();
    {
	json_object_object_add(jfile, "name", json_object_new_string(f_.taglibfile().name()));
	const struct stat&  st = f_.st();
	json_object_object_add(jfile, "size", json_object_new_int64(st.st_size));
	char  mtime[50];
	strftime(mtime, sizeof(mtime)-1, "%c", localtime(&st.st_mtime));
	json_object_object_add(jfile, "mod_time", json_object_new_string(mtime));
    }

    json_object*  jtags = json_object_new_array();
    {
	for (const auto& tag : f_.meta().tags())
	{
	    json_object*           jmetaobj = json_object_new_object();
	    {
		json_object_object_add(jmetaobj, "tag_type", json_object_new_string(tag.first));

		p = AudioTag::_strrep(tag.second->artist());
		json_object_object_add(jmetaobj, "artist", (p=AudioTag::_strrep(tag.second->artist())) ? 
							    json_object_new_string(p) : NULL );
		json_object_object_add(jmetaobj, "title",  (p=AudioTag::_strrep(tag.second->title())) ?
							    json_object_new_string(p) : NULL);
		json_object_object_add(jmetaobj, "album",  (p=AudioTag::_strrep(tag.second->album())) ?
							    json_object_new_string(p) : NULL);
		json_object_object_add(jmetaobj, "track",  (i = tag.second->track()) > 0 ? 
							    json_object_new_int(i) : NULL);
		json_object_object_add(jmetaobj, "year",   (i = tag.second->year()) > 0 ?
							    json_object_new_int(i) : NULL);
		json_object_object_add(jmetaobj, "genre",  (p=AudioTag::_strrep(tag.second->genre())) ?
							    json_object_new_string(p) : NULL);
		json_object_object_add(jmetaobj, "comment",(p=AudioTag::_strrep(tag.second->comment()) ) ? 
							    json_object_new_string(p) : NULL);
		json_object_object_add(jmetaobj, "artwork",json_object_new_boolean(f_.meta().coverart() ? 1 : 0));

		/* this is a horrid hack - however, if you try to use the base class ref
		 * to call properties() you can't seem to find the properties!
		 */
		const TagLib::PropertyMap  m = f_.meta().properties(*tag.second);
		json_object*           jmetaprop = json_object_new_object();
		{
		    for (const auto i : m)
		    {
			json_object*  jmp = json_object_new_array();
			for (const auto& j : i.second) {
			    json_object_array_add(jmp, json_object_new_string( AudioTag::_strrep(j)) );
			}
			json_object_object_add(jmetaprop, i.first.toCString(), jmp);
		    }
		}
		json_object_object_add(jmetaobj, "properties", jmetaprop);
	    }
	    json_object_array_add(jtags, jmetaobj);
	}
    }

    json_object_object_add(jroot, "file", jfile);
    json_object_object_add(jroot, "meta", jtags);

    int  flags = JSON_C_TO_STRING_PRETTY | JSON_C_TO_STRING_SPACED;
#if ( JSON_C_MAJOR_VERSION > 0 || JSON_C_MINOR_VERSION >= 13)
    flags |= JSON_C_TO_STRING_NOSLASHESCAPE;
#endif
    os_ << json_object_to_json_string_ext(jroot,flags) << '\n';
    json_object_put(jroot);
    return os_;
}
#endif


};
