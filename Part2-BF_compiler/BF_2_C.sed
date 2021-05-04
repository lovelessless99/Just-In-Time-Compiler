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