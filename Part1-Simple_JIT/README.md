# Part 1. 認識 JIT 及建立簡單的(?) JIT

# 一、Just-in-time 的概念 
* 使用到的大專案 : Google V8, Java Virtual Machine, C# Framework, RubyJIT, pypy, numba, DolphinDB 等

* 動態編譯技術 (混合型編譯-結合 compiler and interpreter)

* Interpretor 執行的時候，會監看哪段程式碼是熱點(hot)程式碼，例如迴圈。發現後馬上送到 JIT 編譯成機器碼放在記憶體，下次執行時不用重新翻譯，類似快取的概念

* 優點是，可以得到靜態編譯所沒有的訊息，知道哪些函數是不是被大量使用的，或是那些函數的參數一直都沒有變等等。一旦得到這些訊息，可以編譯出比靜態編譯器更好的機器碼。一般程式也是 90-10 原則。執行時 90% 的計算時間是跑 10% 的程式碼，只要找到這些熱點(hot-spot) 程式碼，就能進行更深度的優化


* 缺點是
    * 監看會耗費CPU資源
    * 儲存程式碼會耗費記憶體空間
    * 在 runtime 產生 code，每次重新啟動時又要重新監控(profiling)一次，沒有留紀錄，AOT (ahead-of-time) 改善此缺點


# 二、 Hello, JIT World: The Joy of Simple JITs

這是最簡單的 jit 範例，下面用 mmap 建立一塊可寫可執行的區域，但是一般最好不要用可執行的flag，因為可能會有安全上的問題。這也是為什麼不用 malloc 去宣告一塊記憶體，因為heap, stack 都是不可執行的記憶體區塊。

