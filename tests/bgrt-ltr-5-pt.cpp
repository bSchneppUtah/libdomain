#include <iostream>
#include <domain.hpp>

#define ARR_SIZE (5)

using FType = float;
using Val = dom::Value<FType>;
using Var = bgrt::Variable<FType>;
using Array = std::unordered_map<uint64_t, Val>;
using Conf = std::unordered_map<uint64_t, Var>;

uint64_t ToLinearAddr(int i, int j)
{
	/* The actual array is [0, 1, 2, 3, 4]
	 * To avoid making unnecessary variables, only 5 will be made.
	 * The first 3 are conditional on i = 0, 1, 2, and j = 0.
	 * The remainder are the (0, 1) and (0, -1) cases, at 3 and 4. 
	 */
	if (j == 0)
	{
		return i;
	}

	if (j == 1)
	{
		return 3;
	}
	if (j == 5)
	{
		return 4;
	}
	return 0;
}

Array Function(Array &Arr)
{
	Array RetVal;
	
	Val Coeffs[5];
	for (uint64_t Index = 0; Index < 5; Index++)
	{
		Coeffs[Index] = (dom::hpfloat)1.0;
	}
	
	/* 1 is middle of [0, 2] */
	int j = 1;
	int i = 1;
	
	int offset = ToLinearAddr(i, j);
	RetVal[offset] = (Coeffs[0] * Arr[ToLinearAddr(i+0,j+0)]) 
		+ (Coeffs[1] * Arr[ToLinearAddr(i+0,j+1)])
		+ (Coeffs[2] * Arr[ToLinearAddr(i+0,j-1)])
		+ (Coeffs[3] * Arr[ToLinearAddr(i+1,j+0)])
		+ (Coeffs[4] * Arr[ToLinearAddr(i-1,j+0)]);
	return RetVal;
}

int main()
{
	dom::Init();
	std::cout.precision(128);

	Conf Init;
	for (int i = 0; i < ARR_SIZE; i++)
	{
		Init[i] = Var((dom::hpfloat)0.0, (dom::hpfloat)1.0);
	}

	dom::hpfloat Res = dom::FindErrorMultithread<FType>(Init, Function);
	std::cout << "Worst error: " << Res << std::endl;
}
