#include "hpfloat.hpp"

#ifndef DOMAIN_UTIL_HPP_
#define DOMAIN_UTIL_HPP_

namespace dom
{

typedef struct EvalResults
{
	dom::hpfloat Err;
	dom::hpfloat RelErr;
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
	dom::hpfloat RelErr = (dom::hpfloat)0.0;

	/* To avoid out-of-control hashmap allocations, run groupings of some smaller k
	 * if it's particularly large.
	 */
	if (k > 500)
	{
		/* Split the work in half to avoid crazy memory allocations */
		EvalResults Left = Eval(P, C, k/2);
		EvalResults Right = Eval(P, C, (k/2) + (k%2));
		if (Left.Err > Right.Err)
		{
			return Left;
		}
		return Right;
	}

	using Val = dom::Value<T>;
	using Var = bgrt::Variable<T>;
	using Array = std::unordered_map<uint64_t, Val>;
	using Configuration = std::unordered_map<uint64_t, Var>;

	Array SubmitVals;

	std::vector<Array> SampleConfs(k);

	/* Call F on this configuration K times (Section 3.1) */
	for (const auto &Pair : C)
	{
		/* Sample a point within the given domain */
		uint64_t tileK = 0;
		for (; tileK < SampleConfs.capacity(); tileK++)
		{
			SampleConfs[tileK][Pair.first] = Pair.second.Sample();
		}
	}

	/* Call F on this configuration K times (Section 3.1)*/
	for (uint64_t iK = 0; iK < k; iK++)
	{
		const Array &Next = P(SampleConfs[iK]);

		for (auto &Pair : Next)
		{
			hpfloat Error = Pair.second.Error();
			hpfloat RelError = Pair.second.RelError();

			if (Error > Err)
			{
				Err = Error;
				RelErr = RelError;
			}

			TotalShadowOps += Pair.second.Ops();
		}		
	}

	return {Err, RelErr, TotalShadowOps};	
}

}


#endif