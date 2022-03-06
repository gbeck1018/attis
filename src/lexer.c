/** lexer.c
 * @brief Utilities for lexing a file input
 *
 * STATE: token_list
 */

#include "error_handling.h"
#include "lexer.h"

#include <ctype.h>

#define FALL_THROUGH __attribute__((fallthrough));

//////////////////////////////////////////////////////////////////////////////
// Token List Structures Definition
//////////////////////////////////////////////////////////////////////////////

/**
 * STATE: This holds the token list for lexing
 */
static list_t token_list = {NULL, NULL};

/**
 * @brief Allocate and return a new token node
 * @param[in] input_string The string to copy into the new struct
 * @param[in] reserve_space The ammound of memory to allocate for the
 * string
 * @param[in] type The type of the new token
 * @return The newly allocated node
 */
static token_list_node_t *get_token_node(char const *input_string,
                                         size_t reserve_space,
                                         token_type_enum type)
{
    // Allocate our node and space for the string
    token_list_node_t *return_node = malloc(sizeof(*return_node));
    ASSERT(return_node != NULL, "Failed to allocate token node\n");

    get_string(&return_node->string, input_string, reserve_space);

    return_node->list.next = NULL;
    return_node->list.prev = NULL;
    return_node->token = type;

    return return_node;
}

/**
 * @brief Allocate and add a token_node to the tail of the list
 * @param[in] input_string The string to copy into the new struct
 * @param[in] reserve_space The ammound of memory to allocate for the string
 * @param[in] type The type of token to add
 */
static void add_new_token_node(char const *input_string, size_t reserve_space,
                               token_type_enum type)
{
    add_element_to_end(
        &get_token_node(input_string, reserve_space, type)->list, &token_list);
}

/**
 * @brief Removes and frees a member of the list
 * @param[in] old_token_node The node to remove and free
 */
static void put_token_node(token_list_node_t *old_token_node)
{
    ASSERT(old_token_node, "Attempting to put NULL\n");

    remove_element(&old_token_node->list, &token_list);

    if (old_token_node->string.string != NULL)
    {
        put_string(&old_token_node->string);
    }

    free(old_token_node);
}

/**
 * @brief Deallocate the entire token node list
 */
void put_token_node_list()
{
    token_list_node_t *elem, *temp;
    elem = token_node(token_list.head);
    for_each_element_from(elem, temp, token_list_node_t, list)
    {
        put_token_node(elem);
    }
}

//////////////////////////////////////////////////////////////////////////////
// Lexing
//////////////////////////////////////////////////////////////////////////////

/**
 * @brief Generate a token list for a given file
 * @param[in] input_file An open file to read from
 * @return The token list head
 */
token_list_node_t *lex_file(FILE *input_file)
{
    int current_character = EOF;

    ASSERT(input_file != NULL, "Lexer given invalid file input\n");

    for (;;)
    {
        current_character = getc(input_file);

        // Return is EOF either if the end of the file was reached or there was
        // an error
        if (current_character == EOF)
        {
            ASSERT(!errno, "Error reading input file\n");
            break;
        }
        printf("Lex %c\n", current_character);
        if (isdigit(current_character))
        {
            // Check to see if we're appending characters or making a new token
            if (token_list.tail == NULL
                || token_node(token_list.tail)->token != TokenLiteral)
            {
                ASSERT(token_node(token_list.tail)->token
                           != TokenCloseParenthesis,
                       "No operator before number\n");
                add_new_token_node(NULL, 3, TokenLiteral);
            }
            add_character(&token_node(token_list.tail)->string,
                          (char)current_character);
            continue;
        }

        // Parse the token associated with the current character
        switch (current_character)
        {
        case '\r':
            /*current_token = TokenCR;
            printf("Token: TokenCR\n");*/
            break;
        case '\n':
            /*current_token = TokenLF;
            printf("Token: TokenLF\n");*/
            break;
        case '-':
        case '+':
            // We need a special case if this is a negative/plus sign
            if (token_list.tail == NULL
                || token_node(token_list.tail)->token == TokenBinaryOperator
                || token_node(token_list.tail)->token == TokenOpenParenthesis
                || token_node(token_list.tail)->token == TokenSemicolon)
            {
                if (token_list.tail)
                {
                    ASSERT(token_node(token_list.tail)->token
                               != TokenUnaryOperator,
                           "Bad unary operator\n");
                }
                add_new_token_node(NULL, 2, TokenUnaryOperator);
                add_character(&token_node(token_list.tail)->string,
                              (char)current_character);
                break;
            }
            // If it isn't a negative sign, it's a simple subtraction/add sign.
            FALL_THROUGH
        case '*':
        case '/':
        case '%':
            // Check that we're coming after a number or expression
            ASSERT(
                token_node(token_list.tail) != NULL
                    && (token_node(token_list.tail)->token
                            == TokenCloseParenthesis
                        || token_node(token_list.tail)->token == TokenLiteral),
                "Bad binary operator\n");
            add_new_token_node(NULL, 2, TokenBinaryOperator);
            add_character(&token_node(token_list.tail)->string,
                          (char)current_character);
            break;
        case '(':
            if (token_list.tail != NULL)
            {
                ASSERT(
                    token_node(token_list.tail)->token != TokenCloseParenthesis
                        && token_node(token_list.tail)->token != TokenLiteral,
                    "Bad open parenthesis\n");
            }
            add_new_token_node(NULL, 2, TokenOpenParenthesis);
            add_character(&token_node(token_list.tail)->string,
                          (char)current_character);
            break;
        case ')':
            ASSERT(
                token_list.tail != NULL
                    && (token_node(token_list.tail)->token
                            == TokenCloseParenthesis
                        || token_node(token_list.tail)->token == TokenLiteral),
                "Bad closed parenthesis\n");
            add_new_token_node(NULL, 2, TokenCloseParenthesis);
            add_character(&token_node(token_list.tail)->string,
                          (char)current_character);
            break;
        case ';':
            ASSERT(token_list.tail == NULL
                       || token_node(token_list.tail)->token
                              == TokenCloseParenthesis
                       || token_node(token_list.tail)->token == TokenLiteral
                       || token_node(token_list.tail)->token == TokenSemicolon,
                   "Bad semicolon\n");
            add_new_token_node(NULL, 2, TokenSemicolon);
            add_character(&token_node(token_list.tail)->string,
                          (char)current_character);
            break;
        default:
            printf("Unknown Character %c\n", current_character);
            exit(EXIT_FAILURE);
        }
    }

    if (token_list.tail != NULL)
    {
        ASSERT(token_node(token_list.tail)->token == TokenCloseParenthesis
                   || token_node(token_list.tail)->token == TokenLiteral
                   || token_node(token_list.tail)->token == TokenSemicolon,
               "Invalid EOF\n");
    }

    return token_node(token_list.head);
}
