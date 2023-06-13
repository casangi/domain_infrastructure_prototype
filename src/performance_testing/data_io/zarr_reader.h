#ifndef ZARR_READER_H
#define ZARR_READER_H

#include <fstream>
#include <iostream>
#include <chrono>
#include <nlohmann/json.hpp>
#include <string>
#include <zstd.h>
#include <complex>
using json = nlohmann::json;

template<typename T>
bool decompress_zstd_file(const std::string& filename, T** decompressed_data, size_t& decompressed_size);


void open_no_dask_zarr(std::complex<double>** vis, double** weight, double** uvw, double** chan, long** vis_chunk_shape, const std::string& zarr_name, int time_chunk_id, int chan_chunk_id);
 
#endif
