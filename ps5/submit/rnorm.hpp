//
// This file is part of the course materials for AMATH483/583 at the University of Washington,
// Spring 2020
//
// Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License
// https://creativecommons.org/licenses/by-nc-sa/4.0/
//
// Author: Andrew Lumsdaine
//

#ifndef AMATH583_RNORM_HPP
#define AMATH583_RNORM_HPP

#include "Timer.hpp"
#include "Vector.hpp"
#include "amath583.hpp"
#include "norm_utils.hpp"

#include <cassert>
#include <cmath>
#include <future>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>

std::mutex lockpartialsum;

// Useful utility function for computing sum_of_squares over a range of a vector
double sum_of_squares(const Vector& x, size_t begin, size_t end) {
  double sum = 0.0;
  for (size_t i = begin; i < end; ++i) {
    sum += x(i) * x(i);
  }
  return sum;
}

// Write me
double recursive_worker_a(const Vector& x, size_t begin, size_t end, size_t level) { 
  double sum = 0.0;
  if(level == 0){
    for(size_t i = begin; i < end; i++){
      sum += x(i) * x(i);
    }
    return sum;
  }
  return recursive_worker_a(x, begin, begin+(end-begin)/2, level-1)
         + recursive_worker_a(x, begin+(end-begin)/2, end , level-1) ;
  return sum; 
}

// inner function for thread
void recursive_worker_thread(const Vector& x, size_t begin, size_t end, size_t level, 
double& sum) { 
  double local_partial = 0.0;
  if(level == 0){
    for(size_t i = begin; i < end; i++){
      local_partial += x(i) * x(i);       
    }
    {
    std::lock_guard<std::mutex> Guard(lockpartialsum);
    sum += local_partial;
    }
  }
  else{
    recursive_worker_thread(x, begin, begin+(end-begin)/2, level-1, sum);
    recursive_worker_thread(x, begin+(end-begin)/2, end , level-1, sum) ;
  }
  
}

// two_norm_a using threads
// double recursive_two_norm_a(const Vector& x, size_t levels){
//   double sum = 0.0;
//   std::vector<std::thread> partial_sums;
//   size_t blocksize = x.num_rows() / levels;
//   // launch threads and add reference to vector
//   for(size_t i = 0; i < levels; ++i){
//     partial_sums.push_back(std::thread(recursive_worker_thread, std::cref(x), 
//     i * blocksize, (i + 1) * blocksize, levels, std::ref(sum)));
//   }
//   for (size_t i = 0; i < levels; ++i)
//     partial_sums[i].join();

//   return std::sqrt(sum);
// }

// Dispatch to the actual recursive function -- start with 0 and num_rows() as begin and end
double recursive_two_norm_a(const Vector& x, size_t levels) {
  // launch sync tasks
  double sum = 0.0;
  std::vector<std::future<double>> partial_sums;
  size_t blocksize = x.num_rows() / levels;
  // launch an async task and add reference to vector
  for(size_t i = 0; i < levels; ++i){
    partial_sums.push_back(std::async(std::launch::async, 
    recursive_worker_a, std::cref(x), i * blocksize, (i + 1) * blocksize, levels));
  }
  for (size_t i = 0; i < levels; ++i)
    sum += partial_sums[i].get();

  return std::sqrt(sum);
}

// Write me
double recursive_worker_b(const Vector& x, size_t begin, size_t end, size_t level) { 
  double sum = 0.0;
  if(level == 0){
    for(size_t i = begin; i < end; i++){
      sum += x(i) * x(i);
    }
    return sum;
  }
  return recursive_worker_b(x, begin, begin+(end-begin)/2, level-1)
         + recursive_worker_b(x, begin+(end-begin)/2, end , level-1) ;
  return sum; 
}

// Dispatch to the actual recursive function -- start with 0 and num_rows() as begin and end
double recursive_two_norm_b(const Vector& x, size_t levels) {
  // launch sync tasks
  double sum = 0.0;
  std::vector<std::future<double>> partial_sums;
  size_t blocksize = x.num_rows() / levels;
  // launch an async task and add reference to vector
  for(size_t i = 0; i < levels; ++i){
    partial_sums.push_back(std::async(std::launch::deferred, 
    recursive_worker_b, std::cref(x), i * blocksize, (i + 1) * blocksize, levels));
  }
  for (size_t i = 0; i < levels; ++i)
    sum += partial_sums[i].get();

  return std::sqrt(sum);
}

#endif    // AMATH583_RNORM_HPP
