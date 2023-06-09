#include "single_cf_gridder.h"
#include <iostream>
#include <random>
#include <chrono>
#include <thread>
#include <iostream>
#include <vector>
#include <cmath>
using namespace std;

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
    double minVal = 0.0f;
    double maxVal = 1.0f;
    std::uniform_real_distribution<double> distribution(minVal, maxVal);

    // Create a vector to hold the random float numbers
    std::vector<double> floatArray(array_length);

    // Populate the vector with random float numbers
    for (int i = 0; i < array_length; ++i) {
        floatArray[i] = distribution(generator);
    }
    
    std::cout << "Element 101 is " << floatArray[101] << std::endl;
    std::cout << "I am sleeping " << std::endl;
    std::chrono::seconds duration(10);
    std::this_thread::sleep_for(duration);
    std::cout << "Done sleeping " << std::endl;
}


void single_cf_gridder::grid_vis_data(std::complex<double>**** grid)
{
    std::complex<double> mycomplex(10.0, 2.0);
    grid[1][2][1][0] = mycomplex;
}


void single_cf_gridder::add_to_grid(std::complex<double>* grid, long* grid_shape, double* sum_weight, std::complex<double>* vis_data, long* vis_shape, double* uvw, double* freq_chan, long* chan_map, long* pol_map, double* weight, double* cgk_1D, double* delta_lm, int support, int oversampling) {

    int n_time = vis_shape[0];
    int n_baseline = vis_shape[1];
    int n_chan = vis_shape[2];
    int n_pol = vis_shape[3];

    int n_u = grid_shape[2];
    int n_v = grid_shape[3];
    double c = 299792458.0;

    int support_center = support / 2;
    int u_center = n_u / 2;
    int v_center = n_v / 2;

    int start_support = -support_center;
    int end_support = support - support_center;

    for (int i_time = 0; i_time < n_time; i_time++) {
        for (int i_baseline = 0; i_baseline < n_baseline; i_baseline++) {
            for (int i_chan = 0; i_chan < n_chan; i_chan++) {
                int a_chan = chan_map[i_chan];
                double u = uvw[i_time * n_baseline * 3 + i_baseline * 3 + 0] * (-(freq_chan[i_chan] * delta_lm[0] * n_u) / c);
                double v = uvw[i_time * n_baseline * 3 + i_baseline * 3 + 1] * (-(freq_chan[i_chan] * delta_lm[1] * n_v) / c);

                if (!std::isnan(u) && !std::isnan(v)) {
                    double u_pos = u + u_center;
                    double v_pos = v + v_center;

                    int u_center_indx = static_cast<int>(u_pos + 0.5);
                    int v_center_indx = static_cast<int>(v_pos + 0.5);

                    if (u_center_indx + support_center < n_u && v_center_indx + support_center < n_v && u_center_indx - support_center >= 0 && v_center_indx - support_center >= 0) {
                        double u_offset = u_center_indx - u_pos;
                        int u_center_offset_indx = static_cast<int>(std::floor(u_offset * oversampling + 0.5));
                        double v_offset = v_center_indx - v_pos;
                        int v_center_offset_indx = static_cast<int>(std::floor(v_offset * oversampling + 0.5));

                        for (int i_pol = 0; i_pol < n_pol; i_pol++) {
                            double sel_weight = weight[i_time * n_baseline * n_chan * n_pol + i_baseline * n_chan * n_pol + i_chan * n_pol + i_pol];
                            std::complex<double> weighted_data = vis_data[i_time * n_baseline * n_chan * n_pol + i_baseline * n_chan * n_pol + i_chan * n_pol + i_pol] * weight[i_time * n_baseline * n_chan * n_pol + i_baseline * n_chan * n_pol + i_chan * n_pol + i_pol];

                            if (!std::isnan(weighted_data.real()) && !std::isnan(weighted_data.imag()) && weighted_data.real() != 0.0 && weighted_data.imag() != 0.0) {
                                int a_pol = pol_map[i_pol];
                                double norm = 0.0;

                                for (int i_v = start_support; i_v < end_support; i_v++) {
                                    int v_indx = v_center_indx + i_v;
                                    int v_offset_indx = std::abs(static_cast<int>(oversampling * i_v + v_center_offset_indx));
                                    double conv_v = cgk_1D[v_offset_indx];

                                    for (int i_u = start_support; i_u < end_support; i_u++) {
                                        int u_indx = u_center_indx + i_u;
                                        int u_offset_indx = std::abs(static_cast<int>(oversampling * i_u + u_center_offset_indx));
                                        double conv_u = cgk_1D[u_offset_indx];
                                        double conv = conv_u * conv_v;
                                        grid[a_chan * n_pol * n_u * n_v + a_pol * n_u * n_v + u_indx * n_v + v_indx] += conv * weighted_data;
                                        norm += conv;
                                    }
                                }

                                sum_weight[a_chan * n_pol + a_pol] += sel_weight * norm;
                            }
                        }
                    }
                }
            }
        }
    }
}


