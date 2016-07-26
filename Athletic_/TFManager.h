#pragma once

// 1040~1280 살 1280부터 뼈
const int color_start = 1300, color_end = 2000;	//CPU
const int alpha_start = 1300, alpha_end = 2000;

const int Rcolor_start = 0, Rcolor_end = 255;
const int Gcolor_start = 0, Gcolor_end = 190;
const int Bcolor_start = 0, Bcolor_end = 0;

//TF관리

struct TF{
	float R;	//0-255
	float G;	//0-255
	float B;	//0-255
	float alpha; //0-1
};		

class TFManager
{
	
public:
	TFManager(void);
	~TFManager(void);

	void SetTF(int tf_size);
	TF* GetTFData(void) { return m_tf; }

	int GetSize(void) { return m_tfSize; }

private:
	TF *m_tf;
	int m_tfSize;
};
