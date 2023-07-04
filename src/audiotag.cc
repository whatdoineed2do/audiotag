#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <utime.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <libgen.h>
#include <getopt.h>

#include <iostream>
#include <iomanip>
#include <list>
#include <string>
#include <utility>
#include <algorithm>

// taglib hrs
#include <taglib/tfile.h>
#include <taglib/fileref.h>
#include <taglib/mpegfile.h>
#include <taglib/flacfile.h>
#include <taglib/oggflacfile.h>

#include <taglib/tag.h>
#include <taglib/id3v1tag.h>
#include <taglib/id3v2header.h>
#include <taglib/id3v2frame.h>
#include <taglib/id3v2tag.h>
#include <taglib/apetag.h>
#include <taglib/tlist.h>
#include <taglib/textidentificationframe.h>
#include <taglib/commentsframe.h>

#include <taglib/xiphcomment.h>

#include "audiotag.h"
#include "File.h"
#include "Meta.h"
#include "MetaOut.h"
#include "Ops.h"
#include "Artwork.h"

using namespace  std;

#include "config.h"

#if defined(HAVE_UTIMES) && defined(HAVE_ST_MTIM)
#define AUDIOTAG_HAVE_PRESERVE
#endif


namespace AudioTag
{

const char*  _argv0 = NULL;
bool  _verbose = false;


const char*  _version()
{
    return VERSION;
}

void  _usage()
{
    cout << _argv0 << " " << _version() << endl
         << "usage: " << _argv0 << " [OPTION]... [FILES]\n"
	 << endl
	 << "  [tagging options]\n"
	 << "    -t  --title         <title>\n"
	 << "    -a  --artist        <artist>\n"
	 << "    -R  --album-artist  <album artist>\n"
	 << "    -A  --album         <album<\n"
	 << "    -c  --comment       <comment>\n"
	 << "    -g  --genre         <genre>\n"
	 << "    -y  --date          <year/date>           YYYY[-MM-DD] format - 0000-00-00 unsets\n"
	 << "    -T  --track         <track number [/total tracks]>\n"
	 << "    -D  --disc          <disc/disc total>\n"
	 << "    -s  --rating        <Rating>              0..5 - 0 unsets\n"
	 << "    -P  --properties    <name>:<value>[,<name>:<value>]\n"
	 << "                                              set/remove properties\n"
	 << "                                                -P foo:  delete property call 'foo' (if exists)\n"
	 << "                                                -P foo:bar,coke:cola  add properties: foo=bar and coke=cola\n"
	 << "                                              common properties:\n"
	 << "                                                TITLESORT ALBUMSORT ARTISTSORT ALBUMARTISTSORT\n"
	 << "                                                COMPOSER PERFORMER\n"
	 << "                                                ISRC COPYRIGHT CATALOGNUMBER BARCODE\n"
	 << "        --properties-seperator=:  token to seperate property name/value, default ':'\n"
	 << "    -w  --artwork       <artwork file>[:tag{covr,...}]\n"
	 << '\n'
	 << "  [maintainence options]\n"
	 << "    -l  --list                                list tags\n"
	 << "    -i  --tag           <[1|2|a|f|A]>         add meta to specified tag types\n"
	 << "                                                1 - ID3v1\n"
	 << "                                                2 - ID3v2   (default for mp3)\n"
	 << "                                                a - APE\n"
	 << "                                                f - flac    (default for flac)\n"
	 << "                                                4 - m4a     (default for mp4 audio)\n"
	 << "                                                d - default (use default tag for audio type)\n"
	 << "                                                A - all\n"
	 << "    -d  --delete        <[1|2|a|f|A]>         delete tags (see --tag)\n"
	 << "    -m  --clone-from    <file>                clone dflt tag from file onto dflt tag type of files\n"
	 << "    -n  --clone-tag     <X:Y>                 clone internal tag from X to Y only if X exists\n"
	 << "    -r  --remove-art                          remove album artwork\n"
	 << "    -C                                        clean tags, leaving only basic info\n"
	 << "    -M  --encoding-in   <encoding>            parse current tags and convert from -M <E> -e <E'>\n"
	 << "                                                [warn] will damage tags if you get the encoding wrong!\n"
         << '\n'
	 << "  [out options]\n"
#ifdef HAVE_JSONC
	 << "    -O  --format        <[json,json-c,basic}> output format\n"
#else
	 << "    -O  --format        <[json,basic}>        output format\n"
#endif
	 << "                                              json is very useful for parsing number of files and\n"
	 << "                                              processing with 'jq' to select\n"
	 << "                                                ie.\n"
	 << "                                                  file names only that have ID3 tags\n"
	 << "                                                    " << _argv0 << " ... | jq 'select(.meta[] | .tag_type | . and contains(\"ID3\") ) | .file.name'\n"
	 << "                                                  json objects that have tags other than ID3v2\n"
	 << "                                                    " << _argv0 << " ... | jq 'select(.meta[] | .tag_type != \"ID3v2\")'\n"
	 << "                                              use 'audiotag .... | jq -s.' to generate arrays of objects if required\n"
	 << '\n'
	 << "  [tag encoding options]\n"
	 << "    -e  --encoding-in   <encoding>            encoding= utf16be [latin1|utf8|utf16|utf16be|utf18le]\n"
	 << "    -u  --locale        <locale>]             locale to use for multibyte conversion\n"
	 << '\n'
	 << "  [misc options]\n"
#ifdef AUDIOTAG_HAVE_PRESERVE
	 << "    -p  --preserve-date                       preserve previous modification times" << '\n'
#endif
	 << "    -V  --verbose" << endl;

    exit(1);
}


/* returns the name of the succesful locale
 * if arg is NULL, attempt to use existing locale/utf'd version of it
 * if that fails, attempt the hardcoded fallbacks
 */
const char*  _setlocale(const char*& locale_)
{
    const char*  attempts[] = {
        NULL, // placeholder
        "en_US.UTF-8",
        "en_GB.UTF-8",
        "C.utf8",
        "C.UTF-8"  // debian specific version
    };
    const char*  l = NULL;

    static const char* const  mbdata = "香港hōtiあいうえお";
    const size_t  n = strlen(mbdata)*sizeof(wchar_t);
    wchar_t*  w = (wchar_t*)malloc(n+1);


    std::string  tmp;
    if (locale_ == NULL)
    {
        const char*  env = getenv("LANG");
        l = setlocale(LC_ALL, NULL);
        const std::string  tmpl = l;
        AUDIOTAG_NOTICE_VERBOSE("user default LANG=" << (env ? env : "") << " locale=" << (l ? l : ""));

        memset(w, 0, n+1);
        size_t  res = mbstowcs(w, mbdata, n);
        if (res != (size_t)-1) {
            return l;
        }
    }


    const char*  what = NULL;
    for (unsigned i=0; i<sizeof(attempts)/sizeof(const char*); ++i)
    {
        what = attempts[i];
        if (what == NULL) {
            continue;
        }

        if ( (l = setlocale(LC_ALL, what)) == NULL) {
            AUDIOTAG_WARN("failed setting locale=" << what);
        }
        else
        {
            // validate mb conversion possible with this locale

            memset(w, 0, n+1);
            size_t  res = mbstowcs(w, mbdata, n);
            if (res != (size_t)-1) {
                // all done, found it
                break;
            }
            AUDIOTAG_WARN("failed for requested locale=" << what << "  mb conv res=" << res);
        }
        what = NULL;
    }
    free(w);

    return what;
}


void  _addupdop(AudioTag::OpUpdateTags*&  op_, const AudioTag::MetaTOI& toi_, const AudioTag::Input& input_, AudioTag::Ops&  ops_)
{
    if (op_ == NULL) {
        ops_.add( (op_ = new AudioTag::OpUpdateTags(toi_, input_)) );
    }
}

};


