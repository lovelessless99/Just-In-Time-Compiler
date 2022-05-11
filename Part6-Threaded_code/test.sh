#!/bin/bash
Interpreter=BF_interpreter
thread_code=thread_code_interpreter

echo "========== 1. Interpreter =========="
# using code block, and without subshell
/usr/bin/time --quiet -f "Interpreter,%E,%U,%S"  ./${Interpreter} mandelbrot.bf > /dev/null

echo "========== 2. Interpreter (with threaded code) =========="
# using code block, and without subshell
/usr/bin/time --quiet -f "threaded,%E,%U,%S"  ./${thread_code} mandelbrot.bf > /dev/null
