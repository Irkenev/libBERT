#ifndef _BERT_DATA_H_
#define _BERT_DATA_H_

#include <bert/types.h>
#include <bert/list.h>
#include <bert/dict.h>

#include <stdint.h>

typedef enum
{
	bert_data_none = 0,
	bert_data_boolean,
	bert_data_int,
	bert_data_float,
	bert_data_atom,
	bert_data_string,
	bert_data_tuple,
	bert_data_list,
	bert_data_dict,
	bert_data_bin,
	bert_data_nil
} bert_data_type;

struct bert_data
{
	bert_data_type type;

	union
	{
		unsigned int boolean;
		int integer;
		float floating_point;

		struct
		{
			bert_atom_size_t length;
			char *name;
		} atom;

		struct
		{
			bert_string_size_t length;
			char *text;
		} string;

		struct
		{
			bert_bin_size_t length;
			unsigned char *data;
		} bin;

		struct
		{
			bert_tuple_size_t length;
			struct bert_data **elements;
		} tuple;

		struct bert_list *list;
		struct bert_dict *dict;
	};
};
typedef struct bert_data bert_data_t;

extern bert_data_t * bert_data_create();
extern bert_data_t * bert_data_create_nil();
extern bert_data_t * bert_data_create_true();
extern bert_data_t * bert_data_create_false();
extern bert_data_t * bert_data_create_int(int i);
extern bert_data_t * bert_data_create_float(float f);
extern bert_data_t * bert_data_create_atom(const char *name,bert_atom_size_t length);
extern bert_data_t * bert_data_create_string(const char *name,bert_string_size_t length);
extern bert_data_t * bert_data_create_tuple(bert_tuple_size_t length);
extern bert_data_t * bert_data_create_list();
extern bert_data_t * bert_data_create_dict();
extern bert_data_t * bert_data_create_bin(const unsigned char *data,bert_bin_size_t length);

extern void bert_data_destroy(bert_data_t *data);

#endif
