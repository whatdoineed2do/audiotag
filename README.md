# audiotag
multi audio file meta tagging util

Command line utility based on TagLib (v1.11.1) to read/write basic meta information to the follow file formats:
. MP3
. falc
. M4a (.m4a/.aac MP4 audio)

Basic functionality to:
. clean - strip all tags except for very basic artist/title/genre etc
. sync - copy existing tag (ie ID3v2) to another tag (ie APE) for MP3
. delete - delete tags from file

Based heavily on the tagwritter util from TabLib and previous mp3tag
