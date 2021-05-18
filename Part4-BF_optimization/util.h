#ifndef __UTIL_H
#define __UTIL_H

#define STACKSIZE 100
#include <stdlib.h> // malloc, free, realloc
#include <string.h> // memcpy
#include <stdint.h> // int32_t
#include <assert.h>

static inline
void err(const char * const msg)
{
	fprintf(stderr, "%s\n", msg);
	exit(1);
}

static inline
char* remove_spaces(char* str){
    char* result = (char*)calloc(strlen(str)+1 ,sizeof(char));
    size_t i;
    size_t j;
    
    for (i=0, j=0; i < strlen(str); ++i)
    {
        if (str[i] != ' ') 
            result[j++] = str[i];
    }

    for (; j <= i; j++)
          result[j] = 0; // pad with nulbytes

    return result;
}

// returns a heap allocated string, caller needs to free
static inline
char *read_file(const char * const filename)
{
	if (filename == NULL) return NULL;

	FILE *fp = fopen(filename, "r");
	if (fp == NULL) return NULL;

	assert(!fseek(fp, 0, SEEK_END));
	long file_size = ftell(fp);
	rewind(fp);
	size_t code_size = sizeof(char) * file_size;
	char *code = (char*)malloc(code_size);
	if (code == NULL) return NULL;

	fread(code, 1, file_size, fp);

  for(int i = 0 ; i < file_size; i++)
  {
      if(code[i] == '\r' || code[i] == '\n'){ code[i] = ' '; }
  }

  code = remove_spaces(code);
	assert(!fclose(fp));
	return code;
}


struct stack {
  int size;
  int items [STACKSIZE];
};

int stack_push (struct stack* const, const int);
int stack_pop (struct stack* const, int*);

struct vector {
  int size;
  int capacity;
  char* data;
};

int vector_create (struct vector* const vec, int capacity);
int vector_destroy (struct vector* vec);
int vector_push (struct vector* const vec, char* bytes, int len);
int vector_write32LE (struct vector* const vec, int offset, int value);


int stack_push (struct stack* const p, const int x) {
  if (p->size == STACKSIZE) {
    return -1;
  } else {
    p->items[p->size++] = x;
    return 0;
  }
}

int stack_pop (struct stack* const p, int* x) {
  if (p->size == 0) {
    return -1;
  } else {
    *x = p->items[--p->size];
    return 0;
  }
}


int vector_create (struct vector* const vec, int capacity) {
  vec->size = 0;
  vec->capacity = capacity;
  vec->data = (char*)malloc(capacity * sizeof(char));
  return vec->data == NULL ? -1 : 0;
}

int vector_destroy (struct vector* vec) {
  if (vec == NULL || vec->data == NULL) {
    return -1;
  }
  vec->size = 0;
  vec->capacity = 0;
  free(vec->data);
  vec->data = NULL;
  vec = NULL;
  return 0;
}

int vector_push (struct vector* const vec, char* bytes, int len) {
  if (vec->size + len > vec->capacity) {
    vec->capacity *= 2;
    vec->data = (char*)realloc(vec->data, vec->capacity * sizeof(char));
    if (vec->data == NULL) {
      return -1;
    }
  }
  memcpy(vec->data + vec->size, bytes, len);
  vec->size += len;
  return 0;
}

int vector_write32LE (struct vector* const vec, int offset, int32_t value) {
  if (offset >= vec->size) {
    return -1;
  }
  // offset opposite of usual since we explicitly want LE
  vec->data[offset + 3] = (value & 0xFF000000) >> 24;
  vec->data[offset + 2] = (value & 0x00FF0000) >> 16;
  vec->data[offset + 1] = (value & 0x0000FF00) >> 8;
  vec->data[offset + 0] = (value & 0x000000FF);
  return 0;
}

#endif