import os
import linecache
import numpy as np
import time
from gridder.single_cf_gridder_numba import _standard_grid_jit, _create_prolate_spheroidal_kernel, _create_prolate_spheroidal_kernel_1D
from data_io.zarr_reader import _open_no_dask_zarr
import matplotlib.pyplot as plt


def grid(vis_data_folder,n_time_chunks,n_chan_chunks,image_size):

    field_of_view = 60*np.pi/(180*3600) #radians
    n_imag_chan = 20
    n_imag_pol = 2
    
    time_chunk_size = 600
    chan_chunk_size = 2
    
    data_load_time = 0
    gridding_time = 0
    grid = np.full((n_chan_chunks*chan_chunk_size, n_imag_pol, image_size, image_size), np.complex128(0.0), dtype=np.complex128)
    
    grid_shape = np.array(grid.shape)
    sum_weight = np.zeros((n_chan_chunks*chan_chunk_size, n_imag_pol), dtype=np.double)
    
    oversampling = 100
    support = 7
    cgk, correcting_cgk_image = _create_prolate_spheroidal_kernel(oversampling, support, np.array([image_size,image_size]))
    cgk_1D = _create_prolate_spheroidal_kernel_1D(oversampling, support)
    delta_lm = np.array([field_of_view,field_of_view])/image_size
    
    for i_time_chunk in range(n_time_chunks):
        for i_chan_chunk in range(n_chan_chunks):
            time_slice = slice(i_time_chunk*time_chunk_size,(i_time_chunk+1)*time_chunk_size)
            chan_slice = slice(i_chan_chunk*chan_chunk_size,(i_chan_chunk+1)*chan_chunk_size)
            #print('Gridding chunk: ', time_slice, ',*,', chan_slice)
            
            start = time.time()
            var_select = ['DATA','UVW','WEIGHT']

            vis_ds = _open_no_dask_zarr(vis_data_folder,slice_dict={'time':time_slice,'chan':chan_slice},var_select=var_select)

            vis_data = vis_ds.DATA.values
            vis_data_shape = np.array(vis_data.shape)
            uvw = vis_ds.UVW.values
            freq_chan = vis_ds.chan.values
            chan_map = np.arange(i_chan_chunk*chan_chunk_size,(i_chan_chunk+1)*chan_chunk_size)
            
            pol_map = np.arange(n_imag_pol)
            weight = vis_ds.WEIGHT.values
            data_load_time = data_load_time + (time.time() - start)

            start = time.time()
            _standard_grid_jit(grid=grid,grid_shape=grid_shape, sum_weight=sum_weight, vis_data=vis_data, vis_shape=vis_data_shape, uvw=uvw, freq_chan=freq_chan, chan_map=chan_map, pol_map=pol_map, weight=weight, cgk_1D=cgk_1D, delta_lm=delta_lm, support=support, oversampling=oversampling)
            gridding_time = gridding_time + (time.time()-start)
            
            del uvw
            del weight
            del vis_data
            del vis_ds
            
    #print('grid.shape',grid.shape,grid.dtype,grid.nbytes/(1024*1024*1024))
    return data_load_time, gridding_time


def main(vis_data_folder,image_size,n_time_chunks,n_chan_chunks):
    start = time.time()
    data_load_time, gridding_time = grid(vis_data_folder,n_time_chunks,n_chan_chunks,image_size)
    compute_time = time.time() - start
    print(compute_time, gridding_time, data_load_time)

    return

import argparse

if __name__=="__main__":
   # Create an argument parser to handle the image size parameter
    parser = argparse.ArgumentParser()
    parser.add_argument("--vis_data_folder", type=int, nargs=1, help="Visibility data.")
    parser.add_argument("--image_size", type=int, nargs=1, help="Size of the image.")
    parser.add_argument("--n_time_chunks", type=int, nargs=1, help="Number of time chunks.")
    parser.add_argument("--n_chan_chunks", type=int, nargs=1, help="Number of chan chunks.")

    # Parse the command line arguments
    args = parser.parse_args()

    # Access the image size parameter
    vis_data_folder = args.vis_data[0]
    image_size = args.image_size[0]
    n_time_chunks = args.n_time_chunks[0]
    n_chan_chunks = args.n_chan_chunks[0]
    
    main(vis_data_folder,image_size,n_time_chunks,n_chan_chunks)
