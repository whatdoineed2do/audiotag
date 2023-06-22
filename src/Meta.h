#ifndef AUDIOTAG_META_H
#define AUDIOTAG_META_H

#include <stdio.h>
#include <string.h>

#include <cstdint>
#include <stdexcept>
#include <exception>
#include <iostream>
#include <iomanip>
#include <list>
#include <string>
#include <utility>
#include <algorithm>

// taglib hrs
#include <taglib/tstring.h>
#include <taglib/tlist.h>
#include <taglib/tag.h>
#include <taglib/id3v1tag.h>
#include <taglib/id3v2header.h>
#include <taglib/id3v2frame.h>
#include <taglib/id3v2tag.h>
#include <taglib/apetag.h>
#include <taglib/textidentificationframe.h>
#include <taglib/commentsframe.h>
#include <taglib/xiphcomment.h>
#include <taglib/mp4tag.h>
#include <taglib/tpropertymap.h>

namespace TagLib
{
    namespace MPEG { class File; };
    namespace FLAC { class File; };
    namespace Ogg  { namespace FLAC { class File; }; };
    namespace MP4  { class File; };
}

namespace AudioTag
{

class File;
class FileMP3;
class FileFlac;
class FileOGGFlac;
class FileM4a;
class MetaOut;
class Meta;

class Artwork;

static const TagLib::ID3v2::Tag  _EMPTY_TAG;
const TagLib::String::Type  INTNL_STR_ENC = TagLib::String::UTF16BE;

TagLib::String::Type  parseEnc(const char* optarg_, TagLib::String::Type dflt_);

TagLib::String  _cnvrt(const char* data_);
const char*  _strrep(const TagLib::String&  str_, TagLib::String* ptr_=NULL);

class Input
{
  public:
    const char*  artist;   // TPE1
    const char*  album;    // TALB or TOAL
    const char*  title;    // TIT2
    const char*  comment;  // COMM
    const char*  genre;    // TCON
    const char*  yr;       // TDRC or TYER or TORY
    const char*  trackno;  // TRCK as 'x'
    const char*  trackN;   // last track

// this block are valid tags but no direct taglib get/set i/f
    const char*  date;     // TDRC, DATE
    const char*  disc;     // TPOS or DISCNUMBER or disc (as x/y)
    const char*  albumartist;  // TPE2, ALBUMARTIST, aArt

    TagLib::PropertyMap  properties;

    const char*  rating;  // POPM or RATING

    void  reset()
    {
        artist = NULL;
        album = NULL;
        title = NULL;
        comment = NULL;
        genre = NULL;
        yr = NULL;
        trackno = NULL;
        trackN = NULL;

	date = NULL;
	disc = NULL;
        albumartist = NULL;

        properties.clear();

	rating = NULL;
    }


    Input() :
        artist(NULL),
        album(NULL),
        title(NULL),
        comment(NULL),
        genre(NULL),
        yr(NULL),
        trackno(NULL),
        trackN(NULL),
	date(NULL),
        disc(NULL),
        albumartist(NULL),
	rating(NULL)
    { reset(); }

    Input(const TagLib::Tag* tag_)
    {
        reset();
        populate(tag_);
    }

    Input(const Meta& meta_);

    Input&  operator=(const TagLib::Tag* tag_)
    {
        populate(tag_);
        return *this;
    }

    operator bool() const
    {
	return
	    artist ||
	    album ||
	    title ||
	    comment ||
	    genre ||
	    yr ||
	    trackno ||
	    trackN ||

	    date ||
	    disc ||
	    albumartist ||

	    !properties.isEmpty() ||
	    rating;
    }


    void  strip()
    {
        const char**  a[] = {
	    // keep in sync with Input object string members
	    &artist,
	    &album,
	    &title,
	    &comment,
	    &genre,
	    &yr,
	    &trackno,
	    &trackN,

	    &date,
	    &disc,
	    &albumartist,

	    &rating,

	    NULL
	};

        const char***  p = a;
	while (*p)
	{
	    if (**p) {
		char*  x = (char*)**p;
		char*  y = x;

		while (*x) {
		    ++x;
		}

		/* y = start, x = end */
		if (y == x) {
		}
		else
		{
		    --x;
		    /* get rid of trailing... */
		    while (x > y && isspace(*x)) {
			--x;
		    }
		    *++x = (char)NULL;

		    /* ...and leading */
		    while (y < x && isspace(*y)) {
			++y;
		    }

		    **p = (y == x) ? NULL : y;
		}
	    }
            ++p;
        }
    }

    void  populate(const TagLib::Tag* tag_);
    bool  validate() const;

  private:
    TagLib::String  a,t,A,g;
    char  y[5];
    char  T[3];
    char  r[4];  // 0 .. 255
};

void  multibyteConvert(Input&, TagLib::Tag&  tag_, const TagLib::String::Type);

struct MetaTOI  // tags of interest
{
    bool  all;  // TagLib::{MPEG,FLAC}::File::AllTags

