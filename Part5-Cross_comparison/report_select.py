import matplotlib.pyplot as plt 
import seaborn as sns
import pandas as pd
import datetime
import argparse
import sys

if __name__ == "__main__":
        

        parser = argparse.ArgumentParser()
        parser.add_argument('-cp','--compare', nargs='+', help='[-cp/--compare] <row1> <row2>...', type=str)
        parser.add_argument('-o','--output', nargs=1, help='[-o/--output] <png name>', type=str)
        
        args = parser.parse_args()
        
        data = pd.read_csv("summarize_time")
        data['Execution Time'] = data["system"] + data["user"]
        data = data[data.program.isin(args.compare)].reset_index(drop=True)
        data = data.sort_values(by=['Execution Time'], ascending=False).reset_index(drop=True)
        
        print(data)

        sns.set()
        fig = plt.figure(figsize=(20, 5))
        plt.title("Comparison")
        
        for index, value in enumerate(data['Execution Time']):
                plt.text(x=index-0.3, y=value+0.2 , s=f"{value:.2f} sec" , fontdict=dict(fontsize=10))
        sns.barplot(x="program",y="Execution Time",data=data,ci=95)
        plt.savefig(args.output[0])
        plt.show()
        



