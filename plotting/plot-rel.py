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
1.73E-07,
1.98E-07,
2.07E-07,
2.58E-07,
4.49E-07,
2.10E-07,
2.25E-07,
1.97E-07,
1.40E-07,
1.76E-07,
1.78E-07,
1.64E-07,
1.44E-07,
1.50E-07,
2.40E-07,
1.75E-07,
]


data = {}

for i in range(len(names)):
	data[names[i]] = [values[i]]

sns.barplot(data=pd.DataFrame(data), palette="Pastel1").set(title="Rel Error (64-bit)")
plt.show()

