#include <iostream>
#include <domain.hpp>

#define ARR_SIZE (9)

using FType = float;
using Val = dom::Value<FType>;
using Var = bgrt::Variable<FType>;
using Array = std::unordered_map<uint64_t, Val>;
using Conf = std::unordered_map<uint64_t, Var>;

uint64_t ToLinearAddr(int i, int j)
{
	return i + (3 * j);
}

Array Function(Array &Arr)
{
	Array RetVal;
	
	Val Coeffs[9];
	for (uint64_t Index = 0; Index < 9; Index++)
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
		+ (Coeffs[3] * Arr[ToLinearAddr(i+1,j+1)])
		+ (Coeffs[4] * Arr[ToLinearAddr(i+1,j-1)])
		+ (Coeffs[5] * Arr[ToLinearAddr(i-1,j+1)])
		+ (Coeffs[6] * Arr[ToLinearAddr(i-1,j-1)])
		+ (Coeffs[7] * Arr[ToLinearAddr(i+1,j+0)])
		+ (Coeffs[8] * Arr[ToLinearAddr(i-1,j+0)]);
	return RetVal;
}

int main()
{
	dom::Init();
	std::cout.precision(128);

	Conf Init;
	for (int i = 0; i < ARR_SIZE; i++)
	{
		Init[i] = bgrt::Variable<float>((dom::hpfloat)-1.0, (dom::hpfloat)1.0);
	}

	dom::EvalResults Res = dom::FindErrorMantissaMultithread<float>(Init, Function);
	std::cout << "Absolute error: " << Res.Err << ", " << "Relative error: " << Res.RelErr << std::endl;
}
