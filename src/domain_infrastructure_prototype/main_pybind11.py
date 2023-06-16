#c++ -O3 -Wall -shared -std=c++14 -I /Users/jsteeb/mambaforge-pypy3/envs/zinc11/include/ -I /Users/jsteeb/mambaforge-pypy3/envs/zinc11/include/xtl/ -I /Users/jsteeb/mambaforge-pypy3/envs/zinc11/lib/python3.11/site-packages/numpy/core/include  -undefined dynamic_lookup $(python3 -m pybind11 --includes) gridder/pybind11_wrapper.cpp gridder/single_cf_gridder.cpp -o bin/pybind11_wrapper$(python3-config --extension-suffix)

#Linux
#g++ -O3 -shared -std=c++17 -I /users/jsteeb/mambaforge/envs/zinc/include -fPIC $(python3 -m pybind11 --includes) gridder/pybind11_wrapper.cpp gridder/single_cf_gridder.cpp -o bin/pybind11_wrapper$(python3-config --extension-suffix) -L/usr/local/lib -lstdc++ -lzstd

#g++ -O3 -shared -std=c++17 -I /users/jsteeb/mambaforge/envs/zinc/include -fPIC $(python3 -m pybind11 --includes) gridder/pybind11_wrapper.cpp gridder/single_cf_gridder.cpp -o bin/pybind11_wrapper$(python3-config --extension-suffix) -lzstd

#g++ -O3 -shared -std=c++17 -I /users/jsteeb/mambaforge/envs/zinc/include -fPIC $(python3 -m pybind11 --includes) gridder/pybind11_wrapper.cpp gridder/single_cf_gridder.cpp data_io/zarr_reader.cpp -o bin/pybind11_wrapper$(python3-config --extension-suffix) -lzstd


#g++ -O3 -shared -std=c++17 -I /users/jsteeb/mambaforge/envs/zinc/include -fPIC $(python3 -m pybind11 --includes) gridder/pybind11_wrapper.cpp gridder/single_cf_gridder.cpp data_io/zarr_reader.cpp -o bin/pybind11_wrapper$(python3-config --extension-suffix) -lzstd

#g++ -O3 -shared -std=c++17 -I /mnt/condor/jsteeb/dip/dip_3_11/include -fPIC $(python3 -m pybind11 --includes) gridder/pybind11_wrapper.cpp gridder/single_cf_gridder.cpp data_io/zarr_reader.cpp -o bin/pybind11_wrapper$(python3-config --extension-suffix) -lzstd

import os
import linecache
import numpy as np
import time
from gridder.single_cf_gridder_numba import _standard_grid_jit, _create_prolate_spheroidal_kernel, _create_prolate_spheroidal_kernel_1D
from data_io.zarr_reader import _open_no_dask_zarr
import bin.pybind11_wrapper as py11
import matplotlib.pyplot as plt

def grid(n_time_chunks,n_chan_chunks,image_size,set_grid):

    field_of_view = 60*np.pi/(180*3600) #radians
    n_imag_pol = 2
    
    time_chunk_size = 600
    chan_chunk_size = 2
    
    data_load_time = 0
    gridding_time = 0
#    grid = np.zeros((n_imag_chan, n_imag_pol, image_size, image_size), dtype=np.complex128)
#    sum_weight = np.zeros((n_imag_chan, n_imag_pol), dtype=np.double)
    
    oversampling = 100
    support = 7
    cgk, correcting_cgk_image = _create_prolate_spheroidal_kernel(oversampling, support, np.array([image_size,image_size]))
    cgk_1D = _create_prolate_spheroidal_kernel_1D(oversampling, support)
    delta_lm = np.array([field_of_view,field_of_view])/image_size
    gridder = py11.single_cf_gridder_pybind()
    
    if set_grid == 'true':
        #grid = np.zeros((n_chan_chunks*chan_chunk_size, n_imag_pol, image_size, image_size), dtype=np.complex128)
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
            #vis_ds = _open_no_dask_zarr('/Users/jsteeb/Dropbox/performance_eval/data/ngvla_sim.vis.zarr',slice_dict={'time':time_slice,'chan':chan_slice},var_select=var_select)
            vis_ds = _open_no_dask_zarr('/mnt/condor/jsteeb/dip/ngvla_sim.vis.zarr',slice_dict={'time':time_slice,'chan':chan_slice},var_select=var_select)
 

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
            #print('chan_map',chan_map,freq_chan)
            #print('pol_map',pol_map)


            #complex128 int64 float64 complex128 int64 float64      int64    int64 int64 float64 float64 float64 <class 'int'> <class 'int'>
            #complex128, int64, float64, complex128, int64,float64, float64,   int64, int64, float64, float64, float64, int, int)

            #print(grid.dtype,grid_shape.dtype,sum_weight.dtype,vis_data.dtype, vis_shape.dtype,uvw.dtype, freq_chan.dtype, chan_map.dtype, pol_map.dtype, weight.dtype, cgk_1D.dtype, delta_lm.dtype, type(support), type(oversampling))
            start = time.time()
            #gridder.grid_vis_data(grid,grid_shape,sum_weight,vis_data,vis_shape,uvw, freq_chan, chan_map, pol_map, weight, cgk_1D, delta_lm, support, oversampling)
            gridder.add_to_grid(grid_shape,sum_weight,vis_data,vis_shape,uvw, freq_chan, chan_map, pol_map, weight, cgk_1D, delta_lm, support, oversampling)
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

