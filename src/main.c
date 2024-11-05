#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_LEN 2048

enum TokenType {
    TOKEN_ERROR = 0,
    TOKEN_TERMINAL = 1,
    TOKEN_DEFINE = 2,
    TOKEN_CONCAT = 3,
    TOKEN_TERMINATE = 4,
    TOKEN_ALTERNATE = 5,
    TOKEN_OPTIONAL = 6,
    TOKEN_REPETITION = 7,
    TOKEN_GROUP = 8,
    TOKEN_COMMENT = 9,
    TOKEN_NON_TERMINAL = 10
};

struct Token {
    enum TokenType type;
    char *content;
};

char *get_type_name(enum TokenType type) {
    switch (type) {
    case TOKEN_ERROR: return "TOKEN_ERROR";
    case TOKEN_TERMINAL: return "TOKEN_TERMINAL";
    case TOKEN_DEFINE: return "TOKEN_DEFINE";
    case TOKEN_CONCAT: return "TOKEN_CONCAT";
    case TOKEN_TERMINATE: return "TOKEN_TERMINATE";
    case TOKEN_ALTERNATE: return "TOKEN_ALTERNATE";
    case TOKEN_OPTIONAL: return "TOKEN_OPTIONAL";
    case TOKEN_REPETITION: return "TOKEN_REPETITION";
    case TOKEN_GROUP: return "TOKEN_GROUP";
    case TOKEN_COMMENT: return "TOKEN_COMMENT";
    case TOKEN_NON_TERMINAL: return "TOKEN_NON_TERMINAL";
    }
    return "TOKEN_UNKNOWN";
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: ebnf <file.ebnf>\n");
        return 1;
    }

    FILE *fp = fopen(argv[1], "r");
    assert(fp != NULL);

    size_t tokens_size = 512;
    size_t tokens_index = 0;
    struct Token *tokens = calloc(tokens_size, sizeof(struct Token));
    assert(fp != NULL);

    char line[LINE_LEN];
    for (size_t linenum = 1; fgets(line, LINE_LEN, fp) != NULL; ++linenum) {
        size_t len = strnlen(line, LINE_LEN);
        for (size_t i = 0; i < len; ++i) {
            if (tokens_index >= tokens_size) {
                tokens_size *= 2;
                tokens = realloc(tokens, tokens_size);
                assert(tokens != NULL);
            }

            switch (line[i]) {
            case '=': {
                // example_label =
                //               |
                //               |
                //       "i" is right here
                // Now count from there to 0 (aka. beginning of the line) in order
                // to know the label (non-terminal symbol) name

                char *cont = calloc(i-1, sizeof(char));
                assert(cont != NULL);
                for (int j = i-1; j >= 0; --j) {
                    if (isalpha(line[j]) || isblank(line[j])) {
                        cont[j] = line[j];
                    } else {
                        fprintf(stderr, "Invalid syntax at %s:%zu:%i!\n", argv[1], linenum, j);
                        return 1;
                    }
                }
                tokens[tokens_index++] = (struct Token){
                    .type = TOKEN_NON_TERMINAL,
                    .content = cont
                };

                if (tokens_index >= tokens_size) {
                    tokens_size *= 2;
                    tokens = realloc(tokens, tokens_size);
                    assert(tokens != NULL);
                }
                tokens[tokens_index++] = (struct Token){
                    .type = TOKEN_DEFINE,
                    .content = NULL
                };
            } break;
            case '\"': {
                char *cont = calloc(LINE_LEN, sizeof(char));
                assert(cont != NULL);
                for (size_t j = i+1; j < len; ++j) {
                    if (line[j] == '\"') {
                        i = j;
                        break;
                    }

                    if (isalnum(line[j])) {
                        cont[j-(i+1)] = line[j];
                    } else {
                        fprintf(stderr, "Invalid terminal symbol at %s:%zu:%zu!\n", argv[1], linenum, j);
                        return 1;
                    }
                }
                tokens[tokens_index++] = (struct Token){
                    .type = TOKEN_TERMINAL,
                    .content = cont
                };
            } break;
            case ',': {
                // actual concatenation will happen later in the parsing process
                tokens[tokens_index++] = (struct Token){
                    .type = TOKEN_CONCAT,
                    .content = NULL
                };
            } break;
            case '.':
            case ';': {
                tokens[tokens_index++] = (struct Token){
                    .type = TOKEN_TERMINATE,
                    .content = NULL
                };
            } break;
            case '/':
            case '!':
            case '|': {
                tokens[tokens_index++] = (struct Token){
                    .type = TOKEN_ALTERNATE,
                    .content = NULL
                };
            } break;
            }
        }
    }

    for (size_t i = 0; i < tokens_index; ++i) {
        printf("%zu: %s\n", i, get_type_name(tokens[i].type));
        if (tokens[i].content != NULL) {
            printf("%zu: %s\n", i, tokens[i].content);
            free(tokens[i].content);
        }
    }
    free(tokens);
    return 0;
}
