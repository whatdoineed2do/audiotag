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

    sprintf(T, "%ld", tag_->track());
    sprintf(y, "%ld", tag_->year());
    trackno = T;
    yr = y;
}

bool  Input::validate() const
{
    bool  good = true;
    if (good && trackno)
    {
	unsigned x = 0;
	good = sscanf(trackno, "%ld", &x) == 1;
    }
    if (good && yr)
    {
	unsigned x = 0;
	good = sscanf(yr, "%ld", &x) == 1;
    }
    if (good && disc)
    {
	// expect x/y
	int x = -1, y = -1;
	good = sscanf(disc, "%d/%d", &x, &y) == 2;
    }
    if (good && date)
    {
	if (strcmp(date, "0000-00-00") == 0) {
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
	MP3_TAG_DEBUG("encoded as [" << INTNL_STR_ENC << "]  '" << data_ << "'");
	return TagLib::String(data_);
    }

    wchar_t*      w = new wchar_t[n+1];
    memset(w, 0, n+1);
    const size_t  len = mbstowcs(w, data_, n);

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
	MP3_TAG_DEBUG("multibyte char[" << c << "] = '" << p << "'  (" << x << " bytes)");
    }
#endif

    const TagLib::String  s(w, INTNL_STR_ENC);
    MP3_TAG_DEBUG("encoded as [" << INTNL_STR_ENC << "]  '" << data_ << "', unicode='" << s.toCString(true) << "', latin1='" << s.toCString() << "'");
    delete []  w;
    return s;
}


