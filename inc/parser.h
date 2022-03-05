#pragma once

#include "lexer.h"

typedef enum
{
    NodeUnaryOperator,
    NodeBinaryOperator,
    NodeParenthesis,
    NodeLiteral,
    NodeScope,
    NodeUnknown
} node_type_enum;

typedef struct AST_node
{
    struct AST_node *left;
    struct AST_node *right;
    node_type_enum type;
    union // This contains extra information that might be relevant to some
          // nodes depending on the node type
    {
        struct // NodeParenthesis
        {
            struct AST_node *old_root; // Old root of parenthesis
        };
        struct // NodeScope
        {
            list_t scope_list; // List of nodes contained in a scope
        };
    };
    string_t string;
} AST_node;

typedef struct AST_t
{
    AST_node *root;
} AST_t;

AST_t *parse_lex(token_list_node_t *token_list);
void put_AST(void);
