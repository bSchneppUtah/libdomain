#include <iostream>
#include <domain.hpp>

#define ARR_SIZE (4)

using MyVal = dom::Value<float>;
using MyArray = dom::Array<float, ARR_SIZE>;

MyArray Function(const MyArray &Arr)
{
	MyVal Summation;
	MyArray RetVal;

	for (int i = 1; i < Arr.GetSize() - 1; i++)
	{
		RetVal[i] = (dom::hpfloat)0;
		RetVal[i] += Arr[i] * (dom::hpfloat)0.333333333333333333333333333333333;
		RetVal[i] -= Arr[i-1] * (dom::hpfloat)0.7777777777777777777777777777777;
		RetVal[i] += Arr[i+1] * (dom::hpfloat)0.33333333333333333333333333333333;
	}

	return Arr;
}

int main()
{
	dom::Init();
	std::cout.precision(128);

	MyArray L;
	MyArray R;
	for (int i = 0; i < L.GetSize(); i++)
	{
		L[i] = (dom::hpfloat)-1.1;
		R[i] = (dom::hpfloat)1.1;
	}

	dom::hpfloat Res = dom::FindError<float, ARR_SIZE>(L, R, 1000, 100, 50, Function);
	std::cout << "Worst error: " << Res << std::endl;
}