    bool  deflt;  // whatever is our default tag for file type
    bool  id3v1;  // TagLib::{MPEG,FLAC}::File::ID3v1
    bool  id3v2;  // TagLib::{MPEG,FLAC}::File::ID3v2
    bool  ape;    // TagLib::MPEG::File::APE
    bool  flac;   // TagLib::FLAC::File::XiphComment
    bool  mp4;    // n/a


    void  reset()
    { all = deflt = id3v1 = id3v2 = ape = flac = mp4 = false; }


    MetaTOI()
    { reset(); }

    MetaTOI(const char* optarg_)
    {
        reset();
        *this = optarg_;
    }

    bool  operator!=(const MetaTOI& rhs_)
    { return ! (*this == rhs_); }

    bool  operator==(const MetaTOI& rhs_);

    MetaTOI& operator=(const char optarg_);
    MetaTOI& operator=(const char* optarg_);

    operator bool() const
    { return (all || deflt || id3v1 || id3v2 || ape || flac || mp4); }
};


class Meta
{
  public:
    static constexpr const char*  DATE_REMOVE = "0000-00-00";
    using Tags = std::list<std::pair<const char*, TagLib::Tag*> >;

    virtual ~Meta() = default;

    Meta(const Meta&) = delete;
    void operator=(const Meta&) = delete;


    const TagLib::File&  file() const
    { return _file; }

    const TagLib::Tag*  tag() const
    { return *_tag; }

    virtual Meta::Tags  tags() const = 0;


    // remove all tags except for basic
    virtual void  sanitize();

    virtual void  clone(const MetaTOI& to_, const MetaTOI& from_) { }

    virtual void  remove(const MetaTOI&) = 0;
    virtual void  assign(const MetaTOI&, const Input&) { }



    virtual void   artist(TagLib::Tag&, const char*);
    virtual void    album(TagLib::Tag&, const char*);
    virtual void  albumArtist(TagLib::Tag&, const char*);
    virtual void    title(TagLib::Tag&, const char*);
    virtual void  comment(TagLib::Tag&, const char*);
    virtual void    genre(TagLib::Tag&, const char*);

    virtual void     year(TagLib::Tag&, const unsigned);
    virtual void     date(TagLib::Tag&, const char*);
    virtual void  trackno(TagLib::Tag&, const unsigned);
    virtual void  trackno(TagLib::Tag&, const unsigned, const unsigned);

    virtual void  disc(TagLib::Tag&, const char*);

    virtual void  artwork(Artwork&) { }
    virtual bool  coverart() const { return false; }
    virtual void  removeart() { }

    virtual int   rating() const { return -1; }  // -1 no rating
    virtual void  rating(uint8_t) = 0;  // 0 resets, take 0..5
    virtual void  removerating() { }


    // force any changes to underlying file
    virtual void  save();


    Meta&  operator=(const Input& in_)
    {
        _assign(**_tag, in_);
        return *this;
    }


    virtual TagLib::PropertyMap  properties(const TagLib::Tag& t_) const
    { return t_.properties(); }

    virtual void                 properties(TagLib::Tag& t_, const TagLib::PropertyMap& m_) const
    { t_.setProperties(m_); }

    std::ostream&  out(std::ostream& os_, const File& f_) const;


  protected:
    Meta(TagLib::File& file_, TagLib::Tag** tag_, MetaOut& mo_)
        : _file(file_), _tag(tag_), _mo(mo_)
    {
        if (false && *_tag == NULL) {
            throw std::invalid_argument("Tag is NULL");
        }
    }

    TagLib::String::Type  _overrideEncLatin(const char* data_, TagLib::String::Type enc_)
    {
        const char*  p = data_;
        while (*p) {
            if (!isascii(*p++)) {
                return enc_;
            }
        }
        return TagLib::String::Latin1;
    }


    /* set tags via properties where taglib doesnt provide direct i/f
     * for valid tags (ie albumartist, disc...
     */
    void  _property(TagLib::Tag& tag_, const char* tagname_, const char* data_);

    virtual void  _assign(TagLib::Tag&, const Input&);

    virtual TagLib::PropertyMap&  _mergeproperties(TagLib::PropertyMap&, const TagLib::PropertyMap&) const;

    template <typename T>
    TagLib::PropertyMap  _properties(const T& t_) const
    { return t_.properties(); }

    template <typename T>
    void  _properties(T& t_, const TagLib::PropertyMap& m_) const
    {
	// add or replace ..don't blow away
	TagLib::PropertyMap  m = t_.properties();
	t_.setProperties(_mergeproperties(m, m_));
    }


  private:
    TagLib::File&  _file;
    // default tag of file (for files that support multi tags); may not exist on disk
    TagLib::Tag**   _tag;

    MetaOut&  _mo;
};


class _MetaMulti : public Meta
{
  public:
    virtual ~_MetaMulti() = default;

