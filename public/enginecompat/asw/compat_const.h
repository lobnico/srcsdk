#define ABSOLUTE_PLAYER_LIMIT 64

#define NUM_ENT_ENTRY_BITS		(MAX_EDICT_BITS + 2)
#define ENT_ENTRY_MASK			(( 1 << NUM_SERIAL_NUM_BITS) - 1)
#define NUM_SERIAL_NUM_BITS		16 // (32 - NUM_ENT_ENTRY_BITS)