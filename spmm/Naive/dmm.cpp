#include <iomanip>
#include <iostream>

#include "dmm.hh"

//****************************************************************************
DenseMatrix_t getDense(size_t N)
{
  return DenseMatrix_t(N, std::vector<Scalar_t>(N,0));
}

//****************************************************************************
void outputMatrix(std::ostream &os, DenseMatrix_t const &C,
                  std::string const &label)
{
  os << label << std::endl;
  for (size_t i = 0; i < C.size(); ++i) {
    // We like to start with a little whitespace indent
    os << ((i == 0) ? "  [[" : "   [");

    for (size_t j = 0; j < C[i].size(); ++j) {
      os << ((j == 0) ? "" : ", " );
      os << std::setw(2) << C[i][j];
    }
    os << ((i == C.size() - 1 ) ? "]]\n" : "]\n");
  }
}

//****************************************************************************
DenseMatrix_t dense_matmul_AB(DenseMatrix_t const &A,
                              DenseMatrix_t const &B)
{
  auto C = getDense(A.size());
  for (size_t i = 0; i < A.size(); ++i) {
    for (size_t j = 0; j < B[i].size(); ++j) {
      // assert(A[i].size() == B.size())
      C[i][j] = 0;
      for (size_t k = 0; k < A[i].size(); ++k)
        C[i][j] += A[i][k] * B[k][j];
    }
  }

  //outputMatrix(std::cout, C, "Matmul dot A*B:");
  return C;
}

//****************************************************************************
DenseMatrix_t dense_matmul_ABtrans(DenseMatrix_t const &A,
                                   DenseMatrix_t const &B)
{
  auto C = getDense(A.size());
  for (size_t i = 0; i < A.size(); ++i) {
    for (size_t j = 0; j < B[i].size(); ++j) {
      C[i][j] = 0;
      for (size_t k = 0; k < A[i].size(); ++k)
        C[i][j] += A[i][k] * B[j][k];
    }
  }

  //outputMatrix(std::cout, C, "Matmul dot A*B':");
  return C;
}

//****************************************************************************
DenseMatrix_t dense_matmul_AtransB(DenseMatrix_t const &A,
                                   DenseMatrix_t const &B)
{
  auto C = getDense(A.size());
  for (size_t i = 0; i < A.size(); ++i) {
    for (size_t j = 0; j < B[i].size(); ++j) {
      C[i][j] = 0;
      for (size_t k = 0; k < A[i].size(); ++k)
        C[i][j] += A[k][i] * B[k][j];
    }
  }
  //outputMatrix(std::cout, C, "Matmul dot A'*B:");
  return C;
}

//****************************************************************************
DenseMatrix_t dense_matmul_AtransBtrans(DenseMatrix_t const &A,
                                        DenseMatrix_t const &B)
{
  auto C = getDense(A.size());
  for (size_t i = 0; i < A.size(); ++i) {
    for (size_t j = 0; j < B[i].size(); ++j) {
      C[i][j] = 0;
      for (size_t k = 0; k < A[i].size(); ++k)
        C[i][j] += A[k][i] * B[j][k];
    }
  }

  //outputMatrix(std::cout, C, "Matmul dot A'*B':");
  return C;
}
