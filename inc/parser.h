#pragma once

#include "lexer.h"

typedef enum
{
    NodeLiteral,
    NodeBinaryOperator,
    NodeParenthesis,
    NodeUnknown
} node_type_enum;

typedef struct AST_node
{
    struct AST_node *left;
    struct AST_node *right;
    node_type_enum type;
    string_t string;
} AST_node;

typedef struct AST_t
{
    AST_node *root;
} AST_t;

AST_t *parse_lex(token_list_t *token_list);
void put_AST(void);
