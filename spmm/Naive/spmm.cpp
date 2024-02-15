#include <iostream>
#include <iomanip>
#include <utility>

#include "dmm.hh"
#include "spmm.hh"

//****************************************************************************
DenseMatrix_t toDense(Matrix_t const &M)
{
  auto C = getDense(M.size());
  Index_t num_rows(M.size());

  for (Index_t row_idx = 0; row_idx < num_rows; ++row_idx) {
    if (M[row_idx].empty()) continue;

    // Walk the columns.  A sparse iter would be handy here...
    auto const &row(M[row_idx]);
      
    auto row_it = row.begin();
    while (row_it != row.end()) {
      Index_t col_idx  = std::get<0>(*row_it);
      Scalar_t cell_val = std::get<1>(*row_it);

      C[row_idx][col_idx] = cell_val;
      ++row_it;
    }
  }
  return C;
}

//****************************************************************************
void outputMatrix(std::ostream &os, Matrix_t const &M, std::string const &label)
{
  Index_t num_rows(M.size());
  Index_t num_cols(num_rows);  // assume square for now

  os << label << std::endl;
  for (Index_t row_idx = 0; row_idx < num_rows; ++row_idx) {
    // We like to start with a little whitespace indent
    os << ((row_idx == 0) ? "  [[" : "   [");

    Index_t curr_idx = 0;

    if (M[row_idx].empty()) {
      while (curr_idx < num_cols) {
        os << ((curr_idx == 0) ? "  " : ",   " ); // no comma for first
        ++curr_idx;
      }
    } else {
      // Walk the columns.  A sparse iter would be handy here...
      Index_t col_idx;
      Scalar_t cell_val;
      
      auto const &row(M[row_idx]);
      
      auto row_it = row.begin();
      while (row_it != row.end()) {
        col_idx  = std::get<0>(*row_it);
        cell_val = std::get<1>(*row_it);
        while (curr_idx < col_idx) {
          os << ((curr_idx == 0) ? "  " : ",   " ); // no comma for first
          ++curr_idx;
        }
      
        if (curr_idx != 0)
          os << ", ";
        os << std::setw(2) << cell_val;
      
        ++row_it;
        ++curr_idx;
      }
      
      // Fill in the rest to the end
      while (curr_idx < num_cols) {
        os << ",   ";
        ++curr_idx;
      }
    }
    os << ((row_idx == num_rows - 1 ) ? "]]\n" : "]\n");
  }
}


//****************************************************************************
Matrix_t sparse_matmul_AB(Matrix_t const &A,
                          Matrix_t const &B)
{
  Index_t const N(A.size()); // assume NxN for now

  Matrix_t C(N);

  for (Index_t i = 0; i < N; ++i) {
    C[i].clear();

    for (auto const &Ai_elt : A[i]) { // elements in row
      Index_t k = std::get<0>(Ai_elt);
      Scalar_t a_ik = std::get<1>(Ai_elt);
      if (!B[k].empty()) {
        auto Ci_iter = C[i].begin();

        for (auto const &Bk_elt: B[k]) {
          Index_t j = std::get<0>(Bk_elt);
          Scalar_t b_kj(std::get<1>(Bk_elt));
          // scan forward in C[i] until j element found or passed
          Index_t j_C;
          while ((Ci_iter != C[i].end()) &&
                 ((j_C = std::get<0>(*Ci_iter)) < j)) {
            //skipping C[i][j]
            ++Ci_iter;
          }

          Scalar_t tmp = a_ik*b_kj;
          if (Ci_iter == C[i].end()) {
            C[i].push_back(std::make_pair(j, tmp));
            Ci_iter = C[i].end();
          } else if (std::get<0>(*Ci_iter) == j) {
            std::get<1>(*Ci_iter) += tmp;
          } else {
            Ci_iter = C[i].insert(Ci_iter, std::make_pair(j, tmp));
          }
        }
      }
    }
  }

  return C;
}


