/** main.c
 * @brief Main file, argument parser and error handling
 *
 * STATE: program arguments
 */

#include "error_handling.h"
#include "file.h"
#include "lexer.h"
#include "parser.h"

#include <ctype.h>       // `isprint`
#include <getopt.h>      // Option parsing
#include <stdnoreturn.h> // `noreturn`

//////////////////////////////////////////////////////////////////////////////
// Argument Parsing
//////////////////////////////////////////////////////////////////////////////

/**
 * @brief Short CLI options, with a ':' after if the option takes args
 */
static char const *short_options = "t:h";

/**
 * @brief Long CLI options
 * @note long arg, argument requirements, flags (0), short arg
 */
static struct option const long_options[] = {
    {"threads", required_argument, 0, 't'},
    {   "help",       no_argument, 0, 'h'},
    {        0,                 0, 0,   0}
};

/**
 * @brief Print usage and exit in case of input error and exit
 * @param[in] program_name The name of the program, pass with argv[0]
 * @note This function does not return
 */
noreturn static void usage(char const *program_name)
{
    printf("Usage: '%s [options] filename'\n", program_name);
    printf("\n"
           "attis is a compiler for the language Cybele.\n"
           "\n"
           "Options:\n"
           "    {-h || --help}      Show usage\n"
           "    {-t || --threads}   The maximum number of threads\n");
    exit(EXIT_SUCCESS);
}

//////////////////////////////////////////////////////////////////////////////
// This section is only for testing
//////////////////////////////////////////////////////////////////////////////

#include <features.h>
#include <math.h>

static double TEST_eval_AST_node(AST_node *node)
{
    long ret;
    double temp;
    if (node->type == NodeBinaryOperator)
    {
        switch (node->string.string[0])
        {
        case '+':
            return TEST_eval_AST_node(node->left)
                   + TEST_eval_AST_node(node->right);
        case '-':
            return TEST_eval_AST_node(node->left)
                   - TEST_eval_AST_node(node->right);
        case '*':
            return TEST_eval_AST_node(node->left)
                   * TEST_eval_AST_node(node->right);
        case '/':
            temp = TEST_eval_AST_node(node->right);
            if (temp < 0.01 && temp > -0.01)
            {
                printf("AST divide by 0 error\n");
                exit(EXIT_FAILURE);
            }
            return TEST_eval_AST_node(node->left) / temp;
        case '%':
            temp = TEST_eval_AST_node(node->right);
            if (temp < 0.01 && temp > -0.01)
            {
                printf("AST divide by 0 error\n");
                exit(EXIT_FAILURE);
            }
            return fmod(TEST_eval_AST_node(node->left), temp);
        default:
            printf("Unknown AST token in eval\n");
            exit(EXIT_FAILURE);
        }
    }
    else if (node->type == NodeLiteral)
    {
        ret = strtol(node->string.string, NULL, 10);
        return (double)ret;
    }
    else if (node->type == NodeParenthesis)
    {
        return TEST_eval_AST_node(node->right);
    }
    else
    {
        printf("Unknown AST token in eval\n");
        exit(EXIT_FAILURE);
    }
}

//////////////////////////////////////////////////////////////////////////////
// Main
//////////////////////////////////////////////////////////////////////////////

/**
 * @brief Function called before exiting
 */
static void exit_program()
{
    put_file();
    put_token_node_list();
    put_AST();
}

/**
 * @brief Main function of attis
 * @param[in] argc The number of options passed to the program
 * @param[in] argv The list of string options passed to the program
 * @return 0 on success
 */
int main(int argc, char *argv[])
{
    ASSERT(!atexit(exit_program), "Failed to register atexit\n");
    opterr = 0; // Setting this to 0 prevents getopt_long from printing errors

    { // Parse option arguments
        int opt;
        while ((opt = getopt_long(argc, argv, short_options, long_options, 0))
               != EOF)
        {
            switch (opt)
            {
            case 't':
                // TODO
                fprintf(stderr, "TODO: support multi-threading\n");
                exit(EXIT_FAILURE);
            case 'h':
                usage(argv[0]);
            case '?':
                switch (optopt)
                {
                case 't':
                    fprintf(stderr, "-%c must be passed a value\n", optopt);
                    exit(EXIT_FAILURE);
                default:
                    if (isprint(optopt))
                        fprintf(stderr, "Unknown option -%c\n", optopt);
                    else
                        fprintf(stderr, "Unknown option with hex code 0x%x\n",
                                optopt);
                    exit(EXIT_FAILURE);
                }
            default:
                break;
            }
        }
    }

    FILE *input_file = NULL;

    { // Parse file arguments
        ASSERT(argc > optind, "No input files given\n");

        if (argc > optind + 1)
        {
            // TODO
            fprintf(stderr, "TODO: support multiple input files\n");
            exit(EXIT_FAILURE);
        }

        input_file = get_file(argv[optind], "r");
    }

    token_list_t *token_list = NULL;

    { // Lexer
        token_list = lex_file(input_file);
    }

    AST_t *ast = NULL;

    { // Parser
        ast = parse_lex(token_list);
    }

    //////////////////////////////////////////////////////////////////////////
    // This section is only for testing
    //////////////////////////////////////////////////////////////////////////

    double answer = TEST_eval_AST_node(ast->root);
    if (fabs(answer - round(answer)) < 0.01)
    {
        printf("Answer: %ld\n", (long)answer);
    }
    else
    {
        printf("Answer: %f\n", answer);
    }

    //////////////////////////////////////////////////////////////////////////
    //
    //////////////////////////////////////////////////////////////////////////

    return 0;
}
