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
 * @param c The character to evaluate
 * @note Don't rely on the absolute values here, only the relative ones
 */
static int get_operator_priority(char c)
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
    ASSERT(LHS->type == NodeBinaryOperator && RHS->type == NodeBinaryOperator,
           "Incompadible priority comparison\n");
    return get_operator_priority(LHS->string.string[0])
           < get_operator_priority(RHS->string.string[0]);
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

    get_string_clone(&return_node->string, &node->string);

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

/*static void TEMP_print_ast(AST_node *node)
{
    if (node == NULL)
    {
        return;
    }
    printf("Val: %s\n", node->string.string);
    printf("Left:\n");
    TEMP_print_ast(node->left);
    printf("Right:\n");
    TEMP_print_ast(node->right);
}*/

static token_list_node *rec_elem;

static AST_node *recursive_parse(token_list_node *start)
{
    AST_node *root_AST_node = NULL;

    AST_node *current_AST_node;
    AST_node *temp_AST_node, *prev_AST_node;

    // Place each token into an AST in order
    token_list_node *elem, *temp;
    elem = start;
    for_each_token_node_from(elem, temp)
    {
        printf("P: %s\n", elem->string.string);
        switch (elem->token.type)
        {
        case TokenBinaryOperator:
            ASSERT(root_AST_node != NULL,
                   "Can't begin and AST with a binary operator\n");
            current_AST_node = get_AST_node(elem, NodeBinaryOperator);

            // Search for the appropriate place for this operation by proirity
            temp_AST_node = root_AST_node;
            prev_AST_node = NULL;
            while (temp_AST_node->type == NodeBinaryOperator
                   && lower_priority(temp_AST_node, current_AST_node))
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
                root_AST_node = current_AST_node;
            }

            // Else, replace the old L-value with our new node
            else
            {
                prev_AST_node->right = current_AST_node;
            }
            break;
        case TokenNumber:
            current_AST_node = get_AST_node(elem, NodeLiteral);
            // If this is the first token, it is an L value
            if (root_AST_node == NULL)
            {
                root_AST_node = current_AST_node;
                break;
            }
            // Else, this can be placed in the first open R value location
            temp_AST_node = root_AST_node;
            while (temp_AST_node != NULL && temp_AST_node->right != NULL
                   && temp_AST_node->type == NodeBinaryOperator)
            {
                temp_AST_node = temp_AST_node->right;
            }
            temp_AST_node->right = current_AST_node;
            break;
        case TokenOpenParenthesis:
            current_AST_node = get_AST_node(elem, NodeParenthesis);
            current_AST_node->right = recursive_parse(elem->next);

            // TODO we need to update 'elem' to take into account the
            // already parsed tokens in the recursive case. This is not the
            // right way to do this, it's just temporary.
            temp = rec_elem->next;

            if (root_AST_node == NULL)
            {
                root_AST_node = current_AST_node;

                break;
            }
            temp_AST_node = root_AST_node;
            while (temp_AST_node != NULL && temp_AST_node->right != NULL
                   && temp_AST_node->type == NodeBinaryOperator)
            {
                temp_AST_node = temp_AST_node->right;
            }
            temp_AST_node->right = current_AST_node;
            break;
        case TokenCloseParenthesis:
            // We're somewhere down a recursive rabbit hole, move one level up.
            rec_elem = elem;
            return root_AST_node;
        default:
            printf("TODO handle other tokens in parse_lex\n");
            exit(EXIT_FAILURE);
        }
    }

    return root_AST_node;
}

/**
 * @brief Build an AST from a list of tokens
 * @param token_list The list of token to use to build the AST
 * @return The root of the AST
 */
AST_t *parse_lex(token_list_t *token_list)
{
    AST_node *node = recursive_parse(token_list->head);
    AST.root = node;
    // TEMP_print_ast(AST.root);
    return &AST;
}
