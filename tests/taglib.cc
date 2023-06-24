// dummy test file
// https://github.com/taglib/taglib/issues/835
//

#include <iostream>
#include <functional>

#include <taglib/mpegfile.h>
#include <taglib/tpropertymap.h>

#include <taglib/id3v2tag.h>
#include <taglib/flacfile.h>
#include <taglib/xiphcomment.h>

const char*  prop = "zzz";
const char*  val  = "foo";

void _update(TagLib::PropertyMap&  props_)
{

    auto  where = props_.find(prop);
    if (where == props_.end()) {
	std::cout << "adding     prop '" << prop << "' = '" << val << "'\n";
	props_.insert(prop, TagLib::StringList(val) );
    }
    else {
	std::cout << "preppeding prop '" << prop << "' = '" << val << "'\n";
	where->second.prepend(val);
    }
}

bool _lookup(TagLib::PropertyMap&&  props_, const char* where_)
{
    auto where = props_.find(prop);
    if (where == props_.end()) {
	std::cout << where_ << " prop '" << prop << "' NOT found\n";
	return false;
    }
    else {
	std::cout << where_ << " prop '" << prop << "'     found\n";
	return true;
    }
}


int main(int argc, char *argv[])
{
    const char*  mp3 = "test.mp3";
    const char*  m4a = "test.m4a";
    const char*  flac = "test.flac";

    try
    {
	{
	    std::function<void (TagLib::MPEG::File&)>  mp3lookup = [](TagLib::MPEG::File& f_) -> void {
		std::cout << "looking via base\n";
		_lookup(f_.tag()->properties(), "Tag::properties()");

		std::cout << "looking via child\n";
		_lookup(f_.ID3v2Tag()->properties(), "ID3v2::Tag::properties()");
	    };

	    {
		std::cout << "*** opening..." << mp3 << "\n";
		TagLib::MPEG::File  f(mp3, false);

		TagLib::Tag*  tag = f.tag();
		TagLib::PropertyMap  props = tag->properties();

		_update(props);
		std::cout << "setProperies() via base\n";
		tag->setProperties(props);
		mp3lookup(f);

		std::cout << "setProperies() via child\n";
		f.ID3v2Tag()->setProperties(props);
		mp3lookup(f);

		f.strip(TagLib::MPEG::File::ID3v1 | TagLib::MPEG::File::APE);
		f.save(TagLib::MPEG::File::ID3v2);
	    }

	    {
		std::cout << "*** re-opening..." << mp3 << "\n";
		TagLib::MPEG::File  f(mp3, false);
		mp3lookup(f);
	    }
	}

	{
	    std::function<void (TagLib::FLAC::File&)>  flaclookup = [](TagLib::FLAC::File& f_) -> void {
		std::cout << "looking via base\n";
		_lookup(f_.tag()->properties(), "Tag::properties()");

		std::cout << "looking via child\n";
		_lookup(f_.xiphComment()->properties(), "Flac::Tag::properties()");
	    };

	    {
		std::cout << "*** opening..." << flac<< "\n";
		TagLib::FLAC::File  f(flac);

		TagLib::Tag*  tag = f.tag();
		TagLib::PropertyMap  props = tag->properties();

		_update(props);
		std::cout << "setProperies() via base\n";
		tag->setProperties(props);
		flaclookup(f);

		std::cout << "setProperies() via child\n";
		f.xiphComment()->setProperties(props);
		flaclookup(f);

		f.save();
	    }

	    {
		std::cout << "*** re-opening..." << flac << "\n";
		TagLib::FLAC::File  f(flac);
		flaclookup(f);
	    }
	}}
    catch (const std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
    }
    return 0;
}
