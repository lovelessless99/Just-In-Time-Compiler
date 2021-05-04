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
make ; ./jit_toy 5566; echo $?
```


# 二、Hello, DynASM World!
[DynASM 官方網頁](https://luajit.org/dynasm.html)，特色如下

1. DynASM 是一個程式碼生成的動態組譯器
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





# 參考網站
1. [Hello, JIT World: The Joy of Simple JITs](https://blog.reverberate.org/2012/12/hello-jit-world-joy-of-simple-jits.html)

2. [必讀 : interpreter-compiler-jit](https://github.com/jserv/jit-construct)

3. [有哪些常用 JIT 算法？](https://www.zhihu.com/question/54748092/answer/141903877)

4. [JIT 編譯器](http://accu.cc/content/jit_tour/brainfuck_interpreter/)

5. [jitdemo 的 github ](https://github.com/haberman/jitdemo)

# 延伸閱讀
1.  [Matthew Page - How to JIT: Writing a Python JIT from scratch in pure Python - PyCon 2019](https://www.youtube.com/watch?v=2BB39q6cqPQ&t=905s)

    *   python 在寫較底層(mmap, memmove, peachpy)等，中間的坑不少。使用這些套件會牽涉到 windows 和 unix-like 系統的因素而有所不同，所以想要改成使用 C 語言
2. [PeachPy-類似 dynasm 一樣的套件](https://github.com/Maratyszcza/PeachPy)
3. [DolphinDB JIT教程](https://github.com/dolphindb/Tutorials_CN/blob/master/jit.md)