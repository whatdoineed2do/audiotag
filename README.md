# audiotag
Command line multi audio format meta tagging util

Command line utility based on TagLib (v1.11.1) to read/write basic meta information to the follow file formats:
. MP3
. falc
. M4a (.m4a/.aac MP4 audio)

Basic functionality to:
. unified interface across all supported formats: goodbye to different tag names im metaflac vs id3tag etc fir simple tagging
. clean - strip all tags except for very basic artist/title/genre etc
. sync - copy existing tag (ie ID3v2) to another tag (ie APE) for MP3
. delete - delete tags from file
. UTF8 tag data supported in tags which support it (NOT id3v1)

Based heavily on the tagwritter util from TabLib and previous mp3tag
