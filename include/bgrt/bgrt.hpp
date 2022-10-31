#include <array>
#include <random>
#include <vector>

#include <queue>
#include <hpfloat.hpp>
#include <unordered_map>


#ifndef BGRT_HPP_
#define BGRT_HPP_

namespace bgrt
{

template<typename T>
class Variable
{
public:
	Variable() = default;

	Variable(dom::hpfloat Min, dom::hpfloat Max)
	{
		this->Minimum = dom::Value<T>(Min);
		this->Maximum = dom::Value<T>(Max);
	}

	Variable(dom::Value<T> Min, dom::Value<T> Max)
	{
		this->Minimum = Min;
		this->Maximum = Max;
	}

	~Variable() = default;

	Variable(const Variable &Other)
	{
		if (&Other == this) 
		{
			return;
		}
		
		this->Maximum = Other.Maximum;
		this->Minimum = Other.Minimum;		
	}

	Variable &operator=(const Variable &Other)
	{
		if (&Other == this)
		{
			return *this;
		}
		
		this->Maximum = Other.Maximum;
		this->Minimum = Other.Minimum;
		return *this;
	}

	Variable &operator=(Variable &&Other)
	{
		if (&Other == this)
		{
			return *this;
		}
		
		this->Minimum = std::move(Other.Minimum);
		this->Maximum = std::move(Other.Maximum);
		return *this;
	}

	dom::Value<T> Min() const
	{
		return this->Minimum;
	}

	dom::Value<T> Max() const
	{
		return this->Maximum;
	}

	dom::Value<T> Size() const
	{
		return this->Maximum - this->Minimum;
	}

	dom::Value<T> Average() const
	{
		return this->Minimum + ((this->Maximum - this->Minimum) / (dom::hpfloat)2.0);
	}

	dom::Value<T> Sample() const
	{
		static std::random_device Dev;
		static std::mt19937 Gen(Dev());
		static std::uniform_int_distribution<int> SDist(INT32_MIN, INT32_MAX);


#define OKAY_RANDOM
#ifdef ACCURATE_RANDOM
		dom::hpfloat RandScale = this->Maximum.SVal() - this->Minimum.SVal();
		dom::hpfloat RandNumber = mpfr::random(SDist(Gen));
#elif defined(FAIR_RANDOM)
		std::uniform_real_distribution<double> RDist(0.0, 1.0);
		std::uniform_real_distribution<double> Dist(0.0, (double)(this->Maximum.SVal() - this->Minimum.SVal()));
		dom::hpfloat RandScale = Dist(Gen);
		dom::hpfloat RandNumber = RDist(Gen);
#elif defined(OKAY_RANDOM)
		dom::hpfloat RandScale = this->Maximum.SVal() - this->Minimum.SVal();
		dom::hpfloat RandNumber = (double)rand() / (double)RAND_MAX;
#elif defined(TIME_RANDOM)
		dom::hpfloat RandScale = this->Maximum.SVal() - this->Minimum.SVal();
		uint64_t Time = time(0) % 180381;
		dom::hpfloat RandNumber = (double)Time / 180381;
#endif
		dom::Value<T> FinalSample = this->Minimum + (RandScale * RandNumber);

		if (FinalSample > this->Maximum || FinalSample < this->Minimum)
		{
			std::cerr << "BAD NUMBER GENERATION! ABORT!" << std::endl;
			std::cerr << "WE GOT: " << FinalSample.SVal() << std::endl;
			std::cerr << "(with addend " << RandScale << " * " << RandNumber << "): " << (RandScale * RandNumber) << std::endl;
			std::cerr << "BOUNDS ARE: [" << this->Minimum.SVal() << ", " << this->Maximum.SVal() << "]" << std::endl;
			exit(-1);
		}

		return dom::Value<T>(FinalSample);		
	}

	dom::hpfloat Error() const
	{
		dom::hpfloat VarMinErr = this->Minimum.Error();
		dom::hpfloat VarMaxErr = this->Maximum.Error();
		return (VarMinErr < VarMaxErr) ? VarMaxErr : VarMinErr;
	}

	Variable<T> operator+(dom::Value<T> Other) const
	{
		return Variable<T>{this->Min + Other, this->Max + Other};
	}

	Variable<T> operator-(dom::Value<T> Other) const
	{
		return Variable<T>{this->Min - Other, this->Max - Other};
	}

	Variable<T> operator*(dom::Value<T> Other) const
	{
		return Variable<T>{this->Min * Other, this->Max * Other};
	}

	Variable<T> operator/(dom::Value<T> Other) const
	{
		return Variable<T>{this->Min / Other, this->Max / Other};
	}

	Variable<T> &operator+=(dom::Value<T> Other)
	{
		this->Min += Other;
		this->Max += Other;
		return *this;
	}

	Variable<T> &operator-=(dom::Value<T> Other)
	{
		this->Min -= Other;
		this->Max -= Other;
		return *this;
	}

	Variable<T> &operator*=(dom::Value<T> Other)
	{
		this->Min *= Other;
		this->Max *= Other;
		return *this;
	}

	Variable<T> &operator/=(dom::Value<T> Other)
	{
		this->Min /= Other;
		this->Max /= Other;
		return *this;
	}

	Variable<T> operator+() const
	{
		return Variable<T>{+this->Min, +this->Max};
	}