#    def to_flat_index(index_4d):
#        return index_4d[0]*5000*5000*2 + index_4d[1]*5000*5000 + index_4d[2]*5000 + index_4d[2];
#
#    pixel_list = [[0,0,2500,2500],[0,1,2500,2500],[5,0,2500,2500],[5,1,2500,2500],[10,0,2500,2500],[10,1,2500,2500],[19,0,2500,2500],[19,1,2500,2500]];
#    print(grid[0,0,2500,2500],grid[0,1,2500,2500],grid[5,0,2500,2500],grid[5,1,2500,2500],grid[10,0,2500,2500],grid[10,1,2500,2500],grid[19,0,2500,2500],grid[19,1,2500,2500])
#
#    print(sum_weight)
#    print(list(map(to_flat_index,pixel_list)))
    
    #numpy.ravel_multi_index(multi_index=[10,1,2500,2500], dims=[20,2,5000,5000], order='C')

    #[12502500, 37502500, 262502500, 287502500, 512502500, 537502500, 962502500, 987502500]

# Grid values (-0.10697631072328591+0.0578030094901381j) (-0.10697631072328591+0.0578030094901381j) (0.0002800174907041966+0.0009681689039663092j) (0.0002800174907041966+0.0009681689039663092j) 0j 0j 0j 0j

#chan_map [18 19] [3.873e+11 4.073e+11]
# pol_map [0 1]

# sum of weight
#    [[55182225.01427166 55182225.01427166]
# [51649585.6556574  51649585.6556574 ]
# [49453205.17748479 49453205.17748479]
# [48270115.14828864 48270115.14828864]
# [47397977.41040695 47397977.41040695]
# [46458583.60066321 46458583.60066321]
# [45701915.42228861 45701915.42228861]
# [44566976.31561698 44566976.31561698]
# [43489558.52853308 43489558.52853308]
# [42551810.30028976 42551810.30028976]
# [41324663.38560001 41324663.38560001]
# [40295792.13948147 40295792.13948147]
# [39643588.16421303 39643588.16421303]
# [39039302.56585391 39039302.56585391]
# [38310098.18830668 38310098.18830668]
# [37694651.74131018 37694651.74131018]
# [37032157.12556891 37032157.12556891]
# [36192355.67040902 36192355.67040902]
# [35503173.49673673 35503173.49673673]
# [34786737.28330411 34786737.28330411]]

    return data_load_time, gridding_time


          


def main(image_size,n_time_chunks,n_chan_chunks,set_grid):
    start = time.time()
    data_load_time, gridding_time = grid(n_time_chunks,n_chan_chunks,image_size,set_grid)
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
    parser.add_argument("--set_grid", type=str, nargs=1, help=".")

    # Parse the command line arguments
    args = parser.parse_args()

    # Access the image size parameter
    image_size = args.image_size[0]
    n_time_chunks = args.n_time_chunks[0]
    n_chan_chunks = args.n_chan_chunks[0]
    set_grid = args.set_grid[0]
    
    main(image_size,n_time_chunks,n_chan_chunks,set_grid)




'''Build:
c++ -O3 -Wall -shared -std=c++14 -I /Users/jsteeb/mambaforge-pypy3/envs/zinc11/include/ -I /Users/jsteeb/mambaforge-pypy3/envs/zinc11/include/xtl/ -I /Users/jsteeb/mambaforge-pypy3/envs/zinc11/lib/python3.11/site-packages/numpy/core/include  -undefined dynamic_lookup $(python3 -m pybind11 --includes) gridder/pybind11_wrapper.cpp gridder/single_cf_gridder.cpp -o bin/pybind11_wrapper$(python3-config --extension-suffix)

Don't need -I if ' pip install "pybind11[global]" ' or ' mamba install pybind11 '

c++ -O3 -Wall -shared -std=c++14 -undefined dynamic_lookup $(python3 -m pybind11 --includes) gridder/pybind11_wrapper.cpp gridder/single_cf_gridder.cpp -o bin/pybind11_wrapper$(python3-config --extension-suffix)

'''
'''Run
#python main_pybind11.py --image_size 500 --n_time_chunks 1 --n_chan_chunks 1

'''

























