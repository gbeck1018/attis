#pragma once

#include <stdio.h> // `FILE`

FILE *get_file(char const *restrict filename, char const *restrict modes);
void put_file(void);
