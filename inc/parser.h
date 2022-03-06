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

typedef struct AST_node_t
{
    struct AST_node_t *left;
    struct AST_node_t *right;
    struct AST_node_t *next;
    struct AST_node_t *parent_node;
    struct AST_node_t *parent_scope;
    node_type_enum type;
    string_t string;
    union // This contains extra information that might be relevant to some
          // nodes depending on the node type
    {
        struct // NodeParenthesis
        {
            struct AST_node_t *old_root; // Old root of parenthesis
        };
        struct // NodeScope
        {
            struct AST_node_t *list_head;
            struct AST_node_t *list_tail;
        };
    };
} AST_node_t;

typedef struct AST_t
{
    AST_node_t *root;
} AST_t;

AST_t *parse_lex(token_list_node_t *token_list);
void put_AST(void);
