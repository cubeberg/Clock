
extern volatile unsigned char *owdport, *owiport;
extern unsigned owbit, owtiming;

void one_wire_setup(volatile unsigned char *dir, volatile unsigned char *in, unsigned bitmask, unsigned mhz)
{
	static const unsigned owd[18] = {
	//  us		overhead
		28,		19,		// Zero
		33,		33,
		39,		39,
		7,		14,		// One
		8,	 	16,
		85,		44,
		500,	23,		// Reset
		50,		25,
		450,	20
	};

	unsigned *p = &owtiming;
	const unsigned *q = owd;
	do {
		*p++ = ((q[0] * mhz) - q[1]) << 1;
		q += 2;
	} while((char *)q < ((char *)owd + sizeof(owd)));
	
	owdport = dir;
	owiport = in;
	owbit = bitmask;
}


