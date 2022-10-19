#include <iostream>
#include <domain.hpp>

#define ARR_SIZE (9)

using MyVal = dom::Value<float>;
using MyArray = dom::Array<float, ARR_SIZE>;

uint64_t ToLinearAddr(int i, int j)
{
	return (j * 3) + i;
}

MyArray Function(const MyArray &Arr)
{
	MyArray RetVal;
	
	MyVal Coeffs[5];
	for (uint64_t Index = 0; Index < 5; Index++)
	{
		Coeffs[Index] = (dom::hpfloat)1.0;
	}
	
	int k = 0;	/* 2 is middle of [0, 4] */
	int j = 1;
	int i = 1;
	
	int offset = ToLinearAddr(i, j);
	RetVal[offset] = ((Coeffs[0] * Arr[ToLinearAddr(i+0,j+0)]) 
		+ (Coeffs[1] * Arr[ToLinearAddr(i+0,j+1)]))
		+ ((Coeffs[2] * Arr[ToLinearAddr(i+0,j-1)])
		+ (Coeffs[3] * Arr[ToLinearAddr(i+1,j+0)]))
		+ (Coeffs[4] * Arr[ToLinearAddr(i-1,j+0)]);
	return RetVal;
}

int main()
{
	dom::Init();
	std::cout.precision(128);

	MyArray L;
	MyArray R;
	for (int i = 0; i < L.GetSize(); i++)
	{
		L[i] = (dom::hpfloat)-1.0;
		R[i] = (dom::hpfloat)1.0;
	}

	dom::hpfloat Res = dom::FindError<float, ARR_SIZE>(L, R, Function, 10000, 10000, 50);
	std::cout << "Worst error: " << Res << std::endl;
}
