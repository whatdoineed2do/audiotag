#include "Meta.h"

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <functional>
#include <taglib/tstring.h>
#include <taglib/tpropertymap.h>

#include <taglib/id3v2tag.h>
#include <taglib/attachedpictureframe.h>
#include <taglib/popularimeterframe.h>

#include <taglib/mp4tag.h>
#include <taglib/mp4coverart.h>

#include <taglib/flacfile.h>
#include <taglib/flacpicture.h>

#include "audiotag.h"
#include "File.h"
#include "MetaOut.h"
#include "Artwork.h"


namespace AudioTag
{
const TagLib::String  taglib_string_null;

void  Input::populate(const TagLib::Tag* tag_)
{
    artist = AudioTag::_strrep(tag_->artist(), &a);
    title = AudioTag::_strrep(tag_->title(), &t);
    album = AudioTag::_strrep(tag_->album(), &A);
    genre = AudioTag::_strrep(tag_->genre(), &g);

    //disc = ..
    //albumartist = ..

    sprintf(T, "%d", tag_->track());
    sprintf(y, "%d", tag_->year());
    trackno = T;
    yr = y;
}

bool  Input::validate() const
{
    bool  good = true;
    if (good && trackno)
    {
	unsigned x = 0;
	good = sscanf(trackno, "%d", &x) == 1;
    }
    if (good && yr)
    {
	unsigned x = 0;
	good = sscanf(yr, "%d", &x) == 1;
    }
    if (good && disc)
    {
	// expect x/y
	int x = -1, y = -1;
	good = sscanf(disc, "%d/%d", &x, &y) == 2;
    }
    if (good && date)
    {
	if (strcmp(date, Meta::DATE_REMOVE) == 0) {
	    // special 'remove it' date
	}
	else {
	    struct tm  tm;
	    char*  ret = strptime(date, "%Y-%m-%d", &tm);
	    good = (ret && *ret == '\0');
	}
    }
    if (good && rating)
    {
	int  x = 0;
	good = (sscanf(rating, "%d", &x) == 1 && x >= 0 && x <=5);
    }

    return good;
}


TagLib::String  _cnvrt(const char* data_)
{
    const size_t  n = strlen(data_)*sizeof(wchar_t);
    if (n == 0) {
        return taglib_string_null;
    }

    if (INTNL_STR_ENC == TagLib::String::Latin1) {
        // nice and simple
	AUDIOTAG_DEBUG("encoded as [" << INTNL_STR_ENC << "]  '" << data_ << "'");
	return TagLib::String(data_);
    }

    wchar_t*      w = new wchar_t[n+1];
    memset(w, 0, n+1);
#ifdef DEBUG
    const size_t  len = 
#endif
    mbstowcs(w, data_, n);

#ifdef DEBUG
    char  p[MB_CUR_MAX+1];
    for (int c=0; c<len; c++)
    {
	/* re-convert from wide character to multibyte character */
	int x = wctomb(p, w[c]);

	/* One multibyte character may be two or more bytes.
	 * Thus "%s" is used instead of "%c".
	 */
	if (x>0) p[x]=0;
	AUDIOTAG_DEBUG("multibyte char[" << c << "] = '" << p << "'  (" << x << " bytes)");
    }
#endif

    const TagLib::String  s(w, INTNL_STR_ENC);
    AUDIOTAG_DEBUG("encoded as [" << INTNL_STR_ENC << "]  '" << data_ << "', unicode='" << s.toCString(true) << "', latin1='" << s.toCString() << "'");
    delete []  w;
    return s;
}


const char*  _strrep(const TagLib::String&  str_, TagLib::String* ptr_)
{
    if (str_ == taglib_string_null) {
        static const char*  tmp = "";
        return tmp;
    }

    if (ptr_) {
        *ptr_ = str_;
        return ptr_->toCString(true);
    }

    static TagLib::String  tmp;
    tmp = str_;
    return tmp.toCString(true);
}




TagLib::String::Type  parseEnc(const char* optarg, TagLib::String::Type dflt_)
{
    return strcasecmp(optarg, "utf8") == 0 ?  TagLib::String::UTF8 :
	      strncasecmp(optarg, "latin", strlen("latin")) == 0 ? TagLib::String::Latin1 :
		  strcasecmp(optarg, "utf16") == 0 ? TagLib::String::UTF16 :
		      strcasecmp(optarg, "utf16be") == 0 ? TagLib::String::UTF16BE :
			  strcasecmp(optarg, "utf16le") == 0 ? TagLib::String::UTF16LE :
	  dflt_;
}



 
bool  MetaTOI::operator==(const MetaTOI& rhs_)
{
    if (this == &rhs_) {
        return true;
    }

    return (memcmp(this, &rhs_, sizeof(rhs_)) == 0);
}

MetaTOI&  MetaTOI::operator=(const char optarg_)
{
    char  tmp[2];
    tmp[0] = optarg_;
    tmp[1] = (char)NULL;
    return *this = tmp;
}

MetaTOI&  MetaTOI::operator=(const char* optarg_)
{
    if (strchr(optarg_, '1'))  id3v1 = true;
    if (strchr(optarg_, '2'))  id3v2 = true;
    if (strchr(optarg_, 'f'))  flac  = true;
    if (strchr(optarg_, 'a'))  ape   = true;
    if (strchr(optarg_, '4'))  mp4   = true;
    if (strchr(optarg_, 'd'))  deflt = true;  // default
    if (strchr(optarg_, 'A'))  all   = true;  // all tags!

    return *this;
}


Input::Input(const Meta& meta_)
{
    reset();
    populate(meta_.tag());

    // this is to deal with an apparent bug that we can't use the base class to
    // get the full propertiesMap so we overwrite it using our own method to
    // pull all properties
    properties = meta_.properties();
}


void  Meta::sanitize()
{
    /* need to strip the tag, no API call to clear down - the 
     * setProperties() only works on setters at top level
     */

    if (*_tag && !(*_tag)->isEmpty())
    {
        Input  data(*_tag);
        
        MetaTOI  toi;
        toi.deflt = true;
        remove(toi);
        assign(toi, data);
    }
}

const TagLib::File&  Meta::file() const
{
    return _f.taglibfile();
}

void  Meta::save()
{
    _f.taglibfile().save();
}

void  Meta::artist(TagLib::Tag& tag_, const char* data_)
{
    tag_.setArtist( data_ ? _cnvrt(data_) : taglib_string_null);
}


void  Meta::album(TagLib::Tag& tag_, const char* data_)
{
    tag_.setAlbum( data_ ? _cnvrt(data_) : taglib_string_null);
}

void  Meta::_property(TagLib::Tag& tag_, const char* tagname_, const char* data_)
{
    TagLib::PropertyMap  m = properties();
    if (data_ == NULL) {
        m.erase(tagname_);
    }
    else {
        TagLib::PropertyMap::Iterator  i = m.find(tagname_);
        if (i == m.end()) {
            m.insert(tagname_, _cnvrt(data_) );
        }
        else {
            m.replace(tagname_, _cnvrt(data_));
        }
    }
    properties(tag_, m);
}

void  Meta::albumArtist(TagLib::Tag& tag_, const char* data_)
{
    _property(tag_, "ALBUMARTIST", data_);
}

void  Meta::disc(TagLib::Tag& tag_, const char* data_)
{
    _property(tag_, "DISCNUMBER", data_);
}

void  Meta::title(TagLib::Tag& tag_, const char* data_)
{
    tag_.setTitle( data_ ? _cnvrt(data_) : taglib_string_null);
}


void  Meta::comment(TagLib::Tag& tag_, const char* data_)
{
    tag_.setComment( data_ ? _cnvrt(data_) : taglib_string_null);
}


void  Meta::genre(TagLib::Tag& tag_, const char* data_)
{
    tag_.setGenre( data_ ? _cnvrt(data_) : taglib_string_null);
}

void  Meta::year(TagLib::Tag& tag_, const unsigned data_)
{
    tag_.setYear(data_);
}


void  Meta::date(TagLib::Tag& tag_, const char* data_)
{
    // default use year() if children dont provide it
    unsigned  yr = 0;
    sscanf(data_, "%u", &yr);
    tag_.setYear(yr);
}


void  Meta::trackno(TagLib::Tag& tag_, const unsigned data_)
{
    tag_.setTrack(data_);
}


void  Meta::trackno(TagLib::Tag& tag_, const unsigned x_, const unsigned y_)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "%u/%u", x_, y_);
    _property(tag_, "TRACKNUMBER", buf);
}

