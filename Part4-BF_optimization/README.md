# Part 4. BF optimization
我們在本 Part 要實作 just-in-time compiler，每個小結會附上參考連結，其實 JIT compiler ，即時(just in time)就是在執行期間產生機器碼去執行而已。所以我們需要懂得是

* 機器碼跟組合語言的對應
* 組合語言的知識 (caller-saved, callee-saved register)

再將指令翻譯成機器碼就可以送到可至執行記憶體區段做執行，就是簡單的即時編譯器。因此，JIT 的步驟可以簡化為兩階段
1. 產生機器碼，存到可執行記憶體區段
2. 在執行期間執行機器碼


## 4.1 Interpreter optimization
有鑑於直譯器的速度實在是太慢，因此在這裡介紹幾種方式進行加速，而之後不論是編譯器或是即時編譯器的加速方法也會結合直譯器的最佳化方式，加速加速再加速(摸斗摸斗嗨壓苦🤩)

### 4.1-1 Jumptable
其實這裡就有點像是之前 Part2 實作 compiler 和 JIT compiler 一樣的做法，就是不用來回 scan loops，在每一次的迴圈都要尋找括號配對， 可以把時間複雜度從 O(n) 降至 O(1)
> [參考這個網站](https://eli.thegreenplace.net/2017/adventures-in-jit-compilation-part-1-an-interpreter/)

節錄一段此文章話
>Imagine a realistic program with a hot inner loop (by "hot" here I mean it runs many, many - possibly billions - of times throughtout the execution of the program). Is it really necessary to scan the source to find the matching bracket every single time? Of course not. We can just precompute these jump destinations ahead of time, since the BF program doesn't change throughout its execution.

其實就是說，如果今天的內迴圈很`hot`，可能有上百億次的執行，那每次的執行我們是否還需要找配對括號，抑或是可以先`預計算`解決小小的效能瓶頸 ?
因此我們可以製作一個跳表，當迴圈結束後直接跳到括號結尾的位置

從原來需要來回檢查括號迴圈
```C
case '[':
          if (!(*ptr)) // counter = 0, go to the end bracket
          {
                  int loop = 1;
                  while (loop > 0) 
                  {
                          current_char = input[++i];
                          if (current_char == ']') 
                          { 
                                  --loop; 
                          }
                                  
                          else if (current_char == '[')
                          {
                                  ++loop;
                          }
                  }
          }
          break;

case ']':
          if (*ptr) 
          {
                  int loop = 1;
                  while (loop > 0) // back to start bracket
                  {
                          current_char = input[--i];
                          if (current_char == '[')
                          {
                                  --loop;
                          }

                          else if (current_char == ']')
                          {
                                  ++loop;
                          }
                  }
          }
          break;

```
到直接查表直接跳到迴圈起始終止位置，和 Part 2 的 sed 利用對照表轉成的 C code 迴圈有異曲同工之妙，下列程式碼在
`Part4-BF_optimization/BF_interpreter_opt/BF_interpreter_opt1.c`找的到

```C
// 製作跳表
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

                        if (!bracket_nesting) 
                        {
                                jumptable[pc] = seek;
                                jumptable[seek] = pc;
                        }
                        else 
                        {
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
        // printf("%s\n", input);

        for(int i = 0 ; (current_char = input[i]) != '\0'; i++)
        {
                switch(current_char)
                {
                        case '>': 
                                ++ptr;
                                break;
                        
                        case '<':
                                --ptr;
                                break;
                        
                        case '+':
                                ++(*ptr);
                                break;

                        case '-':
                                --(*ptr);
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

```
### 4.1-2 Jumptable + Contraction
在利用 Jumptable 做加速後，我們再以此為基底，進行運算的壓縮。針對 `>`, `<`, `+`, `-`進行預計算。接下來就敘述一下思考流程。
> * [參考網站1](http://accu.cc/content/jit_tour/brainfuck_interpreter/)
> * [參考網站2](https://hackmd.io/@nKngvyhpQdGagg1V6GKLwA/HJjoxbvke?type=view#2016q3-Homework5---JIT-compiler)


如同參考網站1所述，其實我們仔細看 brainfuck 的 hello world 的程式
```bf
++++++++++[>+++++++>++++++++++>+++>+<<<<-]
>++.>+.+++++++..+++.>++.<<+++++++++++++++.
>.+++.------.--------.>+.>.
```

把它轉成中間碼形式
>中間語言(英語: Intermediate Language, IR), 在計算機科學中, 是指一種應用於抽像機器(abstract machine)的編程語言, 它設計的目的, 是用來幫助我們分析計算機程序. 這個術語源自於編譯器, 在編譯器將源代碼編譯為目的碼的過程中, 會先將源代碼轉換為一個或多個的中間表述, 以方便編譯器進行最佳化, 並產生出目的機器的機器語言.
```
[
    ADD,     ADD,     ADD,     ADD,     ADD,     ADD,     ADD,     ADD,
    ADD,     ADD,     LB,      SHR,     ADD,     ADD,     ADD,     ADD,
    ADD,     ADD,     ADD,     SHR,     ADD,     ADD,     ADD,     ADD,
    ADD,     ADD,     ADD,     ADD,     ADD,     ADD,     SHR,     ADD,
    ADD,     ADD,     SHR,     ADD,     SHL,     SHL,     SHL,     SHL,
    SUB,     RB,      SHR,     ADD,     ADD,     PUTCHAR, SHR,     ADD,
    PUTCHAR, ADD,     ADD,     ADD,     ADD,     ADD,     ADD,     ADD,
    PUTCHAR, PUTCHAR, ADD,     ADD,     ADD,     PUTCHAR, SHR,     ADD,
    ADD,     PUTCHAR, SHL,     SHL,     ADD,     ADD,     ADD,     ADD,
    ADD,     ADD,     ADD,     ADD,     ADD,     ADD,     ADD,     ADD,
    ADD,     ADD,     ADD,     PUTCHAR, SHR,     PUTCHAR, ADD,     ADD,
    ADD,     PUTCHAR, SUB,     SUB,     SUB,     SUB,     SUB,     SUB,
    PUTCHAR, SUB,     SUB,     SUB,     SUB,     SUB,     SUB,     SUB,
    SUB,     PUTCHAR, SHR,     ADD,     PUTCHAR, SHR,     PUTCHAR,
]
```
你會發現，其實蠻多冗於成分可以讓我們去最佳化，例如本小結的重點-contraction，就是壓縮指令。我們可以壓縮 
連續的運算，例如連續 10 個 ADD，用中間碼表示就是 ADD(10) 
```
[
    ADD(10),  JIZ(12),  SHR(1),  ADD(7),  SHR(1),  ADD(10),  SHR(1),  ADD(3),
    SHR(1),   ADD(1),   SHL(4),  SUB(1),  JNZ(1),  SHR(1),   ADD(2),  PUTCHAR,
    SHR(1),   ADD(1),   PUTCHAR, ADD(7),  PUTCHAR, PUTCHAR,  ADD(3),  PUTCHAR,
    SHR(1),   ADD(2),   PUTCHAR, SHL(2),  ADD(15), PUTCHAR,  SHR(1),  PUTCHAR,
    ADD(3),   PUTCHAR,  SUB(6),  PUTCHAR, SUB(8),  PUTCHAR,  SHR(1),  ADD(1),
    PUTCHAR,  SHR(1),   PUTCHAR
]
```
對相鄰的相同操作符進行折疊操作，你可以發現中間程式碼變得很短，其中迴圈的部分

```bf
[>+++++++>++++++++++>+++>+<<<<-]
```
由原本的
```
LB,      SHR,     ADD,     ADD,     ADD,     ADD,
ADD,     ADD,     ADD,     SHR,     ADD,     ADD,     ADD,     ADD,
ADD,     ADD,     ADD,     ADD,     ADD,     ADD,     SHR,     ADD,
ADD,     ADD,     SHR,     ADD,     SHL,     SHL,     SHL,     SHL,SUB,     RB
```

壓縮成
```
JIZ(12),  SHR(1),  ADD(7),  SHR(1),  ADD(10),  SHR(1),  ADD(3),
SHR(1),   ADD(1),   SHL(4),  SUB(1),  JNZ(1)
```
> JIZ(12) 表示往後跳 12 個指令

這是簡單的壓縮，當然你想的到的話也可以做更多其他的最佳化😀

而在 [jserv的homework](https://hackmd.io/@nKngvyhpQdGagg1V6GKLwA/HJjoxbvke?type=view#2016q3-Homework5---JIT-compiler)，也有提到對於壓縮的最佳化，只是沒有轉成中間碼，而是使用自訂義函式去數連續的相同操作個數
```C
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
```
將此方法結合到我們的 jumptable 程式碼內，程式碼在 `BF_interpreter_opt/BF_interpreter_opt2.c`
```C
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
```
### 4.1-3 統計迴圈的動作(loop pattern)，進行更深入的最佳化
這裡有利用 C++ 的 `unorder_map` 容器進行統計所有的運算次數，以及迴圈(不計巢狀迴圈)的運算次數，程式碼在
`BF_interpreter_opt/BF_interpreter_statistic.cpp`，因為這是在 4.1-2 的程式碼內加入統計程式碼，因此
跑 **碎形的brainfuck程式碼可能會比較慢一點** ，輸出結果如下。

首先是BF運算子在執行時的頻率

|BF運算子|執行次數|
|--------|-------|
|.|6240|
|+|173837849|
|-|177623020|
|[|422534152|
|<|596892555|
|>|811756172|
|]|835818921|

另外是單迴圈的執行頻率

|迴圈特徵|執行次數|迴圈動作| 迴圈特徵命名 |
|--------|-------|-----------|-----------|
|[->++>>>+++++>++>+<<<<<<]|12||Multiple Loop|
|[->+>>>-<<<<]|51084||Multiple Loop|
|[->>>>>>>>>+<<<<<<<<<]|306294|LOOP_MOVE_DATA|Copy Loop|
|[>+>>>>>>>>]|9217819||Multiple Loop|
|[-]|12038491|LOOP_SET_TO_ZERO|Clear Loop|
|[<<<<<<<<<]|191420093|LOOP_MOVE_PTR|Move Loop|
|[>>>>>>>>>]|272106406|LOOP_MOVE_PTR|Move Loop|

看到這些使用頻率高的迴圈，我們是不是可以針對較好處理的特徵再處理一下，使他們跑的速度變得更快
例如 : 

1. `[-]` : 把當前元素設成 0 (LOOP_SET_TO_ZERO, Clear Loop) 
   ```C
   for(; *ptr; *ptr -= 1);
   ```
   或是可以直接簡化為
   ```C
   *ptr = 0
   ```

2. `[->>>>>>>>>+<<<<<<<<<]`: 移動資料，將當前資料移到 9 格後的位置值 (LOOP_MOVE_DATA, Copy Loop)，寫成 C 語言就是
   ```C
   for(int target=*ptr; *(ptr+9) != target; (*ptr)--, (*ptr+9)++ );
   ```
   或是可以直接簡化為
   ```C
   *(ptr + 9) = *ptr
   *ptr = 0
   ```      

3. `[->++>>>+++++>++>+<<<<<<]`: 這種迴圈就是一般的迴圈(Multiple Loop)，我們可以利用
**動態規劃**的方法，紀錄位移量跟對應的增加量，以此迴圈為例，從`[-`後開始儲存`>++>>>+++++>++>+<<<<<<]`

    |陣列索引 |0   |1   | 2  | 3 |
    |----|----|----|----|----|
    |位移量|1|4|5|6|
    |位移後該位置加上的值|2|5|2|1|

    之後，假設這個迴圈要跑 10 次，要把第二列全部乘上 10


    |陣列索引 |0   |1   | 2  | 3 |
    |----|----|----|----|----|
    |位移量|1|4|5|6|
    |位移後該位置加上的值|20|50|20|10|

    就可以由此表格快速計算好結果，節省很多計算時間

4. `[>>>>>>>>>]`: 向右移 9 倍格，直到遇到值非零的格子(LOOP_MOVE_PTR, Move Loop)，寫成 C 語言就是
   ```C
   for( ; *ptr; ptr += 9);
   ```


接下來，我們就針對這四種迴圈，進行最佳化吧，首先是針對 1, 2, 3 Case 的最佳化，Case 1, 2, 3 的 **共同特徵是以[-為開頭** ，這裡參考
[jserv's blog](https://hackmd.io/@sysprog/SkBsZoReb?type=view) 提供的`check_loops`程式碼

```C
int check_loops(char *p,int *index,int *mult)
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

```

再來是仿照上述例子寫的 case 4 move-loop 程式碼
```C
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
```


完整程式碼如下，也可以去 `BF_interpreter_opt/BF_interpreter_opt3.c` 看
```C
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
```


## 4.2 Brainfuck JIT compiler with opcode
> * [參考網站](https://nickdesaulniers.github.io/blog/2015/05/25/interpreter-compiler-jit/)
> * [參考網站的Github](https://github.com/nickdesaulniers/bf_interpreter_jit_compiler)


其實這裡的實作方法，和 Part2 的 compiler 相同，主要差異是將原先**印出組合語言直接變成機器碼，再直接執行**，就是一種動態編譯技術

例如:
1. 直譯器
```C
case '>': ++ptr; break;
```
2. 編譯器
```C
case '>':
        puts("  inc %r12");
        break;
```
3. 即時編譯器
```C
case '>':
{
        char opcodes [] = {
                0x49, 0xFF, 0xC4 // inc %r12
        };
        vector_push(&instruction_stream, opcodes, sizeof(opcodes));
}
break;
```

在這裡即時編譯器的行為就跟我們實作的編譯器一樣，只是我們又跳過了組譯這部直接產生機器碼存在記憶體，我們就是**一邊編譯一邊執行**，所以即時編譯器缺點也很明顯

* 仍需要讀檔、重新轉(re-parse, rerun)
* 要動態產生機器碼
* 很佔記憶體空間
* 可執行區段可能會成為漏洞 (所以 ios 系統不允許 JIT 的實作)

因此，我們在這裡只要把前面的 compiler 輸出組語的部分直接轉成 opcode 就好。如果不知道怎麼轉，請參考**Part 1. 揉合 objdump**那段。
然而，列出了缺點，但是優點是可以變快許多，為什麼 ? 
原因是因為直譯器針對每一個讀進來的指令，至少要經過兩個 branch 指令，一個是 for-loop，一個是 switch-case，對於一個加法而言，JIT 只需要 `add 的指令`一行即可，而直譯器需要經過數行指令才可以真正執行加法，在 BF 程式碼複雜情況下，效能上的差異會拉開。

這段程式碼在 `JIT_opcode/jit_opcode.c` 資料夾下

## 4.3. Brainfuck JIT compiler with dynasm
### 4.3-1. JIT compiler
> * [參考網站](https://blog.reverberate.org/2012/12/hello-jit-world-joy-of-simple-jits.html)
> * [參考網站 Github](https://github.com/haberman/jitdemo/blob/master/jit3.dasc)

如果你可能對組合語言有些熟悉的話，又覺得前面直接放入 opcode 到程式碼內可讀性很低，那可以用 dynasm code generator 幫助我們組語寫完轉化成 opcode，程式碼放進 `JIT_Dynasm/jit_dynasm.dasc` 檔案內，值得一提的是，在這邊的程式碼已經有實作類似 4.1-1 jumptable 的方法。

> 有另外一個工具叫做 asmjit，有興趣可以玩玩看 😁

```C
// JIT for Brainf*ck.

#include <stdint.h>

|.arch x64
|.actionlist actions
|
|// Use rbx as our cell pointer.
|// Since rbx is a callee-save register, it will be preserved
|// across our calls to getchar and putchar.
|.define PTR, rbx
|
|// Macro for calling a function.
|// In cases where our target is <=2**32 away we can use
|//   | call &addr
|// But since we don't know if it will be, we use this safe
|// sequence instead.
|.macro callp, addr
|  mov64  rax, (uintptr_t)addr
|  call   rax
|.endmacro

#define Dst &state
#define MAX_NESTING 256

void err(const char *msg) {
  fprintf(stderr, "%s\n", msg);
  exit(1);
}

int main(int argc, char *argv[]) {
  if (argc < 2) err("Usage: jit3 <bf program>");
  dasm_State *state;
  initjit(&state, actions);

  unsigned int maxpc = 0;
  int pcstack[MAX_NESTING];
  int *top = pcstack, *limit = pcstack + MAX_NESTING;

  // Function prologue.
  |  push PTR
  |  mov  PTR, rdi

  for (char *p = argv[1]; *p; p++) {
    switch (*p) {
      case '>':
        |  inc  PTR
        break;
      case '<':
        |  dec  PTR
        break;
      case '+':
        |  inc  byte [PTR]
        break;
      case '-':
        |  dec  byte [PTR]
        break;
      case '.':
        |  movzx edi, byte [PTR]
        |  callp putchar
        break;
      case ',':
        |  callp getchar
        |  mov   byte [PTR], al
        break;
      case '[':
        if (top == limit) err("Nesting too deep.");
        // Each loop gets two pclabels: at the beginning and end.
        // We store pclabel offsets in a stack to link the loop
        // begin and end together.
        maxpc += 2;
        *top++ = maxpc;
        dasm_growpc(&state, maxpc);
        |  cmp  byte [PTR], 0
        |  je   =>(maxpc-2)
        |=>(maxpc-1):
        break;
      case ']':
        if (top == pcstack) err("Unmatched ']'");
        top--;
        |  cmp  byte [PTR], 0
        |  jne  =>(*top-1)
        |=>(*top-2):
        break;
    }
  }

  // Function epilogue.
  |  pop  PTR
  |  ret

  void (*fptr)(char*) = jitcode(&state);
  char *mem = calloc(30000, 1);
  fptr(mem);
  free(mem);
  free_jitcode(fptr);
  return 0;
}
```
比方才的機器語言的程式碼稍微簡潔不複雜，可讀性也提高了(不過組語本來就是提高機器語言的可讀性建立出來的高階語言(跟機器語言比的話)🤣)，因此藉由 dynasm 加上一些組合語言的知識，可以幫我們快速建立一個 JIT compiler


### 4.3-2. Jumptable + Contraction
承接上一個步驟，我們要像 4.1-2 一樣加入運算壓縮的技術，其實改動的地方很簡單，最主要是 `>`, `<`, `+`, `-` 的地方，程式碼在 `JIT_Dynasm/jit_dynasm_opt1.dasc`
```C
for (char *p = argv[1]; *p; p++) {
    switch (*p) {
      case '>':
        |  inc  PTR
        break;
      case '<':
        |  dec  PTR
        break;
      case '+':
        |  inc  byte [PTR]
        break;
      case '-':
        |  dec  byte [PTR]
        break;
      case '.':
        |  movzx edi, byte [PTR]
        |  callp putchar
        break;
...
```

在`.dasm`加入這個函數後
```C
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
```
修改組合語言指令
```C
for (char *p = file_contents; *p; p++) {
    switch (*p) {
      case '>':
        count = continuous_count(p);
        p += count - 1;
        |  add  PTR, count
        break;
      case '<':
        count = continuous_count(p);
        p += count - 1;
        |  sub  PTR, count
        break;
      case '+':
        count = continuous_count(p);
        p += count - 1;
        |  add  byte [PTR], count 
        break;
      case '-':
        count = continuous_count(p);
        p += count - 1;
        |  sub  byte [PTR], count
        break;
...
```

如此一來，可以先利用 `continuous_count` 先計算未來的連續符號個數，例如未來有五個 +，就可以把五個加法指令合併成一個加法指令`add PTR, 5`，如此一來，可以再加速上一個 jit 的實作

### 4.3-3. Clear Loop & Multiple Loop & Copy Loop
這裡如同 `4.1-3 統計迴圈的動作(loop pattern)，進行更深入的最佳化`，我們承接該小節直譯器的函數，來針對特定迴圈字串進行
最佳化，還記得嗎，先前經由分析，我們得到某些迴圈的頻率

|迴圈特徵|執行次數|迴圈動作| 迴圈特徵命名 |
|--------|-------|-----------|-----------|
|[->++>>>+++++>++>+<<<<<<]|12||Multiple Loop|
|[->+>>>-<<<<]|51084||Multiple Loop|
|[->>>>>>>>>+<<<<<<<<<]|306294|LOOP_MOVE_DATA|Copy Loop|
|[>+>>>>>>>>]|9217819||Multiple Loop|
|[-]|12038491|LOOP_SET_TO_ZERO|Clear Loop|
|[<<<<<<<<<]|191420093|LOOP_MOVE_PTR|Move Loop|
|[>>>>>>>>>]|272106406|LOOP_MOVE_PTR|Move Loop|

因此，我們針對這些很**熱**的特別迴圈，進行更深入的最佳化，這裡引用 global_count 來當作 jumptable 的功用，執行完 checkloop 之後的動作後，直接跳到指定位置，不重新執行。程式碼在`JIT_Dynasm/jit_dynasm_opt2.dasc`

```C
int global_count = 0;
int check_loops(char *p,int *index,int *mult)
{
    int res,offset = 0,_index = 0;
    global_count = 0;
    if (*(p+1) != '-') return -1;
    p += 2;
    global_count += 2;
    while (*p != ']') {
        if (*p == '[' || *p == '-' ||
            *p == '.' || *p == ',')
            return -1;
        res = continuous_count(p);
        if (*p == '>') offset += res;
        else if (*p == '<') offset -= res;
        else if (*p == '+') {
            index[_index] = offset;
            mult[_index] = res;
            _index++;
        }
        global_count += res;
        p += res;
   }
   if (offset != 0) return -1;
   return _index;
}
```

原本的程式碼
```C
case '[':
        if (top == limit) err("Nesting too deep.");
        // Each loop gets two pclabels: at the beginning and end.
        // We store pclabel offsets in a stack to link the loop
        // begin and end together.
        maxpc += 2;
        *top++ = maxpc;
        dasm_growpc(&state, maxpc);
        |  cmp  byte [PTR], 0
        |  je   =>(maxpc-2)
        |=>(maxpc-1):
        break;

```
改成下列程式碼
```C
case '[':
      if (top == limit) err("Nesting too deep.");
        count = check_loops(p, index, mult);

      if(count == 0){ // clear loop
          p += global_count;
          |  mov  byte [PTR], 0
          break;

      }else if(count > 0){ // DP solve multiple loop
          |  mov  cx, word [PTR]
          |  mov  r11, PTR
          |  add  PTR, index[0]
          |  mov  ax, word mult[0]
          |  imul  ax, cx
          |  add  byte [PTR], al

          for(int i = 1; i < count; i++){
              |  mov  r9, index[i]
              |  sub  r9, index[i - 1]
              |  add  PTR, r9
              |  mov  ax, mult[i]
              |  imul  ax, cx
              |  add  byte [PTR], al
          }

          |  mov  PTR, r11
          |  mov  byte [PTR], 0
        
          p += global_count;
          break;

      }else{
          maxpc += 2;
          *top++ = maxpc;
          dasm_growpc(&state, maxpc);
          |  cmp  byte [PTR], 0
          |  je   =>(maxpc-2)
          |=>(maxpc-1):
          break;
      }
```


### 4.4 最佳化總結
在本章，我們先從直譯器的最佳化，實作三種最佳化方法
* jumptable
* jumptable + contraction
* jumptable + contraction + loop-pattern

接下來我們又實作了 JIT compiler
* jit in opcode( jumptable )
* Jit with dynasm
   * naive ( jumptable )
   * naive ( jumptable ) + contraction
   * naive ( jumptable ) + contraction + loop-pattern

我們其實還有以下目標未完成，還有很多可以玩的😁
1. opcode ( jumptable ) + contraction
2. opcode ( jumptable ) + contraction + loop-pattern
3. compiler with  contraction + loop-pattern 
4. asmjit, llvm

那在下一章節 Part 5，我們會開始進行全方位的比較。終於迎來了大亂鬥的時刻。

### 執行方式
只要進到 `Part4-BF_optimization` 資料夾，下指令
```bash
make
```
就會自動編譯以上的程式碼，之後進到對應的資料夾即可執行程式
```bash
./<executable> mandelbrot.bf
```