  protected:
    _MetaMulti(TagLib::File& file_, TagLib::Tag** tag_, MetaOut& mo_) : Meta(file_, tag_, mo_) { }
};


// only care about ID3v2
class MetaMP3 : public _MetaMulti
{
  public:
    MetaMP3(FileMP3&, MetaOut&);

    ~MetaMP3() = default;
    MetaMP3(const MetaMP3&) = delete;
    void operator=(const MetaMP3&) = delete;

    Meta::Tags  tags() const;

    void  year(TagLib::Tag&, const unsigned)  override;
    void  date(TagLib::Tag&, const char*)  override;

    void  save();
    void  remove(const MetaTOI&);
    void  assign(const MetaTOI&, const Input&);
    void  clone(const MetaTOI& to_, const MetaTOI& from_);
    void  sanitize();

    void  artwork(Artwork&);
    bool  coverart() const;
    void  removeart();

    int   rating() const;
    void  rating(uint8_t);  // takes 0..255
    void  removerating();

    TagLib::PropertyMap  properties(const TagLib::Tag&) const;
    void                 properties(TagLib::Tag&, const TagLib::PropertyMap&) const;

  private:
    FileMP3&  _f;
    TagLib::MPEG::File&  _tf;

    TagLib::ID3v2::Tag*  _id3v2;
    TagLib::ID3v1::Tag*  _id3v1;
    TagLib::APE::Tag*    _ape;

    void  _tagFrme(TagLib::ID3v2::Tag*  tag_, TagLib::ID3v2::Frame* frme_, const char* data_);
    void  _tag(TagLib::ID3v2::Tag*  tag_, const Input&  flds_, TagLib::String::Type  enc_);

    const char*  _id3v2TxtEnc(const TagLib::ID3v2::Frame* f_);
    int  _svtag;

    static const char*  TAG_DATE;
    static const char*  TAG_RATING;
    static const char*  TAG_COVERART;
    static const char*  TAG_COVERART_OLD;
};



class MetaOGGFlac : public Meta
{
  public:
    MetaOGGFlac(FileOGGFlac&, MetaOut&);

    ~MetaOGGFlac() = default;
    MetaOGGFlac(const MetaOGGFlac&) = delete;
    void operator=(const MetaOGGFlac&) = delete;

    const bool  empty() const;
    //{ return _f.hasXiphComment(); }

    Meta::Tags  tags() const;
    void  remove(const MetaTOI&);

    int   rating() const;
    void  rating(uint8_t);

    TagLib::PropertyMap  properties(const TagLib::Tag&) const;
    void                 properties(TagLib::Tag&, const TagLib::PropertyMap&) const;

  private:
    FileOGGFlac&  _f;
    TagLib::Ogg::FLAC::File&  _tf;

    TagLib::Ogg::XiphComment*  _tag;

    // this can have id3v2/v1 but ignoring
};

class MetaFlac : public Meta
{
  public:
    MetaFlac(FileFlac&, MetaOut&);

    ~MetaFlac() = default;
    MetaFlac(const MetaFlac&) = delete;
    void operator=(const MetaFlac&) = delete;

    Meta::Tags  tags() const;
    void  remove(const MetaTOI&);

    void  date(TagLib::Tag&, const char*)  override;

    void  assign(const MetaTOI&, const Input&);
    void  artwork(Artwork&);
    bool  coverart() const;
    void  removeart();

    int   rating() const;
    void  rating(uint8_t);

    TagLib::PropertyMap  properties(const TagLib::Tag&) const;
    void                 properties(TagLib::Tag&, const TagLib::PropertyMap&) const;

  private:
    FileFlac&  _f;
    TagLib::FLAC::File&  _tf;

    TagLib::Ogg::XiphComment*  _tag;

    // this can have id3v2/v1 but ignoring

    static const char*  TAG_RATING;
    static const char*  TAG_DATE;
};

class MetaM4a : public Meta
{
  public:
    MetaM4a(FileM4a&, MetaOut&);

    ~MetaM4a() = default;
    MetaM4a(const MetaM4a&) = delete;
    void operator=(const MetaM4a&) = delete;

    void  remove(const MetaTOI&);
    void  artwork(Artwork&);
    bool  coverart() const;
    void  removeart();

    void     year(TagLib::Tag&, const unsigned)  override;
    void  trackno(TagLib::Tag&, const unsigned)  override;
    void  trackno(TagLib::Tag&, const unsigned, const unsigned)  override;

    void  assign(const MetaTOI&, const Input&);

    Meta::Tags  tags() const;

    int   rating() const;
    void  rating(uint8_t);

    TagLib::PropertyMap  properties(const TagLib::Tag&) const;
    void                 properties(TagLib::Tag&, const TagLib::PropertyMap&) const;

  private:
    FileM4a&  _f;
    TagLib::MP4::File&  _tf;

    TagLib::MP4::Tag*  _tag;

    static const char*  TAG_TRACK_NO;
    static const char*  TAG_COVERART;
    static const char*  TAG_RATE;
    static const char*  TAG_DATE;
};


};

#endif
