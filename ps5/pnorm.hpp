//
// This file is part of the course materials for AMATH483/583 at the University of Washington,
// Spring 2020
//
// Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License
// https://creativecommons.org/licenses/by-nc-sa/4.0/
//
// Author: Andrew Lumsdaine
//

#ifndef AMATH583_PNORM_HPP
#define AMATH583_PNORM_HPP

#include "Timer.hpp"
#include "Vector.hpp"
#include "amath583.hpp"
#include "norm_utils.hpp"

#include <cassert>
#include <cmath>
#include <cstddef>
#include <future>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>

std::mutex lockpartialsum;
// Fix us
void worker_a(const Vector& x, size_t begin, size_t end, double& sum) {
  double local_partial = 0.0;
  for (size_t i = begin; i < end; ++i) {
    local_partial += x(i) * x(i);
  }  
  {
    std::lock_guard<std::mutex> Guard(lockpartialsum);
    sum += local_partial;

  }
}

// Fix us
double partitioned_two_norm_a(const Vector& x, size_t partitions) {
  size_t                   blocksize = x.num_rows() / partitions;

  double sum = 0;
  std::vector<std::thread> partial_sums; // vector of futures where each future promises a double.
  for (size_t i = 0; i < partitions; ++i) {
    // launch an async thread and add reference to vector
    partial_sums.push_back(std::thread(worker_a, std::cref(x), 
    i * blocksize, (i + 1) * blocksize, std::ref(sum)));
  }
  for (size_t i = 0; i < partitions; ++i)
    partial_sums[i].join();

  return std::sqrt(sum);
}

#endif    //  AMATH583_PNORM_HPP
