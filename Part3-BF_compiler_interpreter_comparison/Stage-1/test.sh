#! bin/bash

interpreter=BF_interpreter
compiler=mandelbrot-compiler
sed_BF=sed_BF

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

echo -e  "\e[92mCongraduation! you can start to test !\e[39m"








echo "========== Stage 2 : Testing the mandelbort =========="
echo -e "program,real,user,system" > summarize_time

echo "========== 1. Testing interpreter ( wait in patience ) =========="
{ /usr/bin/time --quiet -f "interpreter,%E,%U,%S"  ./${interpreter} ../mandelbrot.bf > /dev/null ; } 2>> summarize_time
echo -e "\e[92mInterpreter Success!\e[39m"

echo "========== 2. Testing compiler =========="
# using code block, and without subshell
{ /usr/bin/time --quiet -f "compiler,%E,%U,%S"  ./${compiler} > /dev/null ; } 2>> summarize_time
echo -e "\e[92mcompiler Success!\e[39m"

echo "========== 3. Testing sed =========="
{ /usr/bin/time --quiet -f "sed,%E,%U,%S"  ./${sed_BF} > /dev/null ; } 2>> summarize_time
echo -e "\e[92msed Success!\e[39m"

echo "========== Stage 3 : Final Output and output the plot  =========="
        python3 ../report.py summarize_time
        echo -e "\e[92mcomparison.png is created, please check it out!\e[39m"
