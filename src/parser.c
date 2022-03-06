/** lexer.c
 * @brief Utilities for parsing file input
 *
 * STATE: AST
 */

#include "error_handling.h"
#include "lexer.h"
#include "parser.h"
#include "type/string_t.h"

static AST_t AST = {NULL};

static AST_node_t **get_next_search()
{
    if (AST.root != NULL)
    {
        if (AST.root->type == NodeParenthesis)
        {
            return &AST.root->right;
        }
        if (AST.root->type == NodeScope)
        {
            return &AST.root->right;
        }
    }
    return &AST.root;
}

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
        ASSERT(0, "Unknown operator priority %s\n", s->string);
        return -1;
    }
}

/**
 * @brief Return true if the LHS is lower priority than the RHS
 * @param LHS The node to check if it is lower priority
 * @param RHS The node to check against
 */
static int lower_priority(AST_node_t const *LHS, AST_node_t const *RHS)
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
static AST_node_t *get_AST_node(token_list_node_t *node, node_type_enum type,
                                AST_node_t *current_scope)
{
    // Allocate our node and space for the string
    AST_node_t *return_node = calloc(1, sizeof(*return_node));
    ASSERT(return_node != NULL, "Failed to allocate token node\n");

    if (node != NULL)
    {
        get_string_clone(&return_node->string, &node->string);
        return_node->line_number = node->line_number;
        return_node->column_number = node->column_number;
    }
    else
    {
        get_string(&return_node->string, "__GLOBAL_SCOPE__", NO_EXTRA_SPACE);
        return_node->line_number = -1;
        return_node->column_number = -1;
    }

    return_node->type = type;
    return_node->parent_scope = current_scope;

    return return_node;
}

/**
 * @brief Deallocate an AST node
 * @param current_AST_node The AST node to free, after freeing its children
 */
static void put_AST_node_and_children(AST_node_t *current_AST_node)
{
    if (current_AST_node == NULL)
    {
        return;
    }
    if (current_AST_node->type == NodeScope)
    {
        AST_node_t *temp = current_AST_node->list_head;
        AST_node_t *next;
        while (temp != NULL)
        {
            next = temp->next;
            put_AST_node_and_children(temp);
            temp = next;
        }
    }
    put_AST_node_and_children(current_AST_node->left);
    put_AST_node_and_children(current_AST_node->right);
    put_string(&current_AST_node->string);
    free(current_AST_node);
}

static void print_AST(AST_node_t *root, int space)
{
    if (root == NULL)
    {
        return;
    }

    // Increase distance between levels
    space += 2;

    // Process right child first
    print_AST(root->right, space);

    // Print current node after space
    // count
    for (int i = 2; i < space; i++)
        printf(" ");

    if (root->type == NodeScope)
    {
        AST_node_t *temp = root->list_head;
        while (temp != NULL)
        {
            print_AST(temp, space);
            temp = temp->next;
        }
    }
    printf("%s\n", root->string.string);

    // Process left child
    print_AST(root->left, space);
}

/**
 * @brief Deallocate the entire AST
 */
void put_AST()
{
    AST_node_t *temp_AST_node = AST.root;
    if (temp_AST_node != NULL)
    {
        while (temp_AST_node->parent_scope != NULL)
        {
            temp_AST_node = temp_AST_node->parent_scope;
        }
        // print_AST(temp_AST_node, 0);
        put_AST_node_and_children(temp_AST_node);
    }
    AST.root = NULL;
}

