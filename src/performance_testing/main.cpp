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
#include <zstd.h>
#include <complex>
#include <fstream>
#include <nlohmann/json.hpp>
#include <chrono>
#include <unistd.h>
using json = nlohmann::json;
//template <typename T> string type_name();

using namespace std;

template<typename T>
bool decompress_zstd_file(const string& filename, T** decompressed_data, size_t& decompressed_size)
{
    ifstream file(filename, ios::binary);
    if (!file.is_open())
    {
        cout << "Error opening file: " << filename << endl;
        return false;
    }

    // Get the compressed data size from the file
    file.seekg(0, ios::end);
    size_t compressed_size = file.tellg();
    file.seekg(0, ios::beg);
    if (file.fail())
    {
        cout << "Error reading data size from file: " << filename << endl;
        file.close();
        return false;
    }

    // Allocate memory for the compressed data
    char* compressed_data = new char[compressed_size];

    file.read(compressed_data, compressed_size);
    if (file.fail())
    {
        cout << "Error reading compressed data from file: " << filename << endl;
        delete[] compressed_data;
        file.close();
        return false;
    }

    // Get the decompressed size
    decompressed_size = ZSTD_getFrameContentSize(compressed_data, compressed_size);
    if (decompressed_size == ZSTD_CONTENTSIZE_ERROR)
    {
        cout << "Error getting decompressed size" << endl;
        delete[] compressed_data;
        file.close();
        return false;
    }

    // Allocate memory for the decompressed data
    *decompressed_data = new T[decompressed_size / sizeof(T)];

    // Decompress the data
    size_t result = ZSTD_decompress(*decompressed_data, decompressed_size, compressed_data, compressed_size);

    // Clean up
    delete[] compressed_data;
    file.close();

    if (ZSTD_isError(result))
    {
        cout << "Error decompressing data: " << ZSTD_getErrorName(result) << endl;
        delete[] *decompressed_data;
        return false;
    }

    return true;
}

void open_no_dask_zarr(complex<double>** vis, double** weight,  double** uvw, double** chan, long** vis_chunk_shape, const string& zarr_name, int time_chunk_id, int chan_chunk_id){
    //Read metadata
    //.zarray
    ifstream vis_meta_stream(zarr_name+"/DATA/.zarray", ifstream::in);
    json vis_meta = json::parse(vis_meta_stream);
    
    //cout << vis_meta["chunks"].size() << endl;
    //int vis_chunk_shape[vis_meta["chunks"].size()];
    *vis_chunk_shape = new long[vis_meta["chunks"].size()];
    //cout << vis_meta["chunks"].size() << endl;
    
    int s;
    for(int i=0; i < vis_meta["chunks"].size(); i++){
        long s = static_cast<long>(vis_meta["chunks"][i]);
        (*vis_chunk_shape)[i] = s;
        //cout << "vis_data_meta " << vis_chunk_shape[i] << " ,*, " << typeid(vis_chunk_shape[i]).name() << endl;
    }
    
    //time,baseline,chan,pol
    //cout << zarr_name+"/DATA/"+to_string(time_chunk_id)+".0."+to_string(chan_chunk_id)+".0" << endl;
    
    auto start = chrono::high_resolution_clock::now();
    //complex<double>* vis;
    string filename = zarr_name+"/DATA/"+to_string(time_chunk_id)+".0."+to_string(chan_chunk_id)+".0";
    size_t vis_size;
    decompress_zstd_file(filename, vis, vis_size);
    
    //float* weight;
    filename = zarr_name+"/WEIGHT/"+to_string(time_chunk_id)+".0."+to_string(chan_chunk_id)+".0";
    size_t weight_size;
    decompress_zstd_file(filename, weight, weight_size);
    
    //float* uvw;
    filename = zarr_name+"/UVW/"+to_string(time_chunk_id)+".0.0";
    size_t uvw_size;
    decompress_zstd_file(filename, uvw, uvw_size);
    
    //chan
    filename = zarr_name+"/chan/0";
    //cout << filename << endl;
    size_t chan_size;
    decompress_zstd_file(filename, chan, chan_size);
    
    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    //cout << "Elapsed time " << duration << endl;
    //cout << "The file name is " << filename << endl;
    
    // Print the converted complex values
    //cout << "vis_data_size " << vis_size << ",*," << uvw_size << ",*," << weight_size << ",*," << chan_size << endl;

}

