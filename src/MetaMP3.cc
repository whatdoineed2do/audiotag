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

#include "audiotag.h"
#include "File.h"
#include "MetaOut.h"
#include "Artwork.h"


namespace AudioTag {
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
    if (strcmp(date_, Meta::DATE_REMOVE) == 0) {
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
    AUDIOTAG_DEBUG("mp3 savetag=" << _svtag);
    _tf.save(_svtag == 0 ? TagLib::MPEG::File::ID3v2 : _svtag,
#if TAGLIB_MAJOR_VERSION == 1 && TAGLIB_MINOR_VERSION == 11
	     // ubuntu does not have anything > 1.11.1
	     true, 4, false
#else
	     TagLib::File::StripOthers, TagLib::ID3v2::v4, TagLib::File::DoNotDuplicate
#endif
            );
}

void MetaMP3::remove(const MetaTOI& toi_)
{
    int  rmtag = TagLib::MPEG::File::NoTags;

    if (toi_.all) {
        rmtag = TagLib::MPEG::File::AllTags;
        _svtag = TagLib::MPEG::File::NoTags;
	_id3v1 = NULL;
	_ape = NULL;
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
    AUDIOTAG_DEBUG("mp3 strip=" << rmtag);
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

    AUDIOTAG_DEBUG("mp3 ctor: " << _svtag << "  1:" << std::hex << _id3v1 << " 2: "<< _id3v2 << " a: " << _ape);
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

}
