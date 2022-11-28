#include <iostream>
#include <domain.hpp>

#define ARR_SIZE (9)

using FType = float;
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
	
	/* 1 is middle of [0, 3] */
	int k = 1;
	int j = 1;
	int i = 1;
	
	int offset = ToLinearAddr(i, j, k);
	RetVal[offset] = 2.666 * Arr[ToLinearAddr(i, j, k)]
			- 0.166 * Arr[ToLinearAddr(i, j, k - 1)]
			- 0.166 * Arr[ToLinearAddr(i, j, k + 1)]
			- 0.166 * Arr[ToLinearAddr(i, j - 1, k)]
			- 0.166 * Arr[ToLinearAddr(i, j + 1, k)]
			- 0.166 * Arr[ToLinearAddr(i + 1, j, k)]
			- 0.166 * Arr[ToLinearAddr(i - 1, j, k)]
			- 0.0833 * Arr[ToLinearAddr(i, j - 1, k - 1)]
			- 0.0833 * Arr[ToLinearAddr(i, j - 1, k + 1)]
			- 0.0833 * Arr[ToLinearAddr(i, j + 1, k - 1)]
			- 0.0833 * Arr[ToLinearAddr(i, j + 1, k + 1)]
			- 0.0833 * Arr[ToLinearAddr(i - 1, j, k - 1)]
			- 0.0833 * Arr[ToLinearAddr(i - 1, j, k + 1)]
			- 0.0833 * Arr[ToLinearAddr(i - 1, j - 1, k)]
			- 0.0833 * Arr[ToLinearAddr(i - 1, j + 1, k)]
			- 0.0833 * Arr[ToLinearAddr(i + 1, j, k - 1)]
			- 0.0833 * Arr[ToLinearAddr(i + 1, j, k + 1)]
			- 0.0833 * Arr[ToLinearAddr(i + 1, j - 1, k)]
			- 0.0833 * Arr[ToLinearAddr(i + 1, j + 1, k)];
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
	std::cout << "LTR Poisson" << "\t" << Res.Err << "\t" << Res.RelErr << std::endl;
	return 0;
}
