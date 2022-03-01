#pragma once

#include "string_t.h"

#include <stdio.h> // `FILE`

typedef struct
{
    enum
    {
        TokenNumber,
        TokenBinaryOperator,
        TokenOpenParenthesis,
        TokenCloseParenthesis,
        TokenWhitespace,
        TokenCR,
        TokenLF,
        TokenEOF,
        TokenUnknown
    } type;
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
 * @brief A convenient and safe for_each loop for the token list
 *
 * @param[in, out] node a token_node pointer which will serve as the index
 * @param[in] temp a temp token_node pointer to pre fetch the next value
 * @param[in] list A pointer to the list to iterate through
 * @note temp and list should not be used, they are not safe
 */
#define for_each_token_node(node, temp, list)                            \
    for (node = (list)->head, temp = (node == NULL ? NULL : node->next); \
         node != NULL;                                                   \
         node = temp, temp = (node == NULL ? NULL : node->next))

token_list_t *lex_file(FILE *input_file);
void put_token_node_list(void);
