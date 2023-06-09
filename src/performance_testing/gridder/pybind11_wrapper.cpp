#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#define FORCE_IMPORT_ARRAY
namespace py = pybind11;
#include <iostream>
using namespace std;
using namespace pybind11::literals;
#include "single_cf_gridder.h"
#include <random>

class single_cf_gridder_pybind : private single_cf_gridder {
public:
  single_cf_gridder_pybind() : single_cf_gridder() {}

  int sum(int x,int y){
    cout << "Hallo sum " << endl;
    return single_cf_gridder::sum(x,y);
  }
    
  void increment_array(py::array_t<double>& array_in){
      //https://pybind11.readthedocs.io/en/stable/advanced/pycpp/numpy.html?highlight=array#direct-access
      auto mutable_mapping = array_in.mutable_unchecked<1>();
      auto array = reinterpret_cast<double*>(mutable_mapping.mutable_data(0));
      
      //auto array = reinterpret_cast<double*>(mutable_mapping.mutable_data());
      cout << "2. Hallo increment_array " << array[0] << endl;
      array[0] = array[0] + 10;
      single_cf_gridder::increment_array(array);
    }
    
    void create_array(int array_length=10){
    //Function to test if python tracemalloc is detecting memory usage of C++ code.
        single_cf_gridder::create_array(array_length);
    }
    
    using complex = std::complex<double>;
    using ComplexArr = py::array_t<complex>;
    using DoubleArr  = py::array_t<double>;
    using LongArr = py::array_t<long>;
    
    void add_to_grid(ComplexArr& grid, LongArr& grid_shape, DoubleArr& sum_weight, ComplexArr& vis_data, LongArr& vis_shape, DoubleArr& uvw, DoubleArr& freq_chan, LongArr& chan_map, LongArr& pol_map, DoubleArr& weight, DoubleArr& cgk_1D, DoubleArr& delta_lm, int support, int oversampling ){
        //https://pybind11.readthedocs.io/en/stable/advanced/pycpp/numpy.html?highlight=array#direct-access

        auto grid_ptr = reinterpret_cast<complex*>(grid.mutable_unchecked<4>().mutable_data(0,0,0,0)); // Pointer to first element
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

        std::cout << grid_shape_ptr[0] << std::endl;
        
        
        single_cf_gridder::add_to_grid(grid_ptr, grid_shape_ptr, sum_weight_ptr, vis_data_ptr, vis_shape_ptr, uvw_ptr, freq_chan_ptr, chan_map_ptr, pol_map_ptr, weight_ptr, cgk_1D_ptr, delta_lm_ptr, support, oversampling);


        //auto buffer = grid.request();
        //std::array<long, 4> grid_shape{buffer.shape[0],buffer.shape[1],buffer.shape[2],buffer.shape[3]};
        
        //long grid_shape = reinterpret_cast<long>(buffer.shape[0]);
        //std::array<size_t, 4> grid_shape{buffer.shape[0],buffer.shape[1],buffer.shape[2],buffer.shape[3]};
        
        //size_t grid_shape[4] = {mutable_mapping.shape(0),mutable_mapping.shape(1),mutable_mapping.shape(2),mutable_mapping.shape(3)};
    //size_t  a = mutable_mapping.shape(0);
        
//auto grid_shape = mutable_mapping.shape();
        
    }
    
    
//    void grid_vis_data(py::array_t<std::complex<double>>& grid_in){
//        //https://pybind11.readthedocs.io/en/stable/advanced/pycpp/numpy.html?highlight=array#direct-access
//
//        auto mutable_mapping = grid_in.mutable_unchecked<4>(); //In <> is the number of dimensions
//        auto grid = reinterpret_cast<std::complex<double>*>(mutable_mapping.mutable_data(0,0,0,0)); // Pointer to first element
//
//        //auto array = reinterpret_cast<double*>(mutable_mapping.mutable_data());
//        //cout << "2. Hallo increment_array " << grid[0][0][0][0] << endl;
//        //array[0] = array[0] + 10;
//        //single_cf_gridder::increment_array(array);
//        // Access the dimensions of the array
//
//        auto buf = grid_in.request();
//        size_t dim1 = buf.shape[0];
//        size_t dim2 = buf.shape[1];
//        size_t dim3 = buf.shape[2];
//        size_t dim4 = buf.shape[3];
//
//        std::cout << dim1 << ",*," << dim2 << ",*," << dim3 << ",*," << dim4  << std::endl;
//        // Iterate over the array elements
//        std::complex<double> value;
//
//        for (size_t i = 0; i < dim1; ++i)
//        {
//            for (size_t j = 0; j < dim2; ++j)
//            {
//                for (size_t k = 0; k < dim3; ++k)
//                {
//                    for (size_t l = 0; l < dim4; ++l)
//                    {
//                        // Access the array element at (i, j, k, l)
//                        int flat_index = i * dim2 * dim3 * dim4 + j * dim3 * dim4 + k * dim4 + l;
//                        value = grid[flat_index];
//                        std::cout << flat_index << ",*," << i << ",*," << j << ",*," << k << ",*," << l << ",*,"  << value << std::endl;
//                    }
//                }
//            }
//        }
//    }
        

};


PYBIND11_MODULE(pybind11_wrapper, m)
{
    py::class_<single_cf_gridder_pybind>(m, "single_cf_gridder_pybind")
        .def(py::init<>()) // constructor
        .def("sum", &single_cf_gridder_pybind::sum)
        .def("increment_array", &single_cf_gridder_pybind::increment_array, py::arg().noconvert())
        .def("create_array",&single_cf_gridder_pybind::create_array)
        .def("grid_vis_data",&single_cf_gridder_pybind::add_to_grid, py::arg().noconvert(),py::arg().noconvert(),py::arg().noconvert(),py::arg().noconvert(),py::arg().noconvert(),
             py::arg().noconvert(),
             py::arg().noconvert(),
             py::arg().noconvert(),
             py::arg().noconvert(),
             py::arg().noconvert(),
             py::arg().noconvert(),
             py::arg().noconvert(),
             py::arg().noconvert(),
             py::arg().noconvert()
             );
}


//PYBIND11_MODULE(pybind11_wrapper, m)
//{
//    py::class_<single_cf_gridder>(m, "single_cf_gridder")
//        .def(py::init<>()) // constructor
//        .def("sum", &single_cf_gridder::sum);
//}


//g++ -o  gridder/single_cf_gridder.cpp gridder/pybind11_wrapper.cpp -lpybind11


//g++ -O3 -Wall -shared -std=c++11 -undefined dynamic_lookup $(python3 -m pybind11 --includes) gridder/single_cf_gridder.cpp gridder/pybind11_wrapper.cpp -o pybind11_wrapper$(python3-config --extension-suffix)
