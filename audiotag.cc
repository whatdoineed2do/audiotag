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
#include "audiotagfile.h"
#include "audiotagmeta.h"
#include "audiotagmetaout.h"

using namespace std;


const char*  _argv0 = NULL;
bool  _verbose = false;

const TagLib::String::Type  INTNL_STR_ENC = TagLib::String::UTF16BE;

void  _usage()
{
    cout << "usage: " << _argv0 << " [OPTION]... [mp3 FILES]" << endl
	 << endl
	 << "  [tag encoding options]" << endl
	 << "       -e  encoding  = utf16be [latin1|utf8|utf16|utf16be|utf18le]" << endl
	 << endl
	 << "  [tagging options]" << endl
	 << "       -t  title" << endl
	 << "       -a  artist" << endl
	 << "       -A  album" << endl
	 << "       -c  comment" << endl
	 << "       -g  genre" << endl
	 << "       -y  year" << endl
	 << "       -T  track" << endl
	 << "       -1             add ID3v1 tag" << endl
	 << "       -2             add ID3v2 tag" << endl
	 << endl
	 << "  [maintainence options]" << endl
	 << "       -l             list tags (exclusive maintanence option" << endl
	 << "       -i [1|2|a|f|A] add tags" << endl
	 << "       -d [1|2|a|f|A] delete tags" << endl
	 << "       -C             clean tags, leaving only basic info" << endl
	 << "       -M encoding    parse current tags and convert from -M <E> -e <E'>" << endl
	 << "               [warn] will damage tags if you get the E wrong!" << endl
	 << endl
	 << "  [misc options]" << endl
	 << "       -V             verbose" << endl;

    exit(1);
}


extern int    optind;
extern char*  optarg;


