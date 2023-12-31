
PS2 Questions
=============

Add your answers to this file in plain text after each question.  Leave a blank line between the text of the question and the text of your answer and at least two lines between your answer and the next question.

.. role:: cpp(code)
   :language: c++


Inner Product
-------------

1. How would you use :cpp:`dot` to implement :cpp:`two_norm`?
To implement two_norm using dot, we take the dot product of the vector with itself. After this, we take the square root
of the result. The two_norm_d function in amath583.cpp implements this logic. 


Tensors
-------

2. What is the formula that we would use for indexing if we wanted "slab-major" ordering?  Hint:  What is the size of a slab?  It might be helpful to draw a picture of the data in 3D and see what the size is of a slab.
We want to do slab-major ordering. We assume the dimensions of Tensor are [L,M,N]. L=num_slabs, M=num_rows, N=num_cols.
For a 2d row-major matrix with dimensions [num_rows, num_columns], we know element [i,j] = j + i*num_cols. j is the index that is used to index 
within a row and i is used to jump rows.
Let us assume we want to find location of element [i,j,k] in the Tensor in slab-major order.
k will be used to index within slab, now think of each slab as a matrix.
i*num_rows will be used to jump cols and then j will be used to index within a row so we have j + i*num_rows
Finally, we multiply this by num_cols to jump rows to give us num_cols *(j + i*num_rows)

so indexing strategy is k + num_cols *(j + i*num_rows)


3. (Extra credit.) I claim that there are six distinct indexing formulas.  Show that this claim is correct or that it is incorrect.
Tensor has 3 dimensions, and each element is uniquely identified by specifying a value for each of the 3 dimensions. The order of 
specifying these could be anything thereby leading us to 3! combinations, which is 6 in total.