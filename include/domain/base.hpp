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

#ifndef DOMAIN_BASE_HPP_
#define DOMAIN_BASE_HPP_

namespace dom
{

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
		const uint64_t Iterations = 100, const int64_t Resources = INT32_MAX, const uint64_t RestartPercent = 15,
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

}


#endif