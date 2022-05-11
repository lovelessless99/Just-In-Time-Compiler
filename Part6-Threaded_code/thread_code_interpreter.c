#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "util.h"

// input is a const array to const char.
void interpreter(const char input[])
{
        // ASCII 8 bit.
        uint8_t tape[30000] = { 0 };

        // set pointer to the left most cell of the tape.
        uint8_t *ptr = tape;
        
        const char *current_char = input;

        void* operations[] = {
            ['\0'] = &&END,
            ['\n'] = &&PASS,
            ['\r'] = &&PASS, 
            ['+']  = &&ADD,
            ['-']  = &&SUB,
            ['.']  = &&PRINT,
            [',']  = &&INPUT,
            ['<']  = &&LSHIFT,
            ['>']  = &&RSHIFT,
            ['[']  = &&LBRACE,
            [']']  = &&RBRACE,
        };

        goto *(operations[*current_char]);

        END:
                return;

        PASS:
                goto *(operations[*++current_char]);

        ADD: 
                ++(*ptr);
                goto *(operations[*++current_char]);

        SUB: 
                --(*ptr);
                goto *(operations[*++current_char]);

        PRINT: 
                putchar(*ptr);
                goto *(operations[*++current_char]);

        INPUT: 
                *ptr = getchar();
                goto *(operations[*++current_char]);

        LSHIFT: 
                --ptr;
                goto *(operations[*++current_char]);

        RSHIFT: 
                ++ptr; 
                goto *(operations[*++current_char]);

        LBRACE:
                if (!(*ptr)) // counter = 0, go to the end bracket
                {
                        int loop = 1;
                        while (loop > 0) 
                        {
                                ++current_char;
                                if (*current_char == ']') 
                                { 
                                        --loop; 
                                }
                                        
                                else if (*current_char == '[')
                                {
                                        ++loop;
                                }
                        }
                }
                goto *(operations[*++current_char]);

        RBRACE:
                if (*ptr) 
                {
                        int loop = 1;
                        while (loop > 0) // back to start bracket
                        {
                                --current_char;
                                if (*current_char == '[')
                                {
                                        --loop;
                                }

                                else if (*current_char == ']')
                                {
                                        ++loop;
                                }
                        }
                }
                goto *(operations[*++current_char]);
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