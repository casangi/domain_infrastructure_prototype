//Linux
//gcc -O3 -std=c++17 -I /mnt/condor/jsteeb/dip/dip_3_11/include -o bin/cpp_gridder  gridder/single_cf_gridder.cpp main.cpp data_io/zarr_reader.cpp  -L/usr/local/lib -lstdc++ -lzstd -lm

//gcc -O3 -std=c++17 -I /users/jsteeb/mambaforge/envs/zinc/include -o bin/cpp_gridder  gridder/single_cf_gridder.cpp main.cpp data_io/zarr_reader.cpp  -L/usr/local/lib -lstdc++ -lzstd -lm

//Mac
//gcc -O3 -std=c++17 -I /Users/jsteeb/mambaforge-pypy3/envs/zinc/include/ -o bin/cpp_gridder  gridder/single_cf_gridder.cpp main.cpp -L/usr/local/lib -lstdc++ -lzstd

//./bin/cpp_gridder 500 1 2

//gcc     -std=c++17 -I /Users/jsteeb/mambaforge-pypy3/envs/zinc/include/ -o bin/cpp_gridder gridder/single_cf_gridder.cpp main.cpp -L/usr/local/lib -lstdc++ -lzstd
// Add debug symbols -g
//gcc -O3 -g -std=c++17 -I /Users/jsteeb/mambaforge-pypy3/envs/zinc/include/ -o bin/cpp_gridder  gridder/single_cf_gridder.cpp main.cpp -L/usr/local/lib -lstdc++ -lzstd

//gcc -O0 -g -std=c++17 -I /Users/jsteeb/mambaforge-pypy3/envs/zinc/include/ -o bin/cpp_gridder  gridder/single_cf_gridder.cpp main.cpp -L/usr/local/lib -lstdc++ -lzstd
#define _GLIBCXX_USE_NANOSLEEP
#include <iostream>
using namespace std;
#include "gridder/single_cf_gridder.h"
#include <fstream>
#include <vector>
#include <complex>
#include <fstream>
#include <chrono>
#include <unistd.h>

using namespace std;

int main(int argc, char* argv[])
{
    
    int image_size = std::stoi(argv[1]);
    int n_time_chunks = std::stoi(argv[2]);
    int n_chan_chunks = std::stoi(argv[3]);
    
    auto start = chrono::high_resolution_clock::now();
    single_cf_gridder standard_gridder;
    auto time_info = standard_gridder.grid(image_size, n_time_chunks, n_chan_chunks);
    auto end = chrono::high_resolution_clock::now();
    auto compute_time = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    
    cout << compute_time/1000.0 << " " << time_info.second/1000.0 << " " << time_info.first/1000.0 << endl;

    return 0;
}