std::vector<double> single_cf_gridder::create_prolate_spheroidal_kernel_1d(int oversampling, int support) {
    int support_center = support / 2;
    int oversampling_center = oversampling / 2;

    std::vector<double> u(oversampling * (support_center), 0.0);
    for (int i = 0; i < oversampling * (support_center); i++) {
        u[i] = static_cast<double>(i) / (support_center * oversampling);
    }
    
    std::vector<double> long_half_kernel_1d(oversampling * (support_center + 1), 0.0);
    std::pair<std::vector<double>, std::vector<double>> grdsf = prolate_spheroidal_function(u);

    for (int i = 0; i < oversampling * (support_center); i++) {
        //cout << u[i] << ",*," << grdsf.first[i] << ",*," << grdsf.second[i] << endl;
        long_half_kernel_1d[i] = grdsf.second[i];
    }
    return long_half_kernel_1d;
}



std::pair<std::vector<double>, std::vector<double>> single_cf_gridder::prolate_spheroidal_function(std::vector<double> u) {
    std::vector<std::vector<double>> p = {{8.203343e-2, -3.644705e-1, 6.278660e-1, -5.335581e-1, 2.312756e-1},
                                          {4.028559e-3, -3.697768e-2, 1.021332e-1, -1.201436e-1, 6.412774e-2}};
    std::vector<std::vector<double>> q = {{1.0000000e0, 8.212018e-1, 2.078043e-1},
                                          {1.0000000e0, 9.599102e-1, 2.918724e-1}};

    int n_p = p[0].size();
    int n_q = q[0].size();

    std::vector<double> u_abs(u.size());
    std::transform(u.begin(), u.end(), u_abs.begin(), [](double val) { return std::abs(val); });

    std::vector<double> uend(u.size(), 0.0);
    std::vector<int> part(u.size(), 0);

    for (size_t i = 0; i < u.size(); ++i) {
        if (u_abs[i] >= 0.0 && u_abs[i] < 0.75) {
            part[i] = 0;
            uend[i] = 0.75;
        } else if (u_abs[i] >= 0.75 && u_abs[i] <= 1.0) {
            part[i] = 1;
            uend[i] = 1.0;
        }
    }

    std::vector<double> delusq(u.size());
    for (size_t i = 0; i < u.size(); ++i) {
        delusq[i] = std::pow(u_abs[i], 2) - std::pow(uend[i], 2);
    }

    std::vector<double> top(u.size(), 0.0);
    for (int k = 0; k < n_p; ++k) {
        for (size_t i = 0; i < u.size(); ++i) {
            top[i] += p[part[i]][k] * std::pow(delusq[i], k);
        }
    }

    std::vector<double> bot(u.size(), 0.0);
    for (int k = 0; k < n_q; ++k) {
        for (size_t i = 0; i < u.size(); ++i) {
            bot[i] += q[part[i]][k] * std::pow(delusq[i], k);
        }
    }

    std::vector<double> grdsf(u.size(), 0.0);
    for (size_t i = 0; i < u.size(); ++i) {
        if (bot[i] > 0.0) {
            grdsf[i] = top[i] / bot[i];
        }
        if (std::abs(u_abs[i]) > 1.0) {
            grdsf[i] = 0.0;
        }
    }

    std::vector<double> correcting_image(u.size());
    std::transform(u.begin(), u.end(), grdsf.begin(), correcting_image.begin(), [](double u_val, double grdsf_val) {
        return (1 - std::pow(u_val, 2)) * grdsf_val;
    });

    return std::make_pair(grdsf, correcting_image);
}















