#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shell.h"

char *lsh_read_line(void)
{
    int buffsize = LSH_RL_BUFFSIZE;
    int pos = 0;
    char *buffer = malloc(sizeof(char) * buffsize);

    if (buffer == NULL)
    {
        printf("Error in allocating!");
        return NULL;
    }

    int ch;

    while (1)
    {
        ch = getchar();

        if (ch == EOF || ch == '\n')
        {
            buffer[pos] = '\0';
            return buffer;
        }
        else
        {
            buffer[pos] = ch;
        }

        pos++;

        if (pos >= buffsize)
        {
            buffsize += LSH_RL_BUFFSIZE;
            buffer = realloc(buffer, buffsize);

            if (buffer == NULL)
            {
                printf("Error in allocating!");
                return NULL;
            }
        }
    }
}

char **lsh_split_line(char *line)
{
    int buffsize = LSH_TOKEN_BUFFSIZE;
    int pos = 0;
    char **tokens = malloc(buffsize * sizeof(char *));

    if (tokens == NULL)
    {
        printf("Error in allocating!");
        return NULL;
    }

    char *token;
    token = strtok(line, LSH_TOK_DELIM);

    while (token != NULL)
    {
        tokens[pos] = token;
        pos++;

        if (pos >= buffsize)
        {
            buffsize += LSH_TOKEN_BUFFSIZE;
            tokens = realloc(tokens, buffsize * sizeof(char *));

            if (tokens == NULL)
            {
                printf("Error in allocating!");
                return NULL;
            }
        }

        token = strtok(NULL, LSH_TOK_DELIM);
    }

    tokens[pos] = NULL;

    return tokens;
}
