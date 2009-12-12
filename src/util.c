#include <bert/util.h>

inline uint8_t bert_read_uint8(const unsigned char *ptr)
{
	return (uint8_t)(*ptr);
}

inline uint16_t bert_read_uint16(const unsigned char *ptr)
{
	return (uint16_t)((ptr[0] << 8) | ptr[1]);
}

inline bert_magic_t bert_read_magic(const unsigned char *ptr)
{
	return (bert_magic_t)(*ptr);
}

inline uint32_t bert_read_uint32(const unsigned char *ptr)
{
	return (uint32_t)((ptr[0] << 24 ) | (ptr[1] << 16) | (ptr[2] << 8) | ptr[3]);
}