void  Meta::_assign(TagLib::Tag& tag_, const Input& rhs_)
{
    if (!rhs_) {
        return;
    }

    if (rhs_.artist)      artist(tag_, rhs_.artist);
    if (rhs_.album)        album(tag_, rhs_.album);
    if (rhs_.title)        title(tag_, rhs_.title);
    if (rhs_.comment)    comment(tag_, rhs_.comment);
    if (rhs_.genre)        genre(tag_, rhs_.genre);

    if (rhs_.yr)            year(tag_, atol(rhs_.yr));
    if (rhs_.date)          date(tag_, rhs_.date);

    if (rhs_.trackno && rhs_.trackN)
                         trackno(tag_, atol(rhs_.trackno), atol(rhs_.trackN));
    else
    if (rhs_.trackno)    trackno(tag_, atol(rhs_.trackno));

    if (rhs_.disc)          disc(tag_, rhs_.disc);
    if (rhs_.albumartist)        albumArtist(tag_, rhs_.albumartist);
    if (!rhs_.properties.isEmpty()) {
        properties(tag_, rhs_.properties);
    }
    if (rhs_.rating)     rating(atoi(rhs_.rating));
}

void  multibyteConvert(Input& iflds_, const TagLib::Tag&  tag_, const TagLib::String::Type mbenc_)
{
    static TagLib::String  a,t,A,g;
    static char  y[5];
    static char  T[3];

    static TagLib::ByteVector  va, vt, vA, vg;

    a = tag_.artist();
    t = tag_.title();
    A = tag_.album();
    g = tag_.genre();

    iflds_.artist = (a == taglib_string_null) ? NULL : (va = a.data(mbenc_)).data();
    iflds_.title  = (t == taglib_string_null) ? NULL : (vt = t.data(mbenc_)).data();
    iflds_.album  = (A == taglib_string_null) ? NULL : (vA = A.data(mbenc_)).data();
    iflds_.genre  = (g == taglib_string_null) ? NULL : (vg = g.data(mbenc_)).data();

    sprintf(T, "%d", tag_.track());
    sprintf(y, "%d", tag_.year());
    iflds_.trackno = T;
    iflds_.yr = y;
}


