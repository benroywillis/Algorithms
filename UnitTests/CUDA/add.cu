#include <iostream>
#include <math.h>

// taken from https://developer.nvidia.com/blog/even-easier-introduction-cuda/

#define THREADS_PER_BLOCK 256

// function that adds two floats, the __global__ macro tells the CUDA compiler this can be run on a GPU
__global__
void add( int n, float* x, float* y )
{
	// CUDA kernels have 1 parameters that form a polyhedron
	// 0: a grid of blocks (x, y, z)
	// 1. a block of threads (x, y, z)
	// gridDim: number of blocks in a Dim
	// blockDim: number of threads in a block
	// blockIdx: the index of the current block
	// threadIdx: the index of the current thread

	// the size of a polyhedron
	int stride = blockDim.x*gridDim.x;
	// the index into the current polyhedron
	int index = blockIdx.x*blockDim.x + threadIdx.x;
	for( int i = index; i < n; i += stride )
	{
		y[i] = x[i] + y[i];
	}
}

int main()
{
	int N = 1 << 20;
	float* x,* y;
	//x = new float[N];
	//y = new float[N];
	// cuda equivalents
	cudaMallocManaged(&x, N*sizeof(float));
	cudaMallocManaged(&y, N*sizeof(float));

	for( int i = 0; i < N; i++ )
	{
		x[i] = 1.0f;
		y[i] = 2.0f;
	}
	//add( N, x , y );
	// cuda kernel launch indicated by the <<<>>> syntax
	// numbers mean: <<<num thread blocks, num threads in a thread block>>> and are called execution configuration
	add<<< (N + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK, THREADS_PER_BLOCK>>>( N, x , y );
	// cuda synchronization code to wait for kernel completion
	cudaDeviceSynchronize();

	float maxError = 0.0f;
	for( int i = 0; i < N; i++ )
	{
		maxError = fmax( maxError, fabs(y[i] - 3.0f ) );
	}
	std::cout << "Max error: " << maxError << std::endl;

	//delete [] x;
	//delete [] y;
	// cuda equivalents
	cudaFree(x);
	cudaFree(y);

	return 0;
}
