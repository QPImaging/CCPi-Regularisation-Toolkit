/*
 * This work is part of the Core Imaging Library developed by
 * Visual Analytics and Imaging System Group of the Science Technology
 * Facilities Council, STFC
 *
 * Copyright 2017 Daniil Kazantsev
 * Copyright 2017 Srikanth Nagella, Edoardo Pasca
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "Diffus4th_order_core.h"
#include "utils.h"

#define EPS 1.0e-7

/* C-OMP implementation of fourth-order diffusion scheme [1] for piecewise-smooth recovery (2D/3D case)
 * The minimisation is performed using explicit scheme.
 *
 * Input Parameters:
 * 1. Noisy image/volume
 * 2. lambda - regularization parameter
 * 3. Edge-preserving parameter (sigma)
 * 4. Number of iterations, for explicit scheme >= 150 is recommended
 * 5. tau - time-marching step for the explicit scheme
 * 6. eplsilon: tolerance constant
 *
 * Output:
 * [1] Regularized image/volume
 * [2] Information vector which contains [iteration no., reached tolerance]
 *
 * This function is based on the paper by
 * [1] Hajiaboli, M.R., 2011. An anisotropic fourth-order diffusion filter for image noise removal. International Journal of Computer Vision, 92(2), pp.177-191.
 */

float Diffus4th_CPU_main(float *Input, float *Output, float *infovector, float lambdaPar, float sigmaPar, int iterationsNumb, float tau, float epsil, int dimX, int dimY, int dimZ)
{
    int i,DimTotal,j,count;
    float sigmaPar2, re, re1;
    re = 0.0f; re1 = 0.0f;
    count = 0;
    float *W_Lapl=NULL, *Output_prev=NULL;
    sigmaPar2 = sigmaPar*sigmaPar;
    DimTotal =  dimX*dimY*dimZ;
    
    W_Lapl = calloc(DimTotal, sizeof(float));
    
    if (epsil != 0.0f) Output_prev = calloc(DimTotal, sizeof(float));
    
    /* copy into output */
    copyIm(Input, Output, (long)(dimX), (long)(dimY), (long)(dimZ));
    
    for(i=0; i < iterationsNumb; i++) {
        if ((epsil != 0.0f) && (i % 5 == 0)) copyIm(Output, Output_prev, (long)(dimX), (long)(dimY), (long)(dimZ));
        
        if (dimZ == 1) {
            /* running 2D diffusion iterations */
            /* Calculating weighted Laplacian */
            Weighted_Laplc2D(W_Lapl, Output, sigmaPar2, dimX, dimY);
            /* Perform iteration step */
            Diffusion_update_step2D(Output, Input, W_Lapl, lambdaPar, sigmaPar2, tau, (long)(dimX), (long)(dimY));
        }
        else {
            /* running 3D diffusion iterations */
            /* Calculating weighted Laplacian */
            Weighted_Laplc3D(W_Lapl, Output, sigmaPar2, dimX, dimY, dimZ);
            /* Perform iteration step */
            Diffusion_update_step3D(Output, Input, W_Lapl, lambdaPar, sigmaPar2, tau, (long)(dimX), (long)(dimY), (long)(dimZ));
        }
        
        /* check early stopping criteria */
        if ((epsil != 0.0f) && (i % 5 == 0)) {
            re = 0.0f; re1 = 0.0f;
            for(j=0; j<DimTotal; j++)
            {
                re += powf(Output[j] - Output_prev[j],2);
                re1 += powf(Output[j],2);
            }
            re = sqrtf(re)/sqrtf(re1);
            if (re < epsil)  count++;
            if (count > 3) break;
        }
    }
    free(W_Lapl);
    
    if (epsil != 0.0f) free(Output_prev);
    /*adding info into info_vector */
    infovector[0] = (float)(i);  /*iterations number (if stopped earlier based on tolerance)*/
    infovector[1] = re;  /* reached tolerance */
    return 0;
}
/********************************************************************/
/***************************2D Functions*****************************/
/********************************************************************/
float Weighted_Laplc2D(float *W_Lapl, float *U0, float sigma, long dimX, long dimY)
{
    long i,j,i1,i2,j1,j2,index;
    float gradX, gradX_sq, gradY, gradY_sq, gradXX, gradYY, gradXY, xy_2, denom, V_norm, V_orth, c, c_sq;
    
#pragma omp parallel for shared(W_Lapl) private(i,j,i1,i2,j1,j2,index,gradX, gradX_sq, gradY, gradY_sq, gradXX, gradYY, gradXY, xy_2, denom, V_norm, V_orth, c, c_sq)
    for(j=0; j<dimY; j++) {
        /* symmetric boundary conditions */
        j1 = j+1; if (j1 == dimY) j1 = j-1;
        j2 = j-1; if (j2 < 0) j2 = j+1;
        for(i=0; i<dimX; i++) {
            /* symmetric boundary conditions */
            i1 = i+1; if (i1 == dimX) i1 = i-1;
            i2 = i-1; if (i2 < 0) i2 = i+1;
            index = j*dimX+i;
            
            gradX = 0.5f*(U0[j*dimX+i2] - U0[j*dimX+i1]);
            gradX_sq = gradX*gradX;
            
            gradY = 0.5f*(U0[j2*dimX+i] - U0[j1*dimX+i]);
            gradY_sq = gradY*gradY;
            
            gradXX = U0[j*dimX+i2] + U0[j*dimX+i1] - 2*U0[index];
            gradYY = U0[j2*dimX+i] + U0[j1*dimX+i] - 2*U0[index];
            
            gradXY = 0.25f*(U0[j2*dimX+i2] + U0[j1*dimX+i1] - U0[j1*dimX+i2] - U0[j2*dimX+i1]);
            xy_2 = 2.0f*gradX*gradY*gradXY;
            
            denom =  gradX_sq + gradY_sq;
            
            if (denom <= EPS) {
                V_norm = (gradXX*gradX_sq + xy_2 + gradYY*gradY_sq)/EPS;
                V_orth = (gradXX*gradY_sq - xy_2 + gradYY*gradX_sq)/EPS;
            }
            else  {
                V_norm = (gradXX*gradX_sq + xy_2 + gradYY*gradY_sq)/denom;
                V_orth = (gradXX*gradY_sq - xy_2 + gradYY*gradX_sq)/denom;
            }
            
            c = 1.0f/(1.0f + denom/sigma);
            c_sq = c*c;
            
            W_Lapl[index] = c_sq*V_norm + c*V_orth;
        }}
    return *W_Lapl;
}

