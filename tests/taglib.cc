// dummy test file
// https://github.com/taglib/taglib/issues/835
//

#include <iostream>
#include <taglib/mpegfile.h>

int main(int argc, char *argv[])
{
    if (argc != 2) {
        return 1;
    }
    try
    {
        TagLib::MPEG::File  f(argv[1], false);
        f.strip(TagLib::MPEG::File::ID3v1 | TagLib::MPEG::File::APE);
        f.save(TagLib::MPEG::File::ID3v2);
    }
    catch (const std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
    }
    return 0;
}
