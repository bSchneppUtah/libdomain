#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef MANTISSA_HPP_
#define MANTISSA_HPP_

namespace dom
{

/* If using something else, there needs to exist a mantissa function to get a value out of it. */
template<typename T>
uint64_t MantissaBits(T F)
{
	return mantissa(F);
}

template<>
inline uint64_t MantissaBits(float F)
{
	static_assert(sizeof(float) == sizeof(uint32_t));

	uint32_t Value;
	memcpy(&Value, &F, sizeof(float));

	/* Mask in only the mantissa. Assume IEEE 754 32-bit float format. */
	Value &= ((1ULL << 23) - 1);
	return (uint64_t)Value;
}

template<>
inline uint64_t MantissaBits(double F)
{
	static_assert(sizeof(double) == sizeof(uint64_t));

	uint64_t Value;
	memcpy(&Value, &F, sizeof(float));

	/* Mask in only the mantissa. Assume IEEE 754 64-bit float format. */
	Value &= ((1ULL << 53) - 1);
	return Value;
}

/* If using something else, there needs to exist a mantissa function to get a value out of it. */
template<typename T>
uint64_t ExponentBits(T F)
{
	return exponent(F);
}

template<>
inline uint64_t ExponentBits(float F)
{
	static_assert(sizeof(float) == sizeof(uint32_t));

	uint32_t Value;
	memcpy(&Value, &F, sizeof(float));

	/* Mask in only the mantissa. Assume IEEE 754 32-bit float format. */
	Value &= ((0xF << 23));
	return (uint64_t)Value;
}

template<>
inline uint64_t ExponentBits(double F)
{
	static_assert(sizeof(double) == sizeof(uint64_t));

	uint64_t Value;
	memcpy(&Value, &F, sizeof(float));

	/* Mask in only the exponent. Assume IEEE 754 64-bit float format. */
	Value &= ((0x7FFULL << 52));
	return Value;
}

template<typename T>
uint64_t SignBits(T F)
{
	return F < 0.0;
}

}

#endif