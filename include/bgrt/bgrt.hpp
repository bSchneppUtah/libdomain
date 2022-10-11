#include <array>
#include <random>
#include <vector>

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
	Variable(T Min, T Max)
	{
		this->Minimum = Min;
		this->Maximum = Max;
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

	std::array<Variable<T>, 2> Subranges() const
	{
		std::array<Variable<T>, 2> RetVal;

		std::random_device Dev;
		std::mt19937 Gen(Dev());
		
		T Middle = this->Minimum.Val() + ((this->Maximum.Val() - this->Minimum.Val()) / 2);
		dom::hpfloat MiddleHR = this->Minimum.SVal() + ((this->Maximum.SVal() - this->Minimum.SVal()) / 2);

		dom::Value<T> Mid(Middle, MiddleHR);

		RetVal[0] = Variable(this->Minimum, Mid);
		RetVal[1] = Variable(Mid, this->Maximum);

		return RetVal;
	}

	dom::Value<T> Min() const
	{
		return this->Minimum;
	}

	dom::Value<T> Max() const
	{
		return this->Maximum;
	}

	dom::Value<T> Average() const
	{
		return this->Minimum + ((this->Maximum - this->Minimum) / 2);
	}

private:
	dom::Value<T> Maximum;
	dom::Value<T> Minimum;
};


/*
 * Considerations: 
 * 	- does the index of variables matter?
 */

template<typename T>
class BGRTState
{
	using VarT = Variable<T>;
	using Configuration = std::unordered_map<uint64_t, VarT>;
	using ConfigurationOptions = std::array<Configuration, 2>;

public:
	static ConfigurationOptions HalfConfigs(const Configuration &Vals) 
	{
		ConfigurationOptions RetVal;
		for (auto &Pair : Vals)
		{
			VarT Item = Pair.second;
			std::array<VarT, 2> HalfConf = Item.Subranges();
			for (int InsertIndex = 0; InsertIndex < RetVal.size(); InsertIndex++)
			{
				RetVal[InsertIndex].insert_or_assign(Pair.first, HalfConf[InsertIndex]);
			}

		}	
		return RetVal;
	}

	static Configuration UnionConfigurations(const Configuration &Left, const Configuration &Right)
	{
		Configuration RetVal;
		for (const auto &Pair : Left)
		{
			RetVal[Pair.first] = Pair.second;
		}
		for (const auto &Pair : Right)
		{
			RetVal[Pair.first] = Pair.second;
		}
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
	ConfigurationOptions PartConf() const {
		ConfigurationOptions RetVal;
		
		std::random_device Dev;
		std::mt19937 Gen(Dev());
		std::uniform_int_distribution<int> Dist(0, 1);
		for (auto &Pair : this->Vals)
		{
			/* Mod by 2 just to be safe for sure. */
			int Selection = Dist(Gen) % 2;
			RetVal[Selection][Pair.first] = Pair.second;
		}
		return RetVal;
	}
	
	/* Refer to Section 3.4 in the S3FP paper */
	std::vector<Configuration> NextGen(uint64_t NPart)
	{
		std::vector<Configuration> NextG;
		
		/* Append both halves of the current entry in */
		ConfigurationOptions Configs = HalfConfigs(this->Vals);
		NextG.push_back(Configs[0]);
		NextG.push_back(Configs[1]);
		
		for (int i = 0; i < NPart; i++)
		{
			ConfigurationOptions Arr = PartConf();
			ConfigurationOptions Cx = HalfConfigs(Arr[0]);
			ConfigurationOptions Cy = HalfConfigs(Arr[1]);
			
			/* up(Cx) U down(Cy) */
			NextG.push_back(UnionConfigurations(Cx[0], Cy[1]));
			
			/* down(Cx) U up(Cy) */
			NextG.push_back(UnionConfigurations(Cx[1], Cy[0]));
			
		}
		return NextG;
	}
	
private:
	Configuration Vals;
};

}


#endif
