//
// This file is part of the course materials for AMATH483/583 at the University of Washington,
// Spring 2020
//
// Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License
// https://creativecommons.org/licenses/by-nc-sa/4.0/
//
// Author: Andrew Lumsdaine
//
#ifndef AMATH583_COOMATRIX_HPP
#define AMATH583_COOMATRIX_HPP

#include "Matrix.hpp"
#include "Vector.hpp"
#include <cassert>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <vector>
#include <immintrin.h> // for SIMD

class COOMatrix {
private:
public:
  COOMatrix(size_t M, size_t N) : num_rows_(M), num_cols_(N) {}

  void push_back(size_t i, size_t j, double val) {
    assert(i < num_rows_ && i >= 0);
    assert(j < num_cols_ && j >= 0);

    row_indices_.push_back(i);
    col_indices_.push_back(j);
    storage_.push_back(val);
  }

  void clear() {
    row_indices_.clear();
    col_indices_.clear();
    storage_.clear();
  }

  void reserve(size_t n) {
    assert(n >= 0);

    row_indices_.reserve(n);
    col_indices_.reserve(n);    
    storage_.reserve(n);
  }

  size_t num_rows() const { return num_rows_; }
  size_t num_cols() const { return num_cols_; }
  size_t num_nonzeros() const { return storage_.size(); }

  void stream_coordinates(std::ostream& output_file) const {
    for (size_t i = 0; i < num_nonzeros(); ++i) {
      output_file << row_indices_[i] << " ";
      output_file << col_indices_[i] << " ";
      output_file << storage_[i];
      output_file << std::endl;
    }
  }

 // unoptimized
  void matvec(const Vector& x, Vector& y) const {    
    for (size_t k = 0; k < storage_.size(); ++k) {
      // x is accessed in a non contiguous manner which is a problem.
      y(row_indices_[k]) += storage_[k] * x(col_indices_[k]);
    }  
  }

  //optimized
  // Implements loop unrolling
  // implements SIMD - process 2 doubles at once. 
  void matvec_opt(const Vector& x, Vector& y) const { 
    for (size_t k = 0; k < storage_.size() / 2 * 2; k+=2) {
      // x is accessed in a non contiguous manner which is a problem.
      // fix alignment  
      __m128d storage = _mm_load_pd(&storage_[k]);
      __m128d xt = _mm_set_pd(x(col_indices_[k+1]),x(col_indices_[k]));    
     __m128d res =  _mm_mul_pd(storage, xt);
     //can't get this working with SIMD
      y(row_indices_[k]) +=  _mm_cvtsd_f64(res); ;
       y(row_indices_[k+1]) +=  _mm_cvtsd_f64(_mm_unpackhi_pd(res, res));   
    
      //  __m128d yt = _mm_set_pd(y(row_indices_[k+1]),y(row_indices_[k]));
      // __m128d res =  _mm_add_pd(yt, _mm_mul_pd(storage, xt));
      //  y(row_indices_[k]) = _mm_cvtsd_f64(res); 
      //  y(row_indices_[k+1]) = _mm_cvtsd_f64(_mm_unpackhi_pd(res, res));
    }
    for(size_t i = storage_.size() / 2 * 2; i < storage_.size(); i++){
      y(row_indices_[i]) += storage_[i] * x(col_indices_[i]);
    }
  }

  // unoptimized
  void t_matvec(const Vector& x, Vector& y) const {
    for (size_t k = 0; k < storage_.size(); ++k) {
      y(col_indices_[k]) += storage_[k] * x(row_indices_[k]);
    }
  }

  // unoptimized
  void matmat(const Matrix& B, Matrix& C) const {
    // this matrix is sparse. B is dense. C is the result. 
    assert(num_rows() == C.num_rows());
    assert(B.num_cols() == C.num_cols());
    assert(num_cols() == B.num_rows());

    for(size_t j = 0; j < B.num_cols(); j++){
      for (size_t k = 0; k < storage_.size(); ++k) {
        C(row_indices_[k], j) += storage_[k] * B(col_indices_[k],j);
      }
    }
  }

  // optimized
  // loop reordering gives better row-wise access to B
  // hoisting storage_[k] above the inner loop to save memory access time. Improve cache efficiency. 
  // hoisting indices above the inner loop also saves memory access time.
  // unrolled outer loop to save branch instructions and improve reuse of data. 
  //   
  void matmat_opt(const Matrix& B, Matrix& C) const {
    // this matrix is sparse. B is dense. C is the result. 
    assert(num_rows() == C.num_rows());
    assert(B.num_cols() == C.num_cols());
    assert(num_cols() == B.num_rows());

    int cond =  storage_.size() - storage_.size() % 2;
    for(size_t k = 0; k < cond; k+=2){
      double t = storage_[k];
      double t1 = storage_[k+1];
      size_t colk = col_indices_[k];
      size_t colk1 = col_indices_[k+1];
      size_t rowk = row_indices_[k];
      size_t rowk1 = row_indices_[k+1];
      
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
     double t = storage_[cond];
     size_t row = row_indices_[cond];
     size_t col = col_indices_[cond];
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
  size_t              num_rows_, num_cols_;
  std::vector<size_t> row_indices_, col_indices_;
  std::vector<double> storage_;
};

#endif    // AMATH583_COOMATRIX_HPP
