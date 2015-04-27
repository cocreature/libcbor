/*
 * Copyright (c) 2014-2015 Pavel Kalvoda <me@pavelkalvoda.com>
 *
 * libcbor is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "bytestrings.h"
#include <assert.h>

size_t cbor_bytestring_length(const cbor_item_t *item)
{
	assert(cbor_isa_bytestring(item));
	return item->metadata.bytestring_metadata.length;
}

unsigned char *cbor_bytestring_handle(const cbor_item_t *item)
{
	assert(cbor_isa_bytestring(item));
	return item->data;
}

bool cbor_bytestring_is_definite(const cbor_item_t *item)
{
	assert(cbor_isa_bytestring(item));
	return item->metadata.bytestring_metadata.type == _CBOR_METADATA_DEFINITE;
}

bool cbor_bytestring_is_indefinite(const cbor_item_t *item)
{
	return !cbor_bytestring_is_definite(item);
}

cbor_item_t *cbor_new_definite_bytestring()
{
	cbor_item_t *item = _CBOR_MALLOC(sizeof(cbor_item_t));
	*item = (cbor_item_t) {
		.refcount = 1,
		.type = CBOR_TYPE_BYTESTRING,
		.metadata = {.bytestring_metadata = {_CBOR_METADATA_DEFINITE, 0}}
	};
	return item;
}

cbor_item_t *cbor_new_indefinite_bytestring()
{
	cbor_item_t *item = _CBOR_MALLOC(sizeof(cbor_item_t));
	*item = (cbor_item_t) {
		.refcount = 1,
		.type = CBOR_TYPE_BYTESTRING,
		.metadata = {.bytestring_metadata = {.type = _CBOR_METADATA_INDEFINITE, .length = 0}},
		.data = _CBOR_MALLOC(sizeof(struct cbor_indefinite_string_data))
	};
	*((struct cbor_indefinite_string_data *) item->data) = (struct cbor_indefinite_string_data) {
		.chunk_count = 0,
		.chunk_capacity = 0,
		.chunks = NULL,
	};
	return item;
}

void cbor_bytestring_set_handle(cbor_item_t *item, unsigned char *data, size_t length)
{
	assert(cbor_isa_bytestring(item));
	assert(cbor_bytestring_is_definite(item));
	item->data = data;
	item->metadata.bytestring_metadata.length = length;
}

cbor_item_t **cbor_bytestring_chunks_handle(const cbor_item_t *item)
{
	assert(cbor_isa_bytestring(item));
	assert(cbor_bytestring_is_indefinite(item));
	return ((struct cbor_indefinite_string_data *) item->data)->chunks;
}

size_t cbor_bytestring_chunk_count(const cbor_item_t *item)
{
	assert(cbor_isa_bytestring(item));
	assert(cbor_bytestring_is_indefinite(item));
	return ((struct cbor_indefinite_string_data *) item->data)->chunk_count;

}

cbor_item_t *cbor_bytestring_add_chunk(cbor_item_t *item, cbor_item_t *chunk)
{
	assert(cbor_isa_bytestring(item));
	assert(cbor_bytestring_is_indefinite(item));
	struct cbor_indefinite_string_data *data = (struct cbor_indefinite_string_data *) item->data;
	// TODO optimize this with exponential growth
	if (data->chunk_count == data->chunk_capacity) {
		/* We need more space */
		data->chunk_capacity = data->chunk_capacity == 0 ? 1 : data->chunk_capacity * 2;
		cbor_item_t **new_chunks_data =
			_CBOR_REALLOC(data->chunks, data->chunk_capacity * sizeof(cbor_item_t *));
		// TODO handle failures
		data->chunks = new_chunks_data;
	}
	data->chunks[data->chunk_count++] = chunk;
	return item;
}
