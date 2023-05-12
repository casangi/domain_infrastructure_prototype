// single_cf_gridder.h
#ifndef SINGLE_CF_GRIDDER_H
#define SINGLE_CF_GRIDDER_H
#include <iostream>

class single_cf_gridder
{
private:
  int gx;
  int gy;

public:
  single_cf_gridder();
  int sum(int x,int y);
  void increment_array(double* array);
};

#endif