	Variable<T> operator-() const
	{
		return Variable<T>{-this->Min, -this->Max};
	}

private:
	dom::Value<T> Maximum;
	dom::Value<T> Minimum;
};

/* Remaining operators need to be implemented as such: */
template<typename T>
static Variable<T> operator+(const Variable<T> &Left, const Variable<T> &Right)
{
	return Variable<T>{Left.Min() + Right.Min(), Left.Max() + Right.Max()};
}

template<typename T>
static Variable<T> operator-(const Variable<T> &Left, const Variable<T> &Right)
{
	return Variable<T>{Left.Min() - Right.Min(), Left.Max() - Right.Max()};
}

template<typename T>
static Variable<T> operator*(const Variable<T> &Left, const Variable<T> &Right)
{
	return Variable<T>{Left.Min() * Right.Min(), Left.Max() * Right.Max()};
}

template<typename T>
static Variable<T> operator/(const Variable<T> &Left, const Variable<T> &Right)
{
	return Variable<T>{Left.Min() / Right.Min(), Left.Max() / Right.Max()};
}

template<typename T>
static Variable<T> operator+(const dom::Value<T> &Left, const Variable<T> &Right)
{
	return Variable<T>{Left + Right.Min(), Left + Right.Max()};
}

template<typename T>
static Variable<T> operator-(const dom::Value<T> &Left, const Variable<T> &Right)
{
	return Variable<T>{Left - Right.Min(), Left - Right.Max()};
}

template<typename T>
static Variable<T> operator*(const dom::Value<T> &Left, const Variable<T> &Right)
{
	return Variable<T>{Left * Right.Min(), Left * Right.Max()};
}

template<typename T>
static Variable<T> operator/(const dom::Value<T> &Left, const Variable<T> &Right)
{
	return Variable<T>{Left / Right.Min(), Left / Right.Max()};
}

template<typename T>
class BGRTState
{
	using VarT = Variable<T>;
	using Configuration = std::unordered_map<uint64_t, VarT>;
	using ConfigurationOptions = std::array<Configuration, 2>;

public:
	[[nodiscard]]
	static const ConfigurationOptions HalfConfigs(const Configuration &Vals)
	{
		ConfigurationOptions RetVal;
		for (const auto &Pair : Vals)
		{
			dom::hpfloat MidP = Pair.second.Min().SVal();
			MidP += (Pair.second.Max().SVal() - Pair.second.Min().SVal()) / 2.0;

			RetVal[0][Pair.first] = Variable(Pair.second.Min(), dom::Value<T>(MidP));
			RetVal[1][Pair.first] = Variable(dom::Value<T>(MidP), Pair.second.Max());
		}
		return RetVal;
	}

	[[nodiscard]]
	static Configuration UnionConfigurations(const Configuration &Left, const Configuration &Right)
	{
		/* Pre-allocate buckets to hopefully avoid being stuck in malloc() over and over */
		Configuration RetVal;
		RetVal.reserve(Left.bucket_count() + Right.bucket_count());
		RetVal.insert(Left.begin(), Left.end());
		RetVal.insert(Right.begin(), Right.end());
		return RetVal;
	}
	
public:
	BGRTState() = default;
	~BGRTState() = default;
	
	BGRTState(const Configuration &Values) 
	{
		this->Vals = Values;
	}

	/* Refer to Section 3.4 in the S3FP paper */
	[[nodiscard]]
	ConfigurationOptions PartConf() const 
	{
		/* Pre-allocate buckets to try to avoid realloc cost */
		ConfigurationOptions RetVal;
		RetVal[0].reserve(this->Vals.bucket_count() / 2);
		RetVal[1].reserve(this->Vals.bucket_count() / 2);

		static std::uniform_int_distribution<int> Dist(0, 1);
		static std::random_device Dev;
		static std::mt19937 Gen(Dev());
		for (const auto &Pair : this->Vals)
		{
			int Selection = Dist(Gen);
			RetVal[Selection][Pair.first] = Pair.second;
		}
		return RetVal;
	}
	
	/* Refer to Section 3.4 in the S3FP paper */
	[[nodiscard]]
	std::vector<Configuration> NextGen(uint64_t NPart) const
	{
		std::vector<Configuration> NextG;

		/* Pre-allocate space for efficiency */
		NextG.reserve((NPart * 2) + 2);
		
		/* Append both halves of the current entry in */
		ConfigurationOptions Configs = HalfConfigs(this->Vals);
		NextG.push_back(Configs[0]);
		NextG.push_back(Configs[1]);

		/* Optimization: Gathering all PartConf() first improves cache locality */
		std::vector<ConfigurationOptions> Options(NPart);
		for (int i = 0; i < NPart; i++)
		{
			Options[i] = PartConf();
		}

		/* Optimization: Similarly, putting together both halves also improves cache locality. */
		std::vector<ConfigurationOptions> Options1(NPart);
		std::vector<ConfigurationOptions> Options2(NPart);

		for (int i = 0; i < NPart; i++)
		{
			const ConfigurationOptions &Arr = Options[i];
			Options1[i] = HalfConfigs(Arr[0]);
			Options2[i] = HalfConfigs(Arr[1]);
		}

		for (int i = 0; i < NPart; i++)
		{	
			const ConfigurationOptions &Cx = Options1[i];
			const ConfigurationOptions &Cy = Options2[i];
			
			/* up(Cx) U down(Cy) */
			NextG.push_back(UnionConfigurations(Cx[0], Cy[1]));
			
			/* down(Cx) U up(Cy) */
			NextG.push_back(UnionConfigurations(Cx[1], Cy[0]));
			
		}
		return NextG;
	}

	void SetVals(const Configuration &Conf)
	{
		this->Vals = Conf;
	}
	
private:
	Configuration Vals;
};

}


#endif
