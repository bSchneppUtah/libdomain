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
2.374755859,
3.249989805,
3.632258788,
6.489165551,
14.95517273,
3.659755838,
31.77391682,
-0.669192569,
1.625305176,
2.049022357,
3.045052551,
3.325721574,
4.963671875,
2.490370811,
30.68415179,
-0.5481679786,
]


data = {}

for i in range(len(names)):
	data[names[i]] = [values[i]]

sns.barplot(data=pd.DataFrame(data), palette="Pastel1").set(title="ULP Error (64-bit)")
plt.show()

