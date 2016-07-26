
// Athletic_Doc.h : CAthletic_Doc Ŭ������ �������̽�
//


#pragma once
#include "gl/freeglut.h"
#include "gl/gl.h"
#include "gl/glu.h"
#include "Volume.h"
#include "Cpu_VR.h"
#include "Gpu_VR.h"
class CAthletic_Doc : public CDocument
{
protected: // serialization������ ��������ϴ�.
	CAthletic_Doc();
	DECLARE_DYNCREATE(CAthletic_Doc)

// Ư���Դϴ�.
public:

// �۾��Դϴ�.
public:

// �������Դϴ�.
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// �����Դϴ�.
public:
	virtual ~CAthletic_Doc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// ������ �޽��� �� �Լ�
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// �˻� ó���⿡ ���� �˻� �������� �����ϴ� ����� �Լ�
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
private:
	

public:
	afx_msg void OnFileOpen();
	
	afx_msg void OnCpuVR();
	afx_msg void OnGpuVR();
	afx_msg void OnGpuVR_AO();

public:
	Cpu_VR m_render_cpu;
	Gpu_VR m_render_gpu;
	void MoveLeft(void);
	void MoveRight(void);
	void MoveUp(void);
	void MoveDown(void);
	void RenderType();
	void GLinit(void);
	void GLRenderScene(void);
	//Volume* GetVolume(void);
	
	int m_ResultImgSize[2];
	unsigned char *m_imageBuffer;
	GLuint m_texName;
	float m_Viewing[3];
	float m_Volcenter[3];
	int m_CurrentRenderType;
	TFManager m_TF;
	Volume m_vol;
};
