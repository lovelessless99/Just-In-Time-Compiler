#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "util.h"

int continuous_count(char *p)
{
    char *ptr = p;
    int count = 0;
    while (*ptr == *p) {
        count++;
        ptr++;
    }
    return count;
}

int* compute_jumptable(const char input[]) {
  size_t pc = 0;
  size_t program_size = strlen(input);
  int* jumptable = (int*) calloc(program_size, sizeof(size_t));
  

  while (pc < program_size) {
        char instruction = input[pc];
        if (instruction == '[') {
                int bracket_nesting = 1;
                size_t seek = pc;

                while (bracket_nesting && ++seek < program_size) {
                        if (input[seek] == ']') {
                                bracket_nesting--;
                        } else if (input[seek] == '[') {
                                bracket_nesting++;
                }
        }

        if (!bracket_nesting) {
                jumptable[pc] = seek;
                jumptable[seek] = pc;
        } else {
                printf("unmatched '[' at pc= %lu\n", pc);
        }
        }
        pc++;
  }

  return jumptable;
}

// input is a const array to const char.
void interpreter(const char input[])
{
        // ASCII 8 bit.
        uint8_t tape[30000] = { 0 };

        // set pointer to the left most cell of the tape.
        uint8_t *ptr = tape;
        char current_char;

        int* jumptable = compute_jumptable(input);
        
        for(int i = 0, count = 0 ; (current_char = input[i]) != '\0'; i++)
        {
                switch(current_char)
                {
                        case '>': 
                                count = continuous_count(&input[i]);
                                i += count-1;
                                ptr += count;
                                break;
                        
                        case '<':
                                count = continuous_count(&input[i]);
                                i += count-1;
                                ptr -= count;
                                break;
                        
                        case '+':
                                count = continuous_count(&input[i]);
                                i += count-1;
                                *ptr += count;
                                break;

                        case '-':
                                count = continuous_count(&input[i]);
                                i += count-1;
                                *ptr -= count;
                                break;

                        case '.':
                                putchar(*ptr);
                                break;

                        case ',':
                                *ptr = getchar();
                                break;

                        case '[':

                        
                                if (!(*ptr)) // counter = 0, go to the end bracket
                                {
                                        i  = jumptable[i];
                                }
                                break;

                        case ']':
                                if (*ptr) 
                                {
                                        i = jumptable[i];
                                }
                                break;

                }
        }
}

int main(int argc, char *argv[])
{
        if (argc != 2) 
        {       
                err("Usage: interpreter <inputfile>");
        }
	char *file_contents = read_file(argv[1]);
	if (file_contents == NULL) 
        { 
                err("Couldn't open file");
        }
	interpreter(file_contents);
	free(file_contents);
}