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

#include "domain/util.hpp"

#ifndef DOMAIN_MPI_HPP_
#define DOMAIN_MPI_HPP_

namespace dom
{

/**
 * @brief Implements an MPI variant of the BGRT algorithm to efficiently find floating point errors
 * @author Brian Schnepp
 * @see https://formalverification.cs.utah.edu/grt/publications/ppopp14-s3fp.pdf
 * @param InitConf The initial BGRT variable configuration
 * @param Iterations The number of configurations to create upon every previous configuration given
 * @param Resources The rough limit on the number of floating point computations to perform. Actual executions may exceed this value by a fair amount.
 * @param RestartPercent The percentage, as a whole integer, where the initial configuration is reset to avoid local minima
 * @param F The function which takes a BGRT configuration to check for floating-point error with.
 * @param k The number of times to execute F, looking for potential error
 * @param LogFreq Chance (out of 1000) that a log is printed after any given level of configurations. Default is 4000.
 * @param LogOut A stream to send messages to for logging. Default is std::cout.
 * @param NumThreads The number of threads to be using for finding error. 0 (default) gets all possible threads.
 * @return The highest error of the function that was ever found, described as "WorstError" in the paper
 */
template<typename T>
EvalResults FindErrorBoundConfMPI(const std::unordered_map<uint64_t, bgrt::Variable<T>> &InitConf,
		std::unordered_map<uint64_t, dom::Value<T>> (*F)(std::unordered_map<uint64_t, dom::Value<T>>&),
		const uint64_t Iterations = 1000, const dom::hpfloat MinRange = std::numeric_limits<T>::epsilon(), 
		const uint64_t RestartPercent = 15, uint64_t k = 25, uint64_t LogFreq = 4000, std::ostream &LogOut = std::cout, uint64_t NumThreads = 0)
{
	/* Don't allow using something of the same size as the high precision float. */
	static_assert(sizeof(T) != sizeof(dom::hpfloat));

	if (NumThreads == 0)
	{
		NumThreads = std::thread::hardware_concurrency();
	}

	bool RandomRestart = false;
	EvalResults WorstError = EvalResults{};
	EvalResults LocalError = EvalResults{};
	
	using Var = bgrt::Variable<T>;
	using Configuration = std::unordered_map<uint64_t, Var>;
	
	Configuration LocalConf = InitConf;
	bgrt::BGRTState BGRT(LocalConf);

	static std::random_device Dev;
	static std::mt19937 Gen(Dev());
	static std::uniform_int_distribution<int> Dist(0, 100);

	std::thread Threads[NumThreads];

	std::vector<std::vector<Configuration>> PartNextConfs(NumThreads);

	EvalResults LocalErrors[NumThreads];
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
						dom::hpfloat Err = Res.Err;
						if (Err > LocalErrors[TID].Err)
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
		LocalError = EvalResults{};
		PartNextConfs.clear();

		for (uint64_t TID = 0; TID < NumThreads; TID++)
		{
			while (Continue[TID] != EMPTY) { }
			LocalErrors[TID] = EvalResults{};
			LocalConfs[TID].clear();
		}

		const std::vector<Configuration> NextConfs = BGRT.NextGen(Iterations);
		
		/* Use a lambda to apply filtering to the next configurations to apply.
		 * In this case, prune any job which has the size less than the range
		 * (ie, the delta between min and max interval < some range)
		 */
		uint64_t TotalJobs = 0;
		PartNextConfs = dom::impl::PartitionConfigs<T>(NumThreads, NextConfs, [&TotalJobs, MinRange](const Configuration &Config){
			bool Okay = true;
			for (const auto &Pair : Config)
			{
				if (Pair.second.Size() < MinRange)
				{
					Okay = false;
				}

				TotalJobs += (1 * Okay);
			}
			return Okay;
		});		

		/* We're done if every possible job was too close to the boundary. */
		if (TotalJobs == 0)
		{
			ResourcesAvailable = false;
			break;
		}

		/* Issue work to the worker threads */
		for (uint64_t TID = 0; TID < NumThreads; TID++)
		{
			while (Continue[TID] != EMPTY) { }
			Continue[TID] = WORK_AVAIL;
		}


		/* Collect results on all the worker threads sequentially.
		 * In theory, this can be redone with condition variables
		 * to allow any thread to complete in any order, but since
		 * this isn't a distributed systems problem (and we're assuming SMP)
		 * every core should eventually offer a reply, and roughly at the
		 * same time anyway. If this was not true, then the machine running
		 * this code must have crashed (or ran out of resources), such that
		 * there's no real way to continue anyway.
		 */
		for (uint64_t TID = 0; TID < NumThreads; TID++)
		{
			while (Continue[TID] != EMPTY) { }
			if (LocalErrors[TID].Err > LocalError.Err)
			{
				LocalError = LocalErrors[TID];
				LocalConf = LocalConfs[TID];
				BGRT.SetVals(LocalConf);
			}
		}

		/* Is the error in this configuration higher than the global maximum? */
		if (LocalError.Err > WorstError.Err)
		{
			WorstError = LocalError;
		}

		if ((Dist(Gen) * Dist(Gen)) <= LogFreq)
		{
			LogOut << "(CurError " << "(abs " << WorstError.Err << ")" << ", (rel " << WorstError.RelErr << "))" << std::endl;
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