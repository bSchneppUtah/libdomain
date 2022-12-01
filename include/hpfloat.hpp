#include <mpreal.h>

#ifndef DOMAIN_HPFLOAT_HPP_
#define DOMAIN_HPFLOAT_HPP_

namespace dom
{

using hpfloat = mpfr::mpreal;
namespace hp = mpfr;

static inline hpfloat eps(const hpfloat &Other)
{
	return machine_epsilon(Other);
}

static inline uint64_t ulps(const hpfloat &First, const hpfloat &Second)
{
	bool equal = false;
	uint64_t numUlps = 0;
	while (!(equal = isEqualUlps(First, Second, numUlps)))
	{
		numUlps++;
	}
	return numUlps;
}

}


#endif