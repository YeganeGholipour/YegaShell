/**
 * @file env_utils.h
 * @brief Utilities for handling environment variables.
 *         Adds, deletes, and updates environment variables.
 *         Creates the envp array of pointers.
 *         Envoronment variables are stored in a hash table
 *         implemented as a linked list.
 * @author Yegane Gholipur
 * @date 2025-06-06
 */

#ifndef ENV_UTILS_H
#define ENV_UTILS_H

/**
 * @def TABLESIZE
 * @brief The size of the hash table used to store environment variables.
 */
#define TABLESIZE 100

/**
 * @struct Variable
 * @brief Represents an environment variable.
 *
 * This struct contains the key, value, and export status of an environment
 * variable. It also contains a pointer to the next variable in the linked list.
 */
typedef struct Variable {
  char *key;
  char *value;
  int exported;
  struct Variable *next;
} Variable;

/**
 * @var variable_table
 * @brief An array of pointers to the heads of the linked lists of environment
 * variables.
 *
 * This array is used to implement the hash table.
 */
extern Variable *variable_table[];

/**
 * @brief Looks up an environment variable by its key.
 *
 * @param key The key of the variable to look up.
 * @return A pointer to the variable if found, or NULL if not found.
 */
Variable *lookup(const char *key);

/**
 * @brief Adds or updates an environment variable.
 *
 * @param key The key of the variable to add or update.
 * @param value The value of the variable to add or update.
 * @param exported A flag indicating whether the variable should be exported.
 * @return A pointer to the added or updated variable, or NULL on error.
 */
Variable *add_variable(const char *key, const char *value, int exported);

/**
 * @brief Removes an environment variable by its key.
 *
 * @param key The key of the variable to remove.
 * @return 0 on success, -1 if the variable is not found.
 */
int remove_variable(const char *key);

/**
 * @brief Dumps all environment variables to the console.
 */
void dump_variables(void);

/**
 * @brief Checks whether a string is a valid identifier.
 *
 * @param s The string to check.
 * @return 1 if the string is a valid identifier, 0 otherwise.
 */
int is_valid_identifier(const char *s);

/**
 * @brief Parses a key-value pair from a string.
 *
 * @param input The string to parse.
 * @param key_out A pointer to a pointer to store the key.
 * @param val_out A pointer to a pointer to store the value.
 * @return 0 on success, -1 if the input string is invalid.
 */
int parse_key_value_inplace(char *input, char **key_out, char **val_out);

/**
 * @brief Gets the full path of a command.
 *
 * @param command The command to get the full path for.
 * @return The full path of the command, or NULL if not found.
 */
char *get_full_path(const char *command);

/**
 * @brief Builds an array of environment variable pointers.
 *
 * @return An array of environment variable pointers, or NULL on error.
 */
char **build_envp(void);


void free_variable_table(void);

int initialize_envp(char ***envpp);

void free_envp(char **envp);

#endif