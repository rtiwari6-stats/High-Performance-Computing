//
// This file is part of the course materials for AMATH483/583 at the University of Washington,
// Spring 2018
//
// Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License
// https://creativecommons.org/licenses/by-nc-sa/4.0/
//
// Author: Andrew Lumsdaine
//
#ifndef AMATH583_CSRMATRIX_HPP
#define AMATH583_CSRMATRIX_HPP

#include "Matrix.hpp"
#include "Vector.hpp"
#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <vector>

class CSRMatrix {
public:
  CSRMatrix(size_t M, size_t N) : is_open(false), num_rows_(M), num_cols_(N), row_indices_(num_rows_ + 1, 0) {}

  void open_for_push_back() { is_open = true; }

  void close_for_push_back() {
    is_open = false;
    for (size_t i = 0; i < num_rows_; ++i) {
      row_indices_[i + 1] += row_indices_[i];
    }
    for (size_t i = num_rows_; i > 0; --i) {
      row_indices_[i] = row_indices_[i - 1];
    }
    row_indices_[0] = 0;
  }

  void push_back(size_t i, size_t j, double value) {
    assert(is_open);
    assert(i < num_rows_ && i >= 0);
    assert(j < num_cols_ && j >= 0);

    ++row_indices_[i];
    col_indices_.push_back(j);
    storage_.push_back(value);
  }

  void clear() {
    col_indices_.clear();
    storage_.clear();
    std::fill(row_indices_.begin(), row_indices_.end(), 0);
  }

  size_t num_rows() const { return num_rows_; }
  size_t num_cols() const { return num_cols_; }
  size_t num_nonzeros() const { return storage_.size(); }

  void stream_coordinates(std::ostream& output_file) const {
    for (size_t i = 0; i < num_rows_; ++i) {
      for (size_t j = row_indices_[i]; j < row_indices_[i + 1]; ++j) {
        output_file << i << " ";
        output_file << col_indices_[j] << " ";
        output_file << storage_[j];
        output_file << std::endl;
      }
    }
  }

  //unoptimized
  void matvec(const Vector& x, Vector& y) const {
    //unoptimized
    for (size_t i = 0; i < num_rows_; ++i) {
      //double t = y(i);
      for (size_t j = row_indices_[i]; j < row_indices_[i + 1]; ++j) {
        // x is accessed in a non contiguous manner which is a problem
        y(i) += storage_[j] * x(col_indices_[j]);
      }
      //y(i) = t;
    }
  }

  //optimized
  // unrolling and hoisting
  void matvec_opt(const Vector& x, Vector& y) const {   
    size_t start = row_indices_[0];
    for (size_t i = 0; i < num_rows_; i++) {
      double t = y(i);
      size_t end = row_indices_[i+1];
      //size_t count = end - start;
      size_t diff = end - start;
      size_t loop_count = diff / 2;
      size_t leftover = diff % 2;
      size_t idx = 1;
      while (idx <= loop_count) {
        t += storage_[start] * x(col_indices_[start]); 
        t += storage_[start+1] * x(col_indices_[start+1]);
        start += 2;
        idx += 1;
      }     
      if(leftover == 1){
        t += storage_[end - 1] * x(col_indices_[end - 1]);
      }
      y(i) = t;
      start = end;
    }
  }

  // optimized
  //hoisting
  void t_matvec_opt(const Vector& x, Vector& y) const {
    for (size_t i = 0; i < num_rows_; ++i) {
      double t = x(i);
      size_t start = row_indices_[i];
      size_t end = row_indices_[i + 1];
      for (size_t j = start; j < end; ++j) {
        y(col_indices_[j]) += storage_[j] * t;
      }
      //y(i) = t;
    }
  }

  //unoptimized
  void t_matvec(const Vector& x, Vector& y) const {
    for (size_t i = 0; i < num_rows_; ++i) {   
      for (size_t j = row_indices_[i]; j < row_indices_[i + 1]; ++j) {
        y(col_indices_[j]) += storage_[j] * x(i);
      }
    }
  }
 
 
 // unoptimized
  void matmat(const Matrix& B, Matrix& C) const {
    assert(num_rows() == C.num_rows());
    assert(B.num_cols() == C.num_cols());
    assert(num_cols() == B.num_rows());
    
    for(size_t k = 0; k < B.num_cols(); k++){
      for (size_t i = 0; i < num_rows_; ++i) {
        // for sparse matrices this loop should run few times only so unrolling may not be of benefit.
        for (size_t j = row_indices_[i]; j < row_indices_[i + 1]; ++j) {
          C(i,k) += storage_[j] * B(col_indices_[j],k);
        }
      }
    }
  }
      
  
// optimized
// unrolling hoisting. 
  void matmat_opt(const Matrix& B, Matrix& C) const {
    assert(num_rows() == C.num_rows());
    assert(B.num_cols() == C.num_cols());
    assert(num_cols() == B.num_rows());

    size_t cond = B.num_cols() - B.num_cols() % 2;
    for(size_t k = 0; k < cond; k+=2){
      size_t innercond = num_rows_ - num_rows_ % 2;
      for (size_t i = 0; i < innercond; i+=2) {
        // i part
        double t = C(i,k);
        double t1 = C(i,k+1);
        for (size_t j = row_indices_[i]; j < row_indices_[i + 1]; ++j) {
          t += storage_[j] * B(col_indices_[j],k);
          t1 += storage_[j] * B(col_indices_[j],k+1);
        }
        // i+1 part
        size_t newi = i+1;
        double t2 = C(newi,k);
        double t3 = C(newi,k+1);
        for (size_t j = row_indices_[newi]; j < row_indices_[newi + 1]; ++j) {
          t2 += storage_[j] * B(col_indices_[j],k);
          t3 += storage_[j] * B(col_indices_[j],k+1);
        }
        C(i,k) = t;
        C(i,k+1) = t1;
        C(newi,k) = t2;
        C(newi,k+1) = t3;
      }
      // we have 1 left!
      if(innercond < num_rows_){
        double t = C(innercond,k);
        double t1 = C(innercond,k+1);
        for (size_t j = row_indices_[innercond]; j < row_indices_[innercond + 1]; ++j) {
          t += storage_[j] * B(col_indices_[j],k);
          t1 += storage_[j] * B(col_indices_[j],k+1);
        }
        C(innercond,k) = t;
        C(innercond,k+1) = t1;

      }
    }
    if(cond < B.num_cols()){
      for (size_t i = 0; i < num_rows_; ++i) {
        double t = C(i,cond);
        for (size_t j = row_indices_[i]; j < row_indices_[i + 1]; ++j) {
          t += storage_[j] * B(col_indices_[j],cond);
        }
        C(i,cond) = t;
      }
    }
  }

private:
  bool                is_open;
  size_t              num_rows_, num_cols_;
  std::vector<size_t> row_indices_, col_indices_;
  std::vector<double> storage_;
};

#endif    // AMATH583_CSRMATRIX_HPP
