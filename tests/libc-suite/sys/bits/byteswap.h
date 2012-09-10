inline unsigned short __bswap_16(unsigned short s) {
	short f = ((s >> 8) & 0xFF) |
		((s & 0xFF) << 8);
	return f;
}

#define __bswap_32 __builtin_bswap32
#define __bswap_64 __builtin_bswap64