//std::vector<std::vector<double>> single_cf_gridder::prolate_spheroidal_function(std::vector<double> u) {
//    std::vector<std::vector<double>> grdsf(u.size(), std::vector<double>(2, 0.0));
//
//    std::vector<std::vector<double>> p = {{8.203343e-2, -3.644705e-1, 6.278660e-1, -5.335581e-1, 2.312756e-1},
//                                           {4.028559e-3, -3.697768e-2, 1.021332e-1, -1.201436e-1, 6.412774e-2}};
//    std::vector<std::vector<double>> q = {{1.0000000e0, 8.212018e-1, 2.078043e-1}, {1.0000000e0, 9.599102e-1, 2.918724e-1}};
//
//    int n_p = p[0].size();
//    int n_q = q[0].size();
//
//    int size = u.size();
//    std::vector<double> uend(size, 0.0);
//    std::vector<int> part(size, 0);
//
//    for (int i = 0; i < size; i++) {
//        if (u[i] >= 0.0 && u[i] < 0.75) {
//            part[i] = 0;
//            uend[i] = 0.75;
//        } else if (u[i] >= 0.75 && u[i] < 1.0) {
//            part[i] = 1;
//            uend[i] = 1.0;
//        }
//    }
//
//
//
//    for (int i = 0; i < size; i++) {
//        double v = 0.0;
//        for (int j = 0; j < n_p; j++) {
//            v += p[part[i]][j] * std::pow(u[i]*u[i] - uend[i]*uend[i], n_p - 1 - j);
//        }
//        grdsf[i][0] = v;
//    }
//
//    for (int i = 0; i < size; i++) {
//        double v = 1.0;
//        for (int j = 0; j < n_q; j++) {
//            v += q[part[i]][j] * std::pow(u[i]*u[i] - uend[i]*uend[i], n_q - 1 - j);
//        }
//        grdsf[i][1] = v;
//    }
//
//    return grdsf;
//}







//void single_cf_gridder::standard_grid(std::complex<double>**** grid, int* grid_shape, double** sum_weight, std::complex<double>**** vis_data, int* vis_shape, double*** uvw, double* freq_chan, int* chan_map, int* pol_map, double**** weight, double* cgk_1D, float* delta_lm, double support, double oversampling) {
//    int n_time = vis_shape[0];
//    int n_baseline = vis_shape[1];
//    int n_chan = vis_shape[2];
//    int n_pol = vis_shape[3];
//
//    int n_u = grid_shape[2];
//    int n_v = grid_shape[3];
//    double c = 299792458.0;
//
//    int support_center = support / 2;
//    int u_center = n_u / 2;
//    int v_center = n_v / 2;
//
//    int start_support = -support_center;
//    int end_support = support - support_center;
//
//    for (int i_time = 0; i_time < n_time; i_time++) {
//        for (int i_baseline = 0; i_baseline < n_baseline; i_baseline++) {
//            for (int i_chan = 0; i_chan < n_chan; i_chan++) {
//                int a_chan = chan_map[i_chan];
//                double u = uvw[i_time][i_baseline][0] * (-(freq_chan[i_chan] * delta_lm[0] * n_u) / c);
//                double v = uvw[i_time][i_baseline][1] * (-(freq_chan[i_chan] * delta_lm[1] * n_v) / c);
//
//                if (!std::isnan(u) && !std::isnan(v)) {
//                    double u_pos = u + u_center;
//                    double v_pos = v + v_center;
//
//                    int u_center_indx = static_cast<int>(u_pos + 0.5);
//                    int v_center_indx = static_cast<int>(v_pos + 0.5);
//
//                    if (u_center_indx + support_center < n_u && v_center_indx + support_center < n_v && u_center_indx - support_center >= 0 && v_center_indx - support_center >= 0) {
//                        double u_offset = u_center_indx - u_pos;
//                        int u_center_offset_indx = static_cast<int>(std::floor(u_offset * oversampling + 0.5));
//                        double v_offset = v_center_indx - v_pos;
//                        int v_center_offset_indx = static_cast<int>(std::floor(v_offset * oversampling + 0.5));
//
//                        for (int i_pol = 0; i_pol < n_pol; i_pol++) {
//                            double sel_weight = weight[i_time][i_baseline][i_chan][i_pol];
//                            std::complex<double> weighted_data = vis_data[i_time][i_baseline][i_chan][i_pol] * weight[i_time][i_baseline][i_chan][i_pol];
//
//                            if (!std::isnan(weighted_data.real()) && !std::isnan(weighted_data.imag()) && weighted_data.real() != 0.0 && weighted_data.imag() != 0.0) {
//                                int a_pol = pol_map[i_pol];
//                                double norm = 0.0;
//
//                                for (int i_v = start_support; i_v < end_support; i_v++) {
//                                    int v_indx = v_center_indx + i_v;
//                                    int v_offset_indx = std::abs(static_cast<int>(oversampling * i_v + v_center_offset_indx));
//                                    double conv_v = cgk_1D[v_offset_indx];
//
//                                    for (int i_u = start_support; i_u < end_support; i_u++) {
//                                        int u_indx = u_center_indx + i_u;
//                                        int u_offset_indx = std::abs(static_cast<int>(oversampling * i_u + u_center_offset_indx));
//                                        double conv_u = cgk_1D[u_offset_indx];
//                                        double conv = conv_u * conv_v;
//
//                                        grid[a_chan][a_pol][u_indx][v_indx] += conv * weighted_data;
//                                        norm += conv;
//                                    }
//                                }
//
//                                sum_weight[a_chan][a_pol] += sel_weight * norm;
//                            }
//                        }
//                    }
//                }
//            }
//        }
//    }
//}
