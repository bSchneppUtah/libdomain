#include <array>
#include <vector>

#include <stdint.h>

#include <list>
#include <random>
#include <thread>
#include <atomic>

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

typedef struct EvalResults
{
	dom::hpfloat Err;
	uint64_t TotalShadowOps;
}EvalResults;

/**
 * @brief Implements the Eval function as described in the S3FP paper
 * @author Brian Schnepp
 * @see https://formalverification.cs.utah.edu/grt/publications/ppopp14-s3fp.pdf
 * @param P The function to execute, corresponding to the parameter P as described in the paper
 * @param C The configuration of the variables, corresponding to the parameter C as described in the paper
 * @param k The number of times P(C) is run, matching the description of k in the paper
 * @return The highest error seen in any variable over the function P
 */
template<typename T>
EvalResults Eval(std::unordered_map<uint64_t, dom::Value<T>> (*P)(std::unordered_map<uint64_t, dom::Value<T>>&), 
	     const std::unordered_map<uint64_t, bgrt::Variable<T>> &C, uint64_t k)
{
	uint64_t TotalShadowOps = 0;
	dom::hpfloat Err = (dom::hpfloat)0.0;

	using Val = dom::Value<T>;
	using Var = bgrt::Variable<T>;
	using Array = std::unordered_map<uint64_t, Val>;
	using Configuration = std::unordered_map<uint64_t, Var>;

	Array SubmitVals;
	/* Reserve the same amount of space: performance optimization */
	SubmitVals.reserve(C.bucket_count());

	std::vector<Array> SampleConfs(k);

	/* Call F on this configuration K times (Section 3.1) */
	for (auto &Pair : C)
	{
		/* Sample a point within the given domain */
		for (uint64_t iK = 0; iK < k; iK++)
		{
			SampleConfs[iK][Pair.first] = Pair.second.Sample();
		}
	}

	std::vector<Array> Results(k);
	/* Call F on this configuration K times (Section 3.1)*/
	for (uint64_t iK = 0; iK < k; iK++)
	{
		/* Submit a job to P with the sampled points */
		Results.push_back(P(SampleConfs[iK]));
	}

	for (const Array &Next : Results)
	{
		for (auto &Pair : Next)
		{
			hpfloat Diff = Pair.second.Diff();
			Err = (Diff > Err) ? Diff : Err;
			TotalShadowOps += Pair.second.Ops();
		}		
	}

	return {Err, TotalShadowOps};	
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
 * @brief Implements the BGRT algorithm to efficiently find floating point errors
 * @author Brian Schnepp
 * @see https://formalverification.cs.utah.edu/grt/publications/ppopp14-s3fp.pdf
 * @param InitConf The initial BGRT variable configuration
 * @param Iterations The number of configurations to create upon every previous configuration given
 * @param Resources The number of times new configurations should be processed
 * @param RestartPercent The percentage, as a whole integer, where the initial configuration is reset to avoid local minima
 * @param F The function which takes a BGRT configuration to check for floating-point error with.
 * @param k The number of times to execute F, looking for potential error
 * @param LogFreq What percent of samples will have their error written to the console
 * @param LogOut A stream to send messages to for logging
 * @return The highest error of the function that was ever found, described as "WorstError" in the paper
 */
template<typename T>
hpfloat FindError(const std::unordered_map<uint64_t, bgrt::Variable<T>> &InitConf,
		std::unordered_map<uint64_t, dom::Value<T>> (*F)(std::unordered_map<uint64_t, dom::Value<T>>&),
		const uint64_t Iterations = 1000, const int64_t Resources = INT32_MAX, const uint64_t RestartPercent = 15,
		uint64_t k = 1000, uint64_t LogFreq = 500, std::ostream &LogOut = std::cout)
{
	/* Don't allow using something of the same size as the high precision float. */
	static_assert(sizeof(T) != sizeof(dom::hpfloat));

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

	uint64_t RemainingResources = Resources;
	while (RemainingResources > 0)
	{
		LocalError = 0;
		const std::vector<Configuration> NextConfs = BGRT.NextGen(Iterations);
		for (auto C : NextConfs)
		{
			EvalResults Res = Eval(F, C, k);
			dom::hpfloat Err = Res.Err;
			uint64_t Ops = Res.TotalShadowOps;
			if (Err > LocalError)
			{
				LocalError = Err;
				LocalConf = C;
				BGRT.SetVals(LocalConf);
			}
			RemainingResources -= Ops;
		}

		/* Is the error in this configuration higher than the global maximum? */
		if (LocalError > WorstError)
		{
			WorstError = LocalError;
		}

		/* Sometimes re-issue the original configuration, to avoid getting stuck. */
		RandomRestart = (Dist(Gen) % 100) < RestartPercent;
		if (RandomRestart)
		{
			LocalConf = InitConf;
			BGRT.SetVals(LocalConf);
		}

		if ((RemainingResources % LogFreq) == 0)
		{
			LogOut << "Current Error: " << WorstError << std::endl;
		}
	}
	return WorstError;
}

/**
 * @brief Implements a multi-threaded variant of the BGRT algorithm to efficiently find floating point errors
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
hpfloat FindErrorMultithread(const std::unordered_map<uint64_t, bgrt::Variable<T>> &InitConf,
		std::unordered_map<uint64_t, dom::Value<T>> (*F)(std::unordered_map<uint64_t, dom::Value<T>>&),
		const uint64_t Iterations = 1000, const int64_t Resources = INT32_MAX, const uint64_t RestartPercent = 15,
		uint64_t k = 1000, uint64_t LogFreq = 5000, std::ostream &LogOut = std::cout, uint64_t NumThreads = 0)
{
	/* Don't allow using something of the same size as the high precision float. */
	static_assert(sizeof(T) != sizeof(dom::hpfloat));

	if (NumThreads == 0)
	{
		NumThreads = std::thread::hardware_concurrency();
	}

	struct LazyResourceCounter
	{
		std::atomic_int64_t RealValue;
		int64_t *ThreadCounters;
		uint64_t NumThreads;

	public:
		LazyResourceCounter(uint64_t NumThreads)
		{
			this->RealValue = 0;
			this->NumThreads = NumThreads;
			
			this->ThreadCounters = new int64_t[NumThreads];
			for (uint64_t Index = 0; Index < NumThreads; Index++)
			{
				this->ThreadCounters[Index] = 0;
			}
		}

		~LazyResourceCounter()
		{
			delete[] this->ThreadCounters;
		}

		void Sync()
		{
			for (uint64_t Index = 0; Index < NumThreads; Index++)
			{
				RealValue += ThreadCounters[Index];
				ThreadCounters[Index] = 0;
			}
		}

		int64_t Read()
		{
			this->Sync();
			return this->RealValue;
		}

		void Add(int64_t Value, uint64_t TID)
		{
			this->ThreadCounters[TID] += Value;
		}

		void Sub(int64_t Value, uint64_t TID)
		{
			this->ThreadCounters[TID] -= Value;
		}
	};

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

	/*
	 * Resources in this case is always the number of shadow value computations actually performed.
	 */
	LazyResourceCounter RemainingResources(NumThreads);
	std::vector<std::vector<Configuration>> PartNextConfs(NumThreads);
	
	while (RemainingResources.Read() <= Resources)
	{
		LocalError = 0;
		PartNextConfs.clear();

		hpfloat LocalErrors[NumThreads];
		Configuration LocalConfs[NumThreads];

		const std::vector<Configuration> NextConfs = BGRT.NextGen(Iterations);

		for (uint64_t Index = 0; Index < NextConfs.size(); Index++)
		{
			auto C = NextConfs[Index];
			PartNextConfs[Index % NumThreads].push_back(C);
		}

		for (uint64_t TID = 0; TID < NumThreads; TID++)
		{
			Threads[TID] = std::thread([&LocalErrors, &LocalConfs, &PartNextConfs, &F, &k, &RemainingResources](uint64_t TID){
				for (auto C : PartNextConfs[TID])
				{
					EvalResults Res = Eval(F, C, k);
					dom::hpfloat Err = Res.Err;
					uint64_t Ops = Res.TotalShadowOps;
					if (Err > LocalErrors[TID])
					{
						LocalErrors[TID] = Err;
						LocalConfs[TID] = C;
					}
					RemainingResources.Add(Ops, TID);
				}				
			}, TID);
		}

		for (uint64_t TID = 0; TID < NumThreads; TID++)
		{
			Threads[TID].join();
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
		}

		/* Sometimes re-issue the original configuration, to avoid getting stuck. */
		RandomRestart = (Dist(Gen) % 100) < RestartPercent;
		if (RandomRestart)
		{
			LocalConf = InitConf;
			BGRT.SetVals(LocalConf);
		}

		if ((RemainingResources.Read() % LogFreq) == 0)
		{
			LogOut << "Current Error: " << WorstError << std::endl;
		}
	}
	return WorstError;
}


}

#endif
