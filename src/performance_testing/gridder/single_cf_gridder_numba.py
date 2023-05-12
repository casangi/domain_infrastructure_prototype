
import tracemalloc
import os
import linecache
import numpy as np


def main():

    import pybind11_wrapper
    gridder = pybind11_wrapper.single_cf_gridder()
    
    b = gridder.sum(4,6)
    
    print(b)
    
    
    

if __name__=="__main__":
    main()



'''
// linux: c++ -O3 -Wall -shared -std=c++11 -fPIC $(python3 -m pybind11 --includes) example.cpp -o example$(python3-config --extension-suffix)
// mac:   c++ -O3 -Wall -shared -std=c++11 -undefined dynamic_lookup $(python3 -m pybind11 --includes) example.cpp -o example$(python3-config --extension-suffix)

//Users/jsteeb/mambaforge-pypy3/envs/zinc/include
//c++ -O3 -Wall -shared -std=c++11 -I /Users/jsteeb/mambaforge-pypy3/envs/zinc/include/ -I /Users/jsteeb/mambaforge-pypy3/envs/zinc/include/xtl/  -undefined dynamic_lookup $(python3 -m pybind11 --includes) pybind11_proto.cpp -o pybind11_proto$(python3-config --extension-suffix)

//gcc -O3 -Wall -shared -std=c++14 -I /Users/jsteeb/mambaforge-pypy3/envs/zinc/include/ -I /Users/jsteeb/mambaforge-pypy3/envs/zinc/include/xtl/ -I /Users/jsteeb/mambaforge-pypy3/envs/zinc/lib/python3.8/site-packages/numpy/core/include  -undefined dynamic_lookup $(python3 -m pybind11 --includes) pybind11_proto.cpp -o pybind11_proto$(python3-config --extension-suffix)


// usr/local/Cellar/fftw/3.3.10_1/include

//c++ -O3 -Wall -shared -std=c++14 -I /Users/jsteeb/mambaforge-pypy3/envs/zinc/include/ -I /Users/jsteeb/mambaforge-pypy3/envs/zinc/include/xtl/ -I /Users/jsteeb/mambaforge-pypy3/envs/zinc/lib/python3.8/site-packages/numpy/core/include  -undefined dynamic_lookup $(python3 -m pybind11 --includes) pybind11_proto.cpp -o pybind11_proto$(python3-config --extension-suffix)
'''
