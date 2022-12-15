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
-3.553885434,
-5.631327497,
-8.001902519,
-11.94064166,
-22.42717198,
-8.274845454,
-110.4345196,
0.4617453218,
-4.05996024,
-5.283439875,
-8.085929475,
-9.695849896,
-23.22554151,
-8.003954044,
-113.2525056,
0.3683807735,
]


data = {}

for i in range(len(names)):
	data[names[i]] = [values[i]]

sns.set(font_scale=1.4)
sns.barplot(data=pd.DataFrame(data), palette="bright").set(title="Correct No. (64-bit)")
plt.xticks(rotation=45)
plt.show()

