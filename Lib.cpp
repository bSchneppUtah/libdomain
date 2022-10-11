#include <vector>
#include <stdint.h>

#include "domain.hpp"

/**
 * @file Lib.cpp
 * @brief Definitions necessary for functionality of libdomain
 */

namespace dom
{

/**
 * @brief Initialized libdomain for high-precision numbers.
 * @author Brian Schnepp
 */
void Init()
{
	mpfr::mpreal::set_default_prec(128);
}

}