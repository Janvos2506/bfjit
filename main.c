#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_STACK_SIZE 100
#define VM_MEMORY_SIZE 30000

typedef struct Stack
{
    int data[MAX_STACK_SIZE];
    int top;
} Stack;

Stack *init_stack()
{
    Stack *stack = malloc(sizeof(Stack));
    stack->top = -1; //-1 we need to push after moving so we leave the top pointing to the last element
    return stack;
}

typedef struct Token
{
    char *type;
    int payload;
} Token;

typedef struct Tokens
{
    Token **tokens;
    int size;
} Tokens;

typedef struct Lexer
{
    char *source;
    int current;
} Lexer;

typedef struct Linked_List
{
    int size;
    struct Linked_List_Node *first;
} Linked_List;

typedef struct Linked_List_Node
{
    Token *data;
    struct Linked_List_Node *next;
} Linked_List_Node;

void push(Stack *stack, int data)
{
    if (stack->top == MAX_STACK_SIZE - 1)
    {
        fprintf(stderr, "Error: Stack overflow\n");
        return;
    }
    stack->data[++stack->top] = data;
}

int pop(Stack *stack)
{
    if (stack->top == -1)
    {
        fprintf(stderr, "Error: Stack underflow\n");
        return -1;
    }
    return stack->data[stack->top--];
}

Linked_List *init_linked_list()
{
    Linked_List *list = malloc(sizeof(Linked_List));
    list->size = 0;

    list->first = NULL;
    return list;
}

void append_linked_list(Linked_List *list, Token *data)
{
    Linked_List_Node *node = malloc(sizeof(Linked_List_Node));
    node->next = NULL;
    node->data = data;

    if (list->first == NULL)
    {
        list->first = node;
        list->size++;
        return;
    }

    Linked_List_Node *current = list->first;
    // Find the last node
    while (current->next != NULL)
    {
        current = current->next;
    }
    current->next = node;
    list->size++;
}

int is_bf_char(char c)
{
    // loop over valid chars, returns true if it matches one of them
    char *cmds = "><+-.,[]";
    for (int i = 0; i < 8; i++)
    {
        if (c == cmds[i])
        {
            return 1;
        }
    }
    return 0;
}

// tokenize takes a lexer and returns the next token
Tokens *tokenize(Lexer *lexer)
{
    int max_size = strlen(lexer->source);
    // array of tokens
    Token **tokens = malloc(max_size * sizeof(Token *)); // array of pointers to a token so the array is a pointer to a pointer
    int tokenIndex = 0;
    Stack *stack = init_stack();
    while (lexer->source[lexer->current] != '\0')
    {
        // Get a pointer to char in source
        char *c = &lexer->source[lexer->current];
        switch (*c)
        {
        case '>':
        case '<':
        case '+':
        case '-':
        case '.':
        case ',':
        {
            Token *token = malloc(sizeof(Token));
            token->type = c;
            token->payload = 0;

            while (*c != '\0')
            {
                if (!is_bf_char(*c))
                {
                    // Ignore non bf chars
                    continue;
                }

                if (*c != *token->type)
                {
                    // We are done with this token, encountered a new one
                    break;
                }

                // Count the number of times the token appears and move to the next char
                token->payload++;
                c = &lexer->source[++lexer->current];
            }

            // return the final token
            tokens[tokenIndex++] = token;
            break;
        }
        case '[':
        {
            Token *token = malloc(sizeof(Token));
            token->type = c;
            token->payload = -1;

            // Push the current position in the list of tokens so we can return to it later
            push(stack, tokenIndex + 1);

            // return the final token
            tokens[tokenIndex++] = token;
            lexer->current++;
            break;
        }
        case ']':
        {
            Token *token = malloc(sizeof(Token));
            token->type = c;
            token->payload = pop(stack); // pop the return index, the index of the char after matching [

            // update payload of correspondig [ token
            tokens[token->payload - 1]->payload = tokenIndex + 1;

            // return the final token
            tokens[tokenIndex++] = token;
            lexer->current++;
            break;
        }
        default:
            break;
        }
    }

    // Trim the tokens array to the correct size
    tokens = realloc(tokens, tokenIndex * sizeof(Token *));
    Tokens *tokens_array = malloc(sizeof(Tokens));
    tokens_array->tokens = tokens;
    tokens_array->size = tokenIndex;
    return tokens_array;
}

