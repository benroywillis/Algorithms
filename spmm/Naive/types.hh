#ifndef TYPES_HH
#define TYPES_HH

#include <vector>
#include <tuple>

typedef size_t Index_t;
typedef float  Scalar_t;

typedef std::vector<std::vector<Scalar_t>> DenseMatrix_t;

typedef std::vector<std::pair<Index_t, Scalar_t> > MatrixRow_t;
typedef std::vector<MatrixRow_t> Matrix_t;

#endif // TYPES_HH
