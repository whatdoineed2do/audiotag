#include "Meta.h"

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif
#include <taglib/tstring.h>
#include <taglib/tpropertymap.h>

#include <taglib/flacfile.h>
#include <taglib/flacpicture.h>

#include "audiotag.h"
#include "File.h"
#include "Artwork.h"

namespace AudioTag {

const char*  MetaFlac::TAG_RATING = "RATING";
const char*  MetaFlac::TAG_DATE = "DATE";


MetaFlac::MetaFlac(FileFlac& f_, MetaOut& mo_) 
  : Meta(f_, (TagLib::Tag**)&_tag, mo_), 
    _tf((TagLib::FLAC::File&)f_.taglibfile()),
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
    TagLib::PropertyMap  props = properties();

    if (strcmp(date_, Meta::DATE_REMOVE) == 0) {
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
    AUDIOTAG_DEBUG("flac: pic list sz, file=" <<  _tf.pictureList().size() << " tag=" << _tag->pictureList().size());
    return _tf.pictureList().size() > 0;
}

void  MetaFlac::removeart()
{
    _tf.removePictures();
}

int  MetaFlac::rating() const
{
    const TagLib::PropertyMap  props = properties();
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
        TagLib::PropertyMap  m = properties();
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

}
