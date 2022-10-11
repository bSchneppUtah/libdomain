#include <array>
#include <vector>

#include <stdint.h>

#include <random>

#include <value.hpp>
#include <hpfloat.hpp>

#include <bgrt/bgrt.hpp>
#include <unordered_map>

#ifndef LIBDOMAIN_HPP_
#define LIBDOMAIN_HPP_

/**
 * @file include/domain.hpp
 * @brief Main header for all contents of libdomain, including error-finding functions.
 */


namespace dom
{

void Init();


/**
 * @brief Finds the error for each particular value in a given Array
 * @author Brian Schnepp
 * @return A std::array of hpfloat which has the error of each value
 */
template<typename T, uint64_t Size>
std::array<hpfloat, Size> Diffs(const Array<T, Size> &Content)
{
	std::array<hpfloat, Size> RetVal;
	for (uint64_t Index = 0; Index < Size; Index++)
	{
		RetVal[Index] = Content[Index].Diff();
	}
	return RetVal;
}

/**
 * @brief Finds the maximum error of a given function, based upon a std::array input
 * @author Brian Schnepp
 * @return The highest error found, expressed as an hpfloat.
 */
template<uint64_t Size>
hpfloat MaxError(const std::array<hpfloat, Size> &Arr)
{
	hpfloat Err = 0;
	for (hpfloat Item : Arr)
	{
		if (Item > Err)
		{
			Err = Item;
		}
	}
	return Err;
}


/**
 * @brief Converts a BGRT variable configuration into a libdomain Array
 * @author Brian Schnepp
 * @return A std::array of both Array variants, based upon their lower and higher bounds.
 */
template<typename T, uint64_t Size>
static std::array<Array<T, Size>, 2> ConvertArray(std::unordered_map<uint64_t, bgrt::Variable<T>> &Arr)
{
	std::array<Array<T, Size>, 2> RetVal;
	for (const auto &Pair : Arr)
	{
		RetVal[0][Pair.first] = Pair.second.Min();
		RetVal[1][Pair.first] = Pair.second.Max();
	}
	return RetVal;
}

/**
 * @brief Implements the BGRT algorithm to efficiently find floating point errors
 * @author Brian Schnepp
 * @see https://formalverification.cs.utah.edu/grt/publications/ppopp14-s3fp.pdf
 * @param Lower The lower bound for each variable in the initial configuration
 * @param Higher The higher bound for each variable in the initial configuration
 * @param Iterations The number of configurations to create upon every previous configuration given
 * @param Resources The number of times new configurations should be processed
 * @param RestartPercent The percentage, as a whole integer, where the initial configuration is reset
 * @param F The function which takes a libdomain Array to check for floating-point error with.
 * @return The highest error of the function that was ever found, described as "WorstError" in the paper
 */
template<typename T, uint64_t Size>
hpfloat FindError(const Array<T, Size> &Lower, const Array<T, Size> &Higher, 
	uint64_t Iterations, uint64_t Resources, uint64_t RestartPercent,
	Array<T, Size> (*F)(const Array<T, Size> &))
{
	static_assert(sizeof(T) != sizeof(dom::hpfloat));
	if (Lower.GetSize() != Higher.GetSize())
	{
		/* FIXME: error more gracefully */
		exit(-1);
	}

	bool RandomRestart = false;
	hpfloat WorstError = 0;
	hpfloat LocalError = 0;
	
	using Var = bgrt::Variable<T>;
	using Configuration = std::unordered_map<uint64_t, Var>;
	
	Configuration LocalConf;
	for (uint64_t Index = 0; Index < Lower.GetSize(); Index++)
	{
		LocalConf[Index] = bgrt::Variable<T>(Lower[Index], Higher[Index]);
	}

	Configuration InitConf = LocalConf;
	
	bgrt::BGRTState BGRT(LocalConf);

	std::random_device Dev;
	std::mt19937 Gen(Dev());
	std::uniform_int_distribution<int> Dist(0, 100);

	for (uint64_t Res = 0; Res < Resources; Res++)
	{
		LocalError = 0;
		std::vector<Configuration> NextConfs = BGRT.NextGen(Iterations);
		for (auto C : NextConfs)
		{
			std::array<Array<T, Size>, 2> ConvArr = ConvertArray<T, Size>(C);
			hpfloat Err0 = MaxError(Diffs(F(ConvArr[0])));
			hpfloat Err1 = MaxError(Diffs(F(ConvArr[1])));
			if (Err0 > LocalError)
			{
				LocalError = Err0;
				LocalConf = C;
			}
			if (Err1 > LocalError)
			{
				LocalError = Err1;
				LocalConf = C;
			}
		}
		if (LocalError > WorstError)
		{
			WorstError = LocalError;
		}

		RandomRestart = (Dist(Gen) % 100) < RestartPercent;
		if (RandomRestart)
		{
			LocalConf = InitConf;
		}
	}
	return WorstError;
}



}

#endif
