#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "TFManager.h"

#include <cuda_runtime_api.h>

#include <helper_cuda.h>       // helper functions for CUDA error checking and initialization
#include <helper_math.h>

const float PI = 3.1415926536f;
typedef unsigned short ushort;
typedef unsigned char uchar;


__global__ void cuda_kernel_test(ushort *new_vol_k, ushort *vol_k, int3 dim3, float *gaussianMask_k, int maskSize)
{
	int tx = __umul24(blockIdx.x, blockDim.x) + threadIdx.x;
    int ty = __umul24(blockIdx.y, blockDim.y) + threadIdx.y;

	if (tx >= dim3.x-2 || ty >= dim3.y-2) return;
	if (tx <= 2 || ty <= 2) return;

	int size = maskSize/2;

	for(int tz=1; tz<dim3.z; tz++)
	{
		double sum = 0.f;
		for(int i=-size; i<size+1; i++)
		{
			for(int j=-size; j<size+1; j++)
			{
				for(int k=-size; k<size+1; k++)
				{
					int z=k+1, y=j+1, x=i+1;
					sum += vol_k[(tz+k)*dim3.x*dim3.y + (ty+j)*dim3.y + (tx+i)]*gaussianMask_k[z*maskSize*maskSize + y*maskSize + x];
				}
			}
		}
		new_vol_k[tz*dim3.x*dim3.y + ty*dim3.y + tx] = (ushort)sum;
	}

}

extern "C"
void RunSmoothFilter(ushort* pVol, int *dim)
{
	printf("-GPU RunSmoothFilter \n");

	float fSigma=0.8f;
	float gaussianMask[27];
	int maskSize = 3;
	int allocSize= maskSize*maskSize*maskSize;

	float sum=0.f;
	for(int i=0; i<maskSize; i++)
	{
		float z = fabs((float)i-1.f);
		for(int j=0; j<maskSize; j++)
		{
			float y = fabs((float)j-1.f);
			for(int k=0; k<maskSize; k++)
			{
				float x = fabs((float)k-1.f); 
				float fDist = x+y+z;
				sum += gaussianMask[k*maskSize*maskSize + j*maskSize + i] = 
					exp(-(fDist*fDist)/(2.f*fSigma*fSigma))/(sqrtf(2.f*PI)*fSigma);
			}
		}
	}
	for(int i=0; i<maskSize; i++)
	{
		for(int j=0; j<maskSize; j++)
		{
			for(int k=0; k<maskSize; k++)
			{
				gaussianMask[k*maskSize*maskSize + j*maskSize + i] /= sum;
			}
		}
	}

	//printf("%f %f %f\n", gaussianMask[0], gaussianMask[1], gaussianMask[2]);
	//printf("%f %f %f\n", gaussianMask[3], gaussianMask[4], gaussianMask[5]);
	//printf("%f %f %f\n\n", gaussianMask[6], gaussianMask[7], gaussianMask[8]);

	//printf("%f %f %f\n", gaussianMask[9], gaussianMask[10], gaussianMask[11]);
	//printf("%f %f %f\n", gaussianMask[12], gaussianMask[13], gaussianMask[14]);
	//printf("%f %f %f\n\n", gaussianMask[15], gaussianMask[16], gaussianMask[17]);

	//printf("%f %f %f\n", gaussianMask[18], gaussianMask[19], gaussianMask[20]);
	//printf("%f %f %f\n", gaussianMask[21], gaussianMask[22], gaussianMask[23]);
	//printf("%f %f %f\n\n", gaussianMask[24], gaussianMask[25], gaussianMask[26]);

	float* gaussianMask_k;
	cudaMalloc((void**)&gaussianMask_k, allocSize*sizeof(float));
	cudaMemset(gaussianMask_k, 0, allocSize*sizeof(float));
	cudaMemcpy(gaussianMask_k, gaussianMask, allocSize*sizeof(float), cudaMemcpyHostToDevice);

	ushort *pVol_k, *new_pVol_k;
	int vol_size = dim[0]*dim[1]*dim[2];
	int3 vol_dim3 = {dim[0], dim[1], dim[2]};

	cudaMalloc((void**)&pVol_k, vol_size*sizeof(ushort));
	cudaMemset(pVol_k, 0, vol_size*sizeof(ushort));
	cudaMemcpy(pVol_k, pVol, vol_size*sizeof(ushort), cudaMemcpyHostToDevice);

	cudaMalloc((void**)&new_pVol_k, vol_size*sizeof(ushort));
	cudaMemset(new_pVol_k, 0, vol_size*sizeof(ushort));

	dim3 Db = dim3(32, 32);		// block dimensions are fixed to be 512 threads
    dim3 Dg = dim3((dim[0]+Db.x-1)/Db.x, (dim[1]+Db.y-1)/Db.y);

    cuda_kernel_test<<<Dg,Db>>>(new_pVol_k, pVol_k, vol_dim3, gaussianMask_k, maskSize);

    if (cudaGetLastError() != cudaSuccess)
        printf("cuda_kernel() failed to launch error = %d\n", cudaGetLastError());
    
	memset(pVol, 0, sizeof(ushort)*vol_size);
	cudaMemcpy(pVol, new_pVol_k, vol_size*sizeof(ushort), cudaMemcpyDeviceToHost);

	cudaFree(pVol_k);
	cudaFree(new_pVol_k);
	cudaFree(gaussianMask_k);

}


