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
};


PYBIND11_MODULE(pybind11_wrapper, m)
{
    py::class_<single_cf_gridder_pybind>(m, "single_cf_gridder_pybind")
        .def(py::init<>()) // constructor
        .def("sum", &single_cf_gridder_pybind::sum)
        .def("increment_array", &single_cf_gridder_pybind::increment_array, py::arg().noconvert())
        .def("create_array",&single_cf_gridder_pybind::create_array);
}







//PYBIND11_MODULE(pybind11_wrapper, m)
//{
//    py::class_<single_cf_gridder>(m, "single_cf_gridder")
//        .def(py::init<>()) // constructor
//        .def("sum", &single_cf_gridder::sum);
//}


//g++ -o  gridder/single_cf_gridder.cpp gridder/pybind11_wrapper.cpp -lpybind11


//g++ -O3 -Wall -shared -std=c++11 -undefined dynamic_lookup $(python3 -m pybind11 --includes) gridder/single_cf_gridder.cpp gridder/pybind11_wrapper.cpp -o pybind11_wrapper$(python3-config --extension-suffix)
