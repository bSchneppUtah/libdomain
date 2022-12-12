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
-3.463893769,
-5.830626289,
-8.410524206,
-11.00073912,
-20.11099575,
-8.130186283,
-109.6599942,
0.4275563709,
-4.168145764,
-4.045837161,
-8.012350567,
-8.393081138,
-21.41607738,
-8.042157422,
-109.2714124,
0.3253872367,
]


data = {}

for i in range(len(names)):
	data[names[i]] = [values[i]]

sns.barplot(data=pd.DataFrame(data), palette="Pastel1").set(title="Abs Error (64-bit)")
plt.show()

