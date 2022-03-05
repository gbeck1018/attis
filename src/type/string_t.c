/** string_t.c
 * @brief Utilities for c-style string
 */

#include "error_handling.h"
#include "type/string_t.h"

/**
 * @brief Allocate a string_t
 * @param[in,out] input_struct A pointer to the struct to allocate. Note that
 * the entire struct isn't allocated, only the 'string' member
 * @param[in] input_string A string to copy into the struct. Give NULL or ""
 * to initialize to the empty string.
 * @param[in] reserve_space The ammount of space to reserve for the string
 */
void get_string(string_t *input_struct, char const *input_string,
                size_t reserve_space)
{
    ASSERT(input_struct != NULL, "Invalid string allocation\n");

    // To make sure strlen doesn't complain about a NULL string
    if (input_string == NULL)
    {
        input_string = "";
    }
    size_t input_length = strlen(input_string);
    if (reserve_space == NO_EXTRA_SPACE)
    {
        reserve_space = input_length;
    }
    ASSERT(input_length <= reserve_space,
           "Invalid reserve space given to allocate string: %lu > %lu\n",
           input_length, reserve_space);

    input_struct->string = malloc(reserve_space);
    ASSERT(input_struct->string != NULL,
           "Failed to allocate memory for string\n");

    input_struct->string_length = input_length;
    input_struct->reserve_space = reserve_space;
    strcpy(input_struct->string, input_string);
}

/**
 * @brief Deallocate a string_t
 * @param[in] input_struct A pointer to the struct to deallocate the 'string'
 * member of.
 */
void put_string(string_t *input_struct)
{
    ASSERT(input_struct != NULL, "Bad call to put_string\n");
    if (input_struct->string != NULL)
    {
        free(input_struct->string);
    }
    input_struct->string = NULL;
}

/**
 * @brief Clone a string_t
 * @param[in,out] input_struct A pointer to the struct to copy to.
 * @param[in] copy_struct A pointer to the struct to copy from.
 */
void get_string_clone(string_t *input_struct, string_t const *copy_struct)
{
    input_struct->string_length = copy_struct->string_length;
    input_struct->reserve_space = copy_struct->reserve_space;
    input_struct->string = malloc(copy_struct->reserve_space);
    ASSERT(input_struct->string,
           "Could not allocate memory for copy string\n");
    strcpy(input_struct->string, copy_struct->string);
}

/**
 * @brief Add a character to a string_t
 * @param[in,out] string_struct A pointer to the struct to add to
 * @param[in] character The character to add
 * @note This may allocate heap memory if the existing space is too small
 */
void add_character(string_t *string_struct, char character)
{
    // If we need more room for the new character, realloc
    if (string_struct->string_length == string_struct->reserve_space)
    {
        string_struct->string
            = realloc(string_struct->string, string_struct->reserve_space * 2);
        ASSERT(string_struct->string, "Could not allocate space for string\n");
        string_struct->reserve_space *= 2;
    }

    string_struct->string[string_struct->string_length] = character;
    string_struct->string[string_struct->string_length + 1] = '\0';
    string_struct->string_length += 1;
}
