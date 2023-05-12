#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#define FORCE_IMPORT_ARRAY
namespace py = pybind11;
#include <iostream>
using namespace std;
using namespace pybind11::literals;
#include "single_cf_gridder.h"


PYBIND11_MODULE(pybind11_wrapper, m)
{
    py::class_<single_cf_gridder>(m, "single_cf_gridder")
        .def(py::init<>()) // constructor
        .def("sum", &single_cf_gridder::sum);
}


//g++ -o  gridder/single_cf_gridder.cpp gridder/pybind11_wrapper.cpp -lpybind11


//g++ -O3 -Wall -shared -std=c++11 -undefined dynamic_lookup $(python3 -m pybind11 --includes) gridder/single_cf_gridder.cpp gridder/pybind11_wrapper.cpp -o pybind11_wrapper$(python3-config --extension-suffix)
