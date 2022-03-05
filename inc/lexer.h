#pragma once

#include "type/string_t.h"
#include "type/list_t.h"

#include <stdio.h> // `FILE`

typedef enum
{
    TokenCR,
    TokenLF,
    TokenWhitespace,
    TokenUnaryOperator,
    TokenBinaryOperator,
    TokenOpenParenthesis,
    TokenCloseParenthesis,
    TokenLiteral,
    TokenSemicolon,
    TokenEOF,
    TokenUnknown
} token_type_enum;

typedef struct token_list_node_t
{
    list_entry_t list;
    token_type_enum token;
    string_t string;
} token_list_node_t;

#define token_node(ptr) container_of(ptr, token_list_node_t, list)

token_list_node_t *lex_file(FILE *input_file);
void put_token_node_list(void);
