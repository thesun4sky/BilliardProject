#include "StdAfx.h"
#include "Gpu_VR.h"


Gpu_VR::Gpu_VR(void)
{
}

Gpu_VR::~Gpu_VR(void)
{
}

unsigned char* Gpu_VR::VR_basic(Volume *vol, TFManager *tf, const int *imgSize, const float *ViewingPoint)
{
	printf("-in GPU Render class\n");
	ushort *pVol = vol->m_density;
	int *dim = vol->m_dim;
	double* spacing = vol->m_spacing_voxel;
	double zResolution = spacing[0]/spacing[2];

	int bufferSize = imgSize[0]*imgSize[1]*3;

	uchar *image = new uchar[bufferSize];
	memset(image, 0, sizeof(uchar)*bufferSize);

	TF *transfer = tf->GetTFData();
	int tf_size = tf->GetSize();

	GPU_Render(image, imgSize, pVol, dim, transfer, tf_size, zResolution, vol->m_bInitVolumeInGPU, vol->m_bInitTFInGPU, ViewingPoint);

	return image;
}


unsigned char* Gpu_VR::VR_AmbientOcclusion(Volume *vol, TFManager *tf, const int *imgSize, const float *ViewingPoint)
{
	printf("-in GPU AO Render class\n");
	ushort *pVol = vol->m_density;
	int *dim = vol->m_dim;
	double* spacing = vol->m_spacing_voxel;
	double zResolution = spacing[0]/spacing[2];

	if(!vol->m_bLoadProb){
		vol->LoadProbability();
		vol->m_bLoadProb = true;
	}

	float* Average = vol->m_Average;
	float* Sigma = vol->m_Sigma;

	float* Average_half = vol->m_Average;
	float* Sigma_half = vol->m_Sigma_half;

	if(!vol->m_bInitAvgSigInGPU){
		int dim_half[3] = {dim[0]/2, dim[1]/2, dim[2]/2};
		Average = new float[dim_half[0]*dim_half[1]*dim_half[2]];
		memset(Average, 0, sizeof(float)*dim_half[0]*dim_half[1]*dim_half[2]);
		Sigma = new float[dim_half[0]*dim_half[1]*dim_half[2]];
		memset(Sigma, 0, sizeof(float)*dim_half[0]*dim_half[1]*dim_half[2]);

		Average_half = new float[dim_half[0]*dim_half[1]*dim_half[2]];
		memset(Average_half, 0, sizeof(float)*dim_half[0]*dim_half[1]*dim_half[2]);
		Sigma_half = new float[dim_half[0]*dim_half[1]*dim_half[2]];
		memset(Sigma_half, 0, sizeof(float)*dim_half[0]*dim_half[1]*dim_half[2]);

		ushort* ScaledpVol = RunScaling(pVol, dim, 0.5f);

		printf("[MakeAverageSigma]\n");
		if(!MakeAverageSigma(ScaledpVol, dim_half, Average, Sigma, 7))
			return NULL;
		printf("[MakeAverageSigma Half]\n");
		if(!MakeAverageSigma(ScaledpVol, dim_half, Average_half, Sigma_half, 5))
			return NULL;

		delete[] ScaledpVol;
	}

	if(!vol->m_bSmooth){
		RunSmoothFilter(pVol, dim);
		vol->m_bSmooth = true;
	}

	int bufferSize = imgSize[0]*imgSize[1]*3;

	uchar *image = new uchar[bufferSize];
	memset(image, 0, sizeof(uchar)*bufferSize);

	TF *transfer = tf->GetTFData();
	int tf_size = tf->GetSize();

	float factor[3] = {0.15f, 0.45f, 0.4f};
	GPU_Render_AO(image, imgSize, pVol, dim, transfer, tf_size, zResolution, 
		vol->m_bInitVolumeInGPU, vol->m_bInitTFInGPU, Average, Sigma, Average_half, Sigma_half,
		vol->m_bInitAvgSigInGPU, vol->m_probability, factor, ViewingPoint);

	if(!vol->m_bInitAvgSigInGPU){
		if(Average){
			delete[] Average;
			Average = NULL;
		}
		if(Sigma){
			delete[] Sigma;
			Sigma = NULL;
		}
		if(Average_half){
			delete[] Average_half;
			Average_half = NULL;
		}
		if(Sigma_half){
			delete[] Sigma_half;
			Sigma_half = NULL;
		}

		vol->m_bInitAvgSigInGPU=true;
	}

	return image;
}
