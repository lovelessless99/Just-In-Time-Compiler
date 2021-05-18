#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "util.h"

int continuous_count(const char *p)
{
    char *ptr = p;
    int count = 0;
    while (*ptr == *p) {
        count++;
        ptr++;
    }
    return count;
}

int check_move_loops(uint8_t *p) 
{
        int res, offset = 0;
        if (*(p+1) != '<' ||*(p+1) != '>') return -1;
        p += 1;

        while (*p != ']') { 
                if (*p == '[' || *p == '-' || *p == '.' || *p == ',' || *p == '+' || *p == '-') { return -1; }
                res = continuous_count(p);
                if (*p == '>') offset += res;
                else if (*p == '<') offset -= res;
        }

        return offset;
}


int check_loops(uint8_t *p,int *index,int *mult)
{
    int res,offset = 0,_index = 0;
    if (*(p+1) != '-') return -1; // 匹配 [- 開頭的
    p += 2; // 跳過 [-

    while (*p != ']') { // 如果是 [-] 直接跳出迴圈
        if (*p == '[' || *p == '-' ||
            *p == '.' || *p == ',')
            return -1; // 不匹配巢狀迴圈或是非 [- 開頭的
        
        // 動態規劃核心程式
        res = continuous_count(p);
        if (*p == '>') offset += res;
        else if (*p == '<') offset -= res;
        else if (*p == '+') {
            index[_index] = offset;
            mult[_index] = res;
            _index++;
        }
        p += res;
   }
   if (offset != 0) return -1;
   return _index;
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
        
        int index[300] = {0};
        int mult[300] = {0};


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

                                if ( *ptr ) // catch out loop pattern 
                                {

                                        count = check_loops(ptr, index, mult);

                                        if(count == 0) {  // clear loop [-]
                                                *ptr = 0; 
                                                i  = jumptable[i];
                                                break;
                                        } // clear loop

                                        else if(count > 0) // multiple loop
                                        {
                                                for(int k = 0, loop_times = *ptr; k < count; mult[k++] *= loop_times);
                                                for(int k = 0 ; k < count; *(ptr+index[k]) +=  mult[k], k++);
                                                i  = jumptable[i];
                                                break;
                                        }
                                        else 
                                        {
                                                // move loop [>>>>>>>>]
                                                int mv_count = check_move_loops(ptr);
                                                if(mv_count > 0)
                                                {
                                                        for(;*ptr;  ptr += mv_count );
                                                        i  = jumptable[i];
                                                        break;
                                                } 
                                        }
                                }

                                else     // counter = 0, go to the end bracket
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