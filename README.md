# audiotag
Command line multi audio format meta tagging util

Command line utility based on `TagLib (v1.11.1)` to read/write basic meta information to the follow file formats:
- MP3
- flac
- M4a (.m4a MP4 audio)

Basic functionality to:
- unified interface across all supported formats: goodbye to different tag names im metaflac vs id3tag etc fir simple tagging
- clean - strip all tags except for very basic artist/title/genre etc
- sync - copy existing tag (ie ID3v2) to another tag (ie APE) for MP3
- delete - delete tags from file
- UTF8 tag data supported in tags which support it (NOT id3v1)

`ffmpeg/libavfilter` will allow for more accurate file determination based on actual file container type, rather than file extension.

Based heavily on the `tagwritter` util from `TabLib` and previous mp3tag


Multiple commands can be chained together in a single execution - however the file is only written to disk once.  The output can be of two useful forms:
- mock JSON
- old id3tag output

## Example Usage
Add/change the `artist`, clear the `comment` and display the meta
```
$ audiotag -a "foo bar" -c "" -O json -l test.mp3
{
  "file": {
    "name": "test.mp3",
    "size": 58813,
    "mod_time": "Thu 05 Jul 2018 08:51:07 AM BST"
  },
  "meta": [
    {
      "tag_type": "ID3v1",
      "artist": "ONEartist",
      "title": "ONEtitle",
      "album": null,
      "track": 2,
      "year": 2007,
      "genre": "Pop",
      "comment": "this is a comment",
      "artwork": false,
      "properties": {
        "ARTIST": [ "ONEartist" ],
        "COMMENT": [ "this is a comment" ],
        "DATE": [ "2007" ],
        "GENRE": [ "Pop" ],
        "TITLE": [ "ONEtitle" ],
        "TRACKNUMBER": [ "2" ]
      }
    },
    {
      "tag_type": "APE",
      "artist": "VER2 foo 群星bar",
      "title": "yep星 works",
      "album": null,
      "track": 2,
      "year": 1999,
      "genre": "Pop",
      "comment": null,
      "artwork": false,
      "properties": {
        "ARTIST": [ "VER2 foo 群星bar" ],
        "DATE": [ "1999" ],
        "GENRE": [ "Pop" ],
        "TITLE": [ "yep星 works" ],
        "TRACKNUMBER": [ "2" ]
      }
    }
  ]
}
```
## Cloning Tags
With the example above, we can see there is an `ID3v1` and `APE` tag but no `ID3v2` tag - we can clone the `APE` tag onto the `ID3v2` whilst deleting the `ID3v1` tag (it can't hold unicode text anyway, as seen in the album tag) in one command:
```
$ audiotag -d 1 -n "a:2" -A "ハルカ" -O json -l test.mp3 
{
  "file": {
    "name": "test.mp3",
    "size": 58813,
    "mod_time": "Thu 05 Jul 2018 08:51:21 AM BST"
  },
  "meta": [
    {
      "tag_type": "ID3v2",
      "artist": "VER2 foo 群星bar",
      "title": "yep星 works",
      "album": "ハルカ",
      "track": 2,
      "year": 1999,
      "genre": "Pop",
      "comment": null,
      "artwork": false,
      "properties": {
        "ALBUM": [ "ハルカ" ],
        "ARTIST": [ "VER2 foo 群星bar" ],
        "DATE": [ "1999" ],
        "GENRE": [ "Pop" ],
        "TITLE": [ "yep星 works" ],
        "TRACKNUMBER": [ "2" ]
      }
    },
    {
      "tag_type": "APE",
      "artist": "VER2 foo 群星bar",
      "title": "yep星 works",
      "album": null,
      "track": 2,
      "year": 1999,
      "genre": "Pop",
      "comment": null,
      "artwork": false,
      "properties": {
        "ARTIST": [ "VER2 foo 群星bar" ],
        "DATE": [ "1999" ],
        "GENRE": [ "Pop" ],
        "TITLE": [ "yep星 works" ],
        "TRACKNUMBER": [ "2" ]
      }
    }
  ]
}
```
## Working with arbituary tags
`TagLib` provides a uniform set of property names across all tag formats which are shown in json output.  The `-P` flag can add/remove property tags but the user is responsible for verifying if such a property is valid for that tag type.
```
$ audiotag  -l test.mp3
{
  "file": {
    "name": "test.mp3",
    "size": 59721,
    "mod_time": "Sun 10 Jun 2018 05:46:22 PM BST"
  },
  "meta": [
    {
      "tag_type": "ID3v2",
      "artist": "VER2 foo 群星bar",
      "title": "yep星 works",
      "album": null,
      "track": 2,
      "year": 1999,
      "genre": "Pop",
      "comment": "id3v2 comment",
      "artwork": "false",
      "properties": {
        "ARTIST": [ "VER2 foo 群星bar" ],
        "COMMENT": [ "id3v2 comment" ],
        "DATE": [ "1999" ],
        "GENRE": [ "Pop" ],
        "TITLE": [ "yep星 works" ],
        "TRACKNUMBER": [ "2" ]
      }
    }
  ]
}
```
### Adding Properties
```
$ audiotag  -P foo:bar,coke:cola -l test.mp3
{
  "file": {
    "name": "test.mp3",
    "size": 59721,
    "mod_time": "Sun 10 Jun 2018 05:46:22 PM BST"
  },
  "meta": [
    {
      "tag_type": "ID3v2",
      "artist": "VER2 foo 群星bar",
      "title": "yep星 works",
      "album": null.
      "track": 2,
      "year": 1999,
      "genre": "Pop",
      "comment": "id3v2 comment",
      "artwork": false,
      "properties": {
        "ARTIST": [ "VER2 foo 群星bar" ],
        "COKE": [ "cola" ],
        "COMMENT": [ "id3v2 comment" ],
        "DATE": [ "1999" ],
        "FOO": [ "bar" ],
        "GENRE": [ "Pop" ],
        "TITLE": [ "yep星 works" ],
        "TRACKNUMBER": [ "2" ]
      }
    }
  ]
}
```
### Removing Property value and resassign
```
$ audiotag  -P coke:,COMMENT:"an updated comment" -l test.mp3
{
  "file": {
    "name": "test.mp3",
    "size": 59721,
    "mod_time": "Wed 04 Jul 2018 06:41:53 PM BST"
  },
  "meta": [
    {
      "tag_type": "ID3v2",
      "artist": "VER2 foo 群星bar",
      "title": "yep星 works",
      "album": null,
      "track": 2,
      "year": 1999,
      "genre": "Pop",
      "comment": "an updated comment",
      "artwork": "false",
      "properties": {
        "ARTIST": [ "VER2 foo 群星bar" ],
        "COMMENT": [ "an updated comment" ],
        "DATE": [ "1999" ],
        "FOO": [ "bar" ],
        "GENRE": [ "Pop" ],
        "TITLE": [ "yep星 works" ],
        "TRACKNUMBER": [ "2" ]
      }
    }
  ]
}
```
## Adding artwork/Disc number
```
$ audiotag -w foo.jpg -K 1/3 disc1track1.mp3
```
 
# Further Enhancements
Other file formats supported by `Taglib` can be added:  this util relies on a `File` object to represent the underlying file and a corresponding `Meta` object that is directly coupled to handle the retrieval and updating of metadata.  Any new objects will have to be _registered_ with the `FileFactory`.

Note that the `Taglib` interface provides a `Tag` object that will be sufficient in most instances for the basic tagging operations.
