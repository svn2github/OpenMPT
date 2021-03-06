#pragma once
// ID3v2.4 / etc. tagging class (for mp3 / wav / etc. support)

#include <string>

using std::string;

#pragma pack(1)

///////////////////////////////////////////////////////////////////////////////////////////////////
// ID3v1 Genres

#define NUM_GENRES		148

static LPCSTR gpszGenreNames[NUM_GENRES] =
{
	"Blues",				"Classic Rock",			"Country",			"Dance",
	"Disco",				"Funk",					"Grunge",			"Hip Hop",
	"Jazz",					"Metal",				"New_Age",			"Oldies",
	"Other",				"Pop",					"Rhythm n Blues",	"Rap",
	"Reggae",				"Rock",					"Techno",			"Industrial",
	"Alternative",			"Ska",					"Death Metal",		"Pranks",
	"Soundtrack",			"Euro Techno",			"Ambient",			"Trip_Hop",
	"Vocal",				"Jazz Funk",			"Fusion",			"Trance",
	"Classical",			"Instrumental",			"Acid",				"House",
	"Game",					"Sound Clip",			"Gospel",			"Noise",
	"Alternative Rock",		"Bass",					"Soul",				"Punk",
	"Space",				"Meditative",			"Instrumental Pop",	"Instrumental Rock",
	"Ethnic",				"Gothic",				"Darkwave",			"Techno Industrial",
	"Electronic",			"Pop Folk",				"Eurodance",		"Dream",
	"Southern Rock",		"Comedy",				"Cult",				"Gangsta",
	"Top 40",				"Christian Rap",		"Pop Funk",			"Jungle",
	"Native_American",		"Cabaret",				"New_Wave",			"Psychadelic",
	"Rave",					"ShowTunes",			"Trailer",			"Lo Fi",
	"Tribal",				"Acid Punk",			"Acid Jazz",		"Polka",
	"Retro",				"Musical",				"Rock n Roll",		"Hard_Rock",
	"Folk",					"Folk Rock",			"National Folk",	"Swing",
	"Fast Fusion",			"Bebob",				"Latin",			"Revival",
	"Celtic",				"Bluegrass",			"Avantgarde",		"Gothic Rock",
	"Progressive Rock",		"Psychedelic Rock",		"Symphonic Rock",	"Slow Rock",
	"Big Band",				"Chorus",				"Easy Listening",	"Acoustic",
	"Humour",				"Speech",				"Chanson",			"Opera",
	"Chamber Music",		"Sonata",				"Symphony",			"Booty_Bass",
	"Primus",				"Porn Groove",			"Satire",			"Slow Jam",
	"Club",					"Tango",				"Samba",			"Folklore",
	"Ballad",				"Power Ballad",			"Rhytmic Soul",		"Freestyle",
	"Duet",					"Punk Rock",			"Drum Solo",		"Acapella",
	"Euro House",			"Dance Hall",			"Goa",				"Drum n Bass",
	"Club-House",			"Hardcore",				"Terror",			"Indie",
	"BritPop",				"Negerpunk",			"Polsk Punk",		"Beat",
	"Christian Gangsta",	"Heavy Metal",			"Black Metal",		"Crossover",
	"Contemporary Christian","Christian Rock",		"Merengue",			"Salsa",
	"Thrash Metal",			"Anime",				"JPop",				"SynthPop",
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// ID3v2.4 Tags

typedef struct _TAGID3v2HEADER
{
	BYTE signature[3];
	BYTE version[2];
	BYTE flags;
	UINT32 size;
	// Total: 10 bytes
} TAGID3v2HEADER;

typedef struct _TAGID3v2FRAME
{
	UINT32 frameid;
	UINT32 size;
	UINT16 flags;
	// Total: 10 bytes
} TAGID3v2FRAME;

// we will add some padding bytes to our id3v2 tag (extending tags will be easier this way)
#define ID3v2_PADDING 512

// charset... choose text ending accordingly.
// $00 = ISO-8859-1. Terminated with $00.
// $01 = UTF-16. Terminated with $00 00.
// $02 = UTF-16BE. Terminated with $00 00.
// $03 = UTF-8. Terminated with $00.
#ifdef UNICODE
#define ID3v2_CHARSET '\3'
#define ID3v2_TEXTENDING '\0'
#else
#define ID3v2_CHARSET '\0'
#define ID3v2_TEXTENDING '\0'
#endif

//================
class CFileTagging
//================
{
public:
	// Write Tags
	void WriteID3v2Tags(FILE *f);
	void WriteWaveTags(WAVEDATAHEADER *wdh, WAVEFILEHEADER *wfh, FILE *f);

	// Tag data
	string title, artist, album, year, comments, genre, url, encoder;

	CFileTagging();


private:
	// Convert Integer to Synchsafe Integer (see ID3v2.4 specs)
	UINT32 intToSynchsafe(UINT32 in);
	// Write a frame
	void WriteID3v2Frame(char cFrameID[4], string sFramecontent, FILE *f);
	// Size of our tag
	UINT32 totalID3v2Size;
};

#pragma pack()
