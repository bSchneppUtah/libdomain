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
2.97E-16,
3.84E-16,
4.03E-16,
4.83E-16,
7.40E-16,
3.93E-16,
4.47E-16,
3.22E-16,
2.67E-16,
2.58E-16,
3.34E-16,
3.05E-16,
2.37E-16,
2.76E-16,
4.21E-16,
3.30E-16,
]


data = {}

for i in range(len(names)):
	data[names[i]] = [values[i]]

sns.set(font_scale=1.4)
sns.barplot(data=pd.DataFrame(data), palette="bright").set(title="Rel Error (64-bit)")
plt.xticks(rotation=45)
plt.show()

