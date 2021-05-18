#! bin/bash

interpreter=BF_interpreter
compiler=mandelbrot-compiler
sed_BF=sed_BF
sed_BF_O1=sed_BF_O1
sed_BF_O2=sed_BF_O2
sed_BF_O3=sed_BF_O3

BF_INTERPRETER_opt=BF_interpreter_opt_1 
BF_INTERPRETER_opt2=BF_interpreter_opt_2 
BF_INTERPRETER_opt3=BF_interpreter_opt_3

JIT_DYNASM_FILE=jit_dynasm 
JIT_DYNASM_FILE_opt=jit_dynasm_opt1 
JIT_DYNASM_FILE_opt2=jit_dynasm_opt2

JIT_OPCODE=jit_opcode

echo "========== Stage 1 : Executable File Checking =========="
if [[ ! -f ${interpreter} ]]; then
        echo -e "\e[91mError! ${interpreter} doesn't exist \e[39m"
        exit 1
else
        echo -e "\e[92m${interpreter} exist! \e[39m"
fi
if [[ ! -f ${compiler} ]]; then
        echo -e "\e[91mError! ${compiler} doesn't exist \e[39m"
        exit 1
else
        echo -e "\e[92m${compiler} exist! \e[39m"
fi

if [[ ! -f ${sed_BF} ]]; then
        echo -e "\e[91mError! ${sed_BF} doesn't exist \e[39m"
        exit 1
else
        echo -e "\e[92m${sed_BF} exist! \e[39m"
fi

if [[ ! -f ${sed_BF_O1} ]]; then
        echo -e "\e[91mError! ${sed_BF_O1} doesn't exist \e[39m"
        exit 1
else
        echo -e "\e[92m${sed_BF_O1} exist! \e[39m"
fi

if [[ ! -f ${sed_BF_O2} ]]; then
        echo -e "\e[91mError! ${sed_BF_O2} doesn't exist \e[39m"
        exit 1
else
        echo -e "\e[92m${sed_BF_O2} exist! \e[39m"
fi

if [[ ! -f ${sed_BF_O3} ]]; then
        echo -e "\e[91mError! ${sed_BF_O3} doesn't exist \e[39m"
        exit 1
else
        echo -e "\e[92m${sed_BF_O3} exist! \e[39m"
fi

if [[ ! -f ${BF_INTERPRETER_opt} ]]; then
        echo -e "\e[91mError! ${BF_INTERPRETER_opt} doesn't exist \e[39m"
        exit 1
else
        echo -e "\e[92m${BF_INTERPRETER_opt} exist! \e[39m"
fi

if [[ ! -f ${BF_INTERPRETER_opt2} ]]; then
        echo -e "\e[91mError! ${BF_INTERPRETER_opt2} doesn't exist \e[39m"
        exit 1
else
        echo -e "\e[92m${BF_INTERPRETER_opt2} exist! \e[39m"
fi

if [[ ! -f ${BF_INTERPRETER_opt3} ]]; then
        echo -e "\e[91mError! ${BF_INTERPRETER_opt3} doesn't exist \e[39m"
        exit 1
else
        echo -e "\e[92m${BF_INTERPRETER_opt3} exist! \e[39m"
fi

if [[ ! -f ${JIT_DYNASM_FILE} ]]; then
        echo -e "\e[91mError! ${JIT_DYNASM_FILE} doesn't exist \e[39m"
        exit 1
else
        echo -e "\e[92m${JIT_DYNASM_FILE} exist! \e[39m"
fi

if [[ ! -f ${JIT_DYNASM_FILE_opt} ]]; then
        echo -e "\e[91mError! ${JIT_DYNASM_FILE_opt} doesn't exist \e[39m"
        exit 1
else
        echo -e "\e[92m${JIT_DYNASM_FILE_opt} exist! \e[39m"
fi

if [[ ! -f ${JIT_DYNASM_FILE_opt2} ]]; then
        echo -e "\e[91mError! ${JIT_DYNASM_FILE_opt2} doesn't exist \e[39m"
        exit 1
else
        echo -e "\e[92m${JIT_DYNASM_FILE_opt2} exist! \e[39m"
fi

if [[ ! -f ${JIT_OPCODE} ]]; then
        echo -e "\e[91mError! ${JIT_OPCODE} doesn't exist \e[39m"
        exit 1
else
        echo -e "\e[92m${JIT_OPCODE} exist! \e[39m"
fi

echo -e  "\e[92mCongraduation! you can start to test !\e[39m"

echo "========== Stage 2 : Testing the mandelbort =========="
echo -e "program,real,user,system" > summarize_time

