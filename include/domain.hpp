#include <array>
#include <vector>

#include <float.h>
#include <stdint.h>

#include <list>
#include <mutex>
#include <random>
#include <thread>
#include <atomic>

#include <value.hpp>
#include <hpfloat.hpp>

#include "number.hpp"

#include <bgrt/bgrt.hpp>
#include <unordered_map>

#include <condition_variable>
#include "impl/partition.hpp"

#include "domain/base.hpp"
#include "domain/util.hpp"
#include "domain/multithread.hpp"

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
		RetVal[Index] = Content[Index].Error();
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
 * @deprecated 
 * @author Brian Schnepp
 * @see https://formalverification.cs.utah.edu/grt/publications/ppopp14-s3fp.pdf
 * @param Lower The lower bound for each variable in the initial configuration
 * @param Higher The higher bound for each variable in the initial configuration
 * @param Iterations The number of configurations to create upon every previous configuration given
 * @param Resources The number of times new configurations should be processed
 * @param RestartPercent The percentage, as a whole integer, where the initial configuration is reset
 * @param F The function which takes a libdomain Array to check for floating-point error with.
 * @param k The number of times to execute F, looking for potential error
 * @param LogFreq The number of Resources needed before statistics are logged to the console.
 * @param LogOut A stream to send messages to for logging
 * @return The highest error of the function that was ever found, described as "WorstError" in the paper
 */
template<typename T, uint64_t Size>
hpfloat FindError(const Array<T, Size> &Lower, const Array<T, Size> &Higher, 
		  Array<T, Size> (*F)(const Array<T, Size> &),
		  const uint64_t Iterations = 1000, const uint64_t Resources = INT32_MAX, const uint64_t RestartPercent = 15,
		  uint64_t k = 25, uint64_t LogFreq = 25, std::ostream &LogOut = std::cout)
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
				BGRT.SetVals(LocalConf);
			}
			if (Err1 > LocalError)
			{
				LocalError = Err1;
				LocalConf = C;
				BGRT.SetVals(LocalConf);
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
			BGRT.SetVals(LocalConf);
		}

		if ((Res % LogFreq) == 0)
		{
			LogOut << "Current Error: " << WorstError << std::endl;
		}
	}
	return WorstError;
}

/**
 * @brief Implements a multi-threaded variant of the BGRT algorithm to efficiently find floating point errors
 * @warning NOT YET IMPLEMENTED
 * @author Brian Schnepp
 * @see https://formalverification.cs.utah.edu/grt/publications/ppopp14-s3fp.pdf
 * @param InitConf The initial BGRT variable configuration
 * @param Iterations The number of configurations to create upon every previous configuration given
 * @param Resources The number of bits which need to be ignored in the mantissa of range: numbers differing by less than this range are ignored.
 * @param RestartPercent The percentage, as a whole integer, where the initial configuration is reset to avoid local minima
 * @param F The function which takes a BGRT configuration to check for floating-point error with.
 * @param k The number of times to execute F, looking for potential error
 * @param LogFreq Operand to (Resoruces % LogFreq), for when error will be logged to LogOut. Default is 5000.
 * @param LogOut A stream to send messages to for logging. Default is std::cout.
 * @param NumThreads The number of threads to be using for finding error. 0 (default) gets all possible threads.
 * @return The highest error of the function that was ever found, described as "WorstError" in the paper
 */
template<typename T>
EvalResults FindErrorMantissa(const std::unordered_map<uint64_t, bgrt::Variable<T>> &InitConf,
		std::unordered_map<uint64_t, dom::Value<T>> (*F)(std::unordered_map<uint64_t, dom::Value<T>>&),
		const uint64_t Iterations = 1000, const int64_t Resources = 0, const uint64_t RestartPercent = 15,
		uint64_t k = 50, uint64_t LogFreq = 5000, std::ostream &LogOut = std::cout, uint64_t NumThreads = 0)
{
	T Lim = (dom::hpfloat)std::numeric_limits<T>::epsilon();

	dom::hpfloat hLim = (dom::hpfloat)Lim;

	/* Provide one extra Resource to account for rounding */
	dom::hpfloat mLim = hLim * dom::hp::pow(2.0, (Resources-1));
	return FindErrorBoundConf(InitConf, F, Iterations, mLim, RestartPercent, k, LogFreq, LogOut, NumThreads);
}

}

#endif
