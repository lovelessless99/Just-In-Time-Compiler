#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "util.h"

#ifdef BFTRACE
        #include <unordered_map> // for statistic
        #include <algorithm>
        #include <iostream>
        #include <utility>
        #include <string>
        #include <vector>
        std::unordered_map<char, size_t> op_exec_count;
        std::unordered_map<std::string, size_t> trace_loop_count;
        std::string current_trace;

        template <typename T>
        bool sortbysec(const std::pair<T, int> &a, const std::pair<T, int> &b)
        {
               return (a.second > b.second);
        }

#endif


int continuous_count(const char *p)
{
    const char *ptr = p;
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
                if (current_char == '\r' || current_char == '\n') continue;
                #ifdef BFTRACE
                        op_exec_count[current_char]++;
                #endif
                

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
                                        int bracket_end_idx = jumptable[i];
                                        #ifdef BFTRACE
                                                current_trace = "";
                                        #endif
                                        i = bracket_end_idx;
                                }
                                break;

                        case ']':
                                if (*ptr) // '[-]' pattern jumptable[i] == i-2
                                {
                                        int bracket_start_idx = jumptable[i];
                                        #ifdef BFTRACE
                                        current_trace = std::string(input+bracket_start_idx, input+i+1);
                                        size_t n = std::count(current_trace.begin(), current_trace.end(), '[' );
                                        if (current_trace.size() > 0 && n == 1) {
                                                trace_loop_count[current_trace] += 1;
                                                current_trace = "";
                                        }
                                        #endif
                                        i = bracket_start_idx;
                                }
                                else if (jumptable[i] == i-2){
                                        int bracket_start_idx = jumptable[i];
                                        #ifdef BFTRACE
                                        current_trace = std::string(input+bracket_start_idx, input+i+1);
                                        size_t n = std::count(current_trace.begin(), current_trace.end(), '[' );
                                        if (current_trace.size() > 0 && n == 1) {
                                                trace_loop_count[current_trace] += 1;
                                                current_trace = "";
                                        }
                                        #endif
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
        
        #ifdef BFTRACE
        
        
        printf("instruction frequence\n");
        std::vector<std::pair<char, size_t>> op_exec_vec(op_exec_count.begin(), op_exec_count.end());
        std::sort(op_exec_vec.begin(), op_exec_vec.end(), [](auto &left, auto &right) { return left.second < right.second;});


        std::for_each(op_exec_vec.begin(), op_exec_vec.end(),
                [](const std::pair<char, size_t> &p) {
                    std::cout << "{" << p.first << ": " << p.second << "}\n";
                });


        printf("loop string frequence\n");
        std::vector<std::pair<std::string, size_t>> trace_loop_vec(trace_loop_count.begin(), trace_loop_count.end());
        std::sort(trace_loop_vec.begin(), trace_loop_vec.end(), [](auto &left, auto &right) { return left.second < right.second;});

        std::for_each(trace_loop_vec.begin(), trace_loop_vec.end(),
                [](const std::pair<std::string, size_t> &p) {
                    std::cout << "{" << p.first << ": " << p.second << "}\n";
                });

        #endif
	
        free(file_contents);
}