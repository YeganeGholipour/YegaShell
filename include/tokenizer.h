/**
 * @file tokenizer.h
 * @brief Implements functionality for tokenization of user input. Related to
 * tokenization phase.
 * @author Yegane Gholipur Gholipour
 * @date 2025-06-06
 */

#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stddef.h>

/**
 * @def MAXTOKENS
 * @brief The maximum number of tokens that can be stored in the tokens array.
 */
#define MAXTOKENS 1000

/**
 * @def MAXLEN
 * @brief The maximum length of a single token.
 */
#define MAXLEN 100

/**
 * @def SPECIALCHARLEN
 * @brief The number of special characters recognized by the tokenizer.
 */
#define SPECIALCHARLEN 4

/**
 * @brief Reads a line of input from the user and stores it in the provided
 * buffer.
 *
 * @param line_buffer A pointer to a pointer to store the input line.
 * @param read A pointer to an ssize_t to store the number of characters read.
 * @param buffsize A pointer to a size_t to store the size of the buffer.
 * @return 0 on success, -1 on failure.
 */
int prompt_and_read(char **line_buffer, ssize_t *read, size_t *buffsize);

/**
 * @brief Prints the tokens in the provided array.
 *
 * @param tokens An array of tokens to print.
 * @param token_num The number of tokens in the array.
 */
void print_tokens(char *tokens[], int token_num);

/**
 * @brief Tokenizes a line of input into an array of tokens.
 *
 * @param line The line of input to tokenize.
 * @param tokens An array to store the tokens.
 * @param max_tokens The maximum number of tokens that can be stored in the
 * array.
 * @param max_len The maximum length of a single token.
 * @param token_num A pointer to an int to store the number of tokens found.
 * @return 0 on success, -1 on failure.
 */
int tokenize_line(char *line, char *tokens[], int max_tokens, int max_len,
                  int *token_num);

/**
 * @brief Frees the memory allocated for the tokens in the provided array.
 *
 * @param tokens An array of tokens to free.
 * @param token_num The number of tokens in the array.
 */
void free_memory(char *tokens[], int token_num);

#endif