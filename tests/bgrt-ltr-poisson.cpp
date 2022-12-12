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
	RetVal[offset] = (((((((((((((((((((2.666 * Arr[ToLinearAddr(i, j, k)])
			- 0.166 * Arr[ToLinearAddr(i, j, k - 1)])
			- 0.166 * Arr[ToLinearAddr(i, j, k + 1)])
			- 0.166 * Arr[ToLinearAddr(i, j - 1, k)])
			- 0.166 * Arr[ToLinearAddr(i, j + 1, k)])
			- 0.166 * Arr[ToLinearAddr(i + 1, j, k)])
			- 0.166 * Arr[ToLinearAddr(i - 1, j, k)])
			- 0.0833 * Arr[ToLinearAddr(i, j - 1, k - 1)])
			- 0.0833 * Arr[ToLinearAddr(i, j - 1, k + 1)])
			- 0.0833 * Arr[ToLinearAddr(i, j + 1, k - 1)])
			- 0.0833 * Arr[ToLinearAddr(i, j + 1, k + 1)])
			- 0.0833 * Arr[ToLinearAddr(i - 1, j, k - 1)])
			- 0.0833 * Arr[ToLinearAddr(i - 1, j, k + 1)])
			- 0.0833 * Arr[ToLinearAddr(i - 1, j - 1, k)])
			- 0.0833 * Arr[ToLinearAddr(i - 1, j + 1, k)])
			- 0.0833 * Arr[ToLinearAddr(i + 1, j, k - 1)])
			- 0.0833 * Arr[ToLinearAddr(i + 1, j, k + 1)])
			- 0.0833 * Arr[ToLinearAddr(i + 1, j - 1, k)])
			- 0.0833 * Arr[ToLinearAddr(i + 1, j + 1, k)]);
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

	std::string TestName = "LTR Poisson";
	const dom::hpfloat logCorrect = log2(abs(Res.CorrectValue), dom::HP_ROUNDING);
	const dom::hpfloat Binade = ceil(logCorrect);
	const dom::hpfloat Eps = std::numeric_limits<FType>::epsilon();
	const dom::hpfloat ULPError = Res.Err / (Binade * Eps);

	std::cout << "\tAbsolute Error\tRelative Error\tTime taken (ms)\tCorrect Number\tULP Error" << std::endl;
	std::cout << TestName << "\t" << Res.Err << "\t" << Res.RelErr << "\t" << Duration.count() << "\t" << Res.CorrectValue << "\t" << ULPError << std::endl;
	return 0;
}
