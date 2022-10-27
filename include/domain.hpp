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
 * @param Resources The rough limit on the number of floating point computations to perform. Actual executions may exceed this value by a fair amount.
 * @param RestartPercent The percentage, as a whole integer, where the initial configuration is reset to avoid local minima
 * @param F The function which takes a BGRT configuration to check for floating-point error with.
 * @param k The number of times to execute F, looking for potential error
 * @param LogFreq Operand to (Resoruces % LogFreq), for when error will be logged to LogOut. Default is 5000.
 * @param LogOut A stream to send messages to for logging. Default is std::cout.
 * @param NumThreads The number of threads to be using for finding error. 0 (default) gets all possible threads.
 * @return The highest error of the function that was ever found, described as "WorstError" in the paper
 */
template<typename T>
hpfloat FindErrorMantissa(const std::unordered_map<uint64_t, bgrt::Variable<T>> &InitConf,
		std::unordered_map<uint64_t, dom::Value<T>> (*F)(std::unordered_map<uint64_t, dom::Value<T>>&),
		const uint64_t Iterations = 1000, const int64_t Resources = 18, const uint64_t RestartPercent = 15,
		uint64_t k = 25, uint64_t LogFreq = 5000, std::ostream &LogOut = std::cout, uint64_t NumThreads = 0)
{
	/* Don't allow using something of the same size as the high precision float. */
	static_assert(sizeof(T) != sizeof(dom::hpfloat));

	if (NumThreads == 0)
	{
		NumThreads = std::thread::hardware_concurrency();
	}

	bool RandomRestart = false;
	hpfloat WorstError = 0;
	hpfloat LocalError = 0;
	
	using Var = bgrt::Variable<T>;
	using Configuration = std::unordered_map<uint64_t, Var>;
	
	Configuration LocalConf = InitConf;
	bgrt::BGRTState BGRT(LocalConf);

	static std::random_device Dev;
	static std::mt19937 Gen(Dev());
	static std::uniform_int_distribution<int> Dist(0, 100);

	std::thread Threads[NumThreads];

	std::vector<std::vector<Configuration>> PartNextConfs(NumThreads);

	hpfloat LocalErrors[NumThreads];
	Configuration LocalConfs[NumThreads];
	std::atomic_uint8_t Continue[NumThreads];

	enum ThreadControl
	{
		EMPTY = 0,
		WORKING = 1,
		WORK_AVAIL = 2,
		TERMINATE = 3,
	};
	

	for (uint64_t TID = 0; TID < NumThreads; TID++)
	{
		Continue[TID] = EMPTY;
		Threads[TID] = std::thread([&LocalErrors, &LocalConfs, &PartNextConfs, &F, &k, &Continue](uint64_t TID)
		{
			while (Continue[TID] != TERMINATE)
			{
				if (Continue[TID] == WORK_AVAIL) 
				{
					Continue[TID] = WORKING;
					for (const auto &C : PartNextConfs[TID])
					{
						EvalResults Res = Eval(F, C, k);
						if (Res.Err > LocalErrors[TID].Err)
						{
							LocalErrors[TID] = Res;
							LocalConfs[TID] = C;
						}
					}
					Continue[TID] = EMPTY;
				}
			}
		}, TID);
	}

	bool ResourcesAvailable = true;
	while (ResourcesAvailable)
	{
		LocalError = 0;
		PartNextConfs.clear();

		for (uint64_t TID = 0; TID < NumThreads; TID++)
		{
			while (Continue[TID] != EMPTY) { }
			LocalErrors[TID] = 0;
			LocalConfs[TID].clear();
		}

		const std::vector<Configuration> NextConfs = BGRT.NextGen(Iterations);
		
		uint64_t TotalJobs = 0;
		uint64_t TotalPartIndex = 0;
		for (; TotalPartIndex < NextConfs.size(); TotalPartIndex+=NumThreads)
		{
			for (uint64_t PartitionIndex = 0; PartitionIndex < NumThreads; PartitionIndex++)
			{
				bool Okay = true;
				for (const auto &Pair : NextConfs[TotalPartIndex])
				{
					/* TODO: Refactor to avoid code duplication */
					auto Min = Pair.second.Min().Val();
					auto Max = Pair.second.Max().Val();
					uint64_t MinMant = MantissaBits(Min);
					uint64_t MaxMant = MantissaBits(Min);

					uint64_t MinExp = ExponentBits(Min);
					uint64_t MaxExp = ExponentBits(Max);

					uint64_t MinSign = SignBits(Min);
					uint64_t MaxSign = SignBits(Max);

					if (MinSign == MaxSign && MinExp && MaxExp)
					{
						uint64_t Agree = ~(MinMant ^ MaxMant);
						uint64_t ReqBits = (1ULL << Resources) - 1;

						/* If true, this configuration's space is too close. Trim it. */;
						Okay = !((Agree & ReqBits) == ReqBits);
					}

				}

				if (Okay)
				{
					TotalJobs++;
					PartNextConfs[PartitionIndex].push_back(NextConfs[TotalPartIndex]);
				}
			}
		}

		for (; TotalPartIndex < NextConfs.size(); TotalPartIndex++)
		{
			bool Okay = true;
			for (const auto &Pair : NextConfs[TotalPartIndex])
			{
				/* TODO: Refactor to avoid code duplication */
				auto Min = Pair.second.Min().Val();
				auto Max = Pair.second.Max().Val();
				uint64_t MinMant = MantissaBits(Min);
				uint64_t MaxMant = MantissaBits(Min);

				uint64_t MinExp = ExponentBits(Min);
				uint64_t MaxExp = ExponentBits(Max);

				uint64_t MinSign = SignBits(Min);
				uint64_t MaxSign = SignBits(Max);

				if (MinSign == MaxSign && MinExp && MaxExp)
				{
					uint64_t Agree = ~(MinMant ^ MaxMant);
					uint64_t ReqBits = (1ULL << Resources) - 1;
					
					/* If true, this configuration's space is too close. Trim it. */;
					Okay = !((Agree & ReqBits) == ReqBits);
				}

			}

			if (Okay)
			{
				TotalJobs++;
				PartNextConfs[TotalPartIndex % NumThreads].push_back(NextConfs[TotalPartIndex]);
			}
		}		

		if (TotalJobs == 0)
		{
			/* We're done if every possible job was too close to the boundary. */
			ResourcesAvailable = false;
			break;
		}


		for (uint64_t TID = 0; TID < NumThreads; TID++)
		{
			while (Continue[TID] != EMPTY) { }
			Continue[TID] = WORK_AVAIL;
		}


		for (uint64_t TID = 0; TID < NumThreads; TID++)
		{
			while (Continue[TID] != EMPTY) { }
			if (LocalErrors[TID] > LocalError)
			{
				LocalError = LocalErrors[TID];
				LocalConf = LocalConfs[TID];
				BGRT.SetVals(LocalConf);
			}
		}

		/* Is the error in this configuration higher than the global maximum? */
		if (LocalError > WorstError)
		{
			WorstError = LocalError;
			LogOut << "Current Error: " << WorstError << std::endl;
		}

		/* Sometimes re-issue the original configuration, to avoid getting stuck. */
		RandomRestart = (Dist(Gen) % 100) < RestartPercent;
		if (RandomRestart)
		{
			LocalConf = InitConf;
			BGRT.SetVals(LocalConf);
		}
	}
	for (uint64_t TID = 0; TID < NumThreads; TID++)
	{
		Continue[TID] = TERMINATE;
		Threads[TID].join();
	}

	return WorstError;
}

}

#endif
