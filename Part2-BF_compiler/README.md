# Brainfuck
用最簡單的圖靈完全語言 Brainfuck 程式，寫一個 Brainfuck 的 compiler 及 interpreter

# 一、Brainfuck 簡介
* Brainfuck 是一個圖靈完全語言，你可以用它做一個作業系統，也可以做和其他程式語言一樣的事情
* 只需要無限長的紙帶 (tape)、計數器、當前指標位置及來源程式碼就好

* [Brainfuck wiki 語法](https://zh.wikipedia.org/wiki/Brainfuck)

* Brain fuck與 C 語言 對照表
    | Brainfuck | C               |含義    |
    |-----------|-----------------|--------|
    | >         | ++ptr;          |指標加一|
    | <         | --ptr;          |指標減一|
    | +         | ++*ptr;         |指標指向的位元組其值加一|
    | -         | --*ptr;         |指標指向的位元組其值減一|
    | .         | putchar(*ptr);  |輸出指標指向的位元組內容 (ASCII 碼)|
    | ,         | *ptr=getchar(); |輸入內容到指標指向的位元組 (ASCII 碼)|
    | [         | while(*ptr){    |如果指標指向的位元組其值為零，向後跳轉到對應的 ] 指令的次一指令處|
    | ]         | }               |如果指標指向的位元組其值不為零，向前跳轉到對應的 ] 指令的次一指令處|

    所以下列兩者是等價的
    ```bf
    +++++
    [
     -
    ]
    ```
    ```c
    *p += 5;
    while(0 != *p) {
            *p--;
    }
    ```

* 另外推薦一個網站，[Brainfuck Interpterer and Tape Visualizer](http://fatiherikli.github.io/brainfuck-visualizer/) 可以很清楚的看到 BF 怎麼運作
![](https://coldnew.github.io/6a7474d7/brainfuck_visual.png)

那先試試看執行 Brainfuck 吧，先安裝 brainfuck 編譯器
```bash
sudo apt install bf
```
之後建立 hello.bf
```
++++++++++[>+++++++>++++++++++>+++>+<<<<-]
>++.>+.+++++++..+++.>++.<<+++++++++++++++.
>.+++.------.--------.>+.>.
```
執行以下指令，即可得到 `Hello world!`
```bash
bf hello.bf
```

# 二、Brainfuck 編譯器及直譯器
誠如 [Jserv](http://blog.linux.org.tw/~jserv/archives/002119.html) 所言，打造一個 Brainfuck 編譯器是很簡單的，廣義的編譯器就是將一種語言轉成另
外一種語言，例如

* SaSS 之於 CSS
* Typescript 之於 Javascript
* C 之於 組合語言

等等，我們也可以將 C 語言視為 Brainfuck 的低階語言，並利用上面的對照表，直接用 `sed` 就可以實現了，以下程式也是 [Jserv's blog](http://blog.linux.org.tw/~jserv/archives/002119.html) 提供的

```sed
#! /bin/sed -f
s/[^][+<>.,-]//g
s/\([-+]\)/\1\1*p;/g
s/</p--;/g
s/>/p++;/g
s/\./putchar(*p);/g
s/,/*p=getchar();/g
s/\[/while (*p) {/g
s/\]/}/g

1s/^/#include <stdlib.h>\n#include <stdio.h> \nint main(void){char *p=calloc(1,10000);/
$s/$/}/
```

解釋:
1. `s/[^][+<>.,-]//g` : 把不是[^] `[+<>.,-]` 的字元變成空字串的 
2. `s/\([-+]\)/\1\1*p;/g` : 匹配到 - 或是 + 變成 `--*p` or `++*p`
3. `1s/^/#include <stdlib.h>\n#include <stdio.h> \nint main(void){char *p=calloc(1,10000);/` 在一開始^的地方插入 `#include <stdlib.h>...` 1s 為第一行
4. `$s/$/}/` : 在最後一行結尾加入 `}`

輸入指令
```
sed -f BF_2_C.sed hello.bf
```

輸出
```c
#include <stdlib.h>
#include <stdio.h> 
int main(void){char *p=calloc(1,10000);++*p;++*p;++*p;++*p;++*p;++*p;++*p;++*p;++*p;++*p;while (*p) {p++;++*p;++*p;++*p;++*p;++*p;++*p;++*p;p++;++*p;++*p;++*p;++*p;++*p;++*p;++*p;++*p;++*p;++*p;p++;++*p;++*p;++*p;p++;++*p;p--;p--;p--;p--;--*p;}
p++;++*p;++*p;putchar(*p);p++;++*p;putchar(*p);++*p;++*p;++*p;++*p;++*p;++*p;++*p;putchar(*p);putchar(*p);++*p;++*p;++*p;putchar(*p);p++;++*p;++*p;putchar(*p);p--;p--;++*p;++*p;++*p;++*p;++*p;++*p;++*p;++*p;++*p;++*p;++*p;++*p;++*p;++*p;++*p;putchar(*p);
p++;putchar(*p);++*p;++*p;++*p;putchar(*p);--*p;--*p;--*p;--*p;--*p;--*p;putchar(*p);--*p;--*p;--*p;--*p;--*p;--*p;--*p;--*p;putchar(*p);p++;++*p;putchar(*p);p++;putchar(*p);}
```





# 專案執行
1. 建立 `sed` 把 BF 轉成 C code 再編譯
```
make ; ./sed_BF
``` 




# 參考連結
1. [使用 ClojureScript 來寫 Brainfuck 的直譯器](https://coldnew.github.io/6a7474d7/)

2. [brainfuck 視覺化(推薦😀)](http://fatiherikli.github.io/brainfuck-visualizer/)

3. [打造 Brainfuck 的 JIT compiler](http://blog.linux.org.tw/~jserv/archives/002119.html)