int main(int argc, char *argv[])
{
    _argv0 = basename(argv[0]);

    struct {
        bool  list;
        bool  listP;
        bool  clean;
        bool  mbconvert;
        bool  preserve;

        AudioTag::MetaTOI  remove;
        AudioTag::MetaTOI  toi;

        bool  sync;
        AudioTag::MetaTOI  from;
        AudioTag::MetaTOI  to;


        AudioTag::Input  iflds;

        AudioTag::MetaOut*  mout;
    } opts;
    opts.list = false;
    opts.listP = false;
    opts.clean = false;
    opts.mbconvert = false;
    opts.preserve = false;
    opts.sync = false;
    opts.mout = NULL;

    /* what we're encoding from */
    TagLib::String::Type  mbenc = TagLib::String::UTF8;


    /* this is encoding for the frames, default it to utf8 */
    TagLib::String::Type  enc = TagLib::String::UTF8;

    const char*  l;
    if ( (l = setlocale(LC_ALL, "en_US.UTF-8")) == NULL) {
        MP3_TAG_NOTICE_VERBOSE("locale set");
    }

    int c;
    while ( (c = getopt(argc, argv, "e:12hla:pt:A:y:c:T:g:Dd:n:VM:Ci:O:")) != EOF)
    {
	switch (c) {
	    case 'e':
	    {
		enc = AudioTag::parseEnc(optarg, TagLib::String::UTF8);
	    } break;

	    case 'l':  opts.list = true;  break;
	    case 'L':  opts.listP = true;  break;

	    case 't':  opts.iflds.title = optarg;  break;
	    case 'a':  opts.iflds.artist = optarg;  break;
	    case 'A':  opts.iflds.album = optarg;  break;
	    case 'y':  opts.iflds.yr = optarg;  break;
	    case 'c':  opts.iflds.comment = optarg;  break;
	    case 'T':  opts.iflds.trackno = optarg;  break;
	    case 'g':  opts.iflds.genre = optarg;  break;

            // clone from tag X to Y if X exists
            case 'n':
            {
                // x:y
                if (strlen(optarg) == 3 && optarg[1] == ':') {
                    opts.from = optarg[0];
                    opts.to   = optarg[2];
                }
            } break;

            case 'i':
            {
                opts.toi = optarg;
            } break;

	    case 'd':
	    {
                opts.remove = optarg;
	    } break;

	    case 'V':  _verbose = true;  break;

	    case 'C':  opts.clean = true;  break;
	    case 'M':
	    {
		opts.mbconvert = true;
		mbenc = AudioTag::parseEnc(optarg, TagLib::String::Latin1);
	    } break;

            case 'p':
                opts.preserve = true;
                break;

            case 'O':
                opts.mout = AudioTag::MetaOut::create(optarg);
                break;

	    case 'h':
	    default: _usage();
	}
    }

    if ( !(optind < argc) ) {
        MP3_TAG_ERR("no files specified");
	_usage();
    }
    if (opts.to && opts.from && opts.to.operator!=(opts.from) && 
        opts.to.all == false && opts.from.all == false) {
        opts.sync = true;
    }

    if ( (!opts.list && !opts.iflds && !opts.remove && !opts.clean && !opts.mbconvert && !opts.sync) || (opts.iflds && opts.mbconvert)) {
        opts.list = true;
    }
    opts.iflds.strip();

    if (opts.mout == NULL) {
        opts.mout = new AudioTag::MetaOut();
    }


    const char*  f;

    int  i = optind;
    while (i < argc)
    {
	f = argv[i++];

        AudioTag::File*  ff = NULL;
	TagLib::Tag*  tag = NULL;

        int  err;
        try
        {
            if ( !(ff = AudioTag::FileFactory::create(f, err, *opts.mout))) {
                MP3_TAG_WARN("failed to open - " << f << " - " << AudioTag::FileFactory::what(err) << " - ignored");
            }
        }
        catch (const std::exception& ex)
        {
            MP3_TAG_WARN("failed to open - " << f << " - " << ex.what() << " - ignored");
        }


        if (ff == NULL) {
            continue;
        }


	/* listing takes precidence above the mod actions
	 */
        if (opts.list)
        {
            cout << *ff;

#if 0
            if (opts.listp) 
            {
                const Meta::Tags  tags = ff->meta().tags();
                for (Meta::Tags::const_iterator i=tags.begin(); i!=tags.end(); ++i) {
                    const TagLib::PropertyMap  pm = (*i)->properties();
                }
            }
#endif
        }

        bool  mod = false;

        // remove tags
        if (opts.remove) {
            ff->meta().remove(opts.remove);
            mod = true;
        }

        // clean, only leaving basic tags
        if (opts.clean) {
            ff->meta().sanitize();
            mod = true;
        }

        // multi-byte convert
        if (opts.mbconvert) {
            // TODO
            mod = true;
        }

        // sync from one tag to another
        if (opts.sync) {
            ff->meta().clone(opts.to, opts.from);
            mod = true;
        }

        // write new tag info
        if (opts.iflds)
        {
            if (!opts.toi) {
                // nothing spec, use the default
                ff->meta() = opts.iflds;
            }
            else {
                ff->meta().assign(opts.toi, opts.iflds);
            }

            mod = true;
        }

        if (mod)
        {
            ff->meta().save();

            if (opts.preserve)
            {
                /* try to reset the timestamps on the file
                 */
                struct utimbuf  ub;
                ub.actime = ff->st().st_atime;
                ub.modtime =  ff->st().st_mtime;

                struct timeval  tv[2];
                memset(&tv[0], 0, sizeof(struct timeval));
                tv[1].tv_sec  = ff->st().st_mtim.tv_sec;
                tv[1].tv_usec = ff->st().st_mtim.tv_nsec/1000;

                //if ( utime(f, &ub) < 0) {
                if ( utimes(f, tv) < 0) {
                    MP3_TAG_WARN_VERBOSE("'" << f << "' unable to revert to original access times - " << strerror(errno));
                }
            }
        }

        delete ff;
        delete opts.mout;
    }
    return 0;
}
