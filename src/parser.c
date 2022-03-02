/** lexer.c
 * @brief Utilities for parsing file input
 *
 * STATE: AST
 */

#include "error_handling.h"
#include "lexer.h"
#include "parser.h"
#include "string_t.h"

static AST_t AST = {NULL};

/**
 * @brief Return the relative priority of an operator
 * @param t The type to evaluate
 * @param s The string to evaluate
 * @note Don't rely on the absolute values here, only the relative ones
 */
static int get_operator_priority(node_type_enum t, string_t const *s)
{
    // TODO This shouldn't be a character, because operators like '&&' will
    // need more information.
    switch (t)
    {
    case NodeBinaryOperator:
        switch (s->string[0])
        {
        case '*':
        case '/':
        case '%':
            return 100;
        case '+':
        case '-':
            return 10;
        default:
            ASSERT(0, "Unknown operator priority\n");
            return -1;
        }
    case NodeUnaryOperator:
        switch (s->string[0])
        {
        case '+':
        case '-':
            return 1000;
        default:
            ASSERT(0, "Unknown operator priority\n");
            return -1;
        }
    default:
        ASSERT(0, "Unknown operator priority\n");
        return -1;
    }
}

/**
 * @brief Return true if the LHS is lower priority than the RHS
 * @param LHS The node to check if it is lower priority
 * @param RHS The node to check against
 */
static int lower_priority(AST_node const *LHS, AST_node const *RHS)
{
    return get_operator_priority(LHS->type, &LHS->string)
           < get_operator_priority(RHS->type, &RHS->string);
}

/**
 * @brief Allocate an AST node
 * @param node The node to copy from
 * @param type The type of node to create
 * @return The new AST node
 */
static AST_node *get_AST_node(token_list_node *node, node_type_enum type)
{
    // Allocate our node and space for the string
    AST_node *return_node = malloc(sizeof(*return_node));
    ASSERT(return_node != NULL, "Failed to allocate token node\n");

    if (node != NULL)
    {
        get_string_clone(&return_node->string, &node->string);
    }
    else
    {
        get_string(&return_node->string, NULL, 1);
    }

    return_node->left = NULL;
    return_node->right = NULL;
    return_node->type = type;

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
    if (current_AST_node->type != NodeParenthesis)
    {
        put_AST_node_and_children(current_AST_node->left);
    }
    put_AST_node_and_children(current_AST_node->right);
    put_string(&current_AST_node->string);
    free(current_AST_node);
}

/**
 * @brief Deallocate the entire AST
 */
void put_AST()
{
    AST_node *temp_AST_node = AST.root;
    if (temp_AST_node != NULL)
    {
        while (temp_AST_node->type != NodeRoot)
        {
            temp_AST_node = temp_AST_node->old_root;
        }
        put_AST_node_and_children(temp_AST_node);
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
    size_t parenthesis_depth = 0;

    AST.root = get_AST_node(NULL, NodeRoot);

    AST_node *current_AST_node = NULL;

    AST_node *temp_AST_node, *prev_AST_node;

    // Place each token into an AST in order
    token_list_node *elem, *temp;
    elem = token_list->head;
    for_each_token_node_from(elem, temp)
    {
        printf("Parse: %s\n", elem->string.string);
        switch (elem->token.type)
        {
        case TokenBinaryOperator:
            ASSERT(AST.root->right != NULL,
                   "Can't begin and AST with a binary operator\n");
            current_AST_node = get_AST_node(elem, NodeBinaryOperator);
            // Search for the appropriate place for this operation by proirity
            temp_AST_node = AST.root->right;
            prev_AST_node = AST.root;
            while ((temp_AST_node->type == NodeBinaryOperator
                    || temp_AST_node->type == NodeUnaryOperator)
                   && lower_priority(temp_AST_node, current_AST_node))
            {
                // This node is of a lower priority, iterate to the right
                prev_AST_node = temp_AST_node;
                temp_AST_node = temp_AST_node->right;
            }

            // We've found the location to put our new node. Rotate the lower
            // priority node to the left.
            current_AST_node->left = temp_AST_node;
            prev_AST_node->right = current_AST_node;
            break;
        case TokenUnaryOperator:
            current_AST_node = get_AST_node(elem, NodeUnaryOperator);
            // Search for the appropriate place for this operation by proirity
            temp_AST_node = AST.root->right;
            prev_AST_node = AST.root;
            while (temp_AST_node
                   && (temp_AST_node->type == NodeBinaryOperator
                       || temp_AST_node->type == NodeUnaryOperator)
                   && lower_priority(temp_AST_node, current_AST_node))
            {
                // This node is of a lower priority, iterate to the right
                prev_AST_node = temp_AST_node;
                temp_AST_node = temp_AST_node->right;
            }

            // We've found the location to put our new node. Rotate the lower
            // priority node to the left.
            current_AST_node->left = temp_AST_node;
            prev_AST_node->right = current_AST_node;
            break;
        case TokenNumber:
            current_AST_node = get_AST_node(elem, NodeLiteral);
            // If this is the first token, it is an L value
            if (AST.root->right == NULL)
            {
                AST.root->right = current_AST_node;
                break;
            }
            // Else, this can be placed in the first open R value location
            temp_AST_node = AST.root->right;
            while (temp_AST_node != NULL && temp_AST_node->right != NULL
                   && temp_AST_node->type == NodeBinaryOperator)
            {
                temp_AST_node = temp_AST_node->right;
            }
            temp_AST_node->right = current_AST_node;
            break;
        case TokenOpenParenthesis:
            parenthesis_depth += 1;
            current_AST_node = get_AST_node(elem, NodeParenthesis);
            current_AST_node->old_root = AST.root;

            // If this is the first token, it is an L value
            if (AST.root->right == NULL)
            {
                AST.root->right = current_AST_node;
                AST.root = current_AST_node;
                break;
            }
            // Else, this can be placed in the first open R value location
            temp_AST_node = AST.root->right;
            while (temp_AST_node != NULL && temp_AST_node->right != NULL
                   && temp_AST_node->type == NodeBinaryOperator)
            {
                temp_AST_node = temp_AST_node->right;
            }
            temp_AST_node->right = current_AST_node;

            AST.root = current_AST_node;
            break;
        case TokenCloseParenthesis:
            parenthesis_depth -= 1;
            ASSERT(AST.root->right != NULL, "Invalid close parenthesis\n");
            ASSERT(parenthesis_depth != (size_t)-1,
                   "Unbalanced parenthesis\n");
            ASSERT(current_AST_node, "Improper parenthesis expression\n");
            ASSERT(current_AST_node->type == NodeLiteral,
                   "Improper parenthesis expression\n");
            temp_AST_node = AST.root->old_root;
            AST.root->old_root = NULL;
            AST.root = temp_AST_node;
            break;
        default:
            printf("TODO handle other tokens in parse_lex\n");
            exit(EXIT_FAILURE);
        }
    }
    ASSERT(parenthesis_depth == 0, "Unbalanced parenthesis\n");

    return &AST;
}