// read_file reads the file and returns the content of the file
char *read_file(char *filename)
{
    // Get file handle with absolute path in this case
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        fprintf(stderr, "Error: Could not open file %s\n", filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file); // walk to end of buffer to see how big it is in bytes
    rewind(file);

    char *content = malloc(file_size + 1); //+1 for null terminator
    if (content == NULL)
    {
        fprintf(stderr, "Error: Could not allocate memory\n");
        fclose(file);
        return NULL;
    }

    // Read the file into the buffer "content"
    size_t read_size = fread(content, sizeof(char), file_size, file);
    if (read_size != file_size) // Check if we read the whole file, might be better to use a while loop and a < file_size
    {
        fprintf(stderr, "Error: Could not read file %s\n", filename);
        free(content);
        fclose(file);
        return NULL;
    }
    fclose(file);

    content[file_size] = '\0'; // Add a null terminator to the end of the buffer
    return content;
}

void printTokens(const Tokens *tokens)
{
    for (int i = 0; i < tokens->size; i++)
    {
        Token *token = tokens->tokens[i];
        printf("%d. %c (%d)\n", i, token->type[0], token->payload);
    }
}

typedef struct VM
{
    int memory[VM_MEMORY_SIZE]; // Size of the memory for now, might need to be dynamic
    int instruction_pointer;    // Points to the current instruction
    int memory_pointer;         // Points to the current memory cell
} VM;

// Commands

// >
void cmd_increment_memory_pointer(VM *vm, int times)
{
    // TODO add guards to check if we are going out of bounds
    vm->memory_pointer += times;
    vm->instruction_pointer++;
}

// <
void cmd_decrement_memory_pointer(VM *vm, int times)
{
    // TODO add guards to check if we are going out of bounds
    vm->memory_pointer -= times;
    vm->instruction_pointer++;
}

// +
void cmd_increment_memory_value(VM *vm, int times)
{
    vm->memory[vm->memory_pointer] += times;
    vm->instruction_pointer++;
}

// -
void cmd_decrement_memory_value(VM *vm, int times)
{
    vm->memory[vm->memory_pointer] -= times;
    vm->instruction_pointer++;
}

// .
void cmd_output_memory_value(VM *vm, int times)
{
    for (int i = 0; i < times; i++)
    {
        printf("%c", vm->memory[vm->memory_pointer]);
    }
    vm->instruction_pointer++;
}

// ,
void cmd_input_memory_value(VM *vm, int times)
{
    for (int i = 0; i < times; i++)
    {
        scanf("%d", &vm->memory[vm->memory_pointer]);
    }
    vm->instruction_pointer++;
}

// [
void cmd_jump_forward(VM *vm, Tokens *tokens)
{
    if (vm->memory[vm->memory_pointer] == 0)
    {
        vm->instruction_pointer = tokens->tokens[vm->instruction_pointer]->payload;
    }
    else
    {
        vm->instruction_pointer++;
    }
}

// ]
void cmd_jump_backward(VM *vm, Tokens *tokens)
{
    if (vm->memory[vm->memory_pointer] != 0)
    {
        vm->instruction_pointer = tokens->tokens[vm->instruction_pointer]->payload;
    }
    else
    {
        vm->instruction_pointer++;
    }
}

/////

void interpret(Tokens *tokens)
{
    VM *vm = malloc(sizeof(VM));
    vm->instruction_pointer = 0;
    vm->memory_pointer = 0;

    // init memory to 0
    for (int i = 0; i < 30000; i++)
    {
        vm->memory[i] = 0;
    }

    while (vm->instruction_pointer < tokens->size)
    {
        Token *token = tokens->tokens[vm->instruction_pointer];
        switch (*token->type)
        {
        case '>':
            cmd_increment_memory_pointer(vm, token->payload);
            break;
        case '<':
            cmd_decrement_memory_pointer(vm, token->payload);
            break;
        case '+':
            cmd_increment_memory_value(vm, token->payload);
            break;
        case '-':
            cmd_decrement_memory_value(vm, token->payload);
            break;
        case '.':
            cmd_output_memory_value(vm, token->payload);
            break;
        case ',':
            cmd_input_memory_value(vm, token->payload);
            break;
        case '[':
            cmd_jump_forward(vm, tokens);
            break;
        case ']':
            cmd_jump_backward(vm, tokens);
            break;
        default:
            break;
        }
    }
}

int main()
{
    char *input = read_file("C:\\Users\\janvo\\repos\\getlow\\hello.bf");

    printf("%c\n", input);

    Lexer *lexer = malloc(sizeof(Lexer));
    lexer->source = input;
    lexer->current = 0; // Start at the beginning of the source

    Tokens *tokens = tokenize(lexer);
    printTokens(tokens);
    interpret(tokens);

    free(input);
    free(lexer);
    free(tokens);
    // wait for user to close
    printf("Press enter to exit...\n");
    getchar();
    return 0;
}