echo "========== 1. Testing interpreter ( wait in patience ) =========="
{ /usr/bin/time --quiet -f "interpreter,%E,%U,%S"  ./${interpreter} mandelbrot.bf > /dev/null ; } 2>> summarize_time
echo -e "\e[92mInterpreter Success!\e[39m"

echo "========== 2. Testing compiler =========="
# using code block, and without subshell
{ /usr/bin/time --quiet -f "compiler,%E,%U,%S"  ./${compiler} > /dev/null ; } 2>> summarize_time
echo -e "\e[92mcompiler Success!\e[39m"

echo "========== 3. Testing sed (no optimize)=========="
{ /usr/bin/time --quiet -f "sed,%E,%U,%S"  ./${sed_BF} > /dev/null ; } 2>> summarize_time
echo -e "\e[92msed  Success!\e[39m"


echo "========== 4. Testing sed (optimize O1)=========="
{ /usr/bin/time --quiet -f "sed_O1,%E,%U,%S"  ./${sed_BF_O1} > /dev/null ; } 2>> summarize_time
echo -e "\e[92msed O1 Success!\e[39m"


echo "========== 5. Testing sed (optimize O2)=========="
{ /usr/bin/time --quiet -f "sed_O2,%E,%U,%S"  ./${sed_BF_O2} > /dev/null ; } 2>> summarize_time
echo -e "\e[92msed O2 Success!\e[39m"


echo "========== 6. Testing sed (optimize O3)=========="
{ /usr/bin/time --quiet -f "sed_O3,%E,%U,%S"  ./${sed_BF_O3} > /dev/null ; } 2>> summarize_time
echo -e "\e[92msed O3 Success!\e[39m"

echo "========== 7. Testing optimal interpreter 1 =========="
{ /usr/bin/time --quiet -f "interpreter_opt1,%E,%U,%S"  ./${BF_INTERPRETER_opt} mandelbrot.bf > /dev/null ; } 2>> summarize_time
echo -e "\e[92mInterpreter opt1 Success!\e[39m"

echo "========== 8. Testing optimal interpreter 2 =========="
{ /usr/bin/time --quiet -f "interpreter_opt2,%E,%U,%S"  ./${BF_INTERPRETER_opt2} mandelbrot.bf > /dev/null ; } 2>> summarize_time
echo -e "\e[92minterpreter opt2 Success!\e[39m"

echo "========== 9. Testing optimal interpreter 3 =========="
{ /usr/bin/time --quiet -f "interpreter_opt3,%E,%U,%S"  ./${BF_INTERPRETER_opt3} mandelbrot.bf > /dev/null ; } 2>> summarize_time
echo -e "\e[92minterpreter opt3 Success!\e[39m"


echo "========== 10. Testing jit_dynasm =========="
{ /usr/bin/time --quiet -f "jit_dynasm,%E,%U,%S"  ./${JIT_DYNASM_FILE} mandelbrot.bf > /dev/null ; } 2>> summarize_time
echo -e "\e[92mjit_dynasm Success!\e[39m"

echo "========== 11. Testing optimal jit_dynasm 1 =========="
{ /usr/bin/time --quiet -f "jit_dynasm_opt1,%E,%U,%S"  ./${JIT_DYNASM_FILE_opt} mandelbrot.bf > /dev/null ; } 2>> summarize_time
echo -e "\e[92mjit_dynasm opt1 Success!\e[39m"

echo "========== 12. Testing optimal jit_dynasm 2 =========="
{ /usr/bin/time --quiet -f "jit_dynasm_opt2,%E,%U,%S"  ./${JIT_DYNASM_FILE_opt2} mandelbrot.bf > /dev/null ; } 2>> summarize_time
echo -e "\e[92mjit_dynasm opt2 Success!\e[39m"


echo "========== 13. Testing jit_opcode =========="
{ /usr/bin/time --quiet -f "jit_opcode,%E,%U,%S"  ./${JIT_OPCODE} mandelbrot.bf > /dev/null ; } 2>> summarize_time
echo -e "\e[92mjit_opcode Success!\e[39m"


echo "========== Stage 3 : Final Output and output the plot  =========="
        python3 report.py summarize_time
        python3 report_select.py -cp interpreter interpreter_opt1 interpreter_opt2  interpreter_opt3  sed -o interpreter.png
        python3 report_select.py -cp interpreter_opt1 interpreter_opt2  interpreter_opt3 jit_dynasm -o opt_int_simple_jit.png
        python3 report_select.py -cp jit_dynasm_opt1 jit_dynasm_opt2 jit_opcode compiler sed_O1 sed_O2 sed_O3 -o faster.png 

        echo -e "\e[92mcomparison.png is created, please check it out!\e[39m"
