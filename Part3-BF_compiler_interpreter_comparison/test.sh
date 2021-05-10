#!/bin/bash

output_file=summarize_time


echo "========== Stage 1 : File Checking =========="
if [[ ! -f "Stage-1/summarize_time" ]]; then
        echo -e "\e[91mError! Stage-1/summarize_time doesn't exist \e[39m"
        exit 1
else
        echo -e "\e[92mStage-1/summarize_time exist! \e[39m"
fi
if [[ ! -f "Stage-2/summarize_time" ]]; then
        echo -e "\e[91mError! Stage-2/summarize_time doesn't exist \e[39m"
        exit 1
else
        echo -e "\e[92mStage-2/summarize_time exist! \e[39m"
fi

echo -e  "\e[92mCongraduation! you can start to test !\e[39m"

echo "========== Stage 2 : Merge two stages results =========="
python - <<EOF
import pandas as pd
data1 = pd.read_csv("Stage-1/summarize_time")
data2 = pd.read_csv("Stage-2/summarize_time")
all_data = pd.concat([data1, data2]).drop_duplicates(subset ="program",keep="first")
all_data.to_csv("total_time_summarize", index=False)
EOF

echo -e  "\e[92mMerge Success! you can start to test !\e[39m"

echo "========== Stage 3 : Final Output and output the plot  =========="
        python3 report.py total_time_summarize
        echo -e "\e[92mcomparison.png is created, please check it out!\e[39m"
