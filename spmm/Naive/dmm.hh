#ifndef DMM_HH
#define DMM_HH

#include "types.hh"

DenseMatrix_t getDense(size_t N);
void outputMatrix(std::ostream &os, DenseMatrix_t const &C,
                  std::string const &label);
DenseMatrix_t dense_matmul_AB(DenseMatrix_t const &A,
                              DenseMatrix_t const &B);
DenseMatrix_t dense_matmul_ABtrans(DenseMatrix_t const &A,
                                   DenseMatrix_t const &B);
DenseMatrix_t dense_matmul_AtransB(DenseMatrix_t const &A,
                                   DenseMatrix_t const &B);
DenseMatrix_t dense_matmul_AtransBtrans(DenseMatrix_t const &A,
                                        DenseMatrix_t const &B);

#endif // DMM_HH
