# audiotag
Command line multi audio format meta tagging util

Command line utility based on `TagLib (v1.11.1)` to read/write basic meta information to the follow file formats:
- MP3
- flac
- M4a (.m4a/.aac MP4 audio)

Basic functionality to:
- unified interface across all supported formats: goodbye to different tag names im metaflac vs id3tag etc fir simple tagging
- clean - strip all tags except for very basic artist/title/genre etc
- sync - copy existing tag (ie ID3v2) to another tag (ie APE) for MP3
- delete - delete tags from file
- UTF8 tag data supported in tags which support it (NOT id3v1)

Based heavily on the `tagwritter` util from `TabLib` and previous mp3tag


Multiple commands can be chained together in a single execution - however the file is only written to disk once.  The output can be of two useful forms:
- mock JSON
- old id3tag output

## Example Usage
Add/change the `artist`, clear the `comment` and display the meta
```
$ audiotag -a "foo bar" -c "" -O json -l *.mp3
04 foo.mp3:	[ID3v1]
{
  "file": {
    "name": "04 foo.mp3",
    "meta": {
      "tag": "ID3v1",
      "data": {
        "Artist": "foo bar",
        "Title": "",
        "Album": "¥Ï¥ë¥«",
        "Track": "4",
        "Yr": null,
        "Genre": "Rock",
        "Comment": "",
        "Artwork": "no"
      }
    }
  }
}
04 foo.mp3:	[  APE]
{
  "file": {
    "name": "04 foo.mp3",
    "meta": {
      "tag": "APE",
      "data": {
        "Artist": "foo bar",
        "Title": "",
        "Album": "",
        "Track": "4",
        "Yr": null,
        "Genre": "Rock",
        "Comment": "",
        "Artwork": "no"
      }
    }
  }
}

```
## Cloning Tags
With the example above, we can see there is an `ID3v1` and `APE` tag but no `ID3v2` tag - we can clone the `APE` tag onto the `ID3v2` whilst deleting the `ID3v1` tag (it can't hold unicode text anyway, as seen in the album tag) in one command:
```
$ audiotag -d 1 -n "a:2" -A "ハルカ" -O json -l foo.mp3 
foo.mp3:	[ID3v2]
{
  "file": {
    "name": "foo.mp3",
    "meta": {
      "tag": "ID3v2",
      "data": {
        "Artist": "foo bar",
        "Title": "",
        "Album": "ハルカ",
        "Track": "4",
        "Yr": null,
        "Genre": "Rock",
        "Comment": "",
        "Artwork": "no"
      }
    }
  }
}
foo.mp3:	[  APE]
{
  "file": {
    "name": "foo.mp3",
    "meta": {
      "tag": "APE",
      "data": {
        "Artist": "foo bar",
        "Title": "",
        "Album": "ハルカ",
        "Track": "4",
        "Yr": null,
        "Genre": "Rock",
        "Comment": "",
        "Artwork": "no"
      }
    }
  }
}

```
# Further Enhancements
Other file formats supported by `Taglib` can be added:  this util relies on a `File` object to represent the underlying file and a corresponding `Meta` object that is directly coupled to handle the retrieval and updating of metadata.  Any new objects will have to be _registered_ with the `FileFactory`.

Note that the `Taglib` interface provides a `Tag` object that will be sufficient in most instances for the basic tagging operations.
