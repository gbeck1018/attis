/** lexer.c
 * @brief Utilities for lexing a file input
 *
 * STATE: token_list
 */

#include "error_handling.h"
#include "lexer.h"
#include "string_t.h"

#define FALL_THROUGH __attribute__((fallthrough));

//////////////////////////////////////////////////////////////////////////////
// Token List Structures Definition
//////////////////////////////////////////////////////////////////////////////

/**
 * STATE: This holds the token list for lexing
 */
static token_list_t token_list = {NULL, NULL, 0};

/**
 * @brief Allocate and return a new token node
 * @param[in] input_string The string to copy into the new struct
 * @param[in] reserve_space The ammound of memory to allocate for the
 * string
 * @return The newly allocated node
 */
static token_list_node *get_token_node(char const *input_string,
                                       size_t reserve_space)
{
    // Allocate our node and space for the string
    token_list_node *return_node = malloc(sizeof(*return_node));
    ASSERT(return_node != NULL, "Failed to allocate token node\n");

    get_string(&return_node->string, input_string, reserve_space);

    return_node->next = NULL;
    return_node->prev = NULL;
    return_node->token = TokenUnknown;

    token_list.size += 1;

    return return_node;
}

/**
 * @brief Adds a token_node to the tail of the list
 * @param[in] new_token_node The node to append to the list
 */
static void add_token_node(token_list_node *new_token_node)
{
    new_token_node->next = NULL;
    new_token_node->prev = token_list.tail;

    if (token_list.tail == NULL)
    {
        token_list.head = new_token_node;
    }
    else
    {
        token_list.tail->next = new_token_node;
    }

    token_list.tail = new_token_node;
}

/**
 * @brief Allocate and add a token_node to the tail of the list
 * @param[in] input_string The string to copy into the new struct
 * @param[in] reserve_space The ammound of memory to allocate for the string
 */
static void add_new_token_node(char const *input_string, size_t reserve_space)
{
    add_token_node(get_token_node(input_string, reserve_space));
}

/**
 * @brief Removes and frees a member of the list
 * @param[in] old_token_node The node to remove and free
 */
static void put_token_node(token_list_node *old_token_node)
{
    ASSERT(old_token_node, "Attempting to put NULL\n");

    if (old_token_node->next != NULL)
    {
        old_token_node->next->prev = old_token_node->prev;
    }
    else
    {
        token_list.tail = old_token_node->prev;
    }
    if (old_token_node->prev != NULL)
    {
        old_token_node->prev->next = old_token_node->next;
    }
    else
    {
        token_list.head = old_token_node->next;
    }

    if (old_token_node->string.string != NULL)
    {
        put_string(&old_token_node->string);
    }

    free(old_token_node);

    token_list.size -= 1;
}

/**
 * @brief Deallocate the entire token node list
 */
void put_token_node_list()
{
    token_list_node *elem, *temp;
    elem = token_list.head;
    for_each_token_node_from(elem, temp)
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
 * @return The token list
 */
token_list_t *lex_file(FILE *input_file)
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
                || token_list.tail->token == TokenBinaryOperator
                || token_list.tail->token == TokenOpenParenthesis
                || token_list.tail->token == TokenSemicolon)
            {
                ASSERT(token_list.tail->token != TokenUnaryOperator,
                       "Bad unary operator\n");
                add_new_token_node(NULL, 2);
                token_list.tail->token = TokenUnaryOperator;
                add_character(&token_list.tail->string,
                              (char)current_character);
                break;
            }
            // If it isn't a negative sign, it's a simple subtraction/add sign.
            FALL_THROUGH
        case '*':
        case '/':
        case '%':
            // Check that we're coming after a number or expression
            ASSERT(token_list.tail != NULL
                       && (token_list.tail->token == TokenCloseParenthesis
                           || token_list.tail->token == TokenLiteral),
                   "Bad binary operator\n");
            add_new_token_node(NULL, 2);
            token_list.tail->token = TokenBinaryOperator;
            add_character(&token_list.tail->string, (char)current_character);
            break;
        case '(':
            if (token_list.tail != NULL)
            {
                ASSERT(token_list.tail->token != TokenCloseParenthesis
                           && token_list.tail->token != TokenLiteral,
                       "Bad open parenthesis\n");
            }
            add_new_token_node(NULL, 2);
            token_list.tail->token = TokenOpenParenthesis;
            add_character(&token_list.tail->string, (char)current_character);
            break;
        case ')':
            ASSERT(token_list.tail != NULL
                       && (token_list.tail->token == TokenCloseParenthesis
                           || token_list.tail->token == TokenLiteral),
                   "Bad closed parenthesis\n");
            add_new_token_node(NULL, 2);
            token_list.tail->token = TokenCloseParenthesis;
            add_character(&token_list.tail->string, (char)current_character);
            break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            // Check to see if we're appending characters or making a new token
            if (token_list.tail == NULL
                || token_list.tail->token != TokenLiteral)
            {
                add_new_token_node(NULL, 3);
                token_list.tail->token = TokenLiteral;
            }
            add_character(&token_list.tail->string, (char)current_character);
            break;
        case ';':
            ASSERT(token_list.tail == NULL
                       || token_list.tail->token == TokenCloseParenthesis
                       || token_list.tail->token == TokenLiteral
                       || token_list.tail->token == TokenSemicolon,
                   "Bad semicolon\n");
            add_new_token_node(NULL, 2);
            token_list.tail->token = TokenSemicolon;
            add_character(&token_list.tail->string, (char)current_character);
            break;
        default:
            printf("Unknown Character %c\n", current_character);
            exit(EXIT_FAILURE);
        }
    }

    if (token_list.tail != NULL)
    {
        ASSERT(token_list.tail->token == TokenCloseParenthesis
                   || token_list.tail->token == TokenLiteral
                   || token_list.tail->token == TokenSemicolon,
               "Invalid EOF\n");
    }

    return &token_list;
}
