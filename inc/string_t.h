#pragma once

#include <string.h> // `size_t`

typedef struct string_t
{
    size_t string_length; // Note that this doesn't include the NULL terminator
    size_t reserve_space;
    char *string;
} string_t;

#define NO_EXTRA_SPACE ((size_t)-1)

void get_string(string_t *input_struct, char const *input_string,
                size_t reserve_space);
void put_string(string_t *input_struct);
void get_string_clone(string_t *input_struct, string_t const *copy_struct);
void add_character(string_t *string, char character);
