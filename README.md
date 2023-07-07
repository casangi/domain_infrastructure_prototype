# Objectives 
- To determine the suitability of Python as the language of the domain layer that controls:
  - Data access and iteration (visibility data).
  - Management of in-memory data structures (image grid).
- It is accepted that Python will not be suitable for all processing (for example gridding), consequently the memory and compute overheads must be measured when allowing access to Python controlled memory in C++. 

# Results
https://docs.google.com/presentation/d/1T7TmovYOi1zO8SUA2nR_Ce5aUiT7ENbK4KITfnbWm7M/edit?usp=sharing

# Create a conda python enviroment using dip_env.yml

# Compiling shared library on Linux 

- Go to ```domain_infrastructure_prototype/src/domain_infrastructure_prototype```
- ```g++ -O3  -fno-semantic-interposition -Wall -Wextra -shared -std=c++17 -I/mnt/condor/jsteeb/dip/dip_3_11/include -fpic /$(python3 -m pybind11 --includes) gridder/pybind11_wrapper.cpp gridder/single_cf_gridder.cpp data_io/zarr_reader.cpp -o lib/pybind11_wrapper$(python3-config --extension-suffix) -lzstd```

Remeber to change ```-I/mnt/condor/jsteeb/dip/dip_3_11/include```

# Compiling binary on Linux 

- Go to ```domain_infrastructure_prototype/src/domain_infrastructure_prototype```
- ```g++ -O3 -std=c++17  -I/mnt/condor/jsteeb/dip/dip_3_11/include main.cpp gridder/single_cf_gridder.cpp data_io/zarr_reader.cpp -o bin/cpp_gridder -lzstd``` 

# Examples of running the code

- ```python main_numba.py  --vis_data_folder /mnt/condor/jsteeb/dip/ngvla_sim.vis.zarr --image_size 1000 --n_time_chunks 3 --n_chan_chunks 1```
-``` python main_pybind11.py  --vis_data_folder /mnt/condor/jsteeb/dip/ngvla_sim.vis.zarr --image_size 1000 --n_time_chunks 3 --n_chan_chunks 1 --set_grid true```
- ```python main_pybind11.py  --vis_data_folder /mnt/condor/jsteeb/dip/ngvla_sim.vis.zarr --image_size 1000 --n_time_chunks 3 --n_chan_chunks 1 --set_grid false```
- ```python main_pybind11_cpp_only.py  --vis_data_folder /mnt/condor/jsteeb/dip/ngvla_sim.vis.zarr --image_size 1000 --n_time_chunks 3 --n_chan_chunks 1```
- ```./bin/cpp_gridder /mnt/condor/jsteeb/dip/ngvla_sim.vis.zarr 1000   3   1```
  
# Valgrind
To use valgrind add -g compiler flag.

Example of running valgrind:
- ```valgrind --tool=callgrind --simulate-cache=yes python main_pybind11_cpp_only.py --vis_data_folder /mnt/condor/jsteeb/dip/ngvla_sim.vis.zarr --image_size 5000 --n_time_chunks 1 --n_chan_chunks 3```
callgrind_annotate --auto=yes callgrind.out.<pid> > profile.txt 

# -fPIC info:
- https://stackoverflow.com/questions/15861759/how-much-overhead-can-the-fpic-flag-add
- https://akkadia.org/drepper/dsohowto.pdf
- https://cseweb.ucsd.edu/~gbournou/CSE131/the_inside_story_on_shared_libraries_and_dynamic_loading.pdf

