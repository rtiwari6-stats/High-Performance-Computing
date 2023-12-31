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

// Basic (original) outer -- Optimize and parallelize me
void outer(Matrix& A, const Vector& x) {  
  // exchanging order of loops to get better cache efficiency as A is stored row-major.
  // hoisting x(i) to reduce memory reads
  #pragma omp parallel for
  for (size_t i = 0; i < A.num_rows(); ++i) {  
    double t = x(i);
    size_t innercond = A.num_cols() - A.num_cols() % 2;
    for (size_t j = 0; j < innercond; j += 2) {
      A(i, j) += t * x(j);
      A(i, j+1) += t * x(j+1);
    }
    if(innercond < A.num_cols()){ //check if we missed something!
      A(i, innercond) += t * x(innercond);
    }
  }
}

auto gen_covariance(const std::vector<Vector>& z) {
  size_t sz = z[0].num_rows();
  Matrix C(sz, sz);

  size_t cond = z.size() - z.size() % 2;
  for (size_t k = 0; k < cond; k += 2) {
    outer(C, z[k]);
    outer(C, z[k+1]);
  }
  if(cond < z.size()){//check if we missed something!
    outer(C, z[cond]);
  }

  // Normalize
  #pragma omp parallel for
  for (size_t i = 0; i < C.num_rows(); ++i) {
    size_t innercond = C.num_cols() - C.num_cols() % 2;
    for (size_t j = 0; j < innercond; j += 2) {
      C(i,j) = C(i,j) / z.size();
      C(i,j+1) = C(i,j+1) / z.size();
    }
    if(innercond < C.num_cols()){ //check if we missed something!
      C(i,innercond) = C(i,innercond) / z.size();
    }
  }

  return C;
}

const char* tag = "par";

#include "main.hpp"
