#ifndef AUDIOTAG_ARTWORK_H
#define AUDIOTAG_ARTWORK_H

#include <exception>
#include <algorithm>
#include <string>

#include <taglib/tfile.h>

namespace AudioTag
{
class Artwork : public TagLib::File
{
  public:
    enum Type { JPEG = 0, PNG, UNDEF = 9999 };

    Artwork(const char* file_) : TagLib::File(file_), _mimeType(file_), _type(Artwork::UNDEF)
    {
	auto pos = _mimeType.find_last_of('.');
	if (pos == std::string::npos) {
	    throw std::invalid_argument("unknown image extension");
	}
	_mimeType = _mimeType.substr(pos+1);
	std::transform(_mimeType.begin(), _mimeType.end(), _mimeType.begin(), ::tolower);

	if      (_mimeType == "jpg" || _mimeType == "jpeg") { _mimeType = "image/jpeg"; _type = Artwork::JPEG; }
	else if (_mimeType == "png")                        { _mimeType = "image/png";  _type = Artwork::PNG;  }
	else {
	    throw std::invalid_argument("unknown image extension");
	}
    }

    TagLib::ByteVector  data()
    { return readBlock(length()); }

    const std::string&  mimeType() const { return _mimeType; }
    Artwork::Type  type() const { return _type; }

  private:
    TagLib::Tag*  tag()  const { return NULL; }
    TagLib::AudioProperties* audioProperties() const { return NULL; }
    bool  save() { return false; }

    std::string  _mimeType;
    Artwork::Type  _type;
};

};

#endif
