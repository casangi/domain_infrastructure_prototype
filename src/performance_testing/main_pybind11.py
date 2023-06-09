import os
import linecache
import numpy as np
import time
from gridder.single_cf_gridder_numba import _standard_grid_jit, _create_prolate_spheroidal_kernel, _create_prolate_spheroidal_kernel_1D
from data_io.zarr_reader import _open_no_dask_zarr
import bin.pybind11_wrapper as py11
import matplotlib.pyplot as plt

def grid(n_time_chunks,n_chan_chunks,image_size):

    field_of_view = 60*np.pi/(180*3600) #radians
    n_imag_chan = 20
    n_imag_pol = 2
    
    time_chunk_size = 600
    chan_chunk_size = 2
    
    data_load_time = 0
    gridding_time = 0
#    grid = np.zeros((n_imag_chan, n_imag_pol, image_size, image_size), dtype=np.complex128)
#    sum_weight = np.zeros((n_imag_chan, n_imag_pol), dtype=np.double)
    
    grid = np.zeros((n_chan_chunks*chan_chunk_size, n_imag_pol, image_size, image_size), dtype=np.complex128)
    grid_shape = np.array(grid.shape)
    sum_weight = np.zeros((n_chan_chunks*chan_chunk_size, n_imag_pol), dtype=np.double)
    
    oversampling = 100
    support = 7
    cgk, correcting_cgk_image = _create_prolate_spheroidal_kernel(oversampling, support, np.array([image_size,image_size]))
    cgk_1D = _create_prolate_spheroidal_kernel_1D(oversampling, support)
    delta_lm = np.array([field_of_view,field_of_view])/image_size
    gridder = py11.single_cf_gridder_pybind()

    for i_time_chunk in range(n_time_chunks):
        for i_chan_chunk in range(n_chan_chunks):
            time_slice = slice(i_time_chunk*time_chunk_size,(i_time_chunk+1)*time_chunk_size)
            chan_slice = slice(i_chan_chunk*chan_chunk_size,(i_chan_chunk+1)*chan_chunk_size)
            #print('Gridding chunk: ', time_slice, ',*,', chan_slice)
            
            start = time.time()
            var_select = ['DATA','UVW','WEIGHT']
            vis_ds = _open_no_dask_zarr('/Users/jsteeb/Dropbox/performance_eval/data/ngvla_sim.vis.zarr',slice_dict={'time':time_slice,'chan':chan_slice},var_select=var_select)

            vis_data = vis_ds.DATA.values
            uvw = vis_ds.UVW.values
            freq_chan = vis_ds.chan.values
            chan_map = np.arange(i_chan_chunk*chan_chunk_size,(i_chan_chunk+1)*chan_chunk_size)
            #print(chan_map)
            pol_map = np.arange(n_imag_pol)
            weight = vis_ds.WEIGHT.values
            vis_shape =  np.array(vis_data.shape)
            data_load_time = data_load_time + (time.time() - start)
            
            #gridder.create_array(40000000)



            #complex128 int64 float64 complex128 int64 float64      int64    int64 int64 float64 float64 float64 <class 'int'> <class 'int'>
            #complex128, int64, float64, complex128, int64,float64, float64,   int64, int64, float64, float64, float64, int, int)

            #print(grid.dtype,grid_shape.dtype,sum_weight.dtype,vis_data.dtype, vis_shape.dtype,uvw.dtype, freq_chan.dtype, chan_map.dtype, pol_map.dtype, weight.dtype, cgk_1D.dtype, delta_lm.dtype, type(support), type(oversampling))
            start = time.time()
            gridder.grid_vis_data(grid,grid_shape,sum_weight,vis_data,vis_shape,uvw, freq_chan, chan_map, pol_map, weight, cgk_1D, delta_lm, support, oversampling)
            gridding_time = gridding_time + (time.time() -  start)
            
            #Stops memory spikes
            del uvw
            del weight
            del vis_data
            del vis_ds
            
    
#    plt.figure()
#    plt.imshow(np.abs(grid[0,0,:,:]))
#    plt.colorbar()
#    plt.show()
    return data_load_time, gridding_time


          


def main(image_size,n_time_chunks,n_chan_chunks):
    start = time.time()
    data_load_time, gridding_time = grid(n_time_chunks,n_chan_chunks,image_size)
    compute_time = time.time() - start
    print(compute_time, gridding_time, data_load_time)

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

























