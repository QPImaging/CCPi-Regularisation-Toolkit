#ifndef __DIFF_HO_H_
#define __DIFF_HO_H_

extern "C" void Diff4th_GPU_kernel(float* A, float* B, int N, int M, int Z, float sigma, int iter, float tau, float lambda);

#endif 
