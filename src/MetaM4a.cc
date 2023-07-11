#include "Meta.h"

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif
#include <string.h>
#include <taglib/tstring.h>
#include <taglib/tpropertymap.h>

#include <taglib/mp4tag.h>
#include <taglib/mp4coverart.h>

#include "audiotag.h"
#include "File.h"
#include "Artwork.h"


namespace AudioTag {
const char*  MetaM4a::TAG_TRACK_NO = "trkn";
const char*  MetaM4a::TAG_COVERART = "covr";
const char*  MetaM4a::TAG_RATE = "rate";
const char*  MetaM4a::TAG_DATE = "\251day";


MetaM4a::MetaM4a(FileM4a& f_, MetaOut& mo_)
  : Meta(f_, (TagLib::Tag**)&_tag, mo_), 
    _tf((TagLib::MP4::File&)f_.taglibfile()),
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
    else if (data_ == 0 && _tag == &tag_) _tag->removeItem(MetaM4a::TAG_DATE);
}

void MetaM4a::date(TagLib::Tag& t_, const char* date_)
{
    if (strcmp(date_, Meta::DATE_REMOVE) == 0) {
	_tag->removeItem(MetaM4a::TAG_DATE);
    }
    else {
	_tag->setItem(MetaM4a::TAG_DATE, TagLib::StringList(date_));
    }
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


}
