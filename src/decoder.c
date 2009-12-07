#include <bert/decoder.h>
#include <bert/magic.h>
#include <bert/errno.h>

#include "read.h"

#define BERT_ASSERT_BYTES(bytes)	if ((decoder->length - decoder->index) < (bytes)) { return BERT_ERRNO_SHORT; }

inline uint8_t bert_decode_uint8(bert_decoder_t *decoder)
{
	uint8_t i;

	i = bert_read_uint8(decoder->ptr);

	++(decoder->index);
	++(decoder->ptr);
	return i;
}

inline int8_t bert_decode_int8(bert_decoder_t *decoder)
{
	int8_t i;

	i = bert_read_int8(decoder->ptr);

	++(decoder->index);
	++(decoder->ptr);
	return i;
}

inline uint16_t bert_decode_uint16(bert_decoder_t *decoder)
{
	uint16_t i = bert_read_uint16(decoder->ptr);

	decoder->ptr += 2;
	decoder->index += 2;
	return i;
}

inline uint32_t bert_decode_uint32(bert_decoder_t *decoder)
{
	uint32_t i = bert_read_uint32(decoder->ptr);

	decoder->ptr += 4;
	decoder->index += 4;
	return i;
}

inline int32_t bert_decode_int32(bert_decoder_t *decoder)
{
	int32_t i = bert_read_int32(decoder->ptr);

	decoder->ptr += 4;
	decoder->index += 4;
	return i;
}

inline bert_magic_t bert_decode_magic(bert_decoder_t *decoder)
{
	bert_magic_t m;

	m = bert_read_magic(decoder->ptr);

	++(decoder->ptr);
	++(decoder->index);
	return m;
}

inline const char * bert_read_string(bert_decoder_t *decoder,bert_string_size_t length)
{
	char *s;
	
	s = (char *)(decoder->buffer + decoder->index);
	decoder->index += length;
	return s;
}

inline const unsigned char * bert_read_binary(bert_decoder_t *decoder,bert_binary_size_t length)
{
	unsigned char *s;
	
	s = (unsigned char *)(decoder->buffer + decoder->index);
	decoder->index += length;
	return s;
}

void bert_decode_init(bert_decoder_t *decoder,const unsigned char *buffer,size_t length)
{
	decoder->buffer = buffer;
	decoder->length = length;

	decoder->index = 0;
	decoder->ptr = decoder->buffer;
}

inline int bert_decode_nil(bert_decoder_t *decoder,bert_data_t **data)
{
	bert_data_t *new_data;

	if (!(new_data = bert_data_create_nil()))
	{
		return BERT_ERRNO_MALLOC;
	}

	*data = new_data;
	return BERT_SUCCESS;
}

inline int bert_decode_small_int(bert_decoder_t *decoder,bert_data_t **data)
{
	BERT_ASSERT_BYTES(1)

	int8_t i = bert_read_int8(decoder->ptr);

	bert_data_t *new_data;

	if (!(new_data = bert_data_create_int(i)))
	{
		return BERT_ERRNO_MALLOC;
	}

	*data = new_data;
	return BERT_SUCCESS;
}

inline int bert_decode_big_int(bert_decoder_t *decoder,bert_data_t **data)
{
	BERT_ASSERT_BYTES(4)

	int32_t i = bert_read_int32(decoder->ptr);

	bert_data_t *new_data;

	if (!(new_data = bert_data_create_int(i)))
	{
		return BERT_ERRNO_MALLOC;
	}

	*data = new_data;
	return BERT_SUCCESS;
}

int bert_decode_bignum(bert_decoder_t *decoder,bert_data_t **data,size_t size)
{
	BERT_ASSERT_BYTES(1 + size)

	uint8_t sign = bert_decode_uint8(decoder);

	const unsigned char *ptr = decoder->ptr;

	uint32_t unsigned_integer;
	uint8_t b;
	uint8_t i;

	for (i=0;i<size;i++)
	{
		b = (uint8_t)(ptr[i]);
		unsigned_integer = ((unsigned_integer << 8) | b);
	}

	decoder->index += size;
	decoder->ptr += size;

	int32_t signed_integer = (int32_t)unsigned_integer;

	if (sign)
	{
		signed_integer = -(signed_integer);
	}

	bert_data_t *new_data;

	if (!(new_data = bert_data_create_int(signed_integer)))
	{
		return BERT_ERRNO_MALLOC;
	}

	*data = new_data;
	return BERT_SUCCESS;
}

inline int bert_decode_small_bignum(bert_decoder_t *decoder,bert_data_t **data)
{
	BERT_ASSERT_BYTES(1)

	return bert_read_bignum(decoder,data,bert_read_uint8(decoder->ptr));
}

inline int bert_decode_big_bignum(bert_decoder_t *decoder,bert_data_t **data)
{
	BERT_ASSERT_BYTES(4)

	return bert_read_bignum(decoder,data,bert_read_uint32(decoder->ptr));
}

inline int bert_decode_string(bert_decoder_t *decoder,bert_data_t **data)
{
	BERT_ASSERT_BYTES(4)

	bert_string_size_t size = bert_decode_uint32(decoder);

	BERT_ASSERT_BYTES(size)

	bert_data_t *new_data;

	if (!(new_data = bert_data_create_string((const char *)(decoder->ptr),size)))
	{
		return BERT_ERRNO_MALLOC;
	}

	*data = new_data;
	return BERT_SUCCESS;
}

inline int bert_decode_atom(bert_decoder_t *decoder,bert_data_t **data)
{
	BERT_ASSERT_BYTES(2)

	bert_atom_size_t size = bert_decode_uint16(decoder);

	BERT_ASSERT_BYTES(size)

	bert_data_t *new_data;

	if (!(new_data = bert_data_create_atom((const char *)(decoder->ptr),size)))
	{
		return BERT_ERRNO_MALLOC;
	}

	*data = new_data;
	return BERT_SUCCESS;
}

