#include <vector>
#include <cstdint>
#include <unordered_map>

#include "bgrt/bgrt.hpp"

#ifndef DOMAIN_IMPL_BGRT_PARTITION_
#define DOMAIN_IMPL_BGRT_PARTITION_


namespace dom::impl
{

template<typename T, typename F>
std::vector < std::vector < std::unordered_map < uint64_t, bgrt::Variable<T> > > >  
PartitionConfigs(uint64_t NumThreads, const uint64_t Iterations, bgrt::BGRTState<T> &BGRT, F OkayFn)
{
	std::vector < std::vector < std::unordered_map < uint64_t, bgrt::Variable<T> > > > PartNextConfs(NumThreads);

	uint64_t MyIterations = Iterations / NumThreads;
	for (uint64_t TID = 0; TID < NumThreads; TID++)
	{
		if (TID + 1 == NumThreads)
		{
			/* Get the rest of the values */
			MyIterations += Iterations % NumThreads;
		}
		
		const std::vector<std::unordered_map < uint64_t, bgrt::Variable<T> > > NextConfs = BGRT.NextGen(MyIterations);
		for (uint64_t TotalIndex = 0; TotalIndex < NextConfs.size(); TotalIndex++)
		{
			if (OkayFn(NextConfs[TotalIndex]))
			{
				PartNextConfs[TID].push_back(NextConfs[TotalIndex]);
			}
		}
	}
	return PartNextConfs;
}


}

#endif