extern int    optind;
extern char*  optarg;


int main(int argc, char *argv[])
{
    AudioTag::_argv0 = basename(argv[0]);

    struct {
        bool  list;
        bool  listP;
        bool  clean;
        bool  mbconvert;
        bool  preserve;
        bool  removeart;

        AudioTag::MetaTOI  toi;

        AudioTag::MetaTOI  from;
        AudioTag::MetaTOI  to;

        AudioTag::Input  iflds;
        AudioTag::OpUpdateTags*  iop;  // not owned

	std::unique_ptr<AudioTag::MetaOut>  mout;

	std::unique_ptr<AudioTag::Artwork>  artwork;

	const char* propertiesTok;

        const char*  locale;
    } opts;
    opts.list = false;
    opts.listP = false;
    opts.clean = false;
    opts.mbconvert = false;
    opts.preserve = false;
    opts.removeart = false;
    opts.mout = std::make_unique<AudioTag::MetaOutJson>();
    opts.locale = NULL;
    opts.iop = NULL;
    opts.propertiesTok = ":";

    /* what we're encoding from */
    TagLib::String::Type  mbenc = TagLib::String::UTF8;


    /* this is encoding for the frames, default it to utf8 */
    TagLib::String::Type  enc = TagLib::String::UTF8;

    AudioTag::Ops  ops;
    // ownership will be xfr'd to 'ops' if non-null
    AudioTag::OpNested*  propops = nullptr;

    // we do this as we dont want to do anything wihtin taglib that deals with mbs
    std::list<char*>  propargs;

    const struct option  long_opts[] = {
	{ "title",		1, 0, 't' },
	{ "artist",		1, 0, 'a' },
	{ "album",	        1, 0, 'A' },
	{ "album-artist",	1, 0, 'R' },
	{ "comment",		1, 0, 'c' },
	{ "genre",		1, 0, 'g' },
	{ "date",	        1, 0, 'y' },
	{ "track",		1, 0, 'T' },
	{ "disc",		1, 0, 'D' },
	{ "rating",		1, 0, 's' },
	{ "artwork",		1, 0, 'w' },

	{ "properties",		1, 0, 'P' },
	{ "properties-seperator",1, 0, 255 },

	{ "list",		0, 0, 'l' },
	{ "tag",		1, 0, 'i' },
	{ "detele",		1, 0, 'd' },
	{ "clone-from",		1, 0, 'm' },
	{ "clone-tag",		1, 0, 'n' },
	{ "remove-art",		0, 0, 'r' },
	{ "clean",		0, 0, 'C' },

	{ "locale",		1, 0, 'u' },
	{ "encoding-in",	1, 0, 'M' },
	{ "encoding-out", 	1, 0, 'e' },

	{ "format",		1, 0, 'O' },
#ifdef AUDIOTAG_HAVE_PRESERVE
	{ "preserve-date",	0, 0, 'p' },
#endif
	{ "verbose",		0, 0, 'V' },

	{ "help", 		0, 0, 'h' },

	{ 0, 0, 0,  0 }
    };
    char  opt_args[1+ sizeof(long_opts)*2] = { 0 };
    {
	char*  og = opt_args;
	const struct option* op = long_opts;
	while (op->name) {
	    *og++ = op->val;
	    if (op->has_arg != no_argument) {
		*og++ = ':';
	    }
	    ++op;
	}
    }

    int  c;
    while ( (c=getopt_long(argc, argv, opt_args, long_opts, NULL)) != -1)
    {
	switch (c) {
	    case 'e':
	    {
		enc = AudioTag::parseEnc(optarg, TagLib::String::UTF8);
	    } break;

	    case 'l':
            {
                ops.add(new AudioTag::OpListTags());
            } break;

	    case 'L':  opts.listP = true;  break;

            case 't':  AudioTag::_addupdop(opts.iop, opts.toi, opts.iflds, ops);  opts.iflds.title = optarg;  break;
            case 'a':  AudioTag::_addupdop(opts.iop, opts.toi, opts.iflds, ops);  opts.iflds.artist = optarg;  break;
            case 'R':  AudioTag::_addupdop(opts.iop, opts.toi, opts.iflds, ops);  opts.iflds.albumartist = optarg;  break;
            case 'A':  AudioTag::_addupdop(opts.iop, opts.toi, opts.iflds, ops);  opts.iflds.album = optarg;  break;
            case 'y':
	    {
		AudioTag::_addupdop(opts.iop, opts.toi, opts.iflds, ops);
		opts.iflds.yr = optarg;

		if (strchr(optarg, '-')) {
		    struct tm  tm;
		    char*  ret = strptime(optarg, "%Y-%m-%d", &tm);
		    if (ret && *ret == '\0') {
			opts.iflds.date = optarg;
		    }
		}
	    } break;
            case 'c':  AudioTag::_addupdop(opts.iop, opts.toi, opts.iflds, ops);  opts.iflds.comment = optarg;  break;
            case 'T':
	    {
	        AudioTag::_addupdop(opts.iop, opts.toi, opts.iflds, ops);
		opts.iflds.trackno = optarg;
		const char*  p = strchr(optarg, '/');
		if (p && *(p+1) != '\0') {
		    opts.iflds.trackN = p+1;
		}
	    } break;
            case 'D':  AudioTag::_addupdop(opts.iop, opts.toi, opts.iflds, ops);  opts.iflds.disc = optarg;  break;
            case 'g':  AudioTag::_addupdop(opts.iop, opts.toi, opts.iflds, ops);  opts.iflds.genre = optarg;  break;
            case 's':  AudioTag::_addupdop(opts.iop, opts.toi, opts.iflds, ops);  opts.iflds.rating = optarg;  break;

            case 'P':
            {
		if (propops == nullptr) {
		    propops = new AudioTag::OpNested();
		    ops.add(propops);
		}
		propargs.push_back(optarg);
	    } break;

	    case 255:  opts.propertiesTok = optarg;  break;

	    case 'w':
	    {
		try
		{
		    std::unique_ptr<AudioTag::Artwork>  ptr(new AudioTag::Artwork(optarg));
		    opts.artwork = std::move(ptr);
		}
		catch (const std::exception& ex)
		{
		    AUDIOTAG_ERR("invalid artwork='" << optarg<< "' - " << ex.what());
		    AudioTag::_usage();
		}
		ops.add(new AudioTag::OpAddArt(*opts.artwork) );
            } break;

            // clone internally (from the same file) from tag X to Y if X exists
            case 'n':
            {
                // x:y
                if (strlen(optarg) == 3 && optarg[1] == ':') {
                    opts.from = optarg[0];
                    opts.to   = optarg[2];
                    if (opts.to && opts.from && opts.to.operator!=(opts.from) && 
                        opts.to.all == false && opts.from.all == false) 
                    {
                        ops.add(new AudioTag::OpCloneIntnlTags(opts.to, opts.from));
                    }
                    else
                    {
                        AUDIOTAG_ERR("cloning from/to must be exclusive tags");
                        AudioTag::_usage();
                    }
               }
            } break;

	    // clone (dflt) tag from file to another file (using their dflt tag)
	    case 'm':
	    {
		AudioTag::File*  f;
		int  err;
		if ( (f = AudioTag::FileFactory::create(optarg, err, *opts.mout)) == NULL) {
		    AUDIOTAG_ERR("cloning from invalid file");
		    AudioTag::_usage();
		}

		ops.add(new AudioTag::OpCloneFileMeta(f));
	    } break;

            case 'i':
            {
                opts.toi = optarg;
            } break;

	    case 'd':
	    {
                ops.add(new AudioTag::OpRemoveTags(AudioTag::MetaTOI(optarg)));
	    } break;

            case 'V':
            {
                AudioTag::_verbose = true;
            } break;

	    case 'C':
            {
                opts.clean = true;
                ops.add(new AudioTag::OpCleanTags());
            } break;

	    case 'M':
	    {
		opts.mbconvert = true;
		mbenc = AudioTag::parseEnc(optarg, TagLib::String::Latin1);
	    } break;

            case 'r':
            {
                opts.removeart = true;
                ops.add(new AudioTag::OpRemoveArt());
            } break;

#ifdef AUDIOTAG_HAVE_PRESERVE
            case 'p':
                opts.preserve = true;
                break;
#endif

            case 'O':
	    {
		std::unique_ptr<AudioTag::MetaOut>  ptr(AudioTag::MetaOut::create(optarg));
                opts.mout = std::move(ptr);
	    } break;

            case 'u':
                opts.locale = optarg;
                break;

	    case 'v':
	        cout << AudioTag::_argv0 << " " << AudioTag::_version() << endl;
		return 0;
		break;

	    case 'h':
            default: AudioTag::_usage();
	}
    }

    if ( !(optind < argc) ) {
        AUDIOTAG_ERR("no files specified");
        AudioTag::_usage();
    }
    if ( !opts.iflds.validate()) {
        AUDIOTAG_ERR("invalid options");
        AudioTag::_usage();
    }

    const char*  l;
    if ( (l = AudioTag::_setlocale(opts.locale)) == NULL) {
        AUDIOTAG_ERR("failed to set valid UTF8 locale - tried all fallbacks; verify locales (locale -a)");
        return -1;
    }
    if (opts.locale && l != opts.locale) {
        AUDIOTAG_WARN("requested locale=" << opts.locale << " not suitable for mbyte conversions, using local=" << l);
    }
    AUDIOTAG_NOTICE_VERBOSE("using locale=" << l);

    std::for_each(propargs.cbegin(), propargs.cend(), [&ops, &opts, &propops](char* optarg) {
	AudioTag::OpPropertyTags::Map  m;

	char*  pc = NULL;
	char*  tok = NULL;
	while ( (tok = strtok_r(pc == NULL ? optarg : NULL, ",", &pc)) )
	{
	    const uint8_t  tn = strlen(tok);
	    char*  prop;
	    if ( (prop = strtok(tok, opts.propertiesTok)) == NULL) {
		AUDIOTAG_ERR("invalid arg='" << tok  << "property list format: <property name><seperator (" << opts.propertiesTok<< ")><property value>");
		AudioTag::_usage();
	    }

	    const uint8_t  n = strlen(prop);
	    if (n == tn) {
		AUDIOTAG_ERR("invalid token='" << tok << "' - no value");
		AudioTag::_usage();
	    }
	    prop[n] = NULL;
	    const char*  value = prop + n+1;

	    m.insert(std::make_pair(prop, value));
	}
	propops->_ops.emplace_back(new AudioTag::OpPropertyTags(opts.toi, opts.iflds, m) );
    });


    if (ops.empty()) {
        ops.add(new AudioTag::OpListTags());
    }

    opts.iflds.strip();


    const char*  f;

    int  i = optind;
    while (i < argc)
    {
	f = argv[i++];

        AudioTag::File*  ff = NULL;

        int  err;
        try
        {
            if ( !(ff = AudioTag::FileFactory::create(f, err, *opts.mout))) {
                AUDIOTAG_WARN("failed to open - " << f << " - " << AudioTag::FileFactory::what(err) << " - ignored");
            }
        }
        catch (const std::exception& ex)
        {
            AUDIOTAG_WARN("failed to open - " << f << " - " << ex.what() << " - ignored");
        }


        if (ff == NULL) {
            continue;
        }

        if ( !ops.readonly() && access(ff->taglibfile().name(), W_OK) < 0) {
            AUDIOTAG_ERR(ff->taglibfile().name() << ": no write permissions, all operations skipped");
        }
        else
        {
            // execute in order they appeared on command line

            ops.execute(*ff);

#ifdef AUDIOTAG_HAVE_PRESERVE
	    // macos doesnt have st_mtim
            if (opts.preserve)
            {
                /* try to reset the timestamps on the file
                 */
                struct timeval  tv[2];
                memset(&tv[0], 0, sizeof(struct timeval));
                tv[1].tv_sec  = ff->st().st_mtim.tv_sec;
                tv[1].tv_usec = ff->st().st_mtim.tv_nsec/1000;

                if ( utimes(f, tv) < 0) {
                    AUDIOTAG_WARN_VERBOSE("'" << f << "' unable to revert to original access times - " << strerror(errno));
                }
            }
#endif
        }
        delete ff;
    }
    return 0;
}

