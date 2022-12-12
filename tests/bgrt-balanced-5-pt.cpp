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

	Conf Init;
	for (int i = 0; i < ARR_SIZE; i++)
	{
		Init[i] = bgrt::Variable<float>((dom::hpfloat)-1.0, (dom::hpfloat)1.0);
	}

	auto Start = std::chrono::high_resolution_clock::now();
	dom::EvalResults Res = dom::FindErrorMantissaMultithread<float>(Init, Function);
	auto End = std::chrono::high_resolution_clock::now();
	auto Duration = std::chrono::duration_cast<std::chrono::milliseconds>(End - Start);

	std::string TestName = "Balanced 5pt";
	const dom::hpfloat logCorrect = log2(abs(Res.CorrectValue), dom::HP_ROUNDING);
	const dom::hpfloat Binade = ceil(logCorrect);
	const dom::hpfloat Eps = std::numeric_limits<FType>::epsilon();
	const dom::hpfloat ULPError = Res.Err / (Binade * Eps);
	
	std::cout << "\tAbsolute Error\tRelative Error\tTime taken (ms)\tCorrect Number\tULP Error" << std::endl;
	std::cout << TestName << "\t" << Res.Err << "\t" << Res.RelErr << "\t" << Duration.count() << "\t" << Res.CorrectValue << "\t" << ULPError << std::endl;
	return 0;
}
