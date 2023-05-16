#include "single_cf_gridder.h"


single_cf_gridder::single_cf_gridder()
{
    
}

int single_cf_gridder::sum(int x,int y)
{
    return x + y;
}

void single_cf_gridder::increment_array(double array[])
{
    for (size_t i = 0; i < 10; i++) {
        array[i]++;
    }
}
