#include "audiotagmeta.h"

#include <string.h>
#include <taglib/tstring.h>

#include "audiotag.h"
#include "audiotagfile.h"
#include "audiotagmetaout.h"

using std::cout;
using std::cerr;
using std::endl;


namespace AudioTag
{

TagLib::String  _cnvrt(const char* data_)
{
    const size_t  n = strlen(data_)*sizeof(wchar_t);
    if (n == 0) {
        return TagLib::String::null;
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
    if (str_ == TagLib::String::null) {
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



 

std::ostream&  Meta::_out(std::ostream& os_, const Meta::Tags::value_type& t_) const
{
    // delegate the outputting to the registered MetaOut subclasses
    os_ << "[" << std::right << std::setw(5) << t_.first << "]";
    _mo.out(os_, *this, *t_.second, t_.first);
    return os_;
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
    tag_.setArtist( data_ ? _cnvrt(data_) : TagLib::String::null);
}


void  Meta::album(TagLib::Tag& tag_, const char* data_)
{
    tag_.setAlbum( data_ ? _cnvrt(data_) : TagLib::String::null);
}


void  Meta::title(TagLib::Tag& tag_, const char* data_)
{
    tag_.setTitle( data_ ? _cnvrt(data_) : TagLib::String::null);
}


void  Meta::comment(TagLib::Tag& tag_, const char* data_)
{
    tag_.setComment( data_ ? _cnvrt(data_) : TagLib::String::null);
}


void  Meta::genre(TagLib::Tag& tag_, const char* data_)
{
    tag_.setGenre( data_ ? _cnvrt(data_) : TagLib::String::null);
}

void  Meta::year(TagLib::Tag& tag_, const int data_)
{
    tag_.setYear( data_ > 0 ? data_ : 0);
}


void  Meta::trackno(TagLib::Tag& tag_, const int data_)
{
    tag_.setTrack( data_ > 0 ? data_ : 0);
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
    if (rhs_.trackno)    trackno(tag_, atol(rhs_.trackno));
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

    iflds_.artist = (a == TagLib::String::null) ? NULL : (va = a.data(mbenc_)).data();
    iflds_.title  = (t == TagLib::String::null) ? NULL : (vt = t.data(mbenc_)).data();
    iflds_.album  = (A == TagLib::String::null) ? NULL : (vA = A.data(mbenc_)).data();
    iflds_.genre  = (g == TagLib::String::null) ? NULL : (vg = g.data(mbenc_)).data();

    sprintf(T, "%ld", tag_.track());
    sprintf(y, "%ld", tag_.year());
    iflds_.trackno = T;
    iflds_.yr = y;
}


void  MetaMP3::save()
{
    /* by default, just keep the id3v2 tag;  need to explicitly ask TagLib to 
     * only save that (otherwise it copies id3v2 onto id3v1) but consider if 
     * other tags have modified
     */
    MP3_TAG_DEBUG("mp3 savetag=" << _svtag);
    _tf.save(_svtag == 0 ? TagLib::MPEG::File::ID3v2 : _svtag, true);
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
    _tf.strip(rmtag, true);

    /* BUG - if you ask to remove APE/ID3v1, both appear to be removed but when
     *       saved the ID3v2 tag is duplicated onto ID3v1; save tag values are 
     *       correct (0x0002)
     */

    if (toi_.id3v2 || toi_.deflt) {
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

#if 0 
    _assign(**_tag, input);
    TagLib::Tag*  tag = *_tag;
    artist = _strrep(tag_->artist(), &a);
    title = _strrep(tag_->title(), &t);
    album = _strrep(tag_->album(), &A);
    genre = _strrep(tag_->genre(), &g);

    sprintf(T, "%ld", tag_->track());
    sprintf(y, "%ld", tag_->year());
    trackno = T;
    yr = y;
#endif
}

bool  MetaMP3::coverart() const
{
    return (_id3v2 == NULL) ? false : !_id3v2->frameList("APIC").isEmpty();
}

void  MetaMP3::removeart()
{
    if (_id3v2) {
        _id3v2->removeFrames("APIC");
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

#if 0
    if (flds_.artist)   _tagFrme(tag_, new TagLib::ID3v2::TextIdentificationFrame("TPE1", enc_), flds_.artist);
    if (flds_.album)    _tagFrme(tag_, new TagLib::ID3v2::TextIdentificationFrame("TALB", enc_), flds_.album);
    if (flds_.title)    _tagFrme(tag_, new TagLib::ID3v2::TextIdentificationFrame("TIT2", enc_), flds_.title);
#else
    if (flds_.artist)   _tagFrme(tag_, new TagLib::ID3v2::TextIdentificationFrame("TPE1", _overrideEncLatin(flds_.artist, enc_)), flds_.artist);
    if (flds_.album)    _tagFrme(tag_, new TagLib::ID3v2::TextIdentificationFrame("TALB", _overrideEncLatin(flds_.album, enc_)), flds_.album);
    if (flds_.title)    _tagFrme(tag_, new TagLib::ID3v2::TextIdentificationFrame("TIT2", _overrideEncLatin(flds_.title, enc_)), flds_.title);
#endif

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

void MetaFlac::assign(const MetaTOI& toi_, const Input& rhs_)
{
    if (toi_.flac || toi_.deflt || toi_.all) {
        if (_tag == NULL) {
            _tag = _tf.xiphComment(true);
        }
        _assign(*_tag, rhs_);
    }
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

void MetaM4a::remove(const MetaTOI& toi_)
{
    if (!_tag && !toi_.mp4 && !toi_.deflt) {
        return;
    }

    for (TagLib::MP4::ItemListMap::Iterator i=_tag->itemListMap().begin(); i!=_tag->itemListMap().end(); ++i)
    {
        _tag->removeItem(i->first);
    }
}

bool  MetaM4a::coverart() const
{
    TagLib::MP4::Item  coverItem = _tag->itemListMap()["covr"];
    TagLib::MP4::CoverArtList coverArtList = coverItem.toCoverArtList();
    return !coverArtList.isEmpty();
}

void  MetaM4a::removeart()
{
    // what about the other art... ???
    _tag->removeItem("covr");
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


};
