/** lexer.c
 * @brief Utilities for parsing file input
 *
 * STATE: AST
 */

#include "error_handling.h"
#include "lexer.h"
#include "parser.h"
#include "string_t.h"

/**
 * STATE: This holds the AST for parsing
 */
static AST_t AST = {NULL};

/**
 * @brief Get the priority of an operator
 * @param c The character representing the operator
 * @note Don't rely on the absolute value here, just the relative values.
 */
static int get_priority(char c)
{
    // TODO This shouldn't be a character, because operators like '&&' will
    // need more information.
    switch (c)
    {
    case '*':
    case '/':
    case '%':
        return 100;
    case '+':
    case '-':
        return 10;
    default:
        return -1;
    }
}

/**
 * @brief Allocate an AST node
 * @param token The kind of token to add
 * @param string The string_t to copy
 * @return The new AST node
 */
static AST_node *get_AST_node(token_type_struct token, string_t *string)
{
    // Allocate our node and space for the string
    AST_node *return_node = malloc(sizeof(*return_node));
    ASSERT(return_node != NULL, "Failed to allocate token node\n");

    get_string_clone(&return_node->string, string);

    return_node->left = NULL;
    return_node->right = NULL;
    return_node->token.type = token.type;

    return return_node;
}

/**
 * @brief Deallocate an AST node
 * @param current_AST_node The AST node to free, after freeing its children
 */
static void put_AST_node_and_children(AST_node *current_AST_node)
{
    if (current_AST_node == NULL)
    {
        return;
    }
    put_AST_node_and_children(current_AST_node->left);
    put_AST_node_and_children(current_AST_node->right);
    put_string(&current_AST_node->string);
    free(current_AST_node);
}

/**
 * @brief Deallocate the entire AST
 */
void put_AST()
{
    if (AST.root != NULL)
    {
        put_AST_node_and_children(AST.root);
    }
    AST.root = NULL;
}

/**
 * @brief Build an AST from a list of tokens
 * @param token_list The list of token to use to build the AST
 * @return The root of the AST
 */
AST_t *parse_lex(token_list_t *token_list)
{
    AST_node *current_AST_node;
    AST_node *temp_AST_node, *prev_AST_node;

    // Place each token into an AST in order
    token_list_node *elem, *temp;
    for_each_token_node(elem, temp, token_list)
    {
        current_AST_node = get_AST_node(elem->token, &elem->string);

        if (AST.root == NULL)
        {
            AST.root = current_AST_node;
            continue;
        }

        switch (current_AST_node->token.type)
        {
        case TokenNumber:
            // If this is the first token, it is an L value
            if (AST.root == NULL)
            {
                AST.root = current_AST_node;
                break;
            }
            // Else, this can be placed in the first open R value location
            temp_AST_node = AST.root;
            while (temp_AST_node != NULL && temp_AST_node->right != NULL
                   && temp_AST_node->token.type == TokenBinaryOperator)
            {
                temp_AST_node = temp_AST_node->right;
            }
            temp_AST_node->right = current_AST_node;
            break;
        case TokenBinaryOperator:
            ASSERT(AST.root != NULL,
                   "Can't begin and AST with a binary operator\n");

            // Search for the appropriate place for this operation by proirity
            temp_AST_node = AST.root;
            prev_AST_node = NULL;
            while (temp_AST_node->token.type == TokenBinaryOperator
                   && get_priority(temp_AST_node->string.string[0])
                          < get_priority(current_AST_node->string.string[0]))
            {
                // This node is of a lower priority, iterate to the right
                prev_AST_node = temp_AST_node;
                temp_AST_node = temp_AST_node->right;
            }

            // We've found the location to put our new node. Rotate the lower
            // priority node to the left.
            current_AST_node->left = temp_AST_node;

            // Special case if this is to become the new root
            if (prev_AST_node == NULL)
            {
                AST.root = current_AST_node;
            }

            // Else, replace the old L-value with our new node
            else
            {
                prev_AST_node->right = current_AST_node;
            }

            break;
        default:
            printf("TOOD handle other tokens in parse_lex\n");
            exit(EXIT_FAILURE);
        }
    }

    return &AST;
}
