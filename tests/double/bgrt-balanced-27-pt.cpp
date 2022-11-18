#include <iostream>
#include <domain.hpp>

#define ARR_SIZE (27)

using FType = double;
using Val = dom::Value<FType>;
using Var = bgrt::Variable<FType>;
using Array = std::unordered_map<uint64_t, Val>;
using Conf = std::unordered_map<uint64_t, Var>;

uint64_t ToLinearAddr(int i, int j, int k)
{
	return i + (3 * j) + (9 * k);
}

Array Function(Array &Arr)
{
	Array RetVal;
	
	Val Coeffs[27];
	for (uint64_t Index = 0; Index < 9; Index++)
	{
		Coeffs[Index] = (dom::hpfloat)1.0;
	}
	
	/* 1 is middle of [0, 2] */
	int k = 1;
	int j = 1;
	int i = 1;
	
	int offset = ToLinearAddr(i, j, k);
	RetVal[offset] = ((((((Coeffs[0] * Arr[ToLinearAddr(i+0,j+0,k+0)]) 
		+ (Coeffs[1] * Arr[ToLinearAddr(i+0,j+1,k+0)]))
		+ ((Coeffs[2] * Arr[ToLinearAddr(i+0,j-1,k+0)])
		+ (Coeffs[3] * Arr[ToLinearAddr(i+1,j+1,k+0)])))
		+ (((Coeffs[4] * Arr[ToLinearAddr(i+1,j-1,k+0)])
		+ (Coeffs[5] * Arr[ToLinearAddr(i-1,j+1,k+0)]))
		+ ((Coeffs[6] * Arr[ToLinearAddr(i-1,j-1,k+0)])
		+ (Coeffs[7] * Arr[ToLinearAddr(i+1,j+0,k+0)]))))
		+ ((((Coeffs[8] * Arr[ToLinearAddr(i-1,j+0,k+0)])		
		+ (Coeffs[9] * Arr[ToLinearAddr(i+0,j+0,k+1)])) 
		+ ((Coeffs[10] * Arr[ToLinearAddr(i+0,j+1,k+1)])
		+ (Coeffs[11] * Arr[ToLinearAddr(i+0,j-1,k+1)])))
		+ (((Coeffs[12] * Arr[ToLinearAddr(i+1,j+1,k+1)])
		+ (Coeffs[13] * Arr[ToLinearAddr(i+1,j-1,k+1)]))
		+ ((Coeffs[14] * Arr[ToLinearAddr(i-1,j+1,k+1)])
		+ (Coeffs[15] * Arr[ToLinearAddr(i-1,j-1,k+1)])))))
		+ (((((Coeffs[16] * Arr[ToLinearAddr(i+1,j+0,k+1)])
		+ (Coeffs[17] * Arr[ToLinearAddr(i-1,j+0,k+1)]))
		+ ((Coeffs[18] * Arr[ToLinearAddr(i+0,j+0,k-1)]) 
		+ (Coeffs[19] * Arr[ToLinearAddr(i+0,j+1,k-1)])))
		+ (((Coeffs[20] * Arr[ToLinearAddr(i+0,j-1,k-1)])
		+ (Coeffs[21] * Arr[ToLinearAddr(i+1,j+1,k-1)]))
		+ ((Coeffs[22] * Arr[ToLinearAddr(i+1,j-1,k-1)])
		+ (Coeffs[23] * Arr[ToLinearAddr(i-1,j+1,k-1)]))))
		+ ((((Coeffs[24] * Arr[ToLinearAddr(i-1,j-1,k-1)])
		+ (Coeffs[25] * Arr[ToLinearAddr(i+1,j+0,k-1)]))
		+ ((Coeffs[26] * Arr[ToLinearAddr(i-1,j+0,k-1)]))))));
	return RetVal;
}

int main()
{
	dom::Init();
	std::cout.precision(128);

	Conf Init;
	for (int i = 0; i < ARR_SIZE; i++)
	{
		Init[i] = bgrt::Variable<FType>((dom::hpfloat)-1.0, (dom::hpfloat)1.0);
	}

	dom::EvalResults Res = dom::FindErrorMantissaMultithread<FType>(Init, Function);
	std::cout << "Absolute error: " << Res.Err << ", " << "Relative error: " << Res.RelErr << std::endl;
}