TagLib::PropertyMap& Meta::_mergeproperties(TagLib::PropertyMap& a_, const TagLib::PropertyMap& b_) const
{
    std::function<void (const char*, const TagLib::PropertyMap&)>  dump = [](const char* id_, const TagLib::PropertyMap& m_) -> void {
#ifdef DEBUG_MERGE_PROP_MAP
	std::cout << id_ << ": { ";
	for_each(m_.begin(), m_.end(), [&](auto i) {
	       std::cout << i.first.toCString() << " [ ";
	       for_each(i.second.begin(), i.second.end(), [&](auto j) {
		           std::cout << j.toCString() << " ";
		       });
	       std::cout << "] ";
	    });
	std::cout << "}\n";
#else
#endif
    };
    dump("target (before)", a_);
    dump("merging with", b_);

    // drop any empty lists from target
    TagLib::PropertyMap::Iterator  empties;
    while ( (empties = std::find_if(a_.begin(), a_.end(), [&](auto i) { return i.second.isEmpty(); }) ) != a_.end() ) {
	a_.erase(empties->first);
    }
    dump("target (cleaned)", a_);

    for (auto& i : b_) {
        auto  j = a_.find(i.first);

        if (i.second.isEmpty())
	{
	    if (j != a_.end()) {
		a_.erase(i.first);
	    }
	    // else {
	    //   its not on the file, and its empty .. skip other we'll add empty tag
	    // }

            continue;
        }

        if (j == a_.end()) {
            a_.insert(i.first, i.second);
        }
        else {
            a_.replace(i.first, i.second);
        }
    }
    dump("target (after)", a_);
    return a_;
}

std::ostream&  Meta::out(std::ostream& os_, const File& f_) const
{
    return _mo.out(os_, f_);
}
};
