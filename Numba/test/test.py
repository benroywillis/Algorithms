
import numba
import llvmlite.binding as llvm

# set optimization level to minimum
llvm.set_option("", "--debug-only")

M = 512
K = 512
N = 512

@numba.jit
def aTest():
	A = [float(0)] * M*K
	B = [float(0)] * K*N
	C = [float(0)] * M*N
	for i in range(M):
		for j in range(N):
			for k in range(K):
				C[i*N + j] += A[i*K + k]*B[k*N + j]
	return C

aTest()

llvm_ir_str = aTest.inspect_llvm()
#for k, v in llvm_ir_str.items():
#	print(k)
#	print(v)

with open("test.bc", "w") as f:
	for k, v in llvm_ir_str.items():
		#f.write(k)
		f.write(v)
