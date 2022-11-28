# libdomain
An efficient implementation of the BGRT algorithm

## Usage
Per the S3FP paper, a program P must be defined which is sampled for error. libdomain interprets this as a single-dimension function which
can take an arbitrary (but defined by the user) number of inputs, and returns some arbitrary (but defined) number of outputs.

An example function could be
```cpp
using FType = float;

std::unordered_map<uint64_t, dom::Value<FType>> Function(std::unordered_map<uint64_t, dom::Value<FType>> &Arr)
{
	std::unordered_map<uint64_t, dom::Value<FType>> RetVal;
	RetVal[0] = (0.33 * Arr[0]) + Arr[1];
	return RetVal;
}
```

To set this up then, a driver program has to be created, which will invoke the necessary libdomain functions, as in
```cpp
int main()
{
	dom::Init();
	std::cout.precision(128);	// Set up the precision for outputs to std::cout.

	static const uint64_t ARR_SIZE = 2;

	std::unordered_map<uint64_t, bgrt::Variable<FType>> Init;
	for (int i = 0; i < ARR_SIZE; i++)
	{
		Init[i] = bgrt::Variable<FType>((dom::hpfloat)-1.0, (dom::hpfloat)1.0);
	}

	dom::EvalResults Res = dom::FindErrorMantissaMultithread<float>(Init, Function);
	std::cout << "Absolute error: " << Res.Err << ", " << "Relative error: " << Res.RelErr << std::endl;
	return 0;
}
```

For more details, the examples under `tests/` contain example usage of the code.

## Building
At minimum, CMake version 3.10 is required to build this project. It is possible that earlier versions will also be compatible, but this has not yet been tested.

For Ubuntu (or other similar Debian-based Linux distributions), it should be sufficient to simply install the MPFR C++ header bindings,
```sh
apt install libmpfrc++-dev
```

The only hard dependency on this library is `include/hpfloat.hpp`, which in turn requires some setup in `Lib.cpp`. Should it be desired to remove this dependency, replacing the definition of `hpfloat` with a new class implementing similar code and adjusting Lib.cpp appropriately should be sufficient.

An optional dependency is with OpenMPI, with any implementation which can be found by CMake. This enables the use of `domain/mpi.hpp`, but this module loses some error precision when transferring data between MPI hosts, rounding down to a 64-bit double. See `tests/bgrt-balanced-125pt-mpi.cpp` for details.

A Doxyfile is also presented to enable some documentation at the source-code level for most libdomain code. To generate this, simply
run `doxygen` in the project root. Corresponding documentation for each module can then be accessed by pointing a web browser to `docs/html/index.html`.

## Linking
This software is built as a library: a higher level project may incorporate this code by adding it as a subdirectory, as:
```cmake
add_subdirectory(<path/to/libdomain>)
target_include_directories(<path/to/libdomain>/include)
```
It is left to future work to create CMake variables to wrap these definitions in.

CMake targets wishing to make use of libdomain will need to link with the libdomain target.
This can be done with 

```cmake
target_link_libraries(<target name> domain)
```

See the top-level `CMakeLists.txt` for more details.

## Tests
To run the tests, it is advised to set up the CMake output directory in a subfolder of the source root directory, as in
```sh
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
make -j`nproc`
```

All of the tests (with the exception of the MPI test, and tests using the deprecated APIs) can then be run by opening a new shell under the `run_scripts` subdirectory, and running either `runme.sh` or `runme_double.sh`. These will output the final line of each test under a new directory `bgrt_results` in the user's home directory. 

**It should be noted that if `~/bgrt_results` already exists, its contents will be deleted when running these scripts.**
