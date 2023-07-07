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
#include <string>

using namespace std;

int main(int argc, char* argv[])
{
    std::string vis_data_folder = argv[1];
    int image_size = std::stoi(argv[2]);
    int n_time_chunks = std::stoi(argv[3]);
    int n_chan_chunks = std::stoi(argv[4]);
    
    auto start = chrono::high_resolution_clock::now();
    single_cf_gridder standard_gridder;
    auto time_info = standard_gridder.grid(vis_data_folder,image_size, n_time_chunks, n_chan_chunks);
    auto end = chrono::high_resolution_clock::now();
    auto compute_time = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    
    cout << compute_time/1000.0 << " " << time_info.second/1000.0 << " " << time_info.first/1000.0 << endl;

    return 0;
}
