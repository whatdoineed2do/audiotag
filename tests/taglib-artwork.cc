// based on -- https://gist.github.com/guymac/1468279
//
#include <iostream>

#include <taglib/mpegfile.h>
#include <taglib/mpegfile.h>
#include <taglib/attachedpictureframe.h>
#include <taglib/id3v2tag.h>
#include <taglib/mp4file.h>
#include <taglib/mp4tag.h>
#include <taglib/mp4coverart.h>
#include <taglib/vorbisfile.h>
#include <taglib/xiphcomment.h>
#include <taglib/flacfile.h>
#include <taglib/flacpicture.h>

class ImageFile : public TagLib::File
{
  public:
    ImageFile(const char *file) : TagLib::File(file)
    { }

    TagLib::ByteVector data()
    { return readBlock(length()); }

  private:
    TagLib::Tag* tag() const { return 0; }
    TagLib::AudioProperties* audioProperties() const { return 0; }
    bool save() { return false; }
};


int main(int argc, char* argv[])
{
    if (argc != 3) {
	std::cerr << "usage: <input> <artwork.jpg>\n";
	return -1;
    }

    TagLib::String fileName = argv[1];
    TagLib::String fileType = fileName.substr(fileName.size() - 4).upper();

    try
    {
	ImageFile imageFile(argv[2]);
	TagLib::ByteVector imageData = imageFile.data();

	if (fileType == ".M4A")
	{
	    // read the image file
	    TagLib::MP4::CoverArt coverArt(TagLib::MP4::CoverArt::JPEG, imageData);

	    // read the mp4 file
	    TagLib::MP4::File audioFile(argv[1]);
	    TagLib::MP4::CoverArtList l = audioFile.tag()->item("covr").toCoverArtList();
	    l.append(coverArt);
	    audioFile.tag()->setItem("covr", l);
	    audioFile.save();

	}
	else if (fileType == ".MP3")
	{
	    TagLib::MPEG::File audioFile(argv[1]);

	    TagLib::ID3v2::Tag *tag = audioFile.ID3v2Tag(true);
	    TagLib::ID3v2::AttachedPictureFrame *frame = new TagLib::ID3v2::AttachedPictureFrame;

	    frame->setMimeType("image/jpeg");
	    frame->setPicture(imageData);

	    tag->addFrame(frame);
	    audioFile.save();      
	}
	else if (fileType == "FLAC")
	{
	    TagLib::FLAC::File audioFile(argv[1]);

	    TagLib::FLAC::Picture* picture = new TagLib::FLAC::Picture();
	    picture->setData(imageData);
	    picture->setType(TagLib::FLAC::Picture::FrontCover); // FrontCover
	    picture->setMimeType("image/jpeg");
	    picture->setDescription("Front Cover");

	    audioFile.addPicture(picture);
	    audioFile.save();
	}
	else
	{
	    std::cerr << "unhandled type\n";
	}
    }
    catch (const std::exception& ex)
    {
	std::cerr << "exception - " << ex.what() << "\n";
    }
    return 0;
}
