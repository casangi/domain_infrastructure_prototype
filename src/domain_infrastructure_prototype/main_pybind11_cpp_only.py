#Linux
#g++ -O3 -shared -std=c++17 -I /users/jsteeb/mambaforge/envs/zinc/include -fPIC $(python3 -m pybind11 --includes) gridder/pybind11_wrapper.cpp gridder/single_cf_gridder.cpp data_io/zarr_reader.cpp -o bin/pybind11_wrapper$(python3-config --extension-suffix) -lzstd

import os
import linecache
import numpy as np
import time
from gridder.single_cf_gridder_numba import _standard_grid_jit, _create_prolate_spheroidal_kernel, _create_prolate_spheroidal_kernel_1D
from data_io.zarr_reader import _open_no_dask_zarr
import bin.pybind11_wrapper as py11
import matplotlib.pyplot as plt

def main(image_size,n_time_chunks,n_chan_chunks):
    start = time.time()
    gridder = py11.single_cf_gridder_pybind()
    data_load_time, gridding_time = gridder.grid(image_size, n_time_chunks, n_chan_chunks)
    compute_time = time.time() - start
    print(compute_time, gridding_time/1000, data_load_time/1000)

    return

import argparse

if __name__=="__main__":
   # Create an argument parser to handle the image size parameter
    parser = argparse.ArgumentParser()
    parser.add_argument("--image_size", type=int, nargs=1, help="Size of the image.")
    parser.add_argument("--n_time_chunks", type=int, nargs=1, help="Number of time chunks.")
    parser.add_argument("--n_chan_chunks", type=int, nargs=1, help="Number of chan chunks.")

    # Parse the command line arguments
    args = parser.parse_args()

    # Access the image size parameter
    image_size = args.image_size[0]
    n_time_chunks = args.n_time_chunks[0]
    n_chan_chunks = args.n_chan_chunks[0]
    
    main(image_size,n_time_chunks,n_chan_chunks)




'''Build:
c++ -O3 -Wall -shared -std=c++14 -I /Users/jsteeb/mambaforge-pypy3/envs/zinc11/include/ -I /Users/jsteeb/mambaforge-pypy3/envs/zinc11/include/xtl/ -I /Users/jsteeb/mambaforge-pypy3/envs/zinc11/lib/python3.11/site-packages/numpy/core/include  -undefined dynamic_lookup $(python3 -m pybind11 --includes) gridder/pybind11_wrapper.cpp gridder/single_cf_gridder.cpp -o bin/pybind11_wrapper$(python3-config --extension-suffix)

Don't need -I if ' pip install "pybind11[global]" ' or ' mamba install pybind11 '

c++ -O3 -Wall -shared -std=c++14 -undefined dynamic_lookup $(python3 -m pybind11 --includes) gridder/pybind11_wrapper.cpp gridder/single_cf_gridder.cpp -o bin/pybind11_wrapper$(python3-config --extension-suffix)

'''
'''Run
#python main_pybind11.py --image_size 500 --n_time_chunks 1 --n_chan_chunks 1

'''


























