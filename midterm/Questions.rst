

Sparse-Matrix Vector Product Performance
----------------------------------------

How does the performance (in GFLOP/s) for sparse-matrix by vector
product compare to what you previously achieved for dense-matrix by
dense-matrix product?  Explain, and quantify if you can, (e.g., using
the roofline model).

Referring to the roofline model and the sparse matrix-vector and dense matrix-matrix algorithms, what performance ratio would you theoretically expect to see if neither algorithm was able to obtain any reuse from cache?

How does the performance (in GFLOP/s) for sparse-matrix by vector
product for COO compare to CSR?  Explain, and quantify if you can,
(e.g., using the roofline model).



Sparse-Matrix Dense-Matrix Product
----------------------------------

How does the performance (in GFLOP/s) for sparse-matrix by dense
matrix product (**SPMM**) compare to sparse-matrix by vector product
(**SPMV**)? The performance for SPMM should be about the same as for
SPMV in the case of a 1 column dense matrix.  What is the trend with
increasing numbers of columns?  Explain, and quantify if you can,
using the roofline model.

How does the performance of sparse matrix by dense matrix product (in
GFLOP/s) compare to the results you got dense matrix-matrix product in
previous assignments?  Explain, and quantify if you can, using the
roofline model.


Array of Structs vs Struct of Arrays (AMATH583)
-----------------------------------------------

How does the performance
(in GFLOP/s) for sparse-matrix by vector product of struct of arrays (COOMatrix) compare to that of array of structs (AOSMatrix)? Explain what about the data layout and access pattern would result in better performance.

How does the performance
(in GFLOP/s) for sparse-matrix by dense matrix product of struct of arrays (COOMatrix) compare to that of  array of structs (AOSMatrix)?  Explain what about the data layout and access pattern would result in better performance.

About Midterm
-------------


The most important thing I learned from this assignment was ...


One thing I am still not clear on is ...