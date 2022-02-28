#pragma once

#include <errno.h>  // `errno`
#include <stdlib.h> // `exit`
#include <stdint.h> // `uintptr_t`
#include <stdio.h>  // `fprintf`, `printf`, `stderr`

/**
 * @brief Assert the truth of the a statement or exit
 *
 * @param statement The statement to test
 * @param __VA_ARGS__ A variadic argument which will be fed to printf
 */
#define ASSERT(statement, ...)                                          \
    do                                                                  \
    {                                                                   \
        int temp_err = (int)(uintptr_t)(statement);                     \
        if (!temp_err)                                                  \
        {                                                               \
            fprintf(stderr, "Error on line %d in file %s:\n", __LINE__, \
                    __FILE__);                                          \
            fprintf(stderr, __VA_ARGS__);                               \
            printf("Exiting...\n");                                     \
            exit(errno ? errno : temp_err);                             \
        }                                                               \
    } while (0)
