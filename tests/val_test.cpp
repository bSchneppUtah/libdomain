#include <domain.hpp>

using MyVal = dom::Value<float>;

int main()
{
	dom::Init();
	std::cout.precision(128);

	MyVal One((dom::hpfloat)1.33333333333333333333);
	MyVal Two((dom::hpfloat)1.0);

	std::cout << "One values: " << One.Val() << ", " << One.SVal() << std::endl;
	std::cout << "Two values: " << Two.Val() << ", " << Two.SVal() << std::endl;

	MyVal Res = One - Two;
	std::cout << "Res values: " << Res.Val() << ", " << Res.SVal() << std::endl;
	std::cout << "Diff: " << Res.Diff() << std::endl;
}