#include <iostream>
using namespace std;
#include "gridder/single_cf_gridder.h"
 
int main()
{
    single_cf_gridder standard_gridder;
    cout << "Hallo " << standard_gridder.sum(1,2) << endl;
    
    double array[] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0};
    standard_gridder.increment_array(array);
    for (int i = 0; i < sizeof(array) / sizeof(array[0]); i++) {
      std::cout << array[i] << std::endl;
    }
    
    return 0;
}

//gcc -std=c++17 -I /Users/jsteeb/mambaforge-pypy3/envs/zinc/include/ -I /Users/jsteeb/mambaforge-pypy3/envs/zinc/include/xtl/ -I /Users/jsteeb/mambaforge-pypy3/envs/zinc/lib/python3.8/site-packages/numpy/core/include  -undefined dynamic_lookup $(python3 -m pybind11 --includes) -o grid_example  gridder/single_cf_gridder.cpp main.cpp -L/usr/local/lib -lstdc++


//gcc -std=c++17 -o grid_example  gridder/single_cf_gridder.cpp main.cpp -L/usr/local/lib -lstdc++
