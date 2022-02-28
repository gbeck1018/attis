/** file.c
 * @brief File IO handling
 *
 * STATE: input_file
 */

#include "error_handling.h"
#include "file.h"

/**
 * STATE: This holds the open input file
 */
static FILE *input_file = NULL;

/**
 * @brief Open a file for reading 
 */
FILE *get_file(const char *restrict filename, const char *restrict modes)
{
    input_file = fopen(filename, modes);
    ASSERT(input_file != NULL, "Failed to open file: '%s'\n", filename);
    return input_file;
}

/**
 * @brief Close the open file
 */
void put_file()
{
    if (input_file != NULL)
    {
        ASSERT(fclose(input_file) == 0, "Failed to close file\n");
    }
    input_file = NULL;
}
