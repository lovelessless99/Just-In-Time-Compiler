---
title: Just In Time Compiler - Part 6. Threaded Code - 跳出迴圈的魔咒
date: 2022-05-11
description: Threaded Code
categories:
 - JIT compiler
author: Lawrence
prev: ./Part5
next:
tags: 
   - JIT compiler
   - Compiler
---

# Part  6. Threaded Code - 跳出迴圈的魔咒

## 一、Threaded Code 簡介 
在前一篇，有提到一般 interpreter 的 Instruction Dispatch Overhead 其實蠻高的，因為每執行一個指令都要兩次的 branch (for-loop 跟 switch case)，其實一來一回，指令變比較多外，程式 branch 的次數也變得更多，雖然說 modern CPU 在 branch prediction 已經做得很好了，但是每個指令都要在 for loop 跟 switch 之間來回，為什麼不能順順的往下做就好呢 ? 如下圖。


![](https://i.imgur.com/LPaN1KZ.png)


下面是我們 Brainfuck 的直譯器的外觀，每讀一個指令都要兩次判斷。因此Threaded Code的目標是只需要針對指令做跳轉就好，不需要再經由 for-loop來取指令。
```C
or(int i = 0 ; (current_char = input[i]) != '\0'; i++)
   {
            switch(current_char)
            {
                  case '>':  ...; 
                  case '<':  ...;
                  case '+':  ...;
                  case '-':  ...;
                  case '.':  ...;
                  case ',':  ...;
                  case '[':  ...;
                  case ']':  ...;
            }
   }
```

我們原本的直譯器的程式碼
```c
void interpreter(const char input[])
{
        // ASCII 8 bit.
        uint8_t tape[30000] = { 0 };

        // set pointer to the left most cell of the tape.
        uint8_t *ptr = tape;
        char current_char;

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

                }
        }
}
```

我們改成 threaded code 的形式
```C
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
```

首先，我們先建立一個 brainfuck 符號對 Label 地址的映射表格，Label 地址是 GCC 編譯器支援的語法，透過 `&&lable_name` 就可以取得該 Label 的地址。


```C
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
```

之後我們藉由這兩行，來開啟我們指令的跳轉之旅。
```C
const char *current_char = input;
goto *(operations[*current_char]);
```

你在上面的程式碼可以發現，每次 Label 指定動作執行完之後，一定會直接跳到下一個指令執行。
```C
goto *(operations[*++current_char]);
```

程式碼的架構跟原本的蠻類似的，但是更簡練了，少了回去 main-loop，接下來我們來看一下效能快了多少吧。程式碼在 [Part6-Threaded_code](https://github.com/lovelessless99/Just-In-Time-Compiler/tree/master/Part6-Threaded_code) 

```bash
make clean
```

## 二、效能比較和探討

在一般的直譯器，在我的電腦大約花了 100 秒跑完我們 `mandelbrot.bf` ，而在用 thread-code 優化後，落在了大約 80 秒的地方，效能提升了 20%。證實 threaded code 可以幫忙減輕 dispatch overhead。

Threaded Code 其實不是什麼新的技術，早在 2002 年就被提出 [論文來源](https://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.24.4560&rep=rep1&type=pdf)，並且已經被融入 Java Virtual Machine 及 CPython Interpreter 的程式實做中。在這邊我舉我最愛的 CPython 的程式碼為例。

在 Cpython 的程式碼中，執行 python bytecdoe 直譯器實作在 [cpython/Python/ceval.c](https://github.com/python/cpython/blob/f8a2fab212c4e9ea92a5b667560449904c4cf7af/Python/ceval.c)，你可以發現裡面有些字眼很眼熟

```C
#define TARGET(op) TARGET_##op: INSTRUCTION_START(op);
#define DISPATCH_GOTO() goto *opcode_targets[opcode]
#else
#define TARGET(op) case op: INSTRUCTION_START(op);
#define DISPATCH_GOTO() goto dispatch_opcode
#endif
```
那個 `goto *opcode_targets[opcode]` 是否就跟我們前面實做的 `goto *(operations[*current_char])` 有點像呢 ? 😀
雖然在論文有說大約提升 40% 的效率，但在本次實驗只有提升 20% 的效率。原因還有待確認，不過至少這個方法證實是可行的，不然 JVM 跟 python VM 怎麼會拿來實做呢 XD 

![](https://i.imgur.com/OV4yOtJ.png)


## 三、參考資源
1. [Threaded Code Paper - Threaded Code Variations and Optimizations](https://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.24.4560&rep=rep1&type=pdf)

2. [Speed of various interpreter dispatch techniques V2](http://www.complang.tuwien.ac.at/forth/threading/)
這個網站有提供不同的簡單程式碼給你去感覺 threaded code 跟 switch based 的差異。算是我的啟蒙網站 XD

3. [Advanced Compiler](http://lampwww.epfl.ch/teaching/archive/advanced_compiler/2007/resources/slides/act-2007-03-interpreters-vms_6.pdf)

4. [Dispatch Techniques](https://www.cs.toronto.edu/~matz/dissertation/matzDissertation-latex2html/node6.html)

5. [Decode and dispatch interpretation and Threaded interpretation](https://stackoverflow.com/questions/3848343/decode-and-dispatch-interpretation-vs-threaded-interpretation)

6. [What opcode dispatch strategies are used in efficient interpreters?](https://stackoverflow.com/questions/511566/what-opcode-dispatch-strategies-are-used-in-efficient-interpreters)

7. [Threaded code](https://muforth.nimblemachines.com/threaded-code/)


