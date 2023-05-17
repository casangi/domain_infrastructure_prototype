#include "single_cf_gridder.h"
#include <iostream>
#include <random>
#include <chrono>
#include <thread>

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


void single_cf_gridder::create_array(int array_length)
{
    // Seed the random number generator
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // Create a random number generator
    std::default_random_engine generator(std::rand());

    // Create a distribution for generating random float numbers
    float minVal = 0.0f;
    float maxVal = 1.0f;
    std::uniform_real_distribution<float> distribution(minVal, maxVal);

    // Create a vector to hold the random float numbers
    std::vector<float> floatArray(array_length);

    // Populate the vector with random float numbers
    for (int i = 0; i < array_length; ++i) {
        floatArray[i] = distribution(generator);
    }
    
    std::cout << "Element 100 is " << floatArray[100] << std::endl;
    std::cout << "I am sleeping " << std::endl;
    std::chrono::seconds duration(100);
    std::this_thread::sleep_for(duration);
    std::cout << "Done sleeping " << std::endl;
}