inline int bert_decode_bin(bert_decoder_t *decoder,bert_data_t **data)
{
	BERT_ASSERT_BYTES(4)

	bert_bin_size_t size = bert_decode_uint32(decoder);

	BERT_ASSERT_BYTES(size)

	bert_data_t *new_data;

	if (!(new_data = bert_data_create_bin((const unsigned char *)(decoder->ptr),size)))
	{
		return BERT_ERRNO_MALLOC;
	}

	*data = new_data;
	return BERT_SUCCESS;
}

inline int bert_decode_tuple(bert_decoder_t *decoder,bert_data_t **data,size_t size)
{
	bert_data_t *new_data;

	if (!(new_data = bert_data_create_tuple(size)))
	{
		return BERT_ERRNO_MALLOC;
	}

	unsigned int i;
	int result;

	for (i=0;i<size;i++)
	{
		if ((result = bert_decode_data(decoder,new_data->tuple.elements+i)) != BERT_SUCCESS)
		{
			bert_data_destroy(new_data);
			return result;
		}
	}

	*data = new_data;
	return BERT_SUCCESS;
}

int bert_decode_data(bert_decoder_t *decoder,bert_data_t **data)
{
	BERT_ASSERT_BYTES(1)

	bert_magic_t magic = bert_read_magic(decoder);

	// skip the BERT MAGIC start byte
	if (magic == BERT_MAGIC)
	{
		magic = bert_read_magic(decoder);
	}

	int result;

	// decode primative data first
	switch (magic)
	{
		case BERT_NIL:
			result = bert_decode_nil(decoder,data);
			break;
		case BERT_SMALL_INT:
			result = bert_decode_small_int(decoder,data);
			break;
		case BERT_INT:
			result = bert_decode_big_int(decoder,data);
			break;
		case BERT_SMALL_BIGNUM:
			result = bert_read_small_bignum(decoder,data);
			break;
		case BERT_LARGE_BIGNUM:
			result = bert_read_big_bignum(decoder,data);
			break;
		case BERT_FLOAT:
			// TODO: implement float decoding
			return BERT_NOTHING;
		case BERT_ATOM:
			result = bert_decode_atom(decoder,data);
			break;
		case BERT_STRING:
			result = bert_decode_string(decoder,data);
			break;
		case BERT_BIN:
			result = bert_decode_bin(decoder,data);
			break;
		case BERT_SMALL_TUPLE:
			result = bert_decode_small_tuple(decoder,data);
			break;
		case BERT_LARGE_TUPLE:
			result = bert_decode_big_tuple(decoder,data);
			break;
		case BERT_LIST:
			result = bert_decode_list(decoder,data);
			break;
		default:
			return BERT_ERRNO_INVALID;
	}

	if (result != BERT_SUCCESS)
	{
		return result;
	}

	/*
	switch (magic)
	{
		case BERT_NIL:
			new_data = bert_data_create_nil();
			break;
		case BERT_SMALL_INT:
		case BERT_INT:
		case BERT_SMALL_BIGNUM:
		case BERT_LARGE_BIGNUM:
			new_data = bert_data_create_int(integer);
			break;
		case BERT_FLOAT:
			// TODO: implement float decoding
			break;
		case BERT_ATOM:
			// decode complex types
			if ((string_length == 3) && (strncmp(string,"nil",3) == 0))
			{
				new_data = bert_data_create_nil();
			}
			else if ((string_length == 4) && (strncmp(string,"true",4) == 0))
			{
				new_data = bert_data_create_true();
			}
			else if ((string_length == 5) && (strncmp(string,"false",5) == 0))
			{
				new_data = bert_data_create_false();
			}
			else if ((string_length == 4) && (strncmp(string,"time",4) == 0))
			{
				// TODO: add support for the time data-type
			}
			else if ((string_length == 4) && (strncmp(string,"dict",4) == 0))
			{
				magic = bert_read_magic(decoder);

				switch (magic)
				{
					case BERT_LIST:
						// TODO: add each 2-tuple pair to new_data
					case BERT_NIL:
						// empty dictionary
						break;
					default:
						return BERT_DECODE_INVALID;
				}
			}
			else
			{
				new_data = bert_data_create_atom(string,string_length);
			}
			break;
		case BERT_STRING:
			new_data = bert_data_create_string(string,string_length);
			break;
		case BERT_BIN:
			new_data = bert_data_create_binary(binary_data,binary_data_length);
			break;
		case BERT_SMALL_TUPLE:
		case BERT_LARGE_TUPLE:
			if (!(new_data = bert_data_create_tuple(tuple_length)))
			{
				return BERT_DECODE_MEMORY;
			}

			for (i=0;i<tuple_length;i++)
			{
				result = bert_decoder_data(decoder,&(new_data->tuple.elements[i]));

				if (result != BERT_DECODE_DATA)
				{
					bert_data_destroy(new_data);
					return result;
				}
			}
			break;
		case BERT_LIST:
			if (!(new_data = bert_data_create_list()))
			{
				return BERT_DECODE_MEMORY;
			}

			// read the list length
			list_length = bert_read_uint32(decoder);

			for (i=0;i<list_length;i++)
			{
				result = bert_decoder_data(decoder,&new_list_data);

				if (result != BERT_DECODE_DATA)
				{
					bert_data_destroy(new_data);
					return result;
				}

				bert_list_append(new_data,new_list_data);
			}

			// read the tailing byte
			bert_read_uint8(decoder);
			break;
		default:
			return BERT_DECODE_INVALID;
	}
	*/

	return result;
}
