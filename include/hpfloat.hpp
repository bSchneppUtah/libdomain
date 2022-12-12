#include <mpreal.h>

#ifndef DOMAIN_HPFLOAT_HPP_
#define DOMAIN_HPFLOAT_HPP_

namespace dom
{

using hpfloat = mpfr::mpreal;
namespace hp = mpfr;

constexpr mpfr_rnd_t HP_ROUNDING = MPFR_RNDF;

}


#endif