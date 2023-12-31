//
// This file is part of the course materials for AMATH483/583 at the University of Washington,
// Spring 2020
//
// Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License
// https://creativecommons.org/licenses/by-nc-sa/4.0/
//
// Author: Andrew Lumsdaine
//

#ifndef AMATH583_TEST_MULT_TRANS_HPP
#define AMATH583_TEST_MULT_TRANS_HPP

#include "Matrix.hpp"
#include "amath583.hpp"

#include "catch.hpp"

#include <cstddef>
#include "Matrix.hpp"
#include "amath583.hpp"
// remove
#include <iostream>
#include "Timer.hpp"

template <class Fun>
void run_test(Fun f) {
  for (size_t m_rows = 32; m_rows <= 2048; m_rows *= 2) {
  const size_t n_cols = 64;
  //const size_t n_cols = 32;
  Matrix A(m_rows, n_cols);
  Matrix B(m_rows, n_cols);
  randomize(A);
  randomize(B);

  Matrix C = transpose(B);
  REQUIRE(f_norm(C) != 0.0);

  Matrix E(m_rows, m_rows);
  Matrix F(m_rows, m_rows);
  REQUIRE(f_norm(E) == 0.0);
  REQUIRE(f_norm(F) == 0.0);

  mult_0(A, C, E);
  f(A, B, F);
  REQUIRE(f_norm(E) != 0.0);
  REQUIRE(f_norm(F) != 0.0);
  REQUIRE(f_norm(E-F) < 1.e-12*f_norm(E)); 

  // multiply A with A_T
  // multiply A with A_T but do not pass A_T
  Matrix D(m_rows, n_cols);
  Matrix G(m_rows, m_rows);
  Matrix H(m_rows, m_rows);
  D = transpose(A);
  mult_0(A, D, G);
  Timer t;
  t.start();
  f(A, A, H);
  t.stop();
  std::cout<< "Time elapsed: " << t.elapsed();
  // std::cout << "Printing G " << std::endl;
  // for(size_t i = 0; i < G.num_rows(); i++){
  //   std::cout << std::endl;
  //   for(size_t j = 0; j < G.num_cols(); j++){
  //     std::cout<< G(i,j) << " ";
  //   }
  // }
  //   std::cout << "Printing H " << std::endl;

  // for(size_t i = 0; i < H.num_rows(); i++){
  //   std::cout << std::endl;
  //   for(size_t j = 0; j < H.num_cols(); j++){
  //     std::cout<< H(i,j) << " ";
  //   }
  // }

  REQUIRE(f_norm(G) != 0.0);
  REQUIRE(f_norm(H) != 0.0);
  REQUIRE(f_norm(G-H) < 1.e-12*f_norm(G)); 

  
  }
}

#endif // AMATH583_TEST_MULT_TRANS_HPP
