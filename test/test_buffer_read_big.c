#include <bert/buffer.h>

#include <string.h>
#include <stdio.h>

#include "test.h"

#define OUTPUT_SIZE	(BERT_CHUNK_SIZE * 2)

int main()
{
	unsigned char data[BERT_CHUNK_SIZE];
	bert_buffer_t buffer;

	bert_buffer_init(&buffer);

	memset(data,'A',BERT_CHUNK_SIZE);
	bert_buffer_write(&buffer,data,BERT_CHUNK_SIZE);

	memset(data,'B',BERT_CHUNK_SIZE);
	bert_buffer_write(&buffer,data,BERT_CHUNK_SIZE);

	unsigned char output[OUTPUT_SIZE];
	size_t result;

	if ((result = bert_buffer_read(output,&buffer,OUTPUT_SIZE)) != OUTPUT_SIZE)
	{
		test_fail("bert_buffer_read only read %u bytes, expected %u",result,OUTPUT_SIZE);
	}

	int i;

	for (i=0;i<BERT_CHUNK_SIZE;i++)
	{
		if (output[i] != 'A')
		{
			test_fail("bert_buffer_read return unexpected data");
		}
	}

	for (i=BERT_CHUNK_SIZE;i<OUTPUT_SIZE;i++)
	{
		if (output[i] != 'B')
		{
			test_fail("bert_buffer_read return unexpected data");
		}
	}

	return 0;
}
