#include <iostream>
#include <domain.hpp>

#define ARR_SIZE (7)

using Val = dom::Value<float>;
using Var = bgrt::Variable<float>;
using Array = std::unordered_map<uint64_t, Val>;
using Conf = std::unordered_map<uint64_t, Var>;

uint64_t ToLinearAddr(int i, int j, int k)
{
	/* Transform into center at [0, 0, 0] */
	int ti = i-1;
	int tj = j-1;
	int tk = k-1;

	/* Use a property of C integers to determine if this is the i-axis being worked on.
	 * If ti is NOT zero, then it's on that axis. Otherwise, it's the [0, 0, 0] point is none of them are not non-zero.
	 * Since the original coordinate was transformed from [0, 2] to [-1, 1], just use [0, 2] as the variable index.
	 */
	if (ti || (!tj && !tk))
	{
		return i;
	}

	if (tj)
	{
		if (tj == 1)
		{
			return 3;
		} 
		else if (tj == -1)
		{
			return 4;
		}
	}

	if (tk)
	{
		if (tk == 1)
		{
			return 5;
		} 
		else if (tk == -1)
		{
			return 6;
		}
	}

	return -1;

}

Array Function(Array &Arr)
{
	Array RetVal;

	Val Coeffs[7];
	for (uint64_t Index = 0; Index < 7; Index++)
	{
		Coeffs[Index] = (dom::hpfloat)1.0;
	}
	
	int k = 1;	/* 1 is middle of [0, 2] */
	int j = 1;
	int i = 1;
	
	int offset = ToLinearAddr(i, j, k);	
	RetVal[offset] = (Coeffs[0] * Arr[ToLinearAddr(i+0,j+0, k+0)])
				+ (Coeffs[1] * Arr[ToLinearAddr(i+1,j+0,k+0)])
				+ (Coeffs[2] * Arr[ToLinearAddr(i-1,j+0,k+0)])
				+ (Coeffs[3] * Arr[ToLinearAddr(i+0,j+1,k+0)])
				+ (Coeffs[4] * Arr[ToLinearAddr(i+0,j-1,k+0)])
				+ (Coeffs[5] * Arr[ToLinearAddr(i+0,j+0,k+1)])
				+ (Coeffs[6] * Arr[ToLinearAddr(i+0,j+0,k-1)]);
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
	std::cout << "\tAbsolute Error\tRelative Error" << std::endl;
	std::cout << "LTR 7pt" << "\t" << Res.Err << "\t" << Res.RelErr << std::endl;
	return 0;
}
