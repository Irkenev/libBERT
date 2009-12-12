#include "encode.h"
#include "encoder.h"
#include <bert/magic.h>
#include <bert/util.h>
#include <bert/errno.h>

#include <string.h>

int bert_encode_magic(bert_encoder_t *encoder,bert_magic_t magic)
{
	unsigned char buffer[1];

	bert_write_magic(buffer,magic);
	return bert_encoder_write(encoder,buffer,1);
}

int bert_encode_small_int(bert_encoder_t *encoder,uint8_t i)
{
	size_t length = sizeof(bert_magic_t) + sizeof(uint8_t);
	unsigned char buffer[length];

	bert_write_magic(buffer,BERT_SMALL_INT);
	bert_write_uint8(buffer+sizeof(bert_magic_t),i);

	return bert_encoder_write(encoder,buffer,length);
}

int bert_encode_big_int(bert_encoder_t *encoder,uint32_t i)
{
	size_t length = sizeof(bert_magic_t) + sizeof(uint32_t);
	unsigned char buffer[length];

	bert_write_magic(buffer,BERT_INT);
	bert_write_uint32(buffer+sizeof(bert_magic_t),i);

	return bert_encoder_write(encoder,buffer,length);
}

int bert_encode_int(bert_encoder_t *encoder,unsigned int i)
{
	if (i <= 0xff)
	{
		return bert_encode_small_int(encoder,i);
	}
	else if (i <= 0xffffffff)
	{
		return bert_encode_big_int(encoder,i);
	}

	return BERT_ERRNO_INVALID;
}

int bert_encode_atom(bert_encoder_t *encoder,const char *atom,size_t length)
{
	size_t buffer_length = sizeof(bert_magic_t) + 2 + length;
	unsigned char buffer[buffer_length];

	bert_write_magic(buffer,BERT_ATOM);
	bert_write_uint16(buffer+sizeof(bert_magic_t),length);

	memcpy(buffer+sizeof(bert_magic_t)+2,atom,sizeof(unsigned char)*length);

	return bert_encoder_write(encoder,buffer,buffer_length);
}

int bert_encode_string(bert_encoder_t *encoder,const char *string,size_t length)
{
	size_t buffer_length = sizeof(bert_magic_t) + 4 + length;
	unsigned char buffer[buffer_length];

	bert_write_magic(buffer,BERT_STRING);
	bert_write_uint32(buffer+sizeof(bert_magic_t),length);

	memcpy(buffer+sizeof(bert_magic_t)+4,string,sizeof(unsigned char)*length);

	return bert_encoder_write(encoder,buffer,buffer_length);
}

int bert_encode_bin(bert_encoder_t *encoder,const unsigned char *bin,size_t length)
{
	size_t buffer_length = sizeof(bert_magic_t) + 4 + length;
	unsigned char buffer[buffer_length];

	bert_write_magic(buffer,BERT_BIN);
	bert_write_uint32(buffer+sizeof(bert_magic_t),length);

	memcpy(buffer+sizeof(bert_magic_t)+4,bin,sizeof(unsigned char)*length);

	return bert_encoder_write(encoder,buffer,buffer_length);
}

int bert_encode_tuple_elements(bert_encoder_t *encoder,const bert_data_t **elements,size_t length)
{
	unsigned int i;
	int result;

	for (i=0;i<length;i++)
	{
		if ((result = bert_encoder_push(encoder,elements[i])) != BERT_SUCCESS)
		{
			return result;
		}
	}

	return BERT_SUCCESS;
}

int bert_encode_small_tuple(bert_encoder_t *encoder,const bert_data_t **elements,size_t length)
{
	size_t buffer_length = 1 + 1;
	unsigned char buffer[buffer_length];

	bert_write_magic(buffer,BERT_SMALL_TUPLE);
	bert_write_uint8(buffer,length);

	return bert_encode_tuple_elements(encoder,elements,length);
}

int bert_encode_big_tuple(bert_encoder_t *encoder,const bert_data_t **elements,size_t length)
{
	size_t buffer_length = 1 + 4;
	unsigned char buffer[buffer_length];

	bert_write_magic(buffer,BERT_LARGE_TUPLE);
	bert_write_uint32(buffer,length);

	return bert_encode_tuple_elements(encoder,elements,length);
}

int bert_encode_tuple(bert_encoder_t *encoder,const bert_data_t **elements,size_t length)
{
	if (length <= 0xff)
	{
		return bert_encode_small_tuple(encoder,elements,length);
	}
	else if (length <= 0xffffff)
	{
		return bert_encode_big_tuple(encoder,elements,length);
	}

	return BERT_ERRNO_INVALID;
}

int bert_encode_list(bert_encoder_t *encoder,const bert_list_t *list)
{
	bert_list_node_t *next_node = list->head;
	size_t length = 0;

	while (next_node)
	{
		++length;
		next_node = next_node->next;
	}

	unsigned char buffer[1 + 4];

	bert_write_magic(buffer,BERT_LIST);
	bert_write_uint32(buffer+1,length);

	int result;

	next_node = list->head;

	while (next_node)
	{
		if ((result = bert_encoder_push(encoder,next_node->data)) != BERT_SUCCESS)
		{
			return result;
		}

		next_node = next_node->next;
	}

	return BERT_SUCCESS;
}

int bert_encode_complex(bert_encoder_t *encoder,const char *name)
{
	int result;

	if ((result = bert_encode_atom(encoder,"bert",4)) != BERT_SUCCESS)
	{
		return result;
	}

	if ((result = bert_encode_atom(encoder,name,strlen(name))) != BERT_SUCCESS)
	{
		return result;
	}

	return BERT_SUCCESS;
}

int bert_encode_nil(bert_encoder_t *encoder)
{
	return bert_encode_complex(encoder,"nil");
}

int bert_encode_true(bert_encoder_t *encoder)
{
	return bert_encode_complex(encoder,"true");
}

int bert_encode_false(bert_encoder_t *encoder)
{
	return bert_encode_complex(encoder,"false");
}
