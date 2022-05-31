#ifndef AD_ARR_H
#define AD_ARR_H

#include <memory>
#if defined(_WIN32) or defined(__linux)
#include <assert.h>
#endif

#include "types.hpp"

void memfill(void* dst, int32 size, void* pattern, int32 pattern_size);

template<typename T>
struct array_t {
	int32 size      = 0;
	int32 capacity  = 0;
	T* data         = nullptr;

	T* operator[](uint32 index) { assert(index < size); return data + index; }
};

template<typename T>
void arr_clear(array_t<T>* array) {
	memset(array->data, 0, arr_bytes(array));
	array->size = 0;
}

template<typename T>
void arr_fastclear(array_t<T>* array) {
	array->size = 0;
}

template<typename T>
void arr_fill(array_t<T>* array, T element) {
	//memfill(array->data, arr_bytes(array), &element, sizeof(T));
	array->size = array->capacity; 
}

template<typename T>
void arr_fill(array_t<T>* array, int32 offset, int32 count, T element) {
	//memfill(array->data + offset, count * sizeof(T), &element, sizeof(T));
	// You're on your own as far as the size here
}

template<typename T>
void arr_init(array_t<T>* array, int32 capacity) {
	array->size = 0;
	array->capacity = capacity;
	array->data = (T*)calloc(capacity, sizeof(T));
}

template<typename T>
void arr_init(array_t<T>* array, int32 capacity, T fill) {
	array->size = 0;
	array->capacity = capacity;
	array->data = (T*)calloc(capacity, sizeof(T));
	
	arr_fill(array, T());
}

// Use case: You declare some array on the stack. It's empty, and you only want to modify its elements
// using an array_t. Call this to wrap it in an empty array_t of the correct capacity.
template<typename T>
array_t<T> arr_stack(T* data, int32 capacity) {
	assert(data);

	array_t<T> array;
	array.size = 0;
	array.capacity = capacity;
	array.data = data;
	return array;
}

template<typename T, int32 N>
array_t<T> arr_stack(T (&c_array)[N]) {
	array_t<T> array;
	array.size = 0;
	array.capacity = N;
	array.data = c_array;
	return array;
}

// Use case: You have some contiguous data filled out somewhere (maybe in another array_t, maybe in a C
// array). You want to RW a subarray using array_t functions. Call this to wrap the subarray. 
template<typename T>
array_t<T> arr_slice(array_t<T>* array, int32 index, int32 size) {
	assert(index >= 0);
	assert(index + size <= array->capacity);
	
	array_t<T> view;
	view.size = size;
	view.capacity = size;
	view.data = array->data + index;
	
	return view;
}

template<typename T>
array_t<T> arr_slice(T* data, int32 size) {
	array_t<T> arr;
	arr.size = size;
	arr.capacity = size;
	arr.data = data;
	
	return arr; 
}

template<typename T>
int32 arr_indexof(array_t<T>* array, T* element) {
	int32 index = element - array->data;
	assert(index >= 0);
	assert(index < array->size);
	return index;
}

template<typename T>
T* arr_at(array_t<T>* array, int32 index) {
	return (*array)[index];
}

template<typename T>
T* arr_push(array_t<T>* array, const T* data, int32 count) {
	int32 remaining = array->capacity - array->size;
	if (remaining < count) return nullptr;
	
	memcpy(array->data + array->size, data, sizeof(T) * count);
	T* out = array->data + array->size;
	array->size += count;
	return out;
}

template<typename T>
T* arr_push(array_t<T>* array, T* data, int32 count) {
	int32 remaining = array->capacity - array->size;
	if (remaining < count) return nullptr;
	
	memcpy(array->data + array->size, data, sizeof(T) * count);
	T* out = array->data + array->size;
	array->size += count;
	return out;
}

template<typename T>
T* arr_push(array_t<T>* array, T data) {
	assert(array->size < array->capacity);
	array->data[array->size] = data;
	T* out = array->data + array->size;
	array->size += 1;
	return out;
}

template<typename T>
T* arr_push(array_t<T>* array) {
	assert(array->size < array->capacity);
	array->data[array->size] = T();
	T* out = array->data + array->size;
	array->size += 1;
	return out;
}

template<typename T>
T* arr_concat(array_t<T>* dest, array_t<T>* source) {
	assert(dest->size + source->size < dest->capacity);
	memcpy(dest->data + dest->size, source->data, sizeof(T) * source->count);
	T* out = dest->data + dest->size;
	dest->size += source->size;
	return out;
}

template<typename T>
T* arr_concat(array_t<T>* dest, array_t<T>* source, int32 count) {
	assert(dest->size + count < dest->capacity);
	memcpy(dest->data + dest->size, source->data, sizeof(T) * count);
	T* out = dest->data + dest->size;
	dest->size += count;
	return out;
}

template<typename T>
T* arr_back(array_t<T>* array) {
	return array->data + (array->size - 1);
}

template<typename T>
T* arr_next(array_t<T>* array) {
	assert(array->size != array->capacity);
	return array->data + (array->size);
}

template<typename T>
void arr_free(array_t<T>* array) {
	free(array->data);
	array->data = nullptr;
	array->size = 0;
	array->capacity = 0;
	return;
}

template<typename T>
int32 arr_bytes(array_t<T>* array) {
	return array->capacity * sizeof(T);
}

#define arr_for(array, it) for (auto (it) = (array).data; (it) != ((array).data + (array).size); (it)++)
#define arr_rfor(array, it) for (auto (it) = (array).data + array.size - 1; (it) >= ((array).data); (it)--)



template<typename T>
struct array_view_t {
	int32 size      = 0;
	int32 capacity  = 0;
	T* data         = nullptr;

	// @spader could make this const correct, whatever
	T* operator[](uint32 index) { assert(index < size); return data + index; }
};

template<typename T>
array_view_t<T> arr_view(T* data, int32 size) {
	array_view_t<T> view;
	view.size = size;
	view.capacity = size;
	view.data = data;
	
	return view;
}

template<typename T>
array_view_t<T> arr_view(array_t<T>* array) {
	array_view_t<T> view;
	view.size = array->size;
	view.capacity = array->size;
	view.data = array->data;
	
	return view;
}

template<typename T>
array_view_t<T> arr_view(array_t<T>* array, int32 index, int32 count) {
	assert(index >= 0);
	assert(index + count <= array->capacity);

	array_view_t<T> view;
	view.size = count;
	view.capacity = count;
	view.data = array->data + index;
	
	return view;
}

template<typename T, int32 N>
array_view_t<T> arr_view(T (&array)[N]) {
	array_view_t<T> view;
	view.size = N;
	view.capacity = N;
	view.data = array;
	
	return view;
}


template<typename T>
int32 arr_indexof(array_view_t<T>* array, T* element) {
	int32 index = element - array->data;
	assert(index >= 0);
	assert(index < array->size);
	return index;
}


template<typename T>
struct array_marker_t {
	int32 begin = 0;
	array_t<T>* array  = nullptr;
};

template<typename T>
array_marker_t<T> arr_marker_make(array_t<T>* array) {
	array_marker_t<T> marker;
	arr_marker_init(&marker, array);
	return marker;
}

template<typename T>
void arr_marker_init(array_marker_t<T>* marker, array_t<T>* array) {
	marker->begin = array->size;
	marker->array = array;
}

template<typename T>
int32 arr_marker_count(array_marker_t<T>* marker) {
	return marker->array->size - marker->begin;
}

#endif
