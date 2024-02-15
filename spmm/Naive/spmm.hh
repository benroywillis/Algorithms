#ifndef SPMM_HH
#define SPMM_HH

#include "types.hh"

void outputMatrix(std::ostream &os, Matrix_t const &M,
                  std::string const &label);
DenseMatrix_t toDense(Matrix_t const &M);
Matrix_t sparse_matmul_AB(Matrix_t const &A,
                          Matrix_t const &B);
Matrix_t sparse_matmul_ABtrans(Matrix_t const &A,
                               Matrix_t const &B);
Matrix_t sparse_matmul_AtransB(Matrix_t const &A,
                               Matrix_t const &B);
Matrix_t sparse_matmul_AtransBtrans(Matrix_t const &A,
                                    Matrix_t const &B);

#endif // SPMM_HH
