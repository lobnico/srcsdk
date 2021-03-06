enum
{
	CHAN_REPLACE	= -1,
	CHAN_AUTO		= 0,
	CHAN_WEAPON		= 1,
	CHAN_VOICE		= 2,
	CHAN_ITEM		= 3,
	CHAN_BODY		= 4,
	CHAN_STREAM		= 5,		// allocate stream channel from the static or dynamic area
	CHAN_STATIC		= 6,		// allocate channel from the static area 
	CHAN_VOICE_BASE	= 7,		// allocate channel for network voice data
    CHAN_VOICE2		= 8,
};

#define SND_FLAG_BITS_ENCODE 9

#define MAX_SOUND_INDEX_BITS	13