static void find_and_place_operator(AST_node_t *current_AST_node)
{
    AST_node_t **current_root = get_next_search();
    if (*current_root == NULL)
    {
        current_AST_node->parent_node = AST.root;
        *current_root = current_AST_node;
        AST.root = current_AST_node;
    }
    else
    {
        AST_node_t *old_root = *current_root;
        AST_node_t *prev_root = old_root->parent_node;
        while (old_root
               && (old_root->type == NodeBinaryOperator
                   || old_root->type == NodeUnaryOperator)
               && lower_priority(old_root, current_AST_node))
        {
            prev_root = old_root;
            old_root = old_root->right;
        }
        current_AST_node->parent_node = prev_root;
        if (old_root == *current_root)
        {
            AST.root = current_AST_node;
        }
        prev_root->right = current_AST_node;
        current_AST_node->left = old_root;
    }
}

static void find_and_place_value(AST_node_t *current_AST_node)
{
    AST_node_t **current_root = get_next_search();
    if (*current_root == NULL)
    {
        current_AST_node->parent_node = AST.root;
        *current_root = current_AST_node;
        AST.root = current_AST_node;
    }
    else
    {
        AST_node_t *temp_node = *current_root;
        while (temp_node->right
               && (temp_node->right->type == NodeUnaryOperator
                   || temp_node->right->type == NodeBinaryOperator))
        {
            temp_node = temp_node->right;
        }
        current_AST_node->parent_node = temp_node;
        temp_node->right = current_AST_node;
    }
}

/**
 * @brief Build an AST from a list of tokens
 * @param token_list The list of token to use to build the AST
 * @return The root of the AST
 */
AST_t *parse_lex(token_list_node_t *token_list)
{
    AST.root = get_AST_node(NULL, NodeScope, NULL);
    AST_node_t *current_AST_node = AST.root;
    AST_node_t *current_scope = AST.root;

    AST_node_t *temp_AST_node;

    int parenthesis_depth = 0;

    // Place each token into an AST in order
    token_list_node_t *elem, *temp;
    elem = token_list;
    for_each_element_from(elem, temp, token_list_node_t, list)
    {
        // printf("Parse %s\n", elem->string.string);
        switch (elem->token)
        {
        case TokenUnaryOperator:
            current_AST_node
                = get_AST_node(elem, NodeUnaryOperator, current_scope);
            find_and_place_operator(current_AST_node);
            break;
        case TokenBinaryOperator:
            current_AST_node
                = get_AST_node(elem, NodeBinaryOperator, current_scope);
            find_and_place_operator(current_AST_node);
            break;
        case TokenOpenParenthesis:
            parenthesis_depth += 1;
            current_AST_node
                = get_AST_node(elem, NodeParenthesis, current_scope);
            current_AST_node->old_root = AST.root;
            find_and_place_value(current_AST_node);
            AST.root = current_AST_node;
            break;
        case TokenCloseParenthesis:
            parenthesis_depth -= 1;
            ASSERT(parenthesis_depth >= 0, "Unbalanced parenthesis\n");
            temp_AST_node = AST.root;
            if (AST.root->type == NodeParenthesis)
            {
                AST.root = temp_AST_node->old_root;
                temp_AST_node->old_root = NULL;
            }
            else
            {
                AST.root = temp_AST_node->parent_node->old_root;
                temp_AST_node->parent_node->old_root = NULL;
            }
            break;
        case TokenLiteral:
            current_AST_node = get_AST_node(elem, NodeLiteral, current_scope);
            find_and_place_value(current_AST_node);
            break;
        case TokenSemicolon:
            ASSERT(parenthesis_depth == 0, "Unbalanced parenthesis\n");
            if (current_scope->list_head == NULL)
            {
                current_scope->list_head = *get_next_search();
            }
            else
            {
                current_scope->list_tail->next = *get_next_search();
            }
            current_scope->list_tail = *get_next_search();
            *get_next_search() = current_scope;
            current_scope->right = NULL;
            break;
        default:
            printf("TODO handle other tokens in parse_lex\n");
            exit(EXIT_FAILURE);
        }
        // printf("Parse %s\n", elem->string.string);
        // print_AST(AST.root, 0);
    }
    ASSERT(parenthesis_depth == 0, "Unbalanced parenthesis\n");
    return &AST;
}
