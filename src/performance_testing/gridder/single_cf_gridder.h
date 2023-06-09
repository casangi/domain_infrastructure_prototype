// single_cf_gridder.h
#ifndef SINGLE_CF_GRIDDER_H
#define SINGLE_CF_GRIDDER_H
#include <iostream>
#include <cmath>
#include <complex>
#include <algorithm>

class single_cf_gridder
{
//private:
//  int gx;
//  int gy;

public:
  single_cf_gridder();
  int sum(int x,int y);
  void increment_array(double array[]);
  void create_array(int array_length=10);
  void grid_vis_data(std::complex<double>**** grid);
    
  void add_to_grid(std::complex<double>* grid, long* grid_shape, double* sum_weight, std::complex<double>* vis_data, long* vis_shape, double* uvw, double* freq_chan, long* chan_map, long* pol_map, double* weight, double* cgk_1D, double* delta_lm, int support, int oversampling);
    
    std::vector<double> create_prolate_spheroidal_kernel_1d(int oversampling, int support);
    std::pair<std::vector<double>, std::vector<double>> prolate_spheroidal_function(std::vector<double> u);

private:
    int oversampling_;
    int support_;
};

#endif