float Diffusion_update_step2D(float *Output, float *Input, float *W_Lapl, float lambdaPar, float sigmaPar2, float tau, long dimX, long dimY)
{
    long i,j,i1,i2,j1,j2,index;
    float gradXXc, gradYYc;
    
#pragma omp parallel for shared(Output, Input, W_Lapl) private(i,j,i1,i2,j1,j2,index,gradXXc,gradYYc)
    for(j=0; j<dimY; j++) {
        /* symmetric boundary conditions */
        j1 = j+1; if (j1 == dimY) j1 = j-1;
        j2 = j-1; if (j2 < 0) j2 = j+1;
        for(i=0; i<dimX; i++) {
            /* symmetric boundary conditions */
            i1 = i+1; if (i1 == dimX) i1 = i-1;
            i2 = i-1; if (i2 < 0) i2 = i+1;
            index = j*dimX+i;
            
            gradXXc = W_Lapl[j*dimX+i2] + W_Lapl[j*dimX+i1] - 2*W_Lapl[index];
            gradYYc = W_Lapl[j2*dimX+i] + W_Lapl[j1*dimX+i] - 2*W_Lapl[index];
            
            Output[index] += tau*(-lambdaPar*(gradXXc + gradYYc) - (Output[index] - Input[index]));
        }}
    return *Output;
}
/********************************************************************/
/***************************3D Functions*****************************/
/********************************************************************/
float Weighted_Laplc3D(float *W_Lapl, float *U0, float sigma, long dimX, long dimY, long dimZ)
{
    long i,j,k,i1,i2,j1,j2,k1,k2,index;
    float gradX, gradX_sq, gradY, gradY_sq, gradXX, gradYY, gradXY, xy_2, denom, V_norm, V_orth, c, c_sq, gradZ, gradZ_sq, gradZZ, gradXZ, gradYZ, xyz_1, xyz_2;
    
#pragma omp parallel for shared(W_Lapl) private(i,j,k,i1,i2,j1,j2,k1,k2,index,gradX, gradX_sq, gradY, gradY_sq, gradXX, gradYY, gradXY, xy_2, denom, V_norm, V_orth, c, c_sq, gradZ, gradZ_sq, gradZZ, gradXZ, gradYZ, xyz_1, xyz_2)
    for(k=0; k<dimZ; k++) {
        /* symmetric boundary conditions */
        k1 = k+1; if (k1 == dimZ) k1 = k-1;
        k2 = k-1; if (k2 < 0) k2 = k+1;
        for(j=0; j<dimY; j++) {
            /* symmetric boundary conditions */
            j1 = j+1; if (j1 == dimY) j1 = j-1;
            j2 = j-1; if (j2 < 0) j2 = j+1;
            for(i=0; i<dimX; i++) {
                /* symmetric boundary conditions */
                i1 = i+1; if (i1 == dimX) i1 = i-1;
                i2 = i-1; if (i2 < 0) i2 = i+1;
                
                index = (dimX*dimY)*k + j*dimX+i;
                
                gradX = 0.5f*(U0[(dimX*dimY)*k + j*dimX+i2] - U0[(dimX*dimY)*k + j*dimX+i1]);
                gradX_sq = gradX*gradX;
                
                gradY = 0.5f*(U0[(dimX*dimY)*k + j2*dimX+i] - U0[(dimX*dimY)*k + j1*dimX+i]);
                gradY_sq = gradY*gradY;
                
                gradZ = 0.5f*(U0[(dimX*dimY)*k2 + j*dimX+i] - U0[(dimX*dimY)*k1 + j*dimX+i]);
                gradZ_sq = gradZ*gradZ;
                
                gradXX = U0[(dimX*dimY)*k + j*dimX+i2] + U0[(dimX*dimY)*k + j*dimX+i1] - 2*U0[index];
                gradYY = U0[(dimX*dimY)*k + j2*dimX+i] + U0[(dimX*dimY)*k + j1*dimX+i] - 2*U0[index];
                gradZZ = U0[(dimX*dimY)*k2 + j*dimX+i] + U0[(dimX*dimY)*k1 + j*dimX+i] - 2*U0[index];
                
                gradXY = 0.25f*(U0[(dimX*dimY)*k + j2*dimX+i2] + U0[(dimX*dimY)*k + j1*dimX+i1] - U0[(dimX*dimY)*k + j1*dimX+i2] - U0[(dimX*dimY)*k + j2*dimX+i1]);
                gradXZ = 0.25f*(U0[(dimX*dimY)*k2 + j*dimX+i2] - U0[(dimX*dimY)*k2+j*dimX+i1] - U0[(dimX*dimY)*k1+j*dimX+i2] + U0[(dimX*dimY)*k1+j*dimX+i1]);
                gradYZ = 0.25f*(U0[(dimX*dimY)*k2 +j2*dimX+i] - U0[(dimX*dimY)*k2+j1*dimX+i] - U0[(dimX*dimY)*k1+j2*dimX+i] + U0[(dimX*dimY)*k1+j1*dimX+i]);
                
                xy_2  = 2.0f*gradX*gradY*gradXY;
                xyz_1 = 2.0f*gradX*gradZ*gradXZ;
                xyz_2 = 2.0f*gradY*gradZ*gradYZ;
                
                denom =  gradX_sq + gradY_sq + gradZ_sq;
                
                if (denom <= EPS) {
                    V_norm = (gradXX*gradX_sq + gradYY*gradY_sq + gradZZ*gradZ_sq + xy_2 + xyz_1 + xyz_2)/EPS;
                    V_orth = ((gradY_sq + gradZ_sq)*gradXX + (gradX_sq + gradZ_sq)*gradYY + (gradX_sq + gradY_sq)*gradZZ - xy_2 - xyz_1 - xyz_2)/EPS;
                }
                else  {
                    V_norm = (gradXX*gradX_sq + gradYY*gradY_sq + gradZZ*gradZ_sq + xy_2 + xyz_1 + xyz_2)/denom;
                    V_orth = ((gradY_sq + gradZ_sq)*gradXX + (gradX_sq + gradZ_sq)*gradYY + (gradX_sq + gradY_sq)*gradZZ - xy_2 - xyz_1 - xyz_2)/denom;
                }
                
                c = 1.0f/(1.0f + denom/sigma);
                c_sq = c*c;
                
                W_Lapl[index] = c_sq*V_norm + c*V_orth;
            }}}
    return *W_Lapl;
}

