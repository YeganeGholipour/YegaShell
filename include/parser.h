/**
 * @file parser.h
 * @brief Implements functionalities for parsing user command. Related to
 * parsing phase. Includes functions for allocating and freeing memory for
 * commands. Also includes functions for validating the position of special
 * characters.
 * @author Yegane Gholipur
 * @date 2025-06-06
 */

#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>

/**
 * @def OPERATORSLEN
 * @brief The number of special operator characters recognized by the parser.
 */
#define OPERATORSLEN 4

/**
 * @struct Command
 * @brief Represents a parsed command.
 *
 * This struct contains information about a parsed command, including the
 * command arguments, input and output file names, and flags for output append
 * and background execution.
 */
typedef struct {
  char **argv;
  char *infile;
  char *outfile;
  int append_output;
  int background;
} Command;

/**
 * @brief Frees the memory allocated for a Command struct.
 *
 * This function releases the memory allocated for the Command struct and its
 * members.
 *
 * @param cmd The Command struct to free.
 */
void free_struct_memory(Command *cmd);

/**
 * @brief Parses a command from an array of tokens.
 *
 * This function takes an array of tokens and parses them into a Command struct.
 *
 * @param tokens The array of tokens to parse.
 * @param command_ptr A pointer to the Command struct to fill.
 * @param num_tokens The number of tokens in the array.
 * @return 0 on success, -1 on failure.
 */
int parse(char *tokens[], Command **command_ptr, size_t num_tokens);

/**
 * @brief Prints the contents of a Command struct.
 *
 * This function prints the command arguments, input and output file names, and
 * flags for output append and background execution.
 *
 * @param command_ptr The Command struct to print.
 */
void print_command_ptr(Command *command_ptr);

/**
 * @brief Splits a command into sub-commands separated by pipes.
 *
 * This function takes an array of tokens and splits them into sub-commands
 * separated by pipes.
 *
 * @param tokens The array of tokens to split.
 * @param num_tokens The number of tokens in the array.
 * @param cmd_ptr A pointer to the Command struct to fill.
 * @param indx The index of the current token.
 * @return The index of the next token, or -1 on failure.
 */
int split_on_pipe(char *tokens[], size_t num_tokens, Command **cmd_ptr,
                  int indx);

/**
 * @brief Gets the raw input from the user.
 *
 * This function reads a line of input from the user and returns it as a string.
 *
 * @param line_buffer A buffer to store the input line.
 * @return The input line as a string.
 */
char *get_raw_input(char *line_buffer);

/**
 * @brief Checks if a character is a valid background character.
 *
 * This function checks if a character is a valid background character.
 *
 * @param tokens The array of tokens to check.
 * @param num_tokens The number of tokens in the array.
 * @return 1 if the character is valid, 0 otherwise.
 */
int is_background_char_valid(char *tokens[], size_t num_tokens);

#endif