//****************************************************************************
Matrix_t sparse_matmul_ABtrans(Matrix_t const &A,
                               Matrix_t const &B)
{
  // for i
  //   for j
  //     for k
  //       c[i][j] += a[i][k]*b[j][k]

  Index_t const N(A.size()); // assume A and B are NxN for now

  Matrix_t C(N);
 
  // Task 21 
  for (Index_t i = 0; i < N; ++i) {
    // clear row i of C, or set up a temporary
    C[i].clear();

    if (A[i].empty()) continue; // no-op if there are no entries in the row 
	// Task 16
    for (Index_t j = 0; j < N; j++) { 

      if (B[j].empty()) continue; // cycle loop if no j row of B

      // find k's st A[i][k] != 0 and B[j][k] != 0
      auto Akit = A[i].begin();
      auto Bkit = B[j].begin();
      Scalar_t cij = 0;
	 
	  // Task 2
      while (Akit != A[i].end() && Bkit != B[j].end()) { // k-loop
        auto ak = std::get<0>(*Akit);
        auto bk = std::get<0>(*Bkit);

        if (ak > bk) {
          Bkit++; // col idx of A greater than col idx of B, increase B
        } else if (ak < bk) {
          Akit++; // col idx of A less than col idx of B, increase A
        } else {
          // ak == bk, record contribution
   		  // (this avenue is never taken in Task 2)
          cij += std::get<1>(*Akit) * std::get<1>(*Bkit);
          Akit++;
          Bkit++;
        }
      }
      // record the value. not densifying bc broke out of loop for empty rows
      // If nonzero numbers add to 0, this will put a zero in the matrix.

	  // this is instantiated in the LLVM IR as a temporary (pointer %4), cij is a phi (%106) and when this call is made, %4 is passed to the push back method
      // Cyclebite-template doesn't recognize the temporary (%4) because its only a 12-byte allocation
	  // but that temporary is essential to model the data flow of the task
	  // -- this is a case we haven't accounted for yet
	  //    --- the assumption was that the FGT-level data flow within each task would not use the stack, but STL is doing just that
      //        -> the fact that the task uses the stack is irrelevant, we just need to track the value that goes into the stack store... this will capture all the code that goes into the push
	  //        PROBLEM: when this pair is pushed into the vector, we don't have any record of that value going into the base pointer... because the .push_back() method is empty... so how do we follow the data flow from FGT to storage in the BP?
      C[i].push_back(std::make_pair(j,cij));
    }
  }
  //outputMatrix(std::cout, C, "C = A*B' =");

  return C;
}

//****************************************************************************
void mergeRows(MatrixRow_t &l, MatrixRow_t &r)
{
  if (l.empty()) {
    l.swap(r);
    return;
  }

  if (r.empty()) {
    return;
  }

  MatrixRow_t tmp;

  auto l_it = l.begin();
  auto r_it = r.begin();
  while ((l_it != l.end()) && (r_it != r.end())) {
    Index_t li = std::get<0>(*l_it);
    Index_t ri = std::get<0>(*r_it);
    if (li < ri) {
      tmp.push_back(*l_it);
      ++l_it;
    } else if (ri < li) {
      tmp.push_back(*r_it);
      ++r_it;
    } else {
      tmp.push_back(std::make_pair(li,
                                   std::get<1>(*l_it) + std::get<1>(*r_it)));
      ++l_it;
      ++r_it;
    }
  }

  while (l_it != l.end()) {
    tmp.push_back(*l_it);  ++l_it;
  }

  while (r_it != r.end()) {
    tmp.push_back(*r_it);  ++r_it;
  }

  l.swap(tmp);
}