texture<ushort, 3, cudaReadModeNormalizedFloat> tex_volume_processing;  // 3D texture
cudaArray *d_volumeproArray = 0;


void initVolume(const ushort *h_volume, int x, int y, int z, int bytePerVoxel)
{
	cudaExtent volume_Size = make_cudaExtent(x, y, z);
    // create 3D array
    cudaChannelFormatDesc channelDesc = cudaCreateChannelDesc(bytePerVoxel*8, 0, 0, 0, cudaChannelFormatKindUnsigned);
    checkCudaErrors( cudaMalloc3DArray(&d_volumeproArray, &channelDesc, volume_Size, 0) );

    // copy data to 3D array
    cudaMemcpy3DParms myParams = {0};
    myParams.srcPtr   = make_cudaPitchedPtr((void*)h_volume, x*bytePerVoxel, x, y);
    myParams.dstArray = d_volumeproArray;
    myParams.extent   = volume_Size;
    myParams.kind     = cudaMemcpyHostToDevice;
    checkCudaErrors( cudaMemcpy3D(&myParams) );

    // set texture parameters
    tex_volume_processing.normalized = false;                      // access with normalized texture coordinates
    tex_volume_processing.filterMode = cudaFilterModeLinear;      // linear interpolation
    tex_volume_processing.channelDesc = channelDesc;
	tex_volume_processing.addressMode[0] = cudaAddressModeBorder;   // wrap texture coordinates
    tex_volume_processing.addressMode[1] = cudaAddressModeBorder;
    tex_volume_processing.addressMode[2] = cudaAddressModeBorder;


    // bind array to 3D texture
    checkCudaErrors(cudaBindTextureToArray(tex_volume_processing, d_volumeproArray, channelDesc));
}


__global__ void Scaling(ushort* new_volume, float scalefactor, int fx, int fy, int fz)
{
	int tx = __umul24(blockIdx.x, blockDim.x) + threadIdx.x;
    int ty = __umul24(blockIdx.y, blockDim.y) + threadIdx.y;

	int fx_scaled = fx*scalefactor;
	int fy_scaled = fy*scalefactor;
	int fz_scaled = fz*scalefactor;

	if(tx >= fx_scaled) return;
	if(ty >= fx_scaled) return;

	float divfactor = scalefactor*scalefactor*scalefactor;

	for(int i=0; i<fz_scaled; i++)
	{
		int avgvalue=0;
		int tx_scaled = tx/scalefactor;
		int ty_scaled = ty/scalefactor;
		int tz_scaled = i/scalefactor;

		float den0 = tex3D(tex_volume_processing, tx_scaled,	ty_scaled,		tz_scaled)*65535*divfactor;
		float den1 = tex3D(tex_volume_processing, tx_scaled+1,	ty_scaled,		tz_scaled)*65535*divfactor;
		float den2 = tex3D(tex_volume_processing, tx_scaled,	ty_scaled+1,	tz_scaled)*65535*divfactor;
		float den3 = tex3D(tex_volume_processing, tx_scaled,	ty_scaled,		tz_scaled+1)*65535*divfactor;
		float den4 = tex3D(tex_volume_processing, tx_scaled+1,	ty_scaled+1,	tz_scaled)*65535*divfactor;
		float den5 = tex3D(tex_volume_processing, tx_scaled,	ty_scaled+1,	tz_scaled+1)*65535*divfactor;
		float den6 = tex3D(tex_volume_processing, tx_scaled+1,	ty_scaled,		tz_scaled+1)*65535*divfactor;
		float den7 = tex3D(tex_volume_processing, tx_scaled+1,	ty_scaled+1,	tz_scaled+1)*65535*divfactor;

		avgvalue = (int)(den0+den1+den2+den3+den4+den5+den6+den7);

		new_volume[i*fx_scaled*fy_scaled + ty*fy_scaled + tx] = avgvalue;
	}

	
}

extern "C"
ushort* RunScaling(ushort *pVol, int dim[3], float scalefactor)
{
	initVolume(pVol, dim[0], dim[1], dim[2] , sizeof(ushort));

	dim3 Db = dim3(16, 16);   // block dimensions are fixed to be 256 threads
	//dim3 Db = dim3(32, 32);		// block dimensions are fixed to be 512 threads
    dim3 Dg = dim3((dim[0]+Db.x-1)/Db.x, (dim[1]+Db.y-1)/Db.y);
	
	int fx_scaled = dim[0]*scalefactor;
	int fy_scaled = dim[1]*scalefactor;
	int fz_scaled = dim[2]*scalefactor;
	int newsize = fx_scaled*fy_scaled*fz_scaled;

	ushort *new_vol_k;
	cudaMalloc((void**)&new_vol_k, newsize*sizeof(ushort));
	cudaMemset(new_vol_k, 0, newsize*sizeof(ushort));

	Scaling<<<Dg, Db>>>(new_vol_k, scalefactor, dim[0], dim[1], dim[2]);

	ushort *new_volume = new ushort[newsize];
	memset(new_volume, 0, sizeof(ushort)*newsize);

	cudaMemcpy(new_volume, new_vol_k, newsize*sizeof(ushort), cudaMemcpyDeviceToHost);
	
	cudaFreeArray(d_volumeproArray);
	cudaFree(new_vol_k);

    return new_volume;
}