std::pair<int, int> grid(int image_size, int n_time_chunks, int n_chan_chunks)
{
    
    double field_of_view = 60*M_PI/(180*3600);
    int oversampling = 100;
    int support = 7;
    int n_imag_chan = 20;
    int n_imag_pol = 2;

    single_cf_gridder standard_gridder;
    auto cgk_1D = standard_gridder.create_prolate_spheroidal_kernel_1d(oversampling, support);
    vector<double> delta_lm = {field_of_view/static_cast<double>(image_size), field_of_view/static_cast<double>(image_size)};
    
    int data_load_time = 0;
    int gridding_time = 0;


    complex<double>* vis;
    double* weight;
    double* uvw;
    double* chan;
    long* vis_shape;
    
//    int grid_size = n_imag_chan*n_imag_pol*image_size*image_size;
//    std::vector<std::complex<double>> grid;
//    grid.resize(grid_size, std::complex<double>(0.0, 0.0));
//    vector<long> grid_shape = {n_imag_chan, n_imag_pol, image_size, image_size};
//    std::complex<double>* grid_ptr = grid.data();
//
//    std::vector<double> sum_weight;
//    sum_weight.resize(n_imag_chan*n_imag_pol, 0.0);
//    double* sum_weight_ptr = sum_weight.data();
    
    int chan_chunk_size = 2;
    int grid_size = chan_chunk_size*n_chan_chunks*n_imag_pol*image_size*image_size;
    std::vector<std::complex<double>> grid(grid_size,std::complex<double>(0.0, 0.0));
    //std::vector<std::complex<double>> grid;
    //grid.resize(grid_size, std::complex<double>(0.0, 0.0));
    
    
    vector<long> grid_shape = {chan_chunk_size*n_chan_chunks, n_imag_pol, image_size, image_size};
    std::complex<double>* grid_ptr = grid.data();

    //std::vector<double> sum_weight;
    //sum_weight.resize(chan_chunk_size*n_chan_chunks*n_imag_pol, 0.0);
    std::vector<double> sum_weight(chan_chunk_size*n_chan_chunks*n_imag_pol, 0.0);
    double* sum_weight_ptr = sum_weight.data();
    
    
    size_t memory_reserved = sizeof(std::complex<double>) * grid.size();
    std::cout << "Memory reserved: " << memory_reserved/(1024.0*1024.0*1024.0) << " GiB" << std::endl;






    std::chrono::steady_clock::time_point start;
    std::chrono::steady_clock::time_point end;

    std::vector<double> chan_slice;
    std::vector<long> chan_map;
    std::vector<long> pol_map;

    int start_index;
    int end_index;
    int k;

    for (int i_time_chunk = 0; i_time_chunk < n_time_chunks; ++i_time_chunk) {
       for (int i_chan_chunk = 0; i_chan_chunk < n_chan_chunks; ++i_chan_chunk)
       {
           start = chrono::high_resolution_clock::now();
           open_no_dask_zarr(&vis,&weight,&uvw,&chan,&vis_shape,"/Users/jsteeb/Library/CloudStorage/Dropbox/performance_eval/data/ngvla_sim.vis.zarr", i_time_chunk, i_chan_chunk);
           
           memory_reserved = sizeof(std::complex<double>) * vis_shape[0]* vis_shape[1]* vis_shape[2]* vis_shape[3];
           std::cout << "Memory reserved: " << memory_reserved/(1024.0*1024.0*1024.0) << " GiB" << std::endl;

           chan_slice.resize(vis_shape[2]);
           chan_map.resize(vis_shape[2]);
           pol_map.resize(vis_shape[3]);

           start_index = i_chan_chunk * vis_shape[2];
           end_index = (i_chan_chunk + 1) * vis_shape[2];
           k = 0;
           for (long i = start_index; i < end_index; ++i) {
               chan_map[k] = i;
               chan_slice[k] = chan[i];
               k++;
           }

           for (long i = 0; i < vis_shape[3]; ++i) {
                pol_map[i] = i;
           }

           end = chrono::high_resolution_clock::now();
           data_load_time = data_load_time + chrono::duration_cast<chrono::milliseconds>(end - start).count();


           start = chrono::high_resolution_clock::now();
           standard_gridder.add_to_grid(grid_ptr, grid_shape.data(), sum_weight_ptr, vis, vis_shape, uvw, chan, chan_map.data(), pol_map.data(), weight, cgk_1D.data(), delta_lm.data(), support, oversampling);
           end = chrono::high_resolution_clock::now();
           gridding_time = gridding_time + chrono::duration_cast<chrono::milliseconds>(end - start).count();

           delete[] vis;
           delete[] uvw;
           delete[] weight;
           delete[] chan;
           delete[] vis_shape;
       }
    }
    
    cout << "the grid is " << grid[125250] << endl;
    
    cout << "Data load time " << data_load_time << endl;
    cout << "Data gridding time " << gridding_time << endl;
    cout << image_size << ",*," << n_time_chunks << ",*," << n_chan_chunks << endl;
    
    return std::make_pair(data_load_time, gridding_time);
}


int main(int argc, char* argv[])
{
    
    int image_size = std::stoi(argv[1]);
    int n_time_chunks = std::stoi(argv[2]);
    int n_chan_chunks = std::stoi(argv[3]);
    
    auto start = chrono::high_resolution_clock::now();
    auto time_info = grid(image_size, n_time_chunks, n_chan_chunks);
    auto end = chrono::high_resolution_clock::now();
    auto compute_time = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    
    cout << compute_time/1000.0 << " " << time_info.second/1000.0 << " " << time_info.first/1000.0 << endl;
    
    
    
    
    
    
    
////    int image_size = 500;
//    int i_time_chunk = 0;
//    int i_chan_chunk = 0;
//
//    complex<double>* vis;
//    double* weight;
//    double* uvw;
//    double* chan;
//    long* vis_shape;
//
//    auto start = chrono::high_resolution_clock::now();
//    open_no_dask_zarr(&vis,&weight,&uvw,&chan,&vis_shape,"/Users/jsteeb/Library/CloudStorage/Dropbox/performance_eval/data/ngvla_sim.vis.zarr", 0, 0);
//    auto end = chrono::high_resolution_clock::now();
//    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
//    cout << "Elapsed time " << duration << endl;
//
//    //cout << "2. Real: " << vis_shape[0] << ", Imaginary: " << vis_shape[3] << endl;
//    // Example usage
//    double field_of_view = 60*M_PI/(180*3600);
//    int oversampling = 100;
//    int support = 7;
//    int n_imag_chan = 20;
//    int n_imag_pol = 2;
//
//    single_cf_gridder standard_gridder;
//
//
//    auto cgk_1D = standard_gridder.create_prolate_spheroidal_kernel_1d(oversampling, support);
//    vector<double> delta_lm = {field_of_view/static_cast<double>(image_size), field_of_view/static_cast<double>(image_size)};
//    //cout << cgk_1D.size() << ",*," << delta_lm[0] << endl;
//    //cout << chan[0] << ",*," << chan[19] << endl;
//
//   int grid_size = n_imag_chan*n_imag_pol*image_size*image_size;
////   complex<double>* grid[n_imag_chan*n_imag_pol*image_size*image_size];
////   for (int i = 0; i < grid_size; ++i) {
////         (*grid)[i] = std::complex<double>(0.0, 0.0);
////    }
//
//
//    std::vector<std::complex<double>> grid;
//    grid.resize(grid_size, std::complex<double>(0.0, 0.0));
//    vector<long> grid_shape = {n_imag_chan, n_imag_pol, image_size, image_size};
//
////    int sum_weight_size = n_imag_chan*n_imag_pol;
////    double sum_weight[sum_weight_size];
////    for (int i = 0; i < sum_weight_size; ++i) {
////        sum_weight[i] = 0.0;
////     }
//
////    std::vector<std::vector<double>> sum_weight;
////    // Resize the vector to the desired dimensions and initialize with zeros
////    sum_weight.resize(n_imag_chan, std::vector<double>(n_imag_pol, 0.0));
//
//    std::vector<double> sum_weight;
//    // Resize the vector to the desired dimensions and initialize with zeros
//    sum_weight.resize(n_imag_chan*n_imag_pol, 0.0);
//
//
//    std::vector<double> chan_slice;
//    int start_index = i_chan_chunk * vis_shape[2];
//    int end_index = (i_chan_chunk + 1) * vis_shape[2];
//    std::vector<long> chan_map;
//    for (long i = start_index; i < end_index; ++i) {
//        chan_map.push_back(i);
//        chan_slice.push_back(chan[i]);
//    }
//
//    std::vector<long> pol_map(n_imag_pol);
//    for (long i = 0; i < n_imag_pol; ++i) {
//         pol_map[i] = i;
//    }
//
//
//
//    start = chrono::high_resolution_clock::now();
//
//    auto grid_ptr = grid.data();
//    auto sum_weight_ptr = sum_weight.data();
//
//    standard_gridder.add_to_grid(grid_ptr, grid_shape.data(), sum_weight_ptr, vis, vis_shape, uvw, chan, chan_map.data(), pol_map.data(), weight, cgk_1D.data(), delta_lm.data(), support, oversampling);
//    end = chrono::high_resolution_clock::now();
//    duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
//    cout << "2. Elapsed time " << duration << endl;
//
//    delete[] vis;
//    delete[] uvw;
//    delete[] weight;
//    delete[] chan;
//    delete[] vis_shape;


    
    
    
    
    
    
    
    
    

    
    
    return 0;
}

///Users/jsteeb/Library/CloudStorage/Dropbox/performance_eval/data
//gcc -std=c++17 -o bin/grid_example  gridder/single_cf_gridder.cpp main.cpp -L/usr/local/lib -lstdc++

//gcc -std=c++17 -I /Users/jsteeb/mambaforge-pypy3/envs/zinc/include/ -o bin/grid_example  gridder/single_cf_gridder.cpp main.cpp -L/usr/local/lib -lstdc++
//gcc -std=c++17 -I /Users/jsteeb/mambaforge-pypy3/envs/zinc/include/ -o bin/grid_example  gridder/single_cf_gridder.cpp main.cpp -L/usr/local/lib -lstdc++ -lzstd


// conda install zstd zstd-static --channel conda-forge


//mamba install -c conda-forge nlohmann_json
//mamba install -c conda-forge z5py
