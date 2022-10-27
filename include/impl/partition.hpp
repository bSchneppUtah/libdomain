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
PartitionConfigs(uint64_t NumThreads, const std::vector < std::unordered_map < uint64_t, bgrt::Variable<T> > > &NextConfs,
		 F OkayFn)
{
	std::vector < std::vector < std::unordered_map < uint64_t, bgrt::Variable<T> > > > PartNextConfs(NumThreads);

	uint64_t TotalPartIndex = 0;
	for (; TotalPartIndex < NextConfs.size(); TotalPartIndex+=NumThreads)
	{
		for (uint64_t PartitionIndex = 0; PartitionIndex < NumThreads; PartitionIndex++)
		{
			if (OkayFn(NextConfs[TotalPartIndex])) 
			{
				PartNextConfs[PartitionIndex].push_back(NextConfs[TotalPartIndex]);
			}
		}
	}

	for (; TotalPartIndex < NextConfs.size(); TotalPartIndex++)
	{
		if (OkayFn(NextConfs[TotalPartIndex]))
		{
			PartNextConfs[TotalPartIndex % NumThreads].push_back(NextConfs[TotalPartIndex]);
		}
	}

	return PartNextConfs;
}


}

#endif