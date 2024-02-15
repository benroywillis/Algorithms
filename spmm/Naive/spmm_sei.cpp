#include <cassert>
#include <iostream>

#include "dmm.hh"
#include "spmm.hh"

#ifndef SIZE
#define SIZE 	256
#endif

void ASSERT_EQ_DENSE(Matrix_t const & A, DenseMatrix_t const & B) {
    /*DenseMatrix_t dense_A = toDense(A);
    assert (A.size() == B.size());
    for (Index_t i=0; i<A.size(); i++) {
        assert (A[i].size() == B[i].size());
        for (Index_t j=0; j<A[i].size(); j++) {
            assert (A[i][j] == B[i][j]);
        }
    }*/
}

int main(int argc, char **argv)
{
    /*size_t N = 4;
    Matrix_t A = {{{0, 1.f}, {1, 2.f}, {2, 3.f}},
                  {          {1, 4.f}, {2, 1.f}},
                  {{0, 2.f}, {1, 1.f},           {3, 1.f}},
                  {          {1, 1.f}, {2, 2.f}, {3, 1.f}}};

#ifdef __TESTING__
    DenseMatrix_t A_dense = {{1, 2, 3, 0},
                             {0, 4, 1, 0},
                             {2, 1, 0, 1},
                             {0, 1, 2, 1}};
#endif

    Matrix_t B = {{{0, 1.f},           {2, 2.f}},
                  {{0, 2.f}, {1, 4.f}, {2, 1.f}, {3, 1.f}},
                  {{0, 3.f}, {1, 1.f},           {3, 2.f}},
                  {                    {2, 1.f}, {3, 1.f}}};

#ifdef __TESTING__
    DenseMatrix_t B_dense = {{1, 0, 2, 0},
                             {2, 4, 1, 1},
                             {3, 1, 0, 2},
                             {0, 0, 1, 1}};
#endif
	*/
	Matrix_t A;
	A.reserve(SIZE);
	Matrix_t B;
	B.reserve(SIZE);
	for( unsigned i = 0 ; i < SIZE; i++ ) {
		A.push_back(MatrixRow_t());
		B.push_back(MatrixRow_t());
		for( unsigned j = 0; j < SIZE; j++ ) {
			A[i].push_back(std::pair<int, float>(j, rand()));
			B[i].push_back(std::pair<int, float>(j, rand()));
		}
	}
	std::cout << "Matrix sizes are " << A.size() << ", " << A.front().size() << std::endl;
    Matrix_t C(sparse_matmul_AB(A, B));
    //outputMatrix(std::cout, C, "C = ");

#ifdef __TESTING__
    DenseMatrix_t C_dense = dense_matmul_AB(A_dense, B_dense);
    // compare
    ASSERT_EQ_DENSE(C,C_dense);
#endif

    Matrix_t D(sparse_matmul_ABtrans(A, B));
    //outputMatrix(std::cout, D, "D = ");

#ifdef __TESTING__
    DenseMatrix_t D_dense = dense_matmul_ABtrans(A_dense, B_dense);
    // compare
    ASSERT_EQ_DENSE(D,D_dense);
#endif

    Matrix_t E(sparse_matmul_AtransB(A, B));
    //outputMatrix(std::cout, E, "E = ");

#ifdef __TESTING__
    DenseMatrix_t E_dense = dense_matmul_AtransB(A_dense, B_dense);
    // compare
    ASSERT_EQ_DENSE(E,E_dense);
#endif

    Matrix_t F(sparse_matmul_AtransBtrans(A, B));
    //outputMatrix(std::cout, F, "F = ");

#ifdef __TESTING__
    DenseMatrix_t F_dense = dense_matmul_AtransBtrans(A_dense, B_dense);
    // compare
    ASSERT_EQ_DENSE(F,F_dense);
#endif

    return 0;
}
