//
// This file is part of the course materials for AMATH 483/583 at the University of Washington
// Spring 2021
//
// Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License
// https://creativecommons.org/licenses/by-nc-sa/4.0/
//
// Author: Andrew Lumsdaine
//

#include "amath583.hpp"

// Parallelize, optimize, and cache block me
auto gen_covariance(const std::vector<Vector>& z) {
  size_t sz = z[0].num_rows();
  Matrix C(sz, sz);

  for (size_t k = 0; k < z.size(); ++k) {  
    // using omp to parallelize the outer loop  
    #pragma omp parallel for
    for (size_t i = 0; i < C.num_rows(); ++i){ // unroll
      double t = z[k](i);  // hoist - saves memory accesses which are slow.
      // divide into half and compute
      for (size_t j = 0; j <= i; ++j) {
	      C(i, j) += t * z[k](j);
      }
    }
  }
  // using omp to parallelize the outer loop
  #pragma omp parallel for 
  //process the other half
  for (size_t i = 0; i < C.num_rows(); ++i){
    for (size_t j = i + 1; j < C.num_cols(); ++j){
      C(i, j) += C(j, i);
    }
  }

  //const size_t blocksize = std::min(C.num_rows(), (size_t)32);
  // #pragma omp parallel for
  // for (size_t ii = 0; ii < C.num_rows(); ii += blocksize) {
  //   for (size_t jj = 0; jj < C.num_cols(); jj += blocksize) {
  //     size_t stop_i  = std::min(ii + blocksize, C.num_rows());
  //     size_t stop_j  = std::min(jj + blocksize, C.num_cols());
  //     #pragma omp parallel for
  //     for(size_t k = 0; k < z.size(); ++k){
  //       #pragma omp parallel for
  //       for (size_t i = ii; i < stop_i; i++) {
  //         double t = z[k](i);
  //         for (size_t j = jj; j < stop_j; j++) {
  //           C(i, j) += t * z[k](j);
  //         }
  //       }
  //     }      
  //   }
  //} 

  // Normalize
  // using omp to parallelize the outer loop
  #pragma omp parallel for
  for (size_t i = 0; i < C.num_rows(); ++i) {
    size_t innercond = C.num_cols() - C.num_cols() % 2;
    for (size_t j = 0; j < innercond; j += 2) { // unroll
      C(i,j) = C(i,j) / z.size();
      C(i,j+1) = C(i,j+1) / z.size();
    }
    if(innercond < C.num_cols()){ //check if we missed something!
      C(i,innercond) = C(i,innercond) / z.size();
    }
  }

  return C;
}


const char* tag = "block";

#include "main.hpp"
