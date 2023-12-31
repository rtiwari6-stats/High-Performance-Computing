//
// This file is part of the course materials for AMATH483/583 at the University of Washington,
// Spring 2020
//
// Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License
// https://creativecommons.org/licenses/by-nc-sa/4.0/
//
// Author: Andrew Lumsdaine
//

#ifndef AMATH583_CSCMATRIX_HPP
#define AMATH583_CSCMATRIX_HPP
#include "Matrix.hpp"
#include "Vector.hpp"
#include <algorithm>
#include <cassert>
#include <vector>

class CSCMatrix {
public:
  CSCMatrix(size_t M, size_t N) : is_open(false), num_rows_(M), num_cols_(N), col_indices_(num_cols_ + 1, 0) {}

  void open_for_push_back() { is_open = true; }

  void close_for_push_back() {
    is_open = false;
    for (size_t i = 0; i < num_cols_; ++i) {
      col_indices_[i + 1] += col_indices_[i];
    }
    for (size_t i = num_cols_; i > 0; --i) {
      col_indices_[i] = col_indices_[i - 1];
    }
    col_indices_[0] = 0;
  }

  void push_back(size_t i, size_t j, double value) { /* Write Me */
    assert(is_open);
    assert(i < num_rows_ && i >= 0);
    assert(j < num_cols_ && j >= 0);

    ++col_indices_[j];
    row_indices_.push_back(i);
    storage_.push_back(value);
  }

  void clear() {
    row_indices_.clear();
    storage_.clear();
    std::fill(col_indices_.begin(), col_indices_.end(), 0);
  }

  size_t num_rows() const { return num_rows_; }
  size_t num_cols() const { return num_cols_; }
  size_t num_nonzeros() const { return storage_.size(); }

  void stream_coordinates(std::ostream& output_file) const { /* Write Me */
    for (size_t i = 0; i < num_cols_; ++i) {
      for (size_t j = col_indices_[i]; j < col_indices_[i + 1]; ++j) {
        output_file << row_indices_[j] << " ";
        output_file << i << " ";
        output_file << storage_[j];
        output_file << std::endl;
      }
    }
  }

   // optimized
   // hoisting - reduce reads of x from memory. 
   void matvec_opt(const Vector& x, Vector& y) const {
     for (size_t i = 0; i < num_cols_; ++i) {
      double t = x(i);
      for (size_t j = col_indices_[i]; j < col_indices_[i + 1]; ++j) {
        y(row_indices_[j]) += storage_[j] * t;
      }
      //y(i) = t;
    }
   }

  // unoptimized
  void matvec(const Vector& x, Vector& y) const {   
    for (size_t i = 0; i < num_cols_; ++i) {
      for (size_t j = col_indices_[i]; j < col_indices_[i + 1]; ++j) {
        y(row_indices_[j]) += storage_[j] * x(i);
      }
    }
  }

  //optimized
  // hoisting - reduce reads of y from memory. 
  void t_matvec_opt(const Vector& x, Vector& y) const {
    for (size_t i = 0; i < num_cols_; ++i) {
      double t = y(i);
      for (size_t j = col_indices_[i]; j < col_indices_[i + 1]; ++j) {
        t += storage_[j] * x(row_indices_[j]);
      }
      y(i) = t;
    }
  }

// unoptimized
    void t_matvec(const Vector& x, Vector& y) const {
    for (size_t i = 0; i < num_cols_; ++i) {
      for (size_t j = col_indices_[i]; j < col_indices_[i + 1]; ++j) {
        y(i) += storage_[j] * x(row_indices_[j]);
      }
    }
  }


  
// unoptimized
   void matmat(const Matrix& B, Matrix& C) const {
    assert(num_rows() == C.num_rows());
    assert(B.num_cols() == C.num_cols());
    assert(num_cols() == B.num_rows());

    for(size_t k = 0; k < B.num_cols(); k++){
      for (size_t i = 0; i < num_cols_; ++i) {
        for (size_t j = col_indices_[i]; j < col_indices_[i + 1]; ++j) {
          C(row_indices_[j],k) += storage_[j] * B(i,k);
        }
        //y(i) = t;
      }
    }
  }

  //optimized
  // unrolling and hoisting
  void matmat_opt(const Matrix& B, Matrix& C) const {
    assert(num_rows() == C.num_rows());
    assert(B.num_cols() == C.num_cols());
    assert(num_cols() == B.num_rows());

    size_t cond = B.num_cols() - B.num_cols() % 2;
    for(size_t k = 0; k < cond; k+=2){
      size_t innercond = num_cols_ - num_cols_ % 2;
      for (size_t i = 0; i < innercond; i+=2) {
        // i part       
        double t2 = B(i,k);
        double t3 = B(i,k+1);
        for (size_t j = col_indices_[i]; j < col_indices_[i + 1]; ++j) {
          C(row_indices_[j],k) += storage_[j] * t2;
          C(row_indices_[j],k+1) += storage_[j] * t3;
        }
        // i+1 part
        size_t newi = i+1;
        t2 = B(newi,k);
        t3 = B(newi,k+1);       
        for (size_t j = col_indices_[newi]; j < col_indices_[newi + 1]; ++j) {
          C(row_indices_[j],k) += storage_[j] * t2;
          C(row_indices_[j],k+1) += storage_[j] * t3;
        }       
      }
      // we have 1 left!
      if(innercond < num_rows_){        
        double t2 = B(innercond,k);
        double t3 = B(innercond,k+1);
        for (size_t j = col_indices_[innercond]; j < col_indices_[innercond + 1]; ++j) {
          C(row_indices_[j],k) += storage_[j] * t2;
          C(row_indices_[j],k+1) += storage_[j] * t3;
        }    

      }
    }
    if(cond < B.num_cols()){
      for (size_t i = 0; i < num_rows_; ++i) {
        double t2 = B(i,cond);
        for (size_t j = col_indices_[i]; j < col_indices_[i + 1]; ++j) {
          C(row_indices_[j],cond) += storage_[j] * t2;
        }
      }
    }
  }

private:
  bool                is_open;
  size_t              num_rows_, num_cols_;
  std::vector<size_t> col_indices_, row_indices_;
  std::vector<double> storage_;
};

#endif    // AMATH583_CSCMATRIX_HPP