[這個網站](https://blog.reverberate.org/2012/12/hello-jit-world-joy-of-simple-jits.html) 也提到
>You might wonder why you can’t call a function that just changes the permissions of the memory you get from malloc(). Having to allocate executable memory in a totally different way sounds like a drag. In fact there is a function that can change permissions on memory you already have; it’s called mprotect(). But these permissions can only be set on page boundaries; malloc() will give you some memory from the middle of a page, a page that you do not own in its entirety. If you start changing permissions on that page you’ll affect any other code that might be using memory in that page.

意思就是說，可以用 mprotect 改變 malloc 的權限，但是是以`頁` 為單位設權限，而不是只針對 malloc 分配出來的記憶體區塊。因此，如果只針對 malloc 進行 mprotect，可能會改變到其他使用到這個 page 的指令或是 code

```C
/* jit_toy.c */
int main(int argc,char* argv[])
{
        // Machine code for:
        //   mov eax, 0
        //   ret
        unsigned char code[] = {0xb8, 0x00, 0x00, 0x00, 0x00, 0xc3};

        if (argc < 2) {
                fprintf(stderr, "Usage: jit1 <integer>\n");
                return 1;
        }

        // Overwrite immediate value "0" in the instruction
        // with the user's value.  This will make our code:
        //   mov eax, <user's value>
        //   ret
        int num = atoi(argv[1]);
        memcpy(&code[1], &num, 4);

        // Allocate writable/executable memory.
        // Note: real programs should not map memory both writable
        // and executable because it is a security risk.
        void *mem = mmap(NULL, sizeof(code), PROT_WRITE | PROT_EXEC,
                        MAP_ANON | MAP_PRIVATE, -1, 0);
        memcpy(mem, code, sizeof(code));

        // The function will return the user's value.
        int (*func)() = mem;
        return func();
}
```
```bash
make ; ./jit_toy 55; echo $?
```


# 二、Hello, DynASM World!
[DynASM 官方網頁](https://luajit.org/dynasm.html)，特色如下

1. DynASM 是一個程式碼生成的動態組譯器(code generator)
2. 為 `LuaJIT` 開發主要工具 
3. DynASM 適用以下情境

    * 撰寫一個 JIT compiler <------ 我們要的 ^__^
    * 快速生成程式碼 (高效能的計算) 

4. DynASM 把混合 C/Assembler code 轉成 C code. 
5. 支援 x86, x64, ARM, ARM64, PowerPC and MIPS 指令集架構
6. [其他特色](https://luajit.org/dynasm_features.html)

對 dynasm 有些初步的認識後，那為什麼要使用 dynasm 呢 ? 
節錄至[此網站](https://blog.reverberate.org/2012/12/hello-jit-world-joy-of-simple-jits.html)

>The most difficult part of writing a simple JIT is encoding the instructions so they can be understood by your target CPU. For example, on x86-64, the instruction push rbp is encoded as the byte 0x55. Implementing this encoding is boring and requires reading lots of CPU manuals, so we’re going to skip that part. Instead we’ll use Mike Pall’s very nice library DynASM to handle the encoding. DynASM has a very novel approach that lets you intermix the assembly code you’re generating with the C code of your JIT, which lets you write a JIT in a very natural and readable way. It supports many CPU architectures (x86, x86-64, PowerPC, MIPS, and ARM at the time of this writing) so you’re unlikely to be limited by its hardware support. DynASM is also exceptionally small and unimposing; its entire runtime is contained in a 500-line header file.

就是說，JIT 最麻煩的地方，就是要編碼指令，你必須了解你環境的 CPU 指令及架構，你要去讀厚厚一本手冊 🙁，但我們今天要快速實現 JIT，DynASM 就成了我們的英雄。幫助我們做好 encoding 的工作 ( 但是還是需要懂組合語言啦... )，不過至少省了很多功夫。

接下來我們將 JIT 利用 DynASM 動態產生 machine code 並將它放入 dasm_State (at runtime) ，最後再轉換成可執行的機器碼


在開始前先安裝 LuaJIT，首先先下載專案
```bash
git clone https://luajit.org/git/luajit.git
```
進到資料夾後，再下 make 安裝

```bash
sudo make install
sudo  ln -sf luajit-2.1.0-beta3 /usr/local/bin/luajit # 建立 symbolic 的 link，之後直接下 luajit 指令即可
```

若是不知道怎執行的，可以看[本篇文章作者的Github](https://github.com/haberman/jitdemo/blob/master/Makefile)的makefile 檔案。

```makefile
CFLAGS=-O3 -g -std=gnu99 -Ithird_party

all: jit1 jit2 jit3

jit2: dynasm-driver.c jit2.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -o jit2 dynasm-driver.c -DJIT=\"jit2.h\"
jit2.h: jit2.dasc
	lua dynasm/dynasm.lua jit2.dasc > jit2.h
...
```
可以看到 *.dasc 檔案用 lua 進行前處理，把混和組合語言跟 C 的 code 變成 C 語言的 header 檔案，而 `-DJIT` 是定義出現在作者寫的 `dynasm-driver.c` 的巨集
```C
...
/*dynasm-driver.c*/
#include "dynasm/dasm_proto.h"
#include "dynasm/dasm_x86.h"

void initjit(dasm_State **state, const void *actionlist);
void *jitcode(dasm_State **state);
void free_jitcode(void *code);

#include JIT
...
```
不論是 dynasm 函式庫，或是 `dynasm-driver.c` 的檔案，都是等等所需要的工具，如同下面這張圖
![](https://i.imgur.com/HoyLASQ.png)



----
接下來，我們先試試看把 lua 用在 *.dasm 檔案上吧
先準備 `jit_dynasm.dasc`檔案
```C
// DynASM directives.
|.arch x64
|.actionlist actions

// This define affects "|" DynASM lines.  "Dst" must
// resolve to a dasm_State** that points to a dasm_State*.
#define Dst &state

int main(int argc, char *argv[]) {
        if (argc < 2) {
                fprintf(stderr, "Usage: jit1 <integer>\n");
                return 1;
        }

        int num = atoi(argv[1]);
        dasm_State *state;
        initjit(&state, actions);

        // Generate the code.  Each line appends to a buffer in
        // "state", but the code in this buffer is not fully linked
        // yet because labels can be referenced before they are
        // defined.
        //
        // The run-time value of C variable "num" is substituted
        // into the immediate value of the instruction.
        |  mov eax, num
        |  ret

        // Link the code and write it to executable memory.
        int (*fptr)() = jitcode(&state);

        // Call the JIT-ted function.
        int ret = fptr();
        assert(num == ret);

        // Free the machine code.
        free_jitcode(fptr);

        return ret;
}

```
* `|` DynASM 的 preprocessor 會先進行處理
* `|.arch` 在每一個 DynASM source file 中，必須先指定產生哪種指令集架構，如 x86 或 x64

再來下指令
```bash
luajit third_party/dynasm/dynasm.lua jit_dynasm.dasc > jit_dynasm.h
```
其中，`third_party/dynasm/dynasm.lua` 是方才[連結](https://github.com/haberman/jitdemo) 的 dynasm 的 lua 腳本。

> 題外話，其實用一般的 lua 指令也是可以🤣，不一定要 luajit 才能 run
> ```bash
> sudo apt install lua5.2
> lua third_party/dynasm/dynasm.lua jit_dynasm.dasc > jit_dynasm.h
> ```

輸出的 header 檔 `jit_dynasm.h`
```C
/*
** This file has been pre-processed with DynASM.
** http://luajit.org/dynasm.html
** DynASM version 1.3.0, DynASM x64 version 1.3.0
** DO NOT EDIT! The original file is in "jit_dynasm.dasc".
*/

#if DASM_VERSION != 10300
#error "Version mismatch between DynASM and included encoding engine"
#endif

# 1 "jit_dynasm.dasc"
// DynASM directives.
//|.arch x64
//|.actionlist actions
static const unsigned char actions[4] = {
  184,237,195,255
};

# 4 "jit_dynasm.dasc"

// This define affects "|" DynASM lines.  "Dst" must
// resolve to a dasm_State** that points to a dasm_State*.
#define Dst &state

int main(int argc, char *argv[]) {
        if (argc < 2) {
                fprintf(stderr, "Usage: jit1 <integer>\n");
                return 1;
        }

        int num = atoi(argv[1]);
        dasm_State *state;
        initjit(&state, actions);

        // Generate the code.  Each line appends to a buffer in
        // "state", but the code in this buffer is not fully linked
        // yet because labels can be referenced before they are
        // defined.
        //
        // The run-time value of C variable "num" is substituted
        // into the immediate value of the instruction.
        //|  mov eax, num
        //|  ret
        dasm_put(Dst, 0, num);
# 28 "jit_dynasm.dasc"

        // Link the code and write it to executable memory.
        int (*fptr)() = jitcode(&state);

        // Call the JIT-ted function.
        int ret = fptr();
        assert(num == ret);

        // Free the machine code.
        free_jitcode(fptr);

        return ret;
}
```
接下來進行編譯

```bash
clang -g -Wall -Ithird_party -o jit_dynasm dynasm-driver.c -DJIT=\"jit_dynasm.h\"
```

執行
```bash
./jit_dynasm 42 即可
echo $?
```

也可以用我的專案
```bash
make jit_dynasm 
```
即可自動化編譯
>
> 另外 `echo $?` 要注意值最大是 255 
> 所以 `./jit_dynasm 5566` 會是 `5566 mod 256 = 190`
>
----
再來解析 `header` 檔
```c
static const unsigned char actions[4] = {
  184,237,195,255
};

//|  mov eax, num
//|  ret
dasm_put(Dst, 0, num);

```
是什麼 ?
根據[本篇文章](https://blog.reverberate.org/2012/12/hello-jit-world-joy-of-simple-jits.html)
>
> * 184 – the first byte of an x86 mov eax, [immediate] instruction.
> * 237 – the DynASM bytecode instruction DASM_IMM_D, which indicates that the next argument to dasm_put() should be written as a four-byte value. This will complete the mov instruction.
> * 195 – the x86 encoding of the ret instruction.
> * 255 – the DynASM bytecode instruction DASM_STOP, which indicates that encoding should halt.
>
>This action buffer is then referenced by the parts of the code that actually emit assembly instructions. These instruction-emitting lines are replaced with a call to dasm_put() that provides an offset into the action buffer and passes any runtime values that need to be substituted into the output (like our runtime value of num). dasm_put() will append these instructions (with our runtime value of num) into the buffer stored in state (see the #define Dst &state define above).

也就是說 184 是 mov 指令，而 237 是 DynASM 的 DASM_IMM_D，表示下一個被 dasm_put() 傳進來的 value 填上 mov 的值，
而這個值就是我們 runtime 才會填上的 num。因此，有 dynasm 的幫助，可以順利完成簡單的 JIT

最後，此專案的編譯指令

編譯最原始的 jit (沒用 dynasm)
```bash
make -i
make test_jit_toy
```
編譯 dynasm 的 jit 
```bash
make jit_dynasm
make test_jit_dynasm
```
----
# 三、揉合 Objdump

剛剛最一開始的 simple jit，是直接用 machine code 寫入**可執行的記憶體區段**，再指定給函數指標做執行。之後又談論到
>JIT 最麻煩的地方，就是要編碼指令，你必須了解你環境的 CPU 指令及架構，你要去讀厚厚一本手冊

所以才會使用到第二章節的`Dynasm`的引擎或工具，只要利用該指令集架構的組合語言，`Dynasm`就會很友善的來幫我們生成C語言程式碼。

不過有些人(像我)，平常鮮少碰到組合語言。可能連組合語言蠻陌生的。有組合語言的這些範例程式碼可能還沒辦法使用的駕輕就熟。因此受到[basic-jit](https://nickdesaulniers.github.io/blog/2013/04/03/basic-jit/)這篇文章的啟發，可以先用其他方式來得到機器碼，進而較輕鬆實現simple JIT 的功能，**暫時**跳過組合語言的部分。(但以後還是要還辣，God，人生好難 = =)

那就是使用 `objdump`這個反組譯工具。可以先將你的函式編譯成一個`.o`檔案。之後再利用 objdump 這個工具幫你反組譯得到組合語言跟機器碼的對照，接下來就來示範一下如何使用。

首先，先建立一個`mul.c`檔案
```C
int mul(int a, int b)
{
        return a * b;
}
```

之後編譯成`.o`檔，再利用`objdump`這個工具反組譯

接下來，輸入指令
```bash
objdump -j .text -d mul.o -M intel
```
`-j` : 只顯示指定 section 的區域(section 的概念跟 ELF 檔案格式有關，以後會談到)
![](https://i.imgur.com/g3D5aIM.png)

`-d` : 你要反組譯哪個二進制檔案(*.o, 執行檔等)

`-M` : 選擇你要反組譯的指令集架構組合語言

其他選項不贅談，可以輸入 `objdump -H` 看看

之後得到訊息
```
mul.o:     file format elf64-x86-64

Disassembly of section .text:

0000000000000000 <mul>:
   0:   55                      push   rbp
   1:   48 89 e5                mov    rbp,rsp
   4:   89 7d fc                mov    DWORD PTR [rbp-0x4],edi
   7:   89 75 f8                mov    DWORD PTR [rbp-0x8],esi
   a:   8b 75 fc                mov    esi,DWORD PTR [rbp-0x4]
   d:   0f af 75 f8             imul   esi,DWORD PTR [rbp-0x8]
  11:   89 f0                   mov    eax,esi
  13:   5d                      pop    rbp
  14:   c3                      ret
```

當然，在這裡你也可以不用指定 intel 架構
```bash
objdump -j .text -d mul.o
```
得到
```
mul.o:     file format elf64-x86-64


Disassembly of section .text:

0000000000000000 <mul>:
   0:   55                      push   %rbp
   1:   48 89 e5                mov    %rsp,%rbp
   4:   89 7d fc                mov    %edi,-0x4(%rbp)
   7:   89 75 f8                mov    %esi,-0x8(%rbp)
   a:   8b 75 fc                mov    -0x4(%rbp),%esi
   d:   0f af 75 f8             imul   -0x8(%rbp),%esi
  11:   89 f0                   mov    %esi,%eax
  13:   5d                      pop    %rbp
  14:   c3                      retq
```
這個對我們很有幫助，我們得到機器碼跟它對應的組合語言，也就是說

1. 我們可以利用機器碼完成第一部份的simple JIT
2. 我們也可以指定指令集架構後結合對應的組合語言跟C程式，利用方才的`dynasm` 工具幫我們進行處理

一魚兩吃 ? 不好嗎 😅，不過還是需要理解組合語言跟指令集架構的相關知識，但這個工具著實可以輔助我們更快達成目標。

接下來就把這段 code 拿去用在第一部分跟第二部分吧~~

首先第一部分，就是比較人工一點，把機器碼全部都輸入字元陣列內，再指定到mmap 分配的可執行記憶體區段。

`obj_jit_boy.c`
```C
#include <stdio.h> // printf
#include <string.h> // memcpy
#include <sys/mman.h> // mmap, munmap

int main () {
// Hexadecimal x86_64 machine code for: int mul (int a, int b) { return a * b; }
unsigned char code [] = {
        0x55, // push rbp
        0x48, 0x89, 0xe5, // mov rbp, rsp
        0x89, 0x7d, 0xfc, // mov DWORD PTR [rbp-0x4],edi
        0x89, 0x75, 0xf8, // mov DWORD PTR [rbp-0x8],esi
        0x8b, 0x75, 0xfc, // mov esi,DWORD PTR [rbp-04x]
        0x0f, 0xaf, 0x75, 0xf8, // imul esi,DWORD PTR [rbp-0x8]
        0x89, 0xf0, // mov eax,esi
        0x5d, // pop rbp
        0xc3 // ret
};

        // allocate executable memory via sys call
        void* mem = mmap(NULL, sizeof(code), PROT_WRITE | PROT_EXEC,
                        MAP_ANON | MAP_PRIVATE, -1, 0);

        // copy runtime code into allocated memory
        memcpy(mem, code, sizeof(code));

        // typecast allocated memory to a function pointer
        int (*func) () = mem;

        // call function pointer
        printf("%d * %d = %d\n", 5, 11, func(5, 11));

        // Free up allocated memory
        munmap(mem, sizeof(code));
}
```

再來第二部分，結合組合語言到 dynasm 的方法，這邊要注意幾點
* 注意指令及架構
* 不能把剛剛objdump的組語直接複製貼上

> 關於第二點我試過，執行時堆疊的 push 跟 pop 可能dynasm 幫你做了，dynasm 可能要讓你的組語專注在函式功能本身 ? (這點待確認) 

原本
```
|  push   rbp
|  mov    rbp, rsp
|  mov    dword [rbp-0x4], edi
|  mov    dword [rbp-0x8], esi 
|  mov    esi, dword [rbp-0x4] 
|  imul   esi, dword [rbp-0x8]  
|  mov    eax,esi
|  pop    rbp
|  ret
```
稍微修改
```
//|  push   rbp
//|  mov    rsp, rbp
|  mov    dword [rsp-0x4], edi
|  mov    dword [rsp-0x8], esi 
|  mov    esi, dword [rsp-0x4] 
|  imul   esi, dword [rsp-0x8]  
|  mov    eax,esi
|  ret
//|  pop    rbp
```

```C
// DynASM directives.
|.arch x64
|.actionlist actions

// This define affects "|" DynASM lines.  "Dst" must
// resolve to a dasm_State** that points to a dasm_State*.
#define Dst &state

#include <stdio.h>
int main(int argc, char *argv[]) {
        
        dasm_State *state;
        initjit(&state, actions);

        
        |  mov    dword [rsp-0x4], edi
        |  mov    dword [rsp-0x8], esi 
        |  mov    esi, dword [rsp-0x4] 
        |  imul   esi, dword [rsp-0x8]  
        |  mov    eax,esi
        |  ret

        // Link the code and write it to executable memory.
        int (*fptr)() = jitcode(&state);
        

        // Call the JIT-ted function.
        int ret = fptr(5, 11);
        printf("%d * %d = %d\n", 5, 11, ret);

        // Free the machine code.
        free_jitcode(fptr);

        return ret;
}
```
---
# 四、Part 1 總結
在 Part 1，我們實做了下列幾種方法

* 第一部分: 直接把機器碼放進 mmap 可執行記憶體區段執行，困難的點是要先知道功能對應的機器碼

* 第二部分: 利用 dynasm code generator 幫助我們生成機器碼，困難的點是要熟悉 dynasm 怎麼用，以及需要先具備一些組語的知識

* 第三部分: 我們利用稍微繞遠路的方式，先編譯函數，再用 objdump 反組譯得到機器碼跟組合語言，可以稍微改善前面兩部分的困難的點

但是要學編譯器跟 JIT，組合語言的知識遲早都得補完🤣

---
# 五、專案執行方式
1. 執行第一部份的 Simple JIT
```bash
make jit_toy; ./jittoy 56; echo $?
```
記得測試整數 **<=255**，因為程式回傳狀態只有 8 bits，小心溢位會出現 mod 256 的結果 

2. 執行第二部份的 dynasm JIT
```bash
make jit_dynasm; ./jit_dynasm 56; echo $?
```

3. 執行第三部份的 objdump + simple JIT

```bash
make clean; make obj_jit_toy; ./obj_jit_toy
```

4. 執行第三部份的 objdump + obj_jit_dynasm
```bash
make clean; make obj_jit_dynasm; ./obj_jit_dynasm
```


----
# 參考網站
1. [Hello, JIT World: The Joy of Simple JITs](https://blog.reverberate.org/2012/12/hello-jit-world-joy-of-simple-jits.html)

2. [必讀 : interpreter-compiler-jit](https://github.com/jserv/jit-construct)

3. [有哪些常用 JIT 算法？](https://www.zhihu.com/question/54748092/answer/141903877)

4. [JIT 編譯器](http://accu.cc/content/jit_tour/brainfuck_interpreter/)

5. [jitdemo 的 github ](https://github.com/haberman/jitdemo)

6. [basic-jit](https://nickdesaulniers.github.io/blog/2013/04/03/basic-jit/)


# 延伸閱讀
1.  [Matthew Page - How to JIT: Writing a Python JIT from scratch in pure Python - PyCon 2019](https://www.youtube.com/watch?v=2BB39q6cqPQ&t=905s)

    *   python 在寫較底層(mmap, memmove, peachpy)等，中間的坑不少。使用這些套件會牽涉到 windows 和 unix-like 系統的因素而有所不同，所以想要改成使用 C 語言
2. [PeachPy-類似 dynasm 一樣的套件](https://github.com/Maratyszcza/PeachPy)
3. [DolphinDB JIT教程](https://github.com/dolphindb/Tutorials_CN/blob/master/jit.md)