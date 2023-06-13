#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include <nlohmann/json.hpp>
#define FORCE_IMPORT_ARRAY
namespace py = pybind11;
#include <iostream>
using namespace std;
using namespace pybind11::literals;
#include "single_cf_gridder.h"
#include <random>
#include <chrono>

class single_cf_gridder_pybind : private single_cf_gridder {
public:
  single_cf_gridder_pybind() : single_cf_gridder() {}
    
  void create_grid(long n_imag_chan, long n_imag_pol, long image_size){
      single_cf_gridder::create_grid(n_imag_chan, n_imag_pol, image_size);
    }

    using complex = std::complex<double>;
    using ComplexArr = py::array_t<complex>;
    using DoubleArr  = py::array_t<double>;
    using LongArr = py::array_t<long>;
    
//    void add_to_grid(ComplexArr& grid, LongArr& grid_shape, DoubleArr& sum_weight, ComplexArr& vis_data, LongArr& vis_shape, DoubleArr& uvw, DoubleArr& freq_chan, LongArr& chan_map, LongArr& pol_map, DoubleArr& weight, DoubleArr& cgk_1D, DoubleArr& delta_lm, int support, int oversampling ){
//        //https://pybind11.readthedocs.io/en/stable/advanced/pycpp/numpy.html?highlight=array#direct-access
        
    void add_to_grid(LongArr& grid_shape, DoubleArr& sum_weight, ComplexArr& vis_data, LongArr& vis_shape, DoubleArr& uvw, DoubleArr& freq_chan, LongArr& chan_map, LongArr& pol_map, DoubleArr& weight, DoubleArr& cgk_1D, DoubleArr& delta_lm, int support, int oversampling ){
            //https://pybind11.readthedocs.io/en/stable/advanced/pycpp/numpy.html?highlight=array#direct-access

        auto start = chrono::high_resolution_clock::now();
        
        //auto grid_ptr = reinterpret_cast<complex*>(grid.mutable_unchecked<4>().mutable_data(0,0,0,0)); // Pointer to first element
        auto grid_shape_ptr = reinterpret_cast<long*>(grid_shape.mutable_unchecked<1>().mutable_data(0));
        auto sum_weight_ptr = reinterpret_cast<double*>(sum_weight.mutable_unchecked<2>().mutable_data(0,0));
        auto vis_data_ptr = reinterpret_cast<complex*>(vis_data.mutable_unchecked<4>().mutable_data(0, 0, 0, 0));
        auto vis_shape_ptr = reinterpret_cast<long*>(vis_shape.mutable_unchecked<1>().mutable_data(0));
        auto uvw_ptr = reinterpret_cast<double*>(uvw.mutable_unchecked<3>().mutable_data(0,0,0));
        auto freq_chan_ptr = reinterpret_cast<double*>(freq_chan.mutable_unchecked<1>().mutable_data(0));
        auto chan_map_ptr = reinterpret_cast<long*>(chan_map.mutable_unchecked<1>().mutable_data(0));
        auto pol_map_ptr = reinterpret_cast<long*>(pol_map.mutable_unchecked<1>().mutable_data(0));
        auto weight_ptr = reinterpret_cast<double*>(weight.mutable_unchecked<4>().mutable_data(0, 0, 0, 0));
        auto cgk_1D_ptr = reinterpret_cast<double*>(cgk_1D.mutable_unchecked<1>().mutable_data(0));
        auto delta_lm_ptr = reinterpret_cast<double*>(delta_lm.mutable_unchecked<1>().mutable_data(0));

        auto end = chrono::high_resolution_clock::now();
        auto convert_time = chrono::duration_cast<chrono::milliseconds>(end - start).count();
        
        cout << "**convert_time " << convert_time << endl;
        
        start = chrono::high_resolution_clock::now();
        //single_cf_gridder::add_to_grid(grid_ptr, grid_shape_ptr, sum_weight_ptr, vis_data_ptr, vis_shape_ptr, uvw_ptr, freq_chan_ptr, chan_map_ptr, pol_map_ptr, weight_ptr, cgk_1D_ptr, delta_lm_ptr, support, oversampling);
        single_cf_gridder::add_to_grid(grid_shape_ptr, sum_weight_ptr, vis_data_ptr, vis_shape_ptr, uvw_ptr, freq_chan_ptr, chan_map_ptr, pol_map_ptr, weight_ptr, cgk_1D_ptr, delta_lm_ptr, support, oversampling);
        end = chrono::high_resolution_clock::now();
        auto grid_time = chrono::duration_cast<chrono::milliseconds>(end - start).count();
        
        cout << "**grid_time " << grid_time << endl;

        //auto buffer = grid.request();
        //std::array<long, 4> grid_shape{buffer.shape[0],buffer.shape[1],buffer.shape[2],buffer.shape[3]};
        
        //long grid_shape = reinterpret_cast<long>(buffer.shape[0]);
        //std::array<size_t, 4> grid_shape{buffer.shape[0],buffer.shape[1],buffer.shape[2],buffer.shape[3]};
        
        //size_t grid_shape[4] = {mutable_mapping.shape(0),mutable_mapping.shape(1),mutable_mapping.shape(2),mutable_mapping.shape(3)};
    //size_t  a = mutable_mapping.shape(0);
        
//auto grid_shape = mutable_mapping.shape();
        
    }
    
    std::pair<int, int> grid(int image_size, int n_time_chunks, int n_chan_chunks){
        return single_cf_gridder::grid(image_size,n_time_chunks,n_chan_chunks);
    }
    
    

};


PYBIND11_MODULE(pybind11_wrapper, m)
{
    py::class_<single_cf_gridder_pybind>(m, "single_cf_gridder_pybind")
        .def(py::init<>()) // constructor
        .def("create_grid", &single_cf_gridder_pybind::create_grid)
//        .def("grid_vis_data",&single_cf_gridder_pybind::add_to_grid, py::arg().noconvert(),py::arg().noconvert(),py::arg().noconvert(),py::arg().noconvert(),py::arg().noconvert(),
//             py::arg().noconvert(),
//             py::arg().noconvert(),
//             py::arg().noconvert(),
//             py::arg().noconvert(),
//             py::arg().noconvert(),
//             py::arg().noconvert(),
//             py::arg().noconvert(),
//             py::arg().noconvert(),
//             py::arg().noconvert()
//             );
        .def("grid_vis_data",&single_cf_gridder_pybind::add_to_grid)
        .def("grid", &single_cf_gridder_pybind::grid);
}


//PYBIND11_MODULE(pybind11_wrapper, m)
//{
//    py::class_<single_cf_gridder>(m, "single_cf_gridder")
//        .def(py::init<>()) // constructor
//        .def("sum", &single_cf_gridder::sum);
//}


//g++ -o  gridder/single_cf_gridder.cpp gridder/pybind11_wrapper.cpp -lpybind11


//g++ -O3 -Wall -shared -std=c++11 -undefined dynamic_lookup $(python3 -m pybind11 --includes) gridder/single_cf_gridder.cpp gridder/pybind11_wrapper.cpp -o pybind11_wrapper$(python3-config --extension-suffix)
