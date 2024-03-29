# audiotag
Command line multi audio format meta tagging util

Command line utility based on `TagLib (v1.11.1)` to read/write basic meta information to the following common audio file formats:
- `MP3`
- `flac`
- `M4a`

Basic functionality to:
- unified interface across all supported formats: goodbye to different tag names in [`metaflac`](https://xiph.org/flac/documentation_tools_metaflac.html) vs [`id3tag`](https://man.archlinux.org/man/extra/id3lib/id3tag.1.en) vs [`ffmpeg`](https://ffmpeg.org/ffmpeg-all.html#Metadata-1) etc for simple tagging
- clean - strip all tags except for very basic artist/title/genre etc
- sync - copy existing tags from other files or within same file, ie `APE` to `ID3v2` on `MP3`
- delete - delete tags from file
- UTF8 tag data supported in tags which support it (NOT in `ID3v1`)

Using `ffmpeg/libavfilter` will allow for more accurate file determination based on actual file container type, rather than file extension.

Based heavily on the `tagwritter` util from `TabLib`.


Multiple commands can be chained together in a single execution - however the file is only written to disk once.  The output can be of two useful forms:
- mock `JSON`
- old `id3tag` output

## Example Usage
Drop the `ID3v2` tag, add/change the `artist`, clear the `comment` and display the meta
```
$ audiotag -d2 -a "foo bar" -c "" -O json -l test.mp3
{
  "file": {
    "name": "test.mp3",
    "size": 59938,
    "mod_time": "Thu 22 Jun 2023 07:05:06 PM BST",
    "audio_properties": {
      "length": 3.631,
      "bitrate": 129,
      "samplerate": 44100,
      "channels": 2,
      "hash": "223286320b511145cd4da6a789348bb9ae704f724e413ff950d3eae397f36820"
    }
  },
  "meta": [
    {
      "tag_type": "ID3v1",
      "artist": "foo",
      "title": "Japan Nippon",
      "album": "audiotag test files",
      "track": 2,
      "year": 2007,
      "genre": "Pop",
      "comment": "this is a comment",
      "artwork": false,
      "properties": {
        "ALBUM": [ "audiotag test files" ],
        "ARTIST": [ "foo" ],
        "COMMENT": [ "this is a comment" ],
        "DATE": [ "2007" ],
        "GENRE": [ "Pop" ],
        "TITLE": [ "Japan Nippon" ],
        "TRACKNUMBER": [ "2" ]
      }
    },
    {
      "tag_type": "APE",
      "artist": "foo 香港 Hong Kong",
      "title": "Japan 日本国 Nippon",
      "album": "audiotag test files",
      "track": 2,
      "year": 1999,
      "genre": "Pop",
      "comment": null,
      "artwork": false,
      "properties": {
        "ALBUM": [ "audiotag test files" ],
        "ARTIST": [ "foo 香港 Hong Kong" ],
        "DATE": [ "1999" ],
        "GENRE": [ "Pop" ],
        "TITLE": [ "Japan 日本国 Nippon" ],
        "TRACKNUMBER": [ "2" ]
      }
    }
  ]
}
```
## Cloning Tags
With the example above, we can see there is an `ID3v1` and `APE` tag but no `ID3v2` tag - we can clone the `APE` tag onto the `ID3v2` whilst deleting the `ID3v1` tag (it can't hold unicode text anyway, as seen in the album tag) in one command:
```
$ audiotag -d 1 -n "a:2" -A "music from 香港 by foo" -O json -l test.mp3 
{
  "file": {
    "name": "test.mp3",
    "size": 59938,
    "mod_time": "Thu 22 Jun 2023 07:05:06 PM BST",
    "audio_properties": {
      "length": 3.631,
      "bitrate": 129,
      "samplerate": 44100,
      "channels": 2,
      "hash": "223286320b511145cd4da6a789348bb9ae704f724e413ff950d3eae397f36820"
    }
  },
  "meta": [
    {
      "tag_type": "ID3v2",
      "artist": "foo 香港 Hong Kong",
      "title": "Japan 日本国 Nippon",
      "album": "music from 香港 by foo",
      "track": 2,
      "year": 1999,
      "genre": "Pop",
      "comment": "id3v2 comment",
      "artwork": false,
      "properties": {
        "ALBUM": [ "music from 香港 by foo" ],
        "ARTIST": [ "foo 香港 Hong Kong" ],
        "COMMENT": [ "id3v2 comment" ],
        "DATE": [ "1999" ],
        "GENRE": [ "Pop" ],
        "RELEASEDATE": [ "1999-01-01" ],
        "TITLE": [ "Japan 日本国 Nippon" ],
        "TRACKNUMBER": [ "2" ]
      }
    },
    {
      "tag_type": "APE",
      "artist": "foo 香港 Hong Kong",
      "title": "Japan 日本国 Nippon",
      "album": "audiotag test files",
      "track": 2,
      "year": 1999,
      "genre": "Pop",
      "comment": null,
      "artwork": false,
      "properties": {
        "ALBUM": [ "audiotag test files" ],
        "ARTIST": [ "foo 香港 Hong Kong" ],
        "DATE": [ "1999" ],
        "GENRE": [ "Pop" ],
        "TITLE": [ "Japan 日本国 Nippon" ],
        "TRACKNUMBER": [ "2" ]
      }
    }
  ]
}
```
## Working with arbituary tags
`TagLib` provides a uniform set of property names across all tag formats which are shown in `json` output.  The `-P` flag can add/remove property tags but the user is responsible for verifying if such a property is valid for that tag type.
```
$ audiotag  test.mp3
{
  "file": {
    "name": "test.mp3",
    "size": 59938,
    "mod_time": "Sat 24 Jun 2023 09:17:08 AM BST",
    "audio_properties": {
      "length": 3.631,
      "bitrate": 129,
      "samplerate": 44100,
      "channels": 2,
      "hash": "223286320b511145cd4da6a789348bb9ae704f724e413ff950d3eae397f36820"
    }
  },
  "meta": [
    {
      "tag_type": "ID3v2",
      "artist": "foo 香港 Hong Kong",
      "title": "Japan 日本国 Nippon",
      "album": "audiotag test files",
      "track": 2,
      "year": 1999,
      "genre": "Pop",
      "comment": "id3v2 comment",
      "artwork": false,
      "properties": {
        "ALBUM": [ "audiotag test files" ],
        "ARTIST": [ "foo 香港 Hong Kong" ],
        "COMMENT": [ "id3v2 comment" ],
        "DATE": [ "1999" ],
        "GENRE": [ "Pop" ],
        "RELEASEDATE": [ "1999-12-25" ],
        "TITLE": [ "Japan 日本国 Nippon" ],
        "TRACKNUMBER": [ "2" ]
      }
    }
  ]
}
```
### Adding Properties
```
$ audiotag -P foo:bar,coke:cola -l test.mp3
{
  "file": {
    "name": "test.mp3",
    "size": 59938,
    "mod_time": "Sat 24 Jun 2023 09:17:44 AM BST",
    "audio_properties": {
      "length": 3.631,
      "bitrate": 129,
      "samplerate": 44100,
      "channels": 2,
      "hash": "223286320b511145cd4da6a789348bb9ae704f724e413ff950d3eae397f36820"
    }
  },
  "meta": [
    {
      "tag_type": "ID3v2",
      "artist": "foo 香港 Hong Kong",
      "title": "Japan 日本国 Nippon",
      "album": "audiotag test files",
      "track": 2,
      "year": 1999,
      "genre": "Pop",
      "comment": "id3v2 comment",
      "artwork": false,
      "properties": {
        "ALBUM": [ "audiotag test files" ],
        "ARTIST": [ "foo 香港 Hong Kong" ],
        "COKE": [ "cola" ],
        "COMMENT": [ "id3v2 comment" ],
        "DATE": [ "1999" ],
        "FOO": [ "bar" ],
        "GENRE": [ "Pop" ],
        "RELEASEDATE": [ "1999-12-25" ],
        "TITLE": [ "Japan 日本国 Nippon" ],
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
    "size": 59938,
    "mod_time": "Sat 24 Jun 2023 09:18:01 AM BST",
    "audio_properties": {
      "length": 3.631,
      "bitrate": 129,
      "samplerate": 44100,
      "channels": 2,
      "hash": "223286320b511145cd4da6a789348bb9ae704f724e413ff950d3eae397f36820"
    }
  },
  "meta": [
    {
      "tag_type": "ID3v2",
      "artist": "foo 香港 Hong Kong",
      "title": "Japan 日本国 Nippon",
      "album": "audiotag test files",
      "track": 2,
      "year": 1999,
      "genre": "Pop",
      "comment": "an updated comment",
      "artwork": false,
      "properties": {
        "ALBUM": [ "audiotag test files" ],
        "ARTIST": [ "foo 香港 Hong Kong" ],
        "COMMENT": [ "an updated comment" ],
        "DATE": [ "1999" ],
        "FOO": [ "bar" ],
        "GENRE": [ "Pop" ],
        "RELEASEDATE": [ "1999-12-25" ],
        "TITLE": [ "Japan 日本国 Nippon" ],
        "TRACKNUMBER": [ "2" ]
      }
    }
  ]
}
```
## Adding artwork, Track and Disc number
```
$ audiotag -w foo.jpg  -y 1955-11-05  -T 1/10  -D 1/3  track1of10-on-disc1of3.mp3
```
 
# Further Enhancements
Other file formats supported by `Taglib` can be added:  this util relies on a `File` object to represent the underlying file and a corresponding `Meta` object that is directly coupled to handle the retrieval and updating of metadata.  Any new objects will have to be _registered_ with the `FileFactory`.

Note that the `Taglib` interface provides a `Tag` object that will be sufficient in most instances for the basic tagging operations.
