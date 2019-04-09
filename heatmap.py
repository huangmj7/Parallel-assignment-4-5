import numpy as np
import seaborn as sb
import matplotlib.pyplot as plt

filename = "0.txt" #the name of output 1k*1k file, it test, please replace the 1k*1k text file outputed by text.c
file = open(filename,mode='r',encoding='utf-8')
Map = file.read().split("\n")
print(Map[1024])

data = []
for i in range(1024):
    temp = Map[i].split(" ")
    for x in range(1024):
        data.append(temp[x])

print(len(data))

hdata = np.array(data,dtype=np.float32).reshape((1024,1024))
print(hdata[1023])
#heat_map = sb.heatmap(hdata,cmap="YlOrRd",annot=True, cbar_kws={'label': 'Colorbar'},xticklabels=False, yticklabels=False)
heat_map = sb.heatmap(hdata,cmap="YlOrRd",cbar_kws={'label': 'Colorbar'},xticklabels=False, yticklabels=False)
plt.xlabel("X axis")
plt.ylabel("Y axis")
plt.show()
