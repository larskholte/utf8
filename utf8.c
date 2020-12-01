// utf8.c

#include "utf8.h"

void UTF8_Decoder_init(UTF8_Decoder* decoder) {
	decoder->state = 0;
	decoder->seqLength = 0;
	decoder->c = 0;
}
ucp* UTF8_Decoder_decode(UTF8_Decoder* decoder, byte b, ucp* c, int* error) {
	*error = 0;
	if (decoder->state == 0) {
		if (b < 0x80) {
			// US-ASCII character
			*(c++) = b;
			return c;
		}
		// First byte of multi-byte character
		if ((b & 0xe0) == 0xc0) {
			decoder->c = b & 0x1f;
			decoder->seqLength = 2;
			decoder->state = 1;
			return c;
		}
		if ((b & 0xf0) == 0xe0) {
			decoder->c = b & 0x0f;
			decoder->seqLength = 3;
			decoder->state = 2;
			return c;
		}
		if ((b & 0xf8) == 0xf0) {
			decoder->c = b & 0x07;
			decoder->seqLength = 4;
			decoder->state = 3;
			return c;
		}
		*error = UTF8_ERROR_INVALID_START_BYTE;
		return c;
	}
	// Next byte of multi-byte character
	if ((b & 0xc0) != 0x80) {
		*error = UTF8_ERROR_INVALID_CONTINUATION_BYTE;
		return c;
	}
	decoder->c = decoder->c << 6 | (b & 0x3f);
	decoder->state--;
	if (decoder->state == 0) {
		// Multi-byte character complete
		if (UTF8_BytesForChar(decoder->c) == decoder->seqLength) {
			*(c++) = decoder->c;
		} else {
			*error = UTF8_ERROR_OVERLONG_SEQUENCE;
		}
	}
	return c;
}
int UTF8_Decoder_canTerminate(UTF8_Decoder* decoder) {
	return decoder->state == 0;
}

ucp* UTF8_DecodeArray(byte* ba, size_t bc, ucp* ca, size_t cc, int* error) {
	UTF8_Decoder decoder;
	UTF8_Decoder_init(&decoder);
	byte* bend = ba + bc;
	ucp* cend = ca + cc;
	for (; ba < bend && ca < cend; ba++) {
		ca = UTF8_Decoder_decode(&decoder, *ba, ca, error);
		if (*error) {
			return ca;
		}
	}
	if (ba < bend && ca >= cend) {
		// We ran out of room to store characters
		*error = UTF8_ERROR_CHAR_OVERFLOW;
	} else if (!UTF8_Decoder_canTerminate(&decoder)) {
		// Input bytes did not end on a character boundary
		*error = UTF8_ERROR_UNEXPECTED_TERMINATION;
	} else {
		*error = 0;
	}
	return ca;
}
ucp* UTF8_DecodeString(const char* str, ucp* ca, size_t cc, int* error) {
	UTF8_Decoder decoder;
	UTF8_Decoder_init(&decoder);
	ucp* cend = ca + cc;
	for (; *str && ca < cend; str++) {
		ca = UTF8_Decoder_decode(&decoder, (byte)*str, ca, error);
		if (*error) {
			return ca;
		}
	}
	if (*str) {
		*error = UTF8_ERROR_CHAR_OVERFLOW;
	} else if (!UTF8_Decoder_canTerminate(&decoder)) {
		*error = UTF8_ERROR_UNEXPECTED_TERMINATION;
	} else {
		*error = 0;
	}
	return ca;
}

int UTF8_BytesForChar(ucp c) {
	if (c < 0x80) return 1;
	if (c < 0x800) return 2;
	if (c < 0x10000) return 3;
	return 4;
}
byte* UTF8_EncodeChar(ucp c, byte* b) {
	if (c < 0x80) {
		*(b++) = c;
		return b;
	}
	if (c < 0x800) {
		*(b++) = (c & 0x7ff) >> 6 | 0xc0;
		*(b++) = (c & 0x3f) | 0x80;
		return b;
	}
	if (c < 0x10000) {
		*(b++) = (c & 0xffff) >> 12 | 0xe0;
		*(b++) = (c & 0xfff) >> 6 | 0x80;
		*(b++) = (c & 0x3f) | 0x80;
		return b;
	}
	*(b++) = (c & 0x1fffff) >> 18 | 0xf0;
	*(b++) = (c & 0x3ffff) >> 12 | 0x80;
	*(b++) = (c & 0xfff) >> 6 | 0x80;
	*(b++) = (c & 0x3f) | 0x80;
	return b;
}
byte* UTF8_EncodeArray(ucp* ca, size_t cc, byte* ba, size_t bc, int* error) {
	ucp* cend = ca + cc;
	byte* bend = ba + bc;
	for (; ca < cend && ba + UTF8_BytesForChar(*ca) <= bend; ca++) {
		ba = UTF8_EncodeChar(*ca, ba);
	}
	if (ca < cend) {
		// We did not encode all characters
		*error = UTF8_ERROR_BYTE_OVERFLOW;
	} else {
		*error = 0;
	}
	return ba;
}
