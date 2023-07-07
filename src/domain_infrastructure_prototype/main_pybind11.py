import os
import linecache
import numpy as np
import time
from gridder.single_cf_gridder_numba import _standard_grid_jit, _create_prolate_spheroidal_kernel, _create_prolate_spheroidal_kernel_1D
from data_io.zarr_reader import _open_no_dask_zarr
import bin.pybind11_wrapper as py11
import matplotlib.pyplot as plt

def grid(vis_data_folder,n_time_chunks,n_chan_chunks,image_size,set_grid):

    field_of_view = 60*np.pi/(180*3600) #radians
    n_imag_pol = 2
    
    time_chunk_size = 600
    chan_chunk_size = 2
    
    data_load_time = 0
    gridding_time = 0
    
    oversampling = 100
    support = 7
    cgk, correcting_cgk_image = _create_prolate_spheroidal_kernel(oversampling, support, np.array([image_size,image_size]))
    cgk_1D = _create_prolate_spheroidal_kernel_1D(oversampling, support)
    delta_lm = np.array([field_of_view,field_of_view])/image_size
    gridder = py11.single_cf_gridder_pybind()
    
    if set_grid == 'true':
        grid = np.full((n_chan_chunks*chan_chunk_size, n_imag_pol, image_size, image_size), np.complex128(0.0), dtype=np.complex128)
        gridder.set_grid(grid)
        print('set grid')
    else:
        gridder.create_grid(n_chan_chunks*chan_chunk_size, n_imag_pol, image_size)
        print('grid created')
    
    
    grid_shape = np.array([n_chan_chunks*chan_chunk_size, n_imag_pol, image_size, image_size])
    sum_weight = np.zeros((n_chan_chunks*chan_chunk_size, n_imag_pol), dtype=np.double)
    
    for i_time_chunk in range(n_time_chunks):
        for i_chan_chunk in range(n_chan_chunks):
            time_slice = slice(i_time_chunk*time_chunk_size,(i_time_chunk+1)*time_chunk_size)
            chan_slice = slice(i_chan_chunk*chan_chunk_size,(i_chan_chunk+1)*chan_chunk_size)
            #print('Gridding chunk: ', time_slice, ',*,', chan_slice)
            
            start = time.time()
            var_select = ['DATA','UVW','WEIGHT']
            vis_ds = _open_no_dask_zarr(vis_data_folder,slice_dict={'time':time_slice,'chan':chan_slice},var_select=var_select)
 

            vis_data = vis_ds.DATA.values
            uvw = vis_ds.UVW.values
            freq_chan = vis_ds.chan.values
            chan_map = np.arange(i_chan_chunk*chan_chunk_size,(i_chan_chunk+1)*chan_chunk_size)
            #print(chan_map)
            pol_map = np.arange(n_imag_pol)
            weight = vis_ds.WEIGHT.values
            vis_shape =  np.array(vis_data.shape)
            data_load_time = data_load_time + (time.time() - start)
        
            start = time.time()
            gridder.add_to_grid(grid_shape,sum_weight,vis_data,vis_shape,uvw, freq_chan, chan_map, pol_map, weight, cgk_1D, delta_lm, support, oversampling)
            gridding_time = gridding_time + (time.time() -  start)
              
            del uvw
            del weight
            del vis_data
            del vis_ds

    return data_load_time, gridding_time


          


def main(vis_data_folder,image_size,n_time_chunks,n_chan_chunks,set_grid):
    start = time.time()
    data_load_time, gridding_time = grid(n_time_chunks,n_chan_chunks,image_size,set_grid)
    compute_time = time.time() - start
    print(vis_data_folder,compute_time, gridding_time, data_load_time)

    return

import argparse

if __name__=="__main__":
   # Create an argument parser to handle the image size parameter
    parser = argparse.ArgumentParser()
    parser.add_argument("--vis_data_folder", type=int, nargs=1, help="Visibility data.")
    parser.add_argument("--image_size", type=int, nargs=1, help="Size of the image.")
    parser.add_argument("--n_time_chunks", type=int, nargs=1, help="Number of time chunks.")
    parser.add_argument("--n_chan_chunks", type=int, nargs=1, help="Number of chan chunks.")
    parser.add_argument("--set_grid", type=str, nargs=1, help=".")

    # Parse the command line arguments
    args = parser.parse_args()

    # Access the image size parameter
    image_size = args.image_size[0]
    n_time_chunks = args.n_time_chunks[0]
    n_chan_chunks = args.n_chan_chunks[0]
    set_grid = args.set_grid[0]
    
    main(vis_data_folder,image_size,n_time_chunks,n_chan_chunks,set_grid)