//****************************************************************************
#if 0
Matrix_t sparse_matmul_AtransB(Matrix_t const &A,
                               Matrix_t const &B)
{
  Index_t const N(A.size()); // assume NxN for now

  MatrixRow_t Ci_tmp;
  Matrix_t C(N);

  for (Index_t k = 0; k < N; ++k) {
    C[k].clear();
  }
  
  for (Index_t k = 0; k < N; ++k) {
    
    if (!A[k].empty() && !B[k].empty()) {
      // get row k of A
      auto Ak_it(A[k].begin());

      while (Ak_it != A[k].end()) {
        Index_t i = std::get<0>(*Ak_it);
        float a_ki = std::get<1>(*Ak_it);

        // get row k of B
        auto Bk_it(B[k].begin());

        Ci_tmp.clear();
        while (Bk_it != B[k].end()) {
          Index_t j = std::get<0>(*Bk_it);
          Ci_tmp.push_back(std::make_pair(j, a_ki*std::get<1>(*Bk_it)));
          ++Bk_it;
        }
        mergeRows(C[i], Ci_tmp);
        ++Ak_it;
      }
    }
  }

  return C;
}
#else
Matrix_t sparse_matmul_AtransB(Matrix_t const &A,
                               Matrix_t const &B)
{
  // for k
  //   for i
  //     for j
  //       c[i][j] += a[k][i] * b[k][j]

  Index_t const N(A.size()); // assume NxN for now
  Matrix_t C(N);
  for (Index_t k = 0; k < N; ++k) { // row of A,B
    if (A[k].empty() or B[k].empty()) continue; // skip empty rows of A or B

    auto Akit = A[k].begin();
    for (Index_t i = 0; i < N; ++i) { // row of C

      // Find A[k][i]
      auto aki_idx = std::get<0>(*Akit);
      while (aki_idx < i and Akit != A[k].end()) {
        Akit++;
        if (Akit != A[k].end()) {
          aki_idx = std::get<0>(*Akit);
        } else {
          // hit A[k].end(). Don't deference for aki_idx. set not equal i
          aki_idx = i+1;
        }
      }
      if (aki_idx != i) continue; // a[k][i] equals zero, move to next i

      // find b[k][j], and update c[i][j]
      auto Ciit = C[i].begin();
      auto Bkit = B[k].begin();
      while (Bkit != B[k].end()) { // j loop
        // every b makes a contribution since getting here means a[k][i] != 0
        Scalar_t cij_incr = std::get<1>(*Akit)*std::get<1>(*Bkit);

        auto bkj_idx = std::get<0>(*Bkit);
        if (C[i].empty()) {
          C[i].push_back(std::make_pair(bkj_idx,0.0)); // initialize to zero
          Ciit = C[i].begin();
        }

        // look through elements of C[i] and compare indices
        Index_t cij_idx = std::get<0>(*Ciit); // column index for C
        while (cij_idx < bkj_idx and Ciit != C[i].end()) {
          Ciit++;
          if (Ciit != C[i].end()) {
            cij_idx = std::get<0>(*Ciit);
          } else {
            // hit C[i].end(). Don't deference for cij_idx. set not equal bkj
            cij_idx = bkj_idx+1;
          }
        }

        if (cij_idx == bkj_idx) { // column index C and B the same, add it
          std::get<1>(*Ciit) += cij_incr;
        }
        else { // insert at iterator location
          Ciit = C[i].insert(Ciit,std::make_pair(bkj_idx,cij_incr));
        }

        // move to next b
        Bkit++;
      }
    }
  }
  //outputMatrix(std::cout, C, "tmp");

  return C;
}
#endif

//****************************************************************************
Matrix_t sparse_matmul_AtransBtrans(Matrix_t const &A,
                                    Matrix_t const &B)
{
  Index_t const N(A.size()); // assume NxN for now
  // clear all of C
  Matrix_t C(N);

  for (Index_t j = 0; j < N; ++j) {
    // get row j of B
    if (!B[j].empty()) {
      for (auto const &Bj_elt : B[j]) {
        Index_t k(std::get<0>(Bj_elt));
        auto b_jk(std::get<1>(Bj_elt));

        if (!A[k].empty()) {
          for (auto const &Ak_elt : A[k]) {
            Index_t i(std::get<0>(Ak_elt));
            auto a_ki(std::get<1>(Ak_elt));

            if (!C[i].empty() && (std::get<0>(C[i].back()) == j)) {
              std::get<1>(C[i].back()) += b_jk * a_ki;
            } else {
              C[i].push_back(std::make_pair(j, b_jk * a_ki));
            }
          }
        }
      }
    }
  }

  return C;
}

#if 0
//****************************************************************************
void matmul_AtransBtrans(DenseMatrix_t const &A,
                         DenseMatrix_t const &B)
{
  std::cout << "matmul(A', B')" << std::endl;

  auto C = getDense();
  //assert(??)
  for (size_t i = 0; i < B.size(); ++i) {
    // clear row i of C, or set up a temporary
    for (size_t j = 0; j < A[0].size(); ++j) {
      C[i][j] = 0;  // clear row i of C
    }
  }

  for (size_t j = 0; j < B.size(); ++j) {
    // get row j of B, B[j,:]
    for (size_t k = 0; k < B[j].size(); ++k) {
      // get row k of A
      std::cout << " Read B " << j << " " << k << std::endl;
      float b_jk = B[j][k];

      for (size_t i = 0; i < A[k].size(); ++i) {
        std::cout << " Read A " << k << " " << i << std::endl;
        C[i][j] += b_jk * A[k][i];
        std::cout << "Write C " << i << " " << j << std::endl;
      }
    }
  }
  std::cout << "Matmul A'*B':" << std::endl;
  outputMatrix(C);
}
#endif

#if 0
//****************************************************************************
template <typename ScalarT, typename ReduceT>
class reducing_output_iterator :
  public std::iterator<std::random_access_iterator_tag, ScalarT>
{
public:
  reducing_output_iterator(ScalarT &reduction) : m_reduction{reduction} {}
  void                      operator++() {}
  void                      operator++(int) {}
  reducing_output_iterator& operator*() { return *this; }
  reducing_output_iterator& operator[](size_t) { return *this; }

  template<typename T>
  void operator=(T &&multiplicand)
  {
    m_reduction = ReduceT()(m_reduction, multiplicand);
  }
  ScalarT get_reduction() { return m_reduction; }

private:
  ScalarT &m_reduction;
};
#endif

#if 0
//****************************************************************************
// Just like std::set_intersection except with the addition of a mapper
template<class InputIt1, class InputIt2,
         class OutputIt, class Compare, class Map>
OutputIt set_intersection(InputIt1 begin1, InputIt1 end1,
                          InputIt2 begin2, InputIt2 end2,
                          OutputIt reducer, Compare comp, Map mapper)
{
  while (begin1 != end1 && begin2 != end2) {
    if (comp(*begin1, *begin2)) {
      ++begin1;
    } else {
      if (!comp(*begin2, *begin1)) {
        *reducer = mapper(*begin1, *begin2);
        reducer++;
      }
      ++begin2;
    }
  }
  return reducer;
}
#endif

#if 0
//****************************************************************************
// @todo templatize on semiring operations
Matrix_t sparse_matmul_ABtrans_intersection(Matrix_t const &A,
                                            Matrix_t const &B)
{
  Index_t const N(A.size()); // assume A and B are NxN for now

  Matrix_t C(N);
  //std::cout << "C.size() = " << C.size() << std::endl;

  for (Index_t i = 0; i < N; ++i) {
    //std::cout << "i = " << i << std::endl;
    // clear row i of C, or set up a temporary
    C[i].clear();

    if (!A[i].empty()) {
      // get row i of A
      //for (size_t k = 0; k < A[i].size(); ++k)
      //    std::cout << "A["<<i<<"]=";
      //for (auto const &aik : A[i])
      //    std::cout << " " << std::get<0>(aik) << ":" << std::get<1>(aik);
      //std::cout << std::endl;

      for (Index_t j = 0; j < N; ++j) {
        //std::cout << "\tj = " << j << std::endl;
        // get row j of B
        if (!B[j].empty()) {
          float c_ij = 0;
          reducing_output_iterator<float, std::plus<float>> reducer(c_ij);

          // Perform the "dot product" between row A[i] and row B[j]
          ::set_intersection(A[i].begin(), A[i].end(),
                             B[j].begin(), B[j].end(),
                             reducer,
                             [](auto &&x, auto &&y) -> bool {
                               return std::get<0>(x) < std::get<0>(y);
                             },
                             [](auto&&x, auto &&y) -> float {
                               return std::get<1>(x) * std::get<1>(y);
                             }
                             );

          //std::cout << " = " << c_ij << std::endl;
          C[i].push_back(std::make_pair(j, c_ij));
        }
      }
    }
  }
  outputMatrix(std::cout, C, "C = A*B' =");
  std::cout << "C.size() = " << C.size() << std::endl;
  return C;
}

#endif
//****************************************************************************
