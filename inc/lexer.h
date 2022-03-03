#pragma once

#include "string_t.h"

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
} token_type_struct;

typedef struct token_list_node
{
    struct token_list_node *next;
    struct token_list_node *prev;
    token_type_struct token;
    string_t string;
} token_list_node;

typedef struct token_list_t
{
    token_list_node *head;
    token_list_node *tail;
    size_t size;
} token_list_t;

/**
 * @brief A convenient and safe for_each loop for the token list, starting from
 * a given node
 * @param[in, out] node The starting token node
 * @param[in] temp A temp token_node pointer to pre fetch the next value
 * @note temp and list should not be used, they are not safe. Set node before
 * using.
 */
#define for_each_token_node_from(node, temp)                      \
    for (temp = (node == NULL ? NULL : node->next); node != NULL; \
         node = temp, temp = (node == NULL ? NULL : node->next))

token_list_t *lex_file(FILE *input_file);
void put_token_node_list(void);
