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
1.05E-15,
2.16E-15,
3.23E-15,
5.76E-15,
1.66E-14,
3.25E-15,
4.94E-14,
1.49E-16,
1.08E-15,
1.36E-15,
2.70E-15,
2.95E-15,
5.51E-15,
2.21E-15,
4.77E-14,
1.22E-16,
]


data = {}

for i in range(len(names)):
	data[names[i]] = [values[i]]

sns.set(font_scale=1.4)
sns.barplot(data=pd.DataFrame(data), palette="bright").set(title="Abs Error (64-bit)")
plt.xticks(rotation=45)
plt.show()

