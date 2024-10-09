
#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <stdio.h>
#include <glm/glm.hpp>

//cudaError_t addWithCuda(int* c, const int* a, const int* b, unsigned int size);


__global__ void addArray(float* A, float* B, float* C, int numElements) {
    
    int i = blockDim.x * blockIdx.x + threadIdx.x;

    if (i < numElements) {
        C[i] = A[i] + B[i] + 0.f;
    }
}
extern "C" float routine(float* A, float* B, float* C, int numElements) {
    cudaError_t err = cudaSuccess;

    size_t size = numElements * sizeof(float);

    float* d_A = NULL;
    err = cudaMalloc((void**)&d_A, size );

    float* d_B = NULL;
    err = cudaMalloc((void**)&d_B, size );

    float* d_C = NULL;
    err = cudaMalloc((void**)&d_C, size );

    err = cudaMemcpy(d_A, A, size , cudaMemcpyHostToDevice);
    err = cudaMemcpy(d_B, B, size , cudaMemcpyHostToDevice);

    int threadsPerBlock = 256;
    int blocksPerGrid = (numElements + threadsPerBlock - 1) / threadsPerBlock;

    addArray <<<blocksPerGrid, threadsPerBlock >>> (d_A, d_B, d_C, numElements);

    cudaMemcpy(C, d_C, size, cudaMemcpyDeviceToHost);

    cudaFree(d_A);
    cudaFree(d_B);
    cudaFree(d_C);

    return err;
}


