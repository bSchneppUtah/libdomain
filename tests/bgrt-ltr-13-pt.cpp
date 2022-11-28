#include <iostream>
#include <domain.hpp>

#define ARR_SIZE (13)

using FType = float;
using Val = dom::Value<FType>;
using Var = bgrt::Variable<FType>;
using Array = std::unordered_map<uint64_t, Val>;
using Conf = std::unordered_map<uint64_t, Var>;

uint64_t ToLinearAddr(int i, int j, int k)
{
	/* Transform into center at [0, 0, 0] */
	int ti = i-2;
	int tj = j-2;
	int tk = k-2;

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
			return 5;
		} else if (tj == 2)
		{
			return 6;
		} 
		else if (tj == -1)
		{
			return 7;
		}
		else if (tj == -2)
		{
			return 8;
		}
	}

	if (tk)
	{
		if (tk == 1)
		{
			return 9;
		}
		else if (tk == 2)
		{
			return 10;
		} 
		else if (tk == -1)
		{
			return 11;
		}
		else if (tk == -2)
		{
			return 12;
		}
	}

	return -1;

}

Array Function(Array &Arr)
{
	Array RetVal;
	
	Val Coeffs[13];
	for (uint64_t Index = 0; Index < 13; Index++)
	{
		Coeffs[Index] = (dom::hpfloat)1.0;
	}
	
	/* 1 is middle of [0, 2] */
	int k = 1;
	int j = 1;
	int i = 1;
	
	int offset = ToLinearAddr(i, j, k);
	RetVal[offset] = (Coeffs[0] * Arr[ToLinearAddr(i+0,j+0,k+0)])
				+ (Coeffs[1] * Arr[ToLinearAddr(i+1,j+0,k+0)])
				+ (Coeffs[2] * Arr[ToLinearAddr(i-1,j+0,k+0)])
				+ (Coeffs[3] * Arr[ToLinearAddr(i+0,j+1,k+0)])
				+ (Coeffs[4] * Arr[ToLinearAddr(i+0,j-1,k+0)])
				+ (Coeffs[5] * Arr[ToLinearAddr(i+0,j+0,k+1)])
				+ (Coeffs[6] * Arr[ToLinearAddr(i+0,j+0,k-1)])
				+ (Coeffs[7] * Arr[ToLinearAddr(i+2,j+0,k+0)])
				+ (Coeffs[8] * Arr[ToLinearAddr(i-2,j+0,k+0)])
				+ (Coeffs[9] * Arr[ToLinearAddr(i+0,j+2,k+0)])
				+ (Coeffs[10] * Arr[ToLinearAddr(i+0,j-2,k+0)])
				+ (Coeffs[11] * Arr[ToLinearAddr(i+0,j+0,k+2)])
				+ (Coeffs[12] * Arr[ToLinearAddr(i+0,j+0,k-2)]);
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
	std::cout << "LTR 13pt" << "\t" << Res.Err << "\t" << Res.RelErr << std::endl;
	return 0;
}
