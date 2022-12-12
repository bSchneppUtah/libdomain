import seaborn as sns
import matplotlib.pyplot as plt

import pandas as pd


names = [
"LTR 5pt",
"LTR 7pt",
"LTR 9pt",
"LTR 13pt",
"LTR 25pt",
"LTR 27pt",
"LTR 125pt",
"LTR Poisson",
"Bal 5pt",
"Bal 7pt",
"Bal 9pt",
"Bal 13pt",
"Bal 25pt",
"Bal 27pt",
"Bal 125pt",
"Bal Poisson",
]

values = [
2.509336793,
3.221024181,
3.659539341,
5.950443681,
15.15921393,
3.577050272,
29.61012838,
-0.7065161997,
1.634094252,
1.99260562,
2.98351975,
2.894286536,
5.184313574,
2.522412693,
31.42423519,
-0.4772359911,
]


data = {}

for i in range(len(names)):
	data[names[i]] = [values[i]]

sns.barplot(data=pd.DataFrame(data), palette="Pastel1").set(title="ULP Error (64-bit)")
plt.show()

