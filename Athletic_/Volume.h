#pragma once

typedef unsigned short ushort;
typedef unsigned char uchar;

extern "C"
{
	void FreeGPUVolArray(void);
	void FreeGPUTFArray(void);
	void FreeGPUEtcArray(void);
}

class Volume
{
public:
	Volume(void);
	Volume(short *den, int dim[3], double range[2]);
	~Volume(void);

private:
	ushort **m_slice_ptr;

	//AO 변수

public:
	bool m_bInitVolumeInGPU;
	bool m_bInitTFInGPU;
	bool m_bInitAvgSigInGPU;
	bool m_bLoadProb;
	bool m_bSmooth;
	
	double m_spacing_voxel[3];
	ushort *m_density;
	int m_dim[3];	//x,y,z
	float *m_Average, *m_Average_half;
	float *m_Sigma, *m_Sigma_half;
	float m_probability[310];

public:
	void SetVolume(short *den, int dim[3], double range[2], double spacing[3]);
	void SetVolume(ushort *den, int dim[3]);

	//인라인 함수들.. 여러번 불리는거 유지 / 아닌거 cpp로
	//ushort* GetDensityPointer(void) { return m_density; }
	//int* GetDimension(void) { return m_dim; }
	//double* GetVoxelSpacing(void) { return m_spacing_voxel; }

	//float* GetAveragePointer(void) { return m_Average; }
	//float* GetSigmaPointer(void) { return m_Sigma; }
	//float* GetAverageHalfPointer(void) { return m_Average_half; }
	//float* GetSigmaHalfPointer(void) { return m_Sigma_half; }
	
	//float* GetProbability(void) { return m_probability; }
	ushort GetDensity(float x, float y, float z); //선형 보간
	Volume *GetVolume(void);

	void DeepCopy(Volume *output);
	void LoadProbability();
};
