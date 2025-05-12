#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stddef.h>

#define MAXTOKENS 1000
#define MAXLEN 100
#define SPECIALCHARLEN 4  

int prompt_and_read(char **line_buffer, size_t *read);
void print_tokens(char *tokens[], int token_num);
int tokenize_line(char *line, char *tokens[], int max_tokens, int max_len,
                  int *token_num);
void freeMemory(char *tokens[], int token_num);

#endif