float Diffusion_update_step3D(float *Output, float *Input, float *W_Lapl, float lambdaPar, float sigmaPar2, float tau, long dimX, long dimY, long dimZ)
{
    long i,j,i1,i2,j1,j2,index,k,k1,k2;
    float gradXXc, gradYYc, gradZZc;
    
#pragma omp parallel for shared(Output, Input, W_Lapl) private(i,j,i1,i2,j1,j2,k,k1,k2,index,gradXXc,gradYYc,gradZZc)
    for(k=0; k<dimZ; k++) {
        /* symmetric boundary conditions */
        k1 = k+1; if (k1 == dimZ) k1 = k-1;
        k2 = k-1; if (k2 < 0) k2 = k+1;
        for(j=0; j<dimY; j++) {
            /* symmetric boundary conditions */
            j1 = j+1; if (j1 == dimY) j1 = j-1;
            j2 = j-1; if (j2 < 0) j2 = j+1;
            for(i=0; i<dimX; i++) {
                /* symmetric boundary conditions */
                i1 = i+1; if (i1 == dimX) i1 = i-1;
                i2 = i-1; if (i2 < 0) i2 = i+1;
                
                index = (dimX*dimY)*k + j*dimX+i;
                
                gradXXc = W_Lapl[(dimX*dimY)*k + j*dimX+i2] + W_Lapl[(dimX*dimY)*k + j*dimX+i1] - 2*W_Lapl[index];
                gradYYc = W_Lapl[(dimX*dimY)*k + j2*dimX+i] + W_Lapl[(dimX*dimY)*k + j1*dimX+i] - 2*W_Lapl[index];
                gradZZc = W_Lapl[(dimX*dimY)*k2 + j*dimX+i] + W_Lapl[(dimX*dimY)*k1 + j*dimX+i] - 2*W_Lapl[index];
                
                Output[index] += tau*(-lambdaPar*(gradXXc + gradYYc + gradZZc) - (Output[index] - Input[index]));
            }}}
    return *Output;
}
