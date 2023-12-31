//
// This file is part of the course materials for AMATH483/583 at the University of Washington,
// Spring 2020
//
// Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License
// https://creativecommons.org/licenses/by-nc-sa/4.0/
//
// Author: Andrew Lumsdaine
//
#ifndef AMATH583_AOS_HPP
#define AMATH583_AOS_HPP

#include <cassert>
#include <tuple>
#include <vector>
#include <iostream>

#include "Vector.hpp"
#include "Matrix.hpp"
#include <immintrin.h> // for SIMD

class AOSMatrix {
private:
  typedef std::tuple<size_t, size_t, double> element;

public:
  AOSMatrix(size_t M, size_t N) : num_rows_(M), num_cols_(N) {}

  void push_back(size_t i, size_t j, double val) { 
    assert(i < num_rows_ && i >= 0);
    assert(j < num_cols_ && j >= 0);

    /* Write Me */
    storage_.push_back(element(i, j, val));
  }

  void clear() { storage_.clear(); }

  void reserve(size_t n) {
    assert(n >= 0);

    storage_.reserve(n);
  }

  size_t num_rows() const { return num_rows_; }
  size_t num_cols() const { return num_cols_; }
  size_t num_nonzeros() const { return storage_.size(); }

  void stream_coordinates(std::ostream& output_file) const {
    for (size_t i = 0; i < storage_.size(); ++i) {
      output_file << std::get<0>(storage_[i]) << " ";
      output_file << std::get<1>(storage_[i]) << " ";
      output_file << std::get<2>(storage_[i]) << " ";
      output_file << std::endl;
    }
  }


  //unoptimized
   void matvec(const Vector& x, Vector& y) const {
    for (size_t k = 0; k < storage_.size(); ++k) {
      y(std::get<0>(storage_[k])) += std::get<2>(storage_[k]) * x(std::get<1>(storage_[k]));
    } 
   }

  //optimized
  // Implements loop unrolling
  // implements SIMD - process 2 doubles at once. 
  void matvec_opt(const Vector& x, Vector& y) const {   
    size_t end = storage_.size() - storage_.size() % 2; 
    for (size_t k = 0; k < end; k+=2) {
      // x is accessed in a non contiguous manner which is a problem.
      // fix alignment  
      __m128d storage =   _mm_set_pd(std::get<2>(storage_[k+1]),std::get<2>(storage_[k]));
      __m128d xt = _mm_set_pd(x(std::get<1>(storage_[k+1])),x(std::get<1>(storage_[k])));
      __m128d res =  _mm_mul_pd(storage, xt);
       y(std::get<0>(storage_[k])) += _mm_cvtsd_f64(res);
       y(std::get<0>(storage_[k+1])) += _mm_cvtsd_f64(_mm_unpackhi_pd(res, res));    
    }

    if(end < storage_.size()){
      y(std::get<0>(storage_[end])) += std::get<2>(storage_[end]) * x(std::get<1>(storage_[end]));
    }
  }

  //unoptimized
  void t_matvec(const Vector& x, Vector& y) const {
    for (size_t k = 0; k < storage_.size(); ++k) {
      y(std::get<1>(storage_[k])) += std::get<2>(storage_[k]) * x(std::get<0>(storage_[k]));
    }
  }

  //unoptimized
  void matmat(const Matrix& B, Matrix& C) const {
    // this matrix is sparse. B is dense. C is the result. 
    assert(num_rows() == C.num_rows());
    assert(B.num_cols() == C.num_cols());
    assert(num_cols() == B.num_rows());

    for(size_t j = 0; j < B.num_cols(); j++){
      for (size_t k = 0; k < storage_.size(); ++k) {
        C(std::get<0>(storage_[k]), j) += std::get<2>(storage_[k]) * B(std::get<1>(storage_[k]),j);
      }
    }
  } 

//optimized
// hoisting and unrolling
void matmat_opt(const Matrix& B, Matrix& C) const {
    // this matrix is sparse. B is dense. C is the result. 
    assert(num_rows() == C.num_rows());
    assert(B.num_cols() == C.num_cols());
    assert(num_cols() == B.num_rows());
     
    int cond =  storage_.size() - storage_.size() % 2;
    for(size_t k = 0; k < cond; k+=2){
      double t = std::get<2>(storage_[k]);
      double t1 = std::get<2>(storage_[k+1]);
      size_t colk =std::get<1>(storage_[k]);
      size_t colk1 = std::get<1>(storage_[k+1]);
      size_t rowk = std::get<0>(storage_[k]);
      size_t rowk1 = std::get<0>(storage_[k+1]);
      
      size_t innercond = B.num_cols() - B.num_cols() % 2;
      for (size_t j = 0; j < innercond; j+=2) {
        C(rowk, j) += t * B(colk,j);
        C(rowk1, j) += t1 * B(colk1,j);
        C(rowk, j+1) += t * B(colk,j+1);
        C(rowk1, j+1) += t1 * B(colk1,j+1);
      }
      // there is one element left in B?
      if(innercond < B.num_cols()){
        C(rowk, innercond) += t * B(colk,innercond);
        C(rowk1, innercond) += t1 * B(colk1,innercond);
      }
   } 
   // there is one element left in storage_
   if(cond < storage_.size()){
     double t = std::get<2>(storage_[cond]);
     size_t row = std::get<0>(storage_[cond]);
     size_t col = std::get<1>(storage_[cond]);
     size_t innercond = B.num_cols() - B.num_cols() % 2;
     for (size_t j = 0; j < innercond; j+=2) {
       C(row, j) += t * B(col,j);
       C(row, j+1) += t * B(col,j+1);
     }
     // there is one element left in B?
      if(innercond < B.num_cols()){
        C(row, innercond) += t * B(col,innercond);
      }
   }
} 

private:
  size_t               num_rows_, num_cols_;
  std::vector<element> storage_;
};

#endif    // AMATH583_AOS_HPP
