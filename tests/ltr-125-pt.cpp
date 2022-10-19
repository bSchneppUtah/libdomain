#include <iostream>
#include <domain.hpp>

#define ARR_SIZE (125)

using MyVal = dom::Value<float>;
using MyArray = dom::Array<float, ARR_SIZE>;

uint64_t ToLinearAddr(int i, int j, int k)
{
	return (k * 25) + (j * 5) + i;
}

MyArray Function(const MyArray &Arr)
{
	MyArray RetVal;
	
#if 0
	MyVal Summation;
	MyArray RetVal;

	for (int i = 1; i < Arr.GetSize() - 1; i++)
	{
		RetVal[i] = (dom::hpfloat)0;
		RetVal[i] += Arr[i] * (dom::hpfloat)0.333333333333333333333333333333333;
		RetVal[i] -= Arr[i-1] * (dom::hpfloat)0.7777777777777777777777777777777;
		RetVal[i] += Arr[i+1] * (dom::hpfloat)0.33333333333333333333333333333333;
	}

	return Arr;
#endif
	
	MyVal Coeffs[125];
	for (uint64_t Index = 0; Index < 125; Index++)
	{
		Coeffs[Index] = (dom::hpfloat)1.0;
	}
	
	int k = 2;	/* 2 is middle of [0, 4] */
	int j = 2;
	int i = 2;
	
	int offset = (k * 25) + (j * 5) + i;
	RetVal[offset] = (Coeffs[0] * Arr[ToLinearAddr(i+0,j+1,k+1)]) 
		+ (Coeffs[1] * Arr[ToLinearAddr(i+0,j+0,k+1)])
		+ (Coeffs[2] * Arr[ToLinearAddr(i+0,j-1,k+1)])
		+ (Coeffs[3] * Arr[ToLinearAddr(i+0,j+2,k+1)])
		+ (Coeffs[4] * Arr[ToLinearAddr(i+0,j-2,k+1)])
				
		+ (Coeffs[(5*1) + 0] * Arr[ToLinearAddr(i+1,j+1,k+1)]) 
		+ (Coeffs[(5*1) + 1] * Arr[ToLinearAddr(i+1,j+0,k+1)])
		+ (Coeffs[(5*1) + 2] * Arr[ToLinearAddr(i+1,j-1,k+1)])
		+ (Coeffs[(5*1) + 3] * Arr[ToLinearAddr(i+1,j+2,k+1)])
		+ (Coeffs[(5*1) + 4] * Arr[ToLinearAddr(i+1,j-2,k+1)])

		+ (Coeffs[(5*2) + 0] * Arr[ToLinearAddr(i-1,j+1,k+1)]) 
		+ (Coeffs[(5*2) + 1] * Arr[ToLinearAddr(i-1,j+0,k+1)])
		+ (Coeffs[(5*2) + 2] * Arr[ToLinearAddr(i-1,j-1,k+1)])
		+ (Coeffs[(5*2) + 3] * Arr[ToLinearAddr(i-1,j+2,k+1)])
		+ (Coeffs[(5*2) + 4] * Arr[ToLinearAddr(i-1,j-2,k+1)])		
		
		+ (Coeffs[(5*3) + 0] * Arr[ToLinearAddr(i+2,j+1,k+1)]) 
		+ (Coeffs[(5*3) + 1] * Arr[ToLinearAddr(i+2,j+0,k+1)])
		+ (Coeffs[(5*3) + 2] * Arr[ToLinearAddr(i+2,j-1,k+1)])
		+ (Coeffs[(5*3) + 3] * Arr[ToLinearAddr(i+2,j+2,k+1)])
		+ (Coeffs[(5*3) + 4] * Arr[ToLinearAddr(i+2,j-2,k+1)])				
				
		+ (Coeffs[(5*4) + 0] * Arr[ToLinearAddr(i-2,j+1,k+1)]) 
		+ (Coeffs[(5*4) + 1] * Arr[ToLinearAddr(i-2,j+0,k+1)])
		+ (Coeffs[(5*4) + 2] * Arr[ToLinearAddr(i-2,j-1,k+1)])
		+ (Coeffs[(5*4) + 3] * Arr[ToLinearAddr(i-2,j+2,k+1)])
		+ (Coeffs[(5*4) + 4] * Arr[ToLinearAddr(i-2,j-2,k+1)])

		+ (Coeffs[(5*5) + 0] * Arr[ToLinearAddr(i+0,j+1,k+0)]) 
		+ (Coeffs[(5*5) + 1] * Arr[ToLinearAddr(i+0,j+0,k+0)])
		+ (Coeffs[(5*5) + 2] * Arr[ToLinearAddr(i+0,j-1,k+0)])
		+ (Coeffs[(5*5) + 3] * Arr[ToLinearAddr(i+0,j+2,k+0)])
		+ (Coeffs[(5*5) + 4] * Arr[ToLinearAddr(i+0,j-2,k+0)])
				
		+ (Coeffs[(5*6) + 0] * Arr[ToLinearAddr(i+1,j+1,k+0)]) 
		+ (Coeffs[(5*6) + 1] * Arr[ToLinearAddr(i+1,j+0,k+0)])
		+ (Coeffs[(5*6) + 2] * Arr[ToLinearAddr(i+1,j-1,k+0)])
		+ (Coeffs[(5*6) + 3] * Arr[ToLinearAddr(i+1,j+2,k+0)])
		+ (Coeffs[(5*6) + 4] * Arr[ToLinearAddr(i+1,j-2,k+0)])

		+ (Coeffs[(5*7) + 0] * Arr[ToLinearAddr(i-1,j+1,k+0)]) 
		+ (Coeffs[(5*7) + 1] * Arr[ToLinearAddr(i-1,j+0,k+0)])
		+ (Coeffs[(5*7) + 2] * Arr[ToLinearAddr(i-1,j-1,k+0)])
		+ (Coeffs[(5*7) + 3] * Arr[ToLinearAddr(i-1,j+2,k+0)])
		+ (Coeffs[(5*7) + 4] * Arr[ToLinearAddr(i-1,j-2,k+0)])		
		
		+ (Coeffs[(5*8) + 0] * Arr[ToLinearAddr(i+2,j+1,k+0)]) 
		+ (Coeffs[(5*8) + 1] * Arr[ToLinearAddr(i+2,j+0,k+0)])
		+ (Coeffs[(5*8) + 2] * Arr[ToLinearAddr(i+2,j-1,k+0)])
		+ (Coeffs[(5*8) + 3] * Arr[ToLinearAddr(i+2,j+2,k+0)])
		+ (Coeffs[(5*8) + 4] * Arr[ToLinearAddr(i+2,j-2,k+0)])				
				
		+ (Coeffs[(5*9) + 0] * Arr[ToLinearAddr(i-2,j+1,k+0)]) 
		+ (Coeffs[(5*9) + 1] * Arr[ToLinearAddr(i-2,j+0,k+0)])
		+ (Coeffs[(5*9) + 2] * Arr[ToLinearAddr(i-2,j-1,k+0)])
		+ (Coeffs[(5*9) + 3] * Arr[ToLinearAddr(i-2,j+2,k+0)])
		+ (Coeffs[(5*9) + 4] * Arr[ToLinearAddr(i-2,j-2,k+0)])

		+ (Coeffs[(5*10) + 0] * Arr[ToLinearAddr(i+0,j+1,k-1)]) 
		+ (Coeffs[(5*10) + 1] * Arr[ToLinearAddr(i+0,j+0,k-1)])
		+ (Coeffs[(5*10) + 2] * Arr[ToLinearAddr(i+0,j-1,k-1)])
		+ (Coeffs[(5*10) + 3] * Arr[ToLinearAddr(i+0,j+2,k-1)])
		+ (Coeffs[(5*10) + 4] * Arr[ToLinearAddr(i+0,j-2,k-1)])
				
		+ (Coeffs[(5*11) + 0] * Arr[ToLinearAddr(i+1,j+1,k-1)]) 
		+ (Coeffs[(5*11) + 1] * Arr[ToLinearAddr(i+1,j+0,k-1)])
		+ (Coeffs[(5*11) + 2] * Arr[ToLinearAddr(i+1,j-1,k-1)])
		+ (Coeffs[(5*11) + 3] * Arr[ToLinearAddr(i+1,j+2,k-1)])
		+ (Coeffs[(5*11) + 4] * Arr[ToLinearAddr(i+1,j-2,k-1)])

		+ (Coeffs[(5*12) + 0] * Arr[ToLinearAddr(i-1,j+1,k-1)]) 
		+ (Coeffs[(5*12) + 1] * Arr[ToLinearAddr(i-1,j+0,k-1)])
		+ (Coeffs[(5*12) + 2] * Arr[ToLinearAddr(i-1,j-1,k-1)])
		+ (Coeffs[(5*12) + 3] * Arr[ToLinearAddr(i-1,j+2,k-1)])
		+ (Coeffs[(5*12) + 4] * Arr[ToLinearAddr(i-1,j-2,k-1)])		
		
		+ (Coeffs[(5*13) + 0] * Arr[ToLinearAddr(i+2,j+1,k-1)]) 
		+ (Coeffs[(5*13) + 1] * Arr[ToLinearAddr(i+2,j+0,k-1)])
		+ (Coeffs[(5*13) + 2] * Arr[ToLinearAddr(i+2,j-1,k-1)])
		+ (Coeffs[(5*13) + 3] * Arr[ToLinearAddr(i+2,j+2,k-1)])
		+ (Coeffs[(5*13) + 4] * Arr[ToLinearAddr(i+2,j-2,k-1)])				
				
		+ (Coeffs[(5*14) + 0] * Arr[ToLinearAddr(i-2,j+1,k-1)]) 
		+ (Coeffs[(5*14) + 1] * Arr[ToLinearAddr(i-2,j+0,k-1)])
		+ (Coeffs[(5*14) + 2] * Arr[ToLinearAddr(i-2,j-1,k-1)])
		+ (Coeffs[(5*14) + 3] * Arr[ToLinearAddr(i-2,j+2,k-1)])
		+ (Coeffs[(5*14) + 4] * Arr[ToLinearAddr(i-2,j-2,k-1)])

		+ (Coeffs[(5*15) + 0] * Arr[ToLinearAddr(i+0,j+1,k+2)]) 
		+ (Coeffs[(5*15) + 1] * Arr[ToLinearAddr(i+0,j+0,k+2)])
		+ (Coeffs[(5*15) + 2] * Arr[ToLinearAddr(i+0,j-1,k+2)])
		+ (Coeffs[(5*15) + 3] * Arr[ToLinearAddr(i+0,j+2,k+2)])
		+ (Coeffs[(5*15) + 4] * Arr[ToLinearAddr(i+0,j-2,k+2)])
				
		+ (Coeffs[(5*16) + 0] * Arr[ToLinearAddr(i+1,j+1,k+2)]) 
		+ (Coeffs[(5*16) + 1] * Arr[ToLinearAddr(i+1,j+0,k+2)])
		+ (Coeffs[(5*16) + 2] * Arr[ToLinearAddr(i+1,j-1,k+2)])
		+ (Coeffs[(5*16) + 3] * Arr[ToLinearAddr(i+1,j+2,k+2)])
		+ (Coeffs[(5*16) + 4] * Arr[ToLinearAddr(i+1,j-2,k+2)])

		+ (Coeffs[(5*17) + 0] * Arr[ToLinearAddr(i-1,j+1,k+2)]) 
		+ (Coeffs[(5*17) + 1] * Arr[ToLinearAddr(i-1,j+0,k+2)])
		+ (Coeffs[(5*17) + 2] * Arr[ToLinearAddr(i-1,j-1,k+2)])
		+ (Coeffs[(5*17) + 3] * Arr[ToLinearAddr(i-1,j+2,k+2)])
		+ (Coeffs[(5*17) + 4] * Arr[ToLinearAddr(i-1,j-2,k+2)])		
		
		+ (Coeffs[(5*18) + 0] * Arr[ToLinearAddr(i+2,j+1,k+2)]) 
		+ (Coeffs[(5*18) + 1] * Arr[ToLinearAddr(i+2,j+0,k+2)])
		+ (Coeffs[(5*18) + 2] * Arr[ToLinearAddr(i+2,j-1,k+2)])
		+ (Coeffs[(5*18) + 3] * Arr[ToLinearAddr(i+2,j+2,k+2)])
		+ (Coeffs[(5*18) + 4] * Arr[ToLinearAddr(i+2,j-2,k+2)])				
				
		+ (Coeffs[(5*19) + 0] * Arr[ToLinearAddr(i-2,j+1,k+2)]) 
		+ (Coeffs[(5*19) + 1] * Arr[ToLinearAddr(i-2,j+0,k+2)])
		+ (Coeffs[(5*19) + 2] * Arr[ToLinearAddr(i-2,j-1,k+2)])
		+ (Coeffs[(5*19) + 3] * Arr[ToLinearAddr(i-2,j+2,k+2)])
		+ (Coeffs[(5*19) + 4] * Arr[ToLinearAddr(i-2,j-2,k+2)])

		+ (Coeffs[(5*20) + 0] * Arr[ToLinearAddr(i+0,j+1,k-2)]) 
		+ (Coeffs[(5*20) + 1] * Arr[ToLinearAddr(i+0,j+0,k-2)])
		+ (Coeffs[(5*20) + 2] * Arr[ToLinearAddr(i+0,j-1,k-2)])
		+ (Coeffs[(5*20) + 3] * Arr[ToLinearAddr(i+0,j+2,k-2)])
		+ (Coeffs[(5*20) + 4] * Arr[ToLinearAddr(i+0,j-2,k-2)])
				
		+ (Coeffs[(5*21) + 0] * Arr[ToLinearAddr(i+1,j+1,k-2)]) 
		+ (Coeffs[(5*21) + 1] * Arr[ToLinearAddr(i+1,j+0,k-2)])
		+ (Coeffs[(5*21) + 2] * Arr[ToLinearAddr(i+1,j-1,k-2)])
		+ (Coeffs[(5*21) + 3] * Arr[ToLinearAddr(i+1,j+2,k-2)])
		+ (Coeffs[(5*21) + 4] * Arr[ToLinearAddr(i+1,j-2,k-2)])

		+ (Coeffs[(5*22) + 0] * Arr[ToLinearAddr(i-1,j+1,k-2)]) 
		+ (Coeffs[(5*22) + 1] * Arr[ToLinearAddr(i-1,j+0,k-2)])
		+ (Coeffs[(5*22) + 2] * Arr[ToLinearAddr(i-1,j-1,k-2)])
		+ (Coeffs[(5*22) + 3] * Arr[ToLinearAddr(i-1,j+2,k-2)])
		+ (Coeffs[(5*22) + 4] * Arr[ToLinearAddr(i-1,j-2,k-2)])		
		
		+ (Coeffs[(5*23) + 0] * Arr[ToLinearAddr(i+2,j+1,k-2)]) 
		+ (Coeffs[(5*23) + 1] * Arr[ToLinearAddr(i+2,j+0,k-2)])
		+ (Coeffs[(5*23) + 2] * Arr[ToLinearAddr(i+2,j-1,k-2)])
		+ (Coeffs[(5*23) + 3] * Arr[ToLinearAddr(i+2,j+2,k-2)])
		+ (Coeffs[(5*23) + 4] * Arr[ToLinearAddr(i+2,j-2,k-2)])				
				
		+ (Coeffs[(5*24) + 0] * Arr[ToLinearAddr(i-2,j+1,k-2)]) 
		+ (Coeffs[(5*24) + 1] * Arr[ToLinearAddr(i-2,j+0,k-2)])
		+ (Coeffs[(5*24) + 2] * Arr[ToLinearAddr(i-2,j-1,k-2)])
		+ (Coeffs[(5*24) + 3] * Arr[ToLinearAddr(i-2,j+2,k-2)])
		+ (Coeffs[(5*24) + 4] * Arr[ToLinearAddr(i-2,j-2,k-2)]);
	
	return RetVal;
}

int main()
{
	dom::Init();
	std::cout.precision(128);

	MyArray L;
	MyArray R;
	for (int i = 0; i < L.GetSize(); i++)
	{
		L[i] = (dom::hpfloat)-1.0;
		R[i] = (dom::hpfloat)1.0;
	}

	dom::hpfloat Res = dom::FindError<float, ARR_SIZE>(L, R, Function, 1000, 1000, 50);
	std::cout << "Worst error: " << Res << std::endl;
}
