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
5.98E-07,
1.15E-06,
1.75E-06,
2.84E-06,
9.04E-06,
1.71E-06,
2.47E-05,
8.42E-08,
5.84E-07,
7.13E-07,
1.42E-06,
1.38E-06,
3.09E-06,
1.20E-06,
2.62E-05,
5.69E-08,
]


data = {}

for i in range(len(names)):
	data[names[i]] = [values[i]]

sns.barplot(data=pd.DataFrame(data), palette="Pastel1").set(title="Abs Error (64-bit)")
plt.show()