const char*  _strrep(const TagLib::String&  str_, TagLib::String* ptr_)
{
    /* so fucking stupid..
     */
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


void  Meta::save()
{
    _file.save();
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
    TagLib::PropertyMap  m = properties(tag_);
    if (data_ == NULL || *data_ == '\0') {
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
    snprintf(buf, sizeof(buf), "%lu/%lu", x_, y_);
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

    sprintf(T, "%ld", tag_.track());
    sprintf(y, "%ld", tag_.year());
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

    for (auto i : b_) {
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


// MP3 /////////////////////////////////////////////////////////////////////////
const char*  MetaMP3::TAG_DATE     = "TDRL";
const char*  MetaMP3::TAG_RATING   = "POPM";
const char*  MetaMP3::TAG_COVERART = "APIC";
const char*  MetaMP3::TAG_COVERART_OLD = "PIC";


void  MetaMP3::year(TagLib::Tag& t_, const unsigned year_)
{
    Meta::year(t_, year_);

    // urgh, this gets messy... only add if we already have one
    if (_id3v2)
    {
	const TagLib::ID3v2::FrameList&  fl = _id3v2->frameList(MetaMP3::TAG_DATE);
	if (!fl.isEmpty()) {
	    char  fakedate[11];  // 4+1+2+1+2+1
	    snprintf(fakedate, 11, "%u-01-01", year_);
	    date(t_, fakedate);
	}
    }
}

void  MetaMP3::date(TagLib::Tag& t_, const char* date_)
{
    // delete
    if (strcmp(date_, "0000-00-00") == 0) {
        if (_id3v2) {
	    _id3v2->removeFrames(MetaMP3::TAG_DATE);
	}
    }
    else
    {
	// this is only available for 2.4 tag
	TagLib::ID3v2::Tag*  tag = _tf.ID3v2Tag(_id3v2 ? false : true);
	_tagFrme(tag, new TagLib::ID3v2::TextIdentificationFrame(MetaMP3::TAG_DATE, TagLib::String::Latin1), date_);

	// use default impl to set yr
	Meta::date(t_, date_);
    }
}

void  MetaMP3::properties(TagLib::Tag& t_, const TagLib::PropertyMap& m_) const
{
    TagLib::ID3v2::Tag*  t = dynamic_cast<TagLib::ID3v2::Tag*>(&t_);
    if (t)  _properties(*t, m_);
    else    _properties(t_, m_);
}

TagLib::PropertyMap  MetaMP3::properties(const TagLib::Tag& t_) const
{
    const TagLib::ID3v2::Tag*  t = dynamic_cast<const TagLib::ID3v2::Tag*>(&t_);
    if (t) return _properties(*t);
    else   return _properties(t_);
}

int  MetaMP3::rating() const
{
    if (_id3v2) {
	const TagLib::ID3v2::FrameList&  fl = _id3v2->frameList(MetaMP3::TAG_RATING);
	if (fl.isEmpty()) {
	    return Meta::rating();
	}

	const auto  frame = dynamic_cast<TagLib::ID3v2::PopularimeterFrame*>(fl.front());
	if (frame) {
	    const int  r = frame->rating();
            if (r >   0 && r <=  31)  return 1;
            if (r >  31 && r <=  95)  return 2;
            if (r >  95 && r <= 159)  return 3;
            if (r > 159 && r <= 224)  return 4;
            if (r > 224 && r <= 255)  return 5;
	}
    }
    return Meta::rating();
}

void  MetaMP3::rating(uint8_t r_)
{
    TagLib::ID3v2::Tag*  tag = _tf.ID3v2Tag(_id3v2 ? false : true);
    short  r = 0;
    switch (r_)
    {
	case 1: r =   1; break;
	case 2: r =  64; break;
	case 3: r = 128; break;
	case 4: r = 196; break;
	case 5: r = 255; break;

	case 0:
	default:
	    r = 0;
    }

    removerating();
    if (r > 0)
    {
        TagLib::ID3v2::PopularimeterFrame*  frame = new TagLib::ID3v2::PopularimeterFrame();
        frame->setEmail("whatdoineed2do");
        frame->setRating(r);
        tag->addFrame(frame);
    }
}

void  MetaMP3::removerating()
{
    if (_id3v2) {
        _id3v2->removeFrames(MetaMP3::TAG_RATING);
    }
}

void  MetaMP3::save()
{
    /* by default, just keep the id3v2 tag;  need to explicitly ask TagLib to 
     * only save that (otherwise it copies id3v2 onto id3v1) but consider if 
     * other tags have modified
     */
    MP3_TAG_DEBUG("mp3 savetag=" << _svtag);
    _tf.save(_svtag == 0 ? TagLib::MPEG::File::ID3v2 : _svtag, TagLib::File::StripOthers, TagLib::ID3v2::v4, TagLib::File::DoNotDuplicate);
}

void MetaMP3::remove(const MetaTOI& toi_)
{
    int  rmtag = TagLib::MPEG::File::NoTags;

    if (toi_.all) {
        rmtag = TagLib::MPEG::File::AllTags;
        _svtag = TagLib::MPEG::File::NoTags;
    }
    else
    {
        if (toi_.id3v2 || toi_.deflt) {
            rmtag |= TagLib::MPEG::File::ID3v2;
            _svtag &= ~TagLib::MPEG::File::ID3v2;
        }
        if (toi_.id3v1) {
            rmtag |= TagLib::MPEG::File::ID3v1;  
            _svtag &= ~TagLib::MPEG::File::ID3v1;  // dont add this back!
            _id3v1 = NULL;
        }
        if (toi_.ape) {
            rmtag |= TagLib::MPEG::File::APE;
            _svtag &= ~TagLib::MPEG::File::APE;
            _ape = NULL;
        }
    }

    if (rmtag == TagLib::MPEG::File::NoTags) {
        return;
    }
    MP3_TAG_DEBUG("mp3 strip=" << rmtag);
    _tf.strip(rmtag);

    /* BUG - if you ask to remove APE/ID3v1, both appear to be removed but when
     *       saved the ID3v2 tag is duplicated onto ID3v1; save tag values are 
     *       correct (0x0002)
     */

    if (toi_.id3v2 || toi_.deflt || toi_.all) {
        _id3v2 = _tf.ID3v2Tag(true);
    }
}

void MetaMP3::assign(const MetaTOI& toi_, const Input& rhs_)
{
    TagLib::MPEG::File&  f = _tf;
    if (!toi_ || toi_.id3v2 || toi_.deflt || toi_.all) {
        if (_id3v2 == NULL) {
            _id3v2 = f.ID3v2Tag(true);
        }
        _assign(*_id3v2, rhs_);
        _svtag |= TagLib::MPEG::File::ID3v2;
    }

    if (toi_.id3v1 || toi_.all) {
        if (!_id3v1) {
            _id3v1 = f.ID3v1Tag(true);
        }
        _assign(*_id3v1, rhs_);
        _svtag |= TagLib::MPEG::File::ID3v1;
    }

    if (toi_.ape   || toi_.all) {
        if (!_ape) {
            _ape = f.APETag(true);
        }
        _assign(*_ape, rhs_);
        _svtag |= TagLib::MPEG::File::APE;
    }
}

void  MetaMP3::clone(const MetaTOI& to_, const MetaTOI& from_)
{
    if (!from_ || !to_) {
        return;
    }

    TagLib::Tag*  from = NULL;
    if (from_.id3v1 && !_id3v1->isEmpty()) {
        from = _id3v1;
    }
    if (from_.id3v2 && !_id3v2->isEmpty()) {
        from = _id3v2;
    }
    if (from_.ape && _ape != NULL) {
        from = _ape;
    }

    if (from == NULL) {
        return;
    }

    Input  data(from);
    assign(to_, data);
}

void  MetaMP3::sanitize()
{
    /* need to strip the tag, no API call to clear down - the 
     * setProperties() only works on setters at top level
     */
    TagLib::MPEG::File&  f = _tf;
    if (_id3v2 && !_id3v2->isEmpty())
    {
        Input  data(_id3v2);
        
        MetaTOI  toi;
        toi.id3v2 = true;
        remove(toi);
        assign(toi, data);
    }

    if (_id3v1 && !_id3v1->isEmpty())
    {
        Input  data(_id3v1);
        
        MetaTOI  toi;
        toi.id3v1 = true;
        remove(toi);
        assign(toi, data);
    }

    if (_ape   && !_ape->isEmpty())
    {
        Input  data(_ape);
        
        MetaTOI  toi;
        toi.ape = true;
        remove(toi);
        assign(toi, data);
    }
}

void  MetaMP3::artwork(Artwork& artwork_)
{
    TagLib::ID3v2::Tag*  tag = _tf.ID3v2Tag(_id3v2 ? false : true);
    TagLib::ID3v2::AttachedPictureFrame*  frame = new TagLib::ID3v2::AttachedPictureFrame();

    frame->setMimeType(artwork_.mimeType());
    frame->setPicture(artwork_.data());

    removeart();
    tag->addFrame(frame);
}

bool  MetaMP3::coverart() const
{
    return (_id3v2 == NULL) ? false : !_id3v2->frameList(MetaMP3::TAG_COVERART).isEmpty();
}

void  MetaMP3::removeart()
{
    if (_id3v2) {
        _id3v2->removeFrames(MetaMP3::TAG_COVERART); // v2.3
        _id3v2->removeFrames(MetaMP3::TAG_COVERART_OLD);  // v2.2
    }
}


Meta::Tags  MetaMP3::tags() const
{
    Meta::Tags  tag;
    if (_id3v2 != NULL && (_id3v2 && !_id3v2->isEmpty()))  tag.push_back(Meta::Tags::value_type("ID3v2", _id3v2));
    if (_id3v1 != NULL && (_id3v1 && !_id3v1->isEmpty()))  tag.push_back(Meta::Tags::value_type("ID3v1", _id3v1));
    if (_ape   != NULL && (_ape   && !_ape->isEmpty()))    tag.push_back(Meta::Tags::value_type("APE",   _ape));
    return tag;
}



MetaMP3::MetaMP3(FileMP3& f_, MetaOut& mo_) 
  : _MetaMulti(f_.taglibfile(), (TagLib::Tag**)&_id3v2, mo_),
    _f(f_),
    _tf((TagLib::MPEG::File&)_f.taglibfile()),
    _id3v1(NULL), _id3v2(NULL), _ape(NULL),
    _svtag(0)
{
    TagLib::MPEG::File&  f = _tf;  // (TagLib::MPEG::File&)_f.taglibfile();
    _id3v1 = f.ID3v1Tag(false);  // this is always non null even if the file doesnt have it!
    _id3v2 = f.ID3v2Tag(false);
    _ape   = f.APETag(false);

    if (_id3v1 && !_id3v1->isEmpty())  _svtag |= TagLib::MPEG::File::ID3v1;
    if (_id3v2 && !_id3v2->isEmpty())  _svtag |= TagLib::MPEG::File::ID3v2;
    if (_ape   && !_ape->isEmpty())    _svtag |= TagLib::MPEG::File::APE;

    MP3_TAG_DEBUG("mp3 ctor: " << _svtag << "  1:" << std::hex << _id3v1 << " 2: "<< _id3v2 << " a: " << _ape);
}

const char*  MetaMP3::_id3v2TxtEnc(const TagLib::ID3v2::Frame* f_)
{
    static const char *encodings[] = { "Latin1", "UTF16", "UTF16BE", "UTF8", "UTF16LE" };

    if (f_->frameID().startsWith("T")) {
	const TagLib::ID3v2::TextIdentificationFrame*  txtf = NULL;
	if ( (txtf = dynamic_cast<const TagLib::ID3v2::TextIdentificationFrame*>(f_)) == NULL) {
	    return "";
	}

	return encodings[int(txtf->textEncoding())];
    }

    if ( f_->frameID() == "COMM") {
	const TagLib::ID3v2::CommentsFrame*  cmmf = NULL;
    	if ( (cmmf = dynamic_cast<const TagLib::ID3v2::CommentsFrame*>(f_)) == NULL) {
	    return "";
	}

	return encodings[int(cmmf->textEncoding())];
    }
    return "";
}

void  MetaMP3::_tagFrme(TagLib::ID3v2::Tag*  tag_, TagLib::ID3v2::Frame* frme_, const char* data_)
{
    tag_->removeFrames(frme_->frameID());
    if (strlen(data_)) {
        frme_->setText(_cnvrt(data_));
    }
    tag_->addFrame(frme_);
}


void  MetaMP3::_tag(TagLib::ID3v2::Tag*  tag_, const Input&  flds_, TagLib::String::Type enc_)
{
    if (flds_.genre) {
        if (strlen(flds_.genre) == 0) {
            tag_->removeFrames("TCON");
        }
        else {
            tag_->setGenre(_cnvrt(flds_.genre));
        }
    }
    if (flds_.yr)       tag_->setYear    ((unsigned)atol(flds_.yr));

    if (flds_.artist)   _tagFrme(tag_, new TagLib::ID3v2::TextIdentificationFrame("TPE1", _overrideEncLatin(flds_.artist, enc_)), flds_.artist);
    if (flds_.album)    _tagFrme(tag_, new TagLib::ID3v2::TextIdentificationFrame("TALB", _overrideEncLatin(flds_.album, enc_)), flds_.album);
    if (flds_.title)    _tagFrme(tag_, new TagLib::ID3v2::TextIdentificationFrame("TIT2", _overrideEncLatin(flds_.title, enc_)), flds_.title);

    if (flds_.trackno)  _tagFrme(tag_, new TagLib::ID3v2::TextIdentificationFrame("TRCK", TagLib::String::Latin1), flds_.trackno);

    if (flds_.comment) {
        if (strlen(flds_.comment) == 0) {
            tag_->removeFrames("COMM");
        }
        else {
            _tagFrme(tag_, new TagLib::ID3v2::CommentsFrame(enc_), flds_.comment);
        }
    }
}

MetaOGGFlac::MetaOGGFlac(FileOGGFlac& f_, MetaOut& mo_) 
  : Meta(f_.taglibfile(), (TagLib::Tag**)&_tag, mo_), 
    _f(f_),
    _tf((TagLib::Ogg::FLAC::File&)_f.taglibfile()),
    _tag(NULL)
{
    _tag = _tf.tag();  // always non null, even if disk may not have value
}

Meta::Tags  MetaOGGFlac::tags() const
{
    Meta::Tags  tag;
    if ( ((TagLib::Ogg::FLAC::File&)_f.taglibfile()).hasXiphComment() && !_tag->isEmpty()) {
        tag.push_back(Meta::Tags::value_type("Ogg/Xipth", _tag));
    }
    return tag;
}

void MetaOGGFlac::remove(const MetaTOI& toi_)
{
}

int  MetaOGGFlac::rating() const
{
    return Meta::rating();
}

void MetaOGGFlac::rating(uint8_t r_)
{
}

void  MetaOGGFlac::properties(TagLib::Tag& t_, const TagLib::PropertyMap& m_) const
{
    TagLib::Ogg::XiphComment*  t = dynamic_cast<TagLib::Ogg::XiphComment*>(&t_);
    if (t)  _properties(*t, m_);
    else    _properties(t_, m_);
}

TagLib::PropertyMap  MetaOGGFlac::properties(const TagLib::Tag& t_) const
{
    const TagLib::Ogg::XiphComment*  t = dynamic_cast<const TagLib::Ogg::XiphComment*>(&t_);
    if (t) return _properties(*t);
    else   return _properties(t_);
}



// flac ////////////////////////////////////////////////////////////////////////
const char*  MetaFlac::TAG_RATING = "RATING";
const char*  MetaFlac::TAG_DATE = "DATE";


MetaFlac::MetaFlac(FileFlac& f_, MetaOut& mo_) 
  : Meta(f_.taglibfile(), (TagLib::Tag**)&_tag, mo_), 
    _f(f_),
    _tf((TagLib::FLAC::File&)_f.taglibfile()),
    _tag(NULL)
{
    _tag = _tf.xiphComment();  // always non null, even if disk may not have value
}

Meta::Tags  MetaFlac::tags() const
{
    Meta::Tags  tag;
    //if ( ((TagLib::FLAC::File&)_f.taglibfile()).hasXiphComment() && !_tag->isEmpty()) {
    if ( _tf.hasXiphComment() && !_tag->isEmpty()) {
        tag.push_back(Meta::Tags::value_type("Flac/Xipth", _tag));
    }
    return tag;
}

void MetaFlac::remove(const MetaTOI& toi_)
{
    int  rmtag = TagLib::FLAC::File::NoTags;

    if (toi_.all) {
        rmtag = TagLib::FLAC::File::AllTags;
    }
    else
    {
        if (toi_.flac || toi_.deflt) {
            rmtag |= TagLib::FLAC::File::XiphComment;
        }
        if (toi_.id3v2)  rmtag |= TagLib::FLAC::File::ID3v2;
        if (toi_.id3v1)  rmtag |= TagLib::FLAC::File::ID3v1;
    }

    if (rmtag == TagLib::FLAC::File::NoTags) {
        return;
    }
    _tf.strip(rmtag);

    // get back a valid ptr
    _tag = _tf.xiphComment();
}

void MetaFlac::date(TagLib::Tag& t_, const char* date_)
{
    TagLib::PropertyMap  props = properties(*_tag);

    if (strcmp(date_, "0000-00-00") == 0) {
	props.erase(MetaFlac::TAG_DATE);
	_tag->setProperties(props);
        return;
    }

    const auto  prop = props.find(MetaFlac::TAG_DATE);

    if (prop != props.end()) {
	props.erase(MetaFlac::TAG_DATE);
    }

    _property(*_tag, MetaFlac::TAG_DATE, date_);
    // taglib sets the year for us
}

void MetaFlac::assign(const MetaTOI& toi_, const Input& rhs_)
{
    if (toi_.flac || toi_.deflt || toi_.all) {
        if (_tag == NULL) {
            _tag = _tf.xiphComment(true);
        }
        _assign(*_tag, rhs_);
    }
}

void  MetaFlac::artwork(Artwork& artwork_)
{
    TagLib::FLAC::Picture* picture = new TagLib::FLAC::Picture();

    TagLib::FLAC::Picture::Type  atype = TagLib::FLAC::Picture::FrontCover;

    picture->setType(atype);
    picture->setMimeType(artwork_.mimeType());
    picture->setData(artwork_.data());
    picture->setDescription("Front Cover");

    TagLib::List<TagLib::FLAC::Picture*>  pics = _tf.pictureList();
    for (auto i=pics.begin(); i!=pics.end(); ++i)
    {
	if ((*i)->type() == atype) {
	    _tf.removePicture(*i);
	}
    }
    _tf.addPicture(picture);
}


/* FLAC user error/bug?  pictureList() and removeAllPictures dont work on the 
 * _tag object; examining the .flac file with 'metaflac --list foo.flac' we are
 * presnted with:
 *
 METADATA block #x
  type: 6 (PICTURE)
  is last: false
  length: 3937
  type: 3 (Cover (front))
  MIME type: image/jpeg
 */

bool  MetaFlac::coverart() const
{
    MP3_TAG_DEBUG("flac: pic list sz, file=" <<  _tf.pictureList().size() << " tag=" << _tag->pictureList().size());
    return _tf.pictureList().size() > 0;
}

void  MetaFlac::removeart()
{
    _tf.removePictures();
}

int  MetaFlac::rating() const
{
    const TagLib::PropertyMap  props = properties(*_tag);
    const auto  prop = props.find(MetaFlac::TAG_RATING);

    if (prop == props.end()) {
	return Meta::rating();
    }

    const auto  l = *prop;
    if (l.second.isEmpty()) {
	return Meta::rating();
    }

    const int  r = atoi(l.second.front().toCString());
    if (r >  0 && r <=  20)  return 1;
    if (r > 20 && r <=  40)  return 2;
    if (r > 40 && r <=  60)  return 3;
    if (r > 60 && r <=  80)  return 4;
    if (r > 80 && r <= 100)  return 5;

    return Meta::rating();
}

void  MetaFlac::rating(uint8_t r_)
{
    // map 0..5 to 0..100
    const char*  r;
    switch (r_)
    {
	case 1: r = "20"; break;
	case 2: r = "40"; break;
	case 3: r = "60"; break;
	case 4: r = "80"; break;
	case 5: r = "100"; break;

	case 0:
	default:
	    r = NULL;
    }

    if (r == NULL) {
        // TODO the _property() merges but not deletes tags - prob need to handle that
        TagLib::PropertyMap  m = properties(*_tag);
        m.erase(MetaFlac::TAG_RATING);
	_tag->setProperties(m);
    }
    else {
        _property(*_tag, MetaFlac::TAG_RATING, r);
    }
}

void  MetaFlac::properties(TagLib::Tag& t_, const TagLib::PropertyMap& m_) const
{
    TagLib::Ogg::XiphComment*  t = dynamic_cast<TagLib::Ogg::XiphComment*>(&t_);
    if (t)  _properties(*t, m_);
    else    _properties(t_, m_);
}

TagLib::PropertyMap  MetaFlac::properties(const TagLib::Tag& t_) const
{
    const TagLib::Ogg::XiphComment*  t = dynamic_cast<const TagLib::Ogg::XiphComment*>(&t_);
    if (t) return _properties(*t);
    else   return _properties(t_);
}


// m4a /////////////////////////////////////////////////////////////////////////
const char*  MetaM4a::TAG_TRACK_NO = "trkn";
const char*  MetaM4a::TAG_COVERART = "covr";
const char*  MetaM4a::TAG_RATE = "rate";


MetaM4a::MetaM4a(FileM4a& f_, MetaOut& mo_)
  : Meta(f_.taglibfile(), (TagLib::Tag**)&_tag, mo_), 
    _f(f_),
    _tf((TagLib::MP4::File&)_f.taglibfile()),
    _tag(NULL)
{
    _tag = _tf.tag();  // always non null, even if disk may not have value
}

Meta::Tags  MetaM4a::tags() const
{
    Meta::Tags  tag;
    if ( _tf.hasMP4Tag() && !_tag->isEmpty()) {
        tag.push_back(Meta::Tags::value_type("MP4", _tag));
    }
    return tag;
}

void  MetaM4a::year(TagLib::Tag& tag_, const unsigned data_)
{
    // for some reason taglib 1.11.1 seems to have a couple of problems:
    // .. set{Year,Date}(0) does NOT clear these values for m4a
    // .. can't use symbolic DATE/TRACKNUMBER to remove
 
    if (data_ > 0) _tag->setYear ( data_);
    else if (data_ == 0 && _tag == &tag_) _tag->removeItem("\251day");
}

void  MetaM4a::trackno(TagLib::Tag& tag_, const unsigned data_)
{
    if (data_ > 0) tag_.setTrack(data_);
    else if (data_ == 0 && _tag == &tag_) _tag->removeItem(MetaM4a::TAG_TRACK_NO);
}

void  MetaM4a::trackno(TagLib::Tag& tag_, const unsigned x_, const unsigned y_)
{
    if (x_ && y_) Meta::trackno(tag_, x_, y_);
    else  _tag->removeItem(MetaM4a::TAG_TRACK_NO);
}


void MetaM4a::remove(const MetaTOI& toi_)
{
    if (!_tag && !toi_.mp4 && !toi_.deflt) {
        return;
    }

    // non const method deprecated, workaround
    // _tag->itemListMap().clear();
    const auto&  items = _tag->itemMap();
    TagLib::MP4::ItemMap::ConstIterator i = items.begin();
    while (i != items.end()) {
        const auto&  key = i->first;
        ++i;

        _tag->removeItem(key);
    }
}

void  MetaM4a::artwork(Artwork& artwork_)
{
    TagLib::MP4::CoverArt coverArt(artwork_.type() == Artwork::JPEG ? TagLib::MP4::CoverArt::JPEG : TagLib::MP4::CoverArt::PNG, artwork_.data());

    TagLib::MP4::CoverArtList l;
    l.append(coverArt);
    _tf.tag()->setItem(MetaM4a::TAG_COVERART, l);
}

bool  MetaM4a::coverart() const
{
    return _tag->contains(MetaM4a::TAG_COVERART);
}

void  MetaM4a::removeart()
{
    // what about the other art... ???
    _tag->removeItem(MetaM4a::TAG_COVERART);
}

void MetaM4a::assign(const MetaTOI& toi_, const Input& rhs_)
{
    if (toi_.mp4 || toi_.deflt) {
        if (_tag == NULL) {
            _tag = _tf.tag();
        }
        _assign(*_tag, rhs_);
    }
}

int  MetaM4a::rating() const
{
    const TagLib::String  key = MetaM4a::TAG_RATE;
    if (_tag->contains(key)) {
        const int  r = _tag->item(key).toInt();
        if (r >  0 && r <=  20)  return 1;
        if (r > 20 && r <=  40)  return 2;
        if (r > 40 && r <=  60)  return 3;
        if (r > 60 && r <=  80)  return 4;
        if (r > 80 && r <= 100)  return 5;
    }
    return Meta::rating();
}

void  MetaM4a::rating(uint8_t r_)
{
    const TagLib::String  key = MetaM4a::TAG_RATE;

    // map 0..5 to 0..100
    int  r = 0;
    switch (r_)
    {
	case 1: r =  20; break;
	case 2: r =  40; break;
	case 3: r =  60; break;
	case 4: r =  80; break;
	case 5: r = 100; break;

	case 0:
	default:
	    r = 0;
    }
    if (r == 0) {
        _tf.tag()->removeItem(key);
    }
    else {
        _tf.tag()->setItem(key, r);
    }
}

void  MetaM4a::properties(TagLib::Tag& t_, const TagLib::PropertyMap& m_) const
{
    TagLib::MP4::Tag*  t = dynamic_cast<TagLib::MP4::Tag*>(&t_);
    if (t)  _properties(*t, m_);
    else    _properties(t_, m_);
}

TagLib::PropertyMap  MetaM4a::properties(const TagLib::Tag& t_) const
{
    const TagLib::MP4::Tag*  t = dynamic_cast<const TagLib::MP4::Tag*>(&t_);
    if (t) return _properties(*t);
    else   return _properties(t_);
}

std::ostream&  Meta::out(std::ostream& os_, const File& f_) const
{
    return _mo.out(os_, f_);
}


};
