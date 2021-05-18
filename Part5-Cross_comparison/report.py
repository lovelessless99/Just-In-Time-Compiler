import matplotlib.pyplot as plt 
import seaborn as sns
import pandas as pd
import datetime
import sys

if __name__ == "__main__":
        
        if len(sys.argv) == 1 : 
                print("python3 report.py <csv_file>")
                exit(1)

        file_name = sys.argv[1]
        data = pd.read_csv(file_name)
        data['Execution Time'] = data["system"] + data["user"]
        data = data.sort_values(by=['Execution Time'], ascending=False).reset_index(drop=True)
        print(data)

        sns.set()
        fig = plt.figure(figsize=(30, 5))
        plt.title("Comparison")
        
        for index, value in enumerate(data['Execution Time']):
                plt.text(x=index-0.3, y=value+0.2 , s=f"{value:.2f} sec" , fontdict=dict(fontsize=10))
        sns.barplot(x="program",y="Execution Time",data=data,ci=95)
        plt.savefig("comparison.png")
        plt.show()
        



