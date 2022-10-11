#include <hpfloat.hpp>

#ifndef VALUE_HPP_
#define VALUE_HPP_

/**
 * @file include/value.hpp
 * @brief Definition for a generic Value and Array type needed for processing values
 */

namespace dom
{

/*
 * @brief Wrapper type for a floating point value
 * @details Implements a pseudo-floating point type, based upon a lower precision type 
 * T, and a 128-bit precision MPFR shadow value. The size of type T must be smaller than the size of the MPFR float.
 * 
 * @param T The lower precision floating point value to use: may be anything which appears as some kind of float type.
 * @author Brian Schnepp
 */
template<typename T>
struct Value
{
	static_assert(sizeof(T) != sizeof(dom::hpfloat));
	static_assert(sizeof(T) < sizeof(dom::hpfloat));
	
	/**
	 * @brief Implements the generic 0-value constructor 
	 */
	Value()
	{
		this->OrigVal = 0.0;
		this->Shadow = OrigVal;
	}

	/**
	 * @brief Construct a Value based upon an existing hpfloat and low-precision number 
	 */
	Value(T Orig, hpfloat Shadow)
	{
		this->OrigVal = Orig;
		this->Shadow = Shadow;
	}

	/**
	 * @brief Constuct a new Value based upon a high-precision number. 
	 * This is the default way numbers should be produced. 
	 */
	Value(const hpfloat &Val)
	{
		this->OrigVal = (T)Val;
		this->Shadow = (hpfloat)Val;
	}

	Value(const Value &Other)
	{
		this->OrigVal = (T)Other.OrigVal;
		this->Shadow = (hpfloat)Other.Shadow;		
	}

	Value(Value &&Other)
	{
		this->OrigVal = (T)Other.OrigVal;
		this->Shadow = (hpfloat)Other.Shadow;

		Other.OrigVal = T();
		Other.Shadow = T();
	}

	Value<T> operator+(Value<T> Other)
	{
		T NOrigVal = this->OrigVal + Other.OrigVal;
		hpfloat NShadow = (hpfloat)this->Shadow + (hpfloat)Other.Shadow;
		return Value<T>{NOrigVal, NShadow};
	}

	Value<T> operator-(Value<T> Other)
	{
		T NOrigVal = this->OrigVal - Other.OrigVal;
		hpfloat NShadow = (hpfloat)this->Shadow - (hpfloat)Other.Shadow;
		return Value<T>{NOrigVal, NShadow};
	}

	Value<T> operator*(Value<T> Other)
	{
		T NOrigVal = this->OrigVal * Other.OrigVal;
		hpfloat NShadow = (hpfloat)this->Shadow * (hpfloat)Other.Shadow;
		return Value<T>{NOrigVal, NShadow};
	}

	Value<T> operator/(Value<T> Other)
	{
		T NOrigVal = this->OrigVal / Other.OrigVal;
		hpfloat NShadow = (hpfloat)this->Shadow / (hpfloat)Other.Shadow;
		return Value<T>{NOrigVal, NShadow};
	}

	Value<T> &operator+=(Value<T> Other)
	{
		this->OrigVal += Other.OrigVal;
		this->Shadow += (hpfloat)Other.Shadow;
		return *this;
	}

	Value<T> &operator-=(Value<T> Other)
	{
		this->OrigVal -= Other.OrigVal;
		this->Shadow -= (hpfloat)Other.Shadow;
		return *this;
	}

	Value<T> &operator*=(Value<T> Other)
	{
		this->OrigVal *= Other.OrigVal;
		this->Shadow *= (hpfloat)Other.Shadow;
		return *this;
	}

	Value<T> &operator/=(Value<T> Other)
	{
		this->OrigVal /= Other.OrigVal;
		this->Shadow /= (hpfloat)Other.Shadow;
		return *this;
	}

	Value<T> &operator=(Value<T> Other)
	{
		this->OrigVal = Other.OrigVal;
		this->Shadow = (hpfloat)Other.Shadow;
		return *this;
	}

	bool operator==(Value<T> Other)
	{
		return this->OrigVal == Other.OrigVal 
			|| this->Shadow == Other.Shadow;
	}

	bool operator!=(Value<T> Other)
	{
		return this->OrigVal != Other.OrigVal 
			&& this->Shadow != Other.Shadow;
	}

	Value<T> operator+()
	{
		T NOrigVal = +this->OrigVal;
		hpfloat NShadow = +this->Shadow;
		return Value<T>{NOrigVal, NShadow};
	}

	Value<T> operator-()
	{
		T NOrigVal = -this->OrigVal;
		hpfloat NShadow = -this->Shadow;
		return Value<T>{NOrigVal, NShadow};
	}

	/**
	 * @brief Compares the high-precision shadow value with the low-precision value, returning their difference
	 * @return An hpfloat which describes the absolute value of the difference between the shadow value and traced value
	 */
	hpfloat Diff()
	{
		/* Guarantee absolute value */
		hpfloat Tmp = this->Shadow;
		Tmp -= (hpfloat)this->OrigVal;
		return (Tmp < 0) ? -Tmp : Tmp;
	}

	/**
	 * @brief Obtains the low-precision version of this value
	 * @return T a low-precision representation of this Value
	 */
	T Val() const
	{
		return this->OrigVal;
	}

	/**
	 * @brief Obtains the high-precision version of this value (via the shadow value)
	 * @return T a high-precision representation of this Value
	 */
	hpfloat SVal() const
	{
		return this->Shadow;
	}

private:
	T OrigVal;
	hpfloat Shadow;
};


template<typename T, uint64_t Size>
struct Array
{
	/* Consider aliasing this to just be using Array<T, Size> = std::array<Value<T>, Size> */

	Array() = default;

	constexpr Value<T> &operator[](uint64_t Index)
	{
		return Vals[Index];
	}

	constexpr Value<T> operator[](uint64_t Index) const
	{
		return Vals[Index];
	}

	[[nodiscard]]
	constexpr uint64_t GetSize() const {
		return Size;
	}

	std::array<Value<T>, Size> Vals;
};

}

#endif