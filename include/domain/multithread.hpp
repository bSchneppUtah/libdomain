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

#ifndef DOMAIN_MULTITHREAD_HPP_
#define DOMAIN_MULTITHREAD_HPP_

namespace dom
{

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
EvalResults FindErrorMultithread(const std::unordered_map<uint64_t, bgrt::Variable<T>> &InitConf,
		std::unordered_map<uint64_t, dom::Value<T>> (*F)(std::unordered_map<uint64_t, dom::Value<T>>&),
		const uint64_t Iterations = 100, const int64_t Resources = INT32_MAX, const uint64_t RestartPercent = 5,
		uint64_t k = 1000, uint64_t LogFreq = 5000, std::ostream &LogOut = std::cout, uint64_t NumThreads = 0)
{
	/* Don't allow using something of the same size as the high precision float. */
	static_assert(sizeof(T) != sizeof(dom::hpfloat));

	/* If no count provided, use whatever C++ says the number of threads we have is. */
	if (NumThreads == 0)
	{
		NumThreads = std::thread::hardware_concurrency();
	}

	bool RandomRestart = false;

	/* Refer to the paper for precise meanings of these values */
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

	/**
	 * @brief Implements a partition counter, which allows for a "lazy" synchronization of a global counter variable.
	 */
	struct PartitionCounter
	{
		/* The "actualized" value of this counter at any given time */
		std::atomic_int64_t RealValue;

		/* Some list of local counters for each thread */
		int64_t *ThreadCounters;

		/* And the number of threads there are. */
		uint64_t NumThreads;

	public:
		/**
		 * @brief Initialized the partition counter with a given number of threads
		 * @param NumThreads The number of threads to support
		 */
		PartitionCounter(uint64_t NumThreads)
		{
			this->RealValue = 0;
			this->NumThreads = NumThreads;
			
			this->ThreadCounters = new int64_t[NumThreads];
			for (uint64_t Index = 0; Index < NumThreads; Index++)
			{
				this->ThreadCounters[Index] = 0;
			}
		}

		~PartitionCounter()
		{
			delete[] this->ThreadCounters;
		}

		/**
		 * @brief Forces all threads to keep their read of the value in-sync with the global view (as in a flush)
		 */
		void Sync()
		{
			for (uint64_t Index = 0; Index < NumThreads; Index++)
			{
				RealValue += ThreadCounters[Index];
				ThreadCounters[Index] = 0;
			}
		}

		/**
		 * @brief Reads the immediate value of the global counter, after issuing a sync.
		 */
		int64_t Read()
		{
			this->Sync();
			return this->RealValue;
		}

		/**
		 * @brief Adds to the thread-local counter, which will eventually be synced with the global counter.
		 * @param Value The value to add
		 * @param TID The current Thread ID
		 */
		void Add(int64_t Value, uint64_t TID)
		{
			this->ThreadCounters[TID] += Value;
		}


		/**
		 * @brief Subtracts from the thread-local counter, which will eventually be synced with the global counter.
		 * @param Value The value to subtract
		 * @param TID The current Thread ID
		 */
		void Sub(int64_t Value, uint64_t TID)
		{
			this->ThreadCounters[TID] -= Value;
		}
	};

	/*
	 * Resources in this case is always the number of shadow value computations actually performed.
	 * A performance trick is done here to get better parallelism: each core gets a local counter
	 * which at some point is synced up with the main counter.
	 * 
	 * This utilizes a data structure called a Partition Counter, which can avoid some of the IPC costs of atomic instructions.
	 */
	PartitionCounter RemainingResources(NumThreads);

	/* Partition the configuration up into chunks per thread. */
	std::vector<std::vector<Configuration>> PartNextConfs(NumThreads);

	/* Since this is multi-threaded, all of the variations of these need to be done per-thread, then synced up
	 * when the current wave of configurations is completed.
	 */
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

	/* The first NumThreads is for work input, the second half for work output. */
	std::mutex WorkerMutex[NumThreads * 2];
	std::condition_variable WorkerCV[NumThreads * 2];

	/* Create the actual worker threads. Use the atomic enum to set flags on work. */
	for (uint64_t TID = 0; TID < NumThreads; TID++)
	{
		Continue[TID] = EMPTY;
		Threads[TID] = std::thread([&LocalErrors, &LocalConfs, &PartNextConfs, &F, &k, &RemainingResources, &Continue, &WorkerMutex, &WorkerCV, &NumThreads](uint64_t TID)
		{
			while (Continue[TID] != TERMINATE)
			{
				/* Hold an exclusive lock on the worker thread for taking in data */
				std::unique_lock<std::mutex> Lck(WorkerMutex[TID]);

				/* Either try to proceed every 500 milliseconds, or whenever work is issued, whichever comes first. */
				WorkerCV[TID].wait_for(Lck, std::chrono::milliseconds(500));

				if (Continue[TID] == WORK_AVAIL) 
				{
					Continue[TID] = WORKING;
					/* Every thread has some partition of work: 
					 * go through all of them, and then perform the work. 
					 */
					for (const auto &C : PartNextConfs[TID])
					{
						EvalResults Res = Eval(F, C, k);
						uint64_t Ops = Res.TotalShadowOps;
						if (Res.Err > LocalErrors[TID].Err)
						{
							LocalErrors[TID] = Res;
							LocalConfs[TID] = C;
						}
						RemainingResources.Add(Ops, TID);
					}

					/* When done, say we are and then notify the coordinator thread. */
					Continue[TID] = EMPTY;
					WorkerCV[TID + NumThreads].notify_all();
				}
			}
		}, TID);
	}

	/*
	 * The main part of BGRT: while we still have resources available... 
	 */
	while (RemainingResources.Read() <= Resources)
	{
		LocalError = EvalResults{};
		PartNextConfs.clear();

		/* Make sure all the workers are done first */
		for (uint64_t TID = 0; TID < NumThreads; TID++)
		{
			while (Continue[TID] != EMPTY) { }
			LocalErrors[TID] = EvalResults{};
			LocalConfs[TID].clear();
		}

		/* Create a partition of all the configurations possible from the current BGRT state, for the number of threads we have. */
		PartNextConfs = dom::impl::PartitionConfigs<T>(NumThreads, Iterations, BGRT, [](const Configuration &Config){
			return true;
		});

		/* Send a notification to all the workers that PartNextConfs has new data for them */
		for (uint64_t TID = 0; TID < NumThreads; TID++)
		{
			while (Continue[TID] != EMPTY) { }
			Continue[TID] = WORK_AVAIL;
			WorkerCV[TID].notify_all();
		}


		/* For all of the threads:
			- wait until work is complete on all of them
			- Update the error if we encountered a higher one
		 */
		for (uint64_t TID = 0; TID < NumThreads; TID++)
		{
			while (Continue[TID] != EMPTY) 
			{
				std::unique_lock<std::mutex> Lck(WorkerMutex[TID + NumThreads]);
				WorkerCV[TID + NumThreads].wait_for(Lck, std::chrono::milliseconds(500));		
			}

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

		/* Also sometimes send something to std::out or wherever the LogOut is, this is just to confirm liveness and also show how much progress we're making. */
		if ((RemainingResources.Read() % LogFreq) == 0)
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
		WorkerCV[TID].notify_all();
		Threads[TID].join();
	}

	return WorstError;
}

/**
 * @brief Implements a bounded multi-threaded variant of the BGRT algorithm to efficiently find floating point errors
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
EvalResults FindErrorBoundConfMultithread(const std::unordered_map<uint64_t, bgrt::Variable<T>> &InitConf,
		std::unordered_map<uint64_t, dom::Value<T>> (*F)(std::unordered_map<uint64_t, dom::Value<T>>&),
		const uint64_t Iterations = 100, const dom::hpfloat MinRange = std::numeric_limits<T>::epsilon(), 
		const uint64_t RestartPercent = 5, uint64_t k = 1000, uint64_t LogFreq = 4000, std::ostream &LogOut = std::cout, uint64_t NumThreads = 0)
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
	
	/* The first NumThreads is for work input, the second half for work output. */
	std::mutex WorkerMutex[NumThreads * 2];
	std::condition_variable WorkerCV[NumThreads * 2];

	for (uint64_t TID = 0; TID < NumThreads; TID++)
	{
		Continue[TID] = EMPTY;
		Threads[TID] = std::thread([&LocalErrors, &LocalConfs, &PartNextConfs, &F, &k, &Continue, &WorkerCV, &WorkerMutex, NumThreads](uint64_t TID)
		{
			while (Continue[TID] != TERMINATE)
			{
				std::unique_lock<std::mutex> Lck(WorkerMutex[TID]);
				WorkerCV[TID].wait_for(Lck, std::chrono::milliseconds(500));
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
					WorkerCV[TID + NumThreads].notify_all();
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


		uint64_t LocalIterations = Iterations / NumThreads;
		
		/* Use a lambda to apply filtering to the next configurations to apply.
		 * In this case, prune any job which has the size less than the range
		 * (ie, the delta between min and max interval < some range)
		 */
		uint64_t TotalJobs = 0;
		PartNextConfs = dom::impl::PartitionConfigs<T>(NumThreads, Iterations, BGRT, [&TotalJobs, MinRange](const Configuration &Config){
			bool Okay = true;
			for (const auto &Pair : Config)
			{
				if (Pair.second.Size().SVal() < MinRange)
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
			WorkerCV[TID].notify_all();
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
			while (Continue[TID] != EMPTY) 
			{ 
				std::unique_lock<std::mutex> Lck(WorkerMutex[TID + NumThreads]);
				WorkerCV[TID + NumThreads].wait_for(Lck, std::chrono::milliseconds(500));				
			}
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
		WorkerCV[TID].notify_all();
		Threads[TID].join();
	}

	return WorstError;
}

/**
 * @brief Implements a multi-threaded variant of the BGRT algorithm to efficiently find floating point errors
 * @author Brian Schnepp
 * @see https://formalverification.cs.utah.edu/grt/publications/ppopp14-s3fp.pdf
 * @param InitConf The initial BGRT variable configuration
 * @param Iterations The number of configurations to create upon every previous configuration given
 * @param Resources The number of bits which need to be ignored in the mantissa of range: numbers differing by less than this range are ignored.
 * @param Scale The scaling to apply to the epsilon calculation
 * @param RestartPercent The percentage, as a whole integer, where the initial configuration is reset to avoid local minima
 * @param F The function which takes a BGRT configuration to check for floating-point error with.
 * @param k The number of times to execute F, looking for potential error
 * @param LogFreq Operand to (Resoruces % LogFreq), for when error will be logged to LogOut. Default is 5000.
 * @param LogOut A stream to send messages to for logging. Default is std::cout.
 * @param NumThreads The number of threads to be using for finding error. 0 (default) gets all possible threads.
 * @return The highest error of the function that was ever found, described as "WorstError" in the paper
 */
template<typename T>
EvalResults FindErrorMantissaMultithread(const std::unordered_map<uint64_t, bgrt::Variable<T>> &InitConf,
		std::unordered_map<uint64_t, dom::Value<T>> (*F)(std::unordered_map<uint64_t, dom::Value<T>>&),
		const uint64_t Iterations = 100, const int64_t Resources = 0, T Scale = 1.0, const uint64_t RestartPercent = 5,
		uint64_t k = 1000, uint64_t LogFreq = 5000, std::ostream &LogOut = std::cout, uint64_t NumThreads = 0)
{
	dom::hpfloat Lim = (dom::hpfloat)std::numeric_limits<T>::epsilon();

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
	
	/* The first NumThreads is for work input, the second half for work output. */
	std::mutex WorkerMutex[NumThreads * 2];
	std::condition_variable WorkerCV[NumThreads * 2];

	for (uint64_t TID = 0; TID < NumThreads; TID++)
	{
		Continue[TID] = EMPTY;
		Threads[TID] = std::thread([&LocalErrors, &LocalConfs, &PartNextConfs, &F, &k, &Continue, &WorkerCV, &WorkerMutex, NumThreads](uint64_t TID)
		{
			while (Continue[TID] != TERMINATE)
			{
				std::unique_lock<std::mutex> Lck(WorkerMutex[TID]);
				WorkerCV[TID].wait_for(Lck, std::chrono::milliseconds(500));
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
					WorkerCV[TID + NumThreads].notify_all();
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


		uint64_t LocalIterations = Iterations / NumThreads;
		
		/* Use a lambda to apply filtering to the next configurations to apply.
		 * In this case, prune any job which has the size less than the range
		 * (ie, the delta between min and max interval < some range)
		 */
		uint64_t TotalJobs = 0;
		PartNextConfs = dom::impl::PartitionConfigs<T>(NumThreads, Iterations, BGRT, [&TotalJobs, Lim, Resources, Scale](const Configuration &Config){
			bool Okay = true;
			for (const auto &Pair : Config)
			{
				dom::hpfloat RangeSize = Pair.second.Size().SVal();
				
				dom::hpfloat Min = Pair.second.Min().SVal();
				dom::hpfloat Max = Pair.second.Max().SVal();
				dom::hpfloat Bigger = (Min < Max) ? Min : Max;

				dom::hpfloat Eps = 0.5 * Lim * (Resources + 1);
				dom::hpfloat Filter = Scale * (Bigger * Eps);
				Filter = (Filter < 0) ? -Filter : Filter;

				if (RangeSize < Filter)
				{
					Okay = false;
				}

				TotalJobs += (1 * Okay);
			}
			return Okay;
		});		

		/* We're done if every possible job was too close to the boundary. */
		std::cout << "Total Jobs: " << TotalJobs << std::endl;
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
			WorkerCV[TID].notify_all();
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
			while (Continue[TID] != EMPTY) 
			{ 
				std::unique_lock<std::mutex> Lck(WorkerMutex[TID + NumThreads]);
				WorkerCV[TID + NumThreads].wait_for(Lck, std::chrono::milliseconds(500));				
			}
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
		WorkerCV[TID].notify_all();
		Threads[TID].join();
	}

	return WorstError;
}


}


#endif
