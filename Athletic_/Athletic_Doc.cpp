
// Athletic_Doc.cpp : CAthletic_Doc Ŭ������ ����
//

#include "stdafx.h"
// SHARED_HANDLERS�� �̸� ����, ����� �׸� �� �˻� ���� ó���⸦ �����ϴ� ATL ������Ʈ���� ������ �� ������
// �ش� ������Ʈ�� ���� �ڵ带 �����ϵ��� �� �ݴϴ�.
#ifndef SHARED_HANDLERS
#include "Athletic_.h"
#endif

#include "Athletic_Doc.h"
#include "Athletic_View.h"
#include "MainFrm.h"
#include "ChildFrm.h"
#include <propkey.h>

#include "vtkDICOMImageReader.h"
#include "vtkSmartPointer.h"
#include "vtkImageData.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CAthletic_Doc

IMPLEMENT_DYNCREATE(CAthletic_Doc, CDocument)

BEGIN_MESSAGE_MAP(CAthletic_Doc, CDocument)
	ON_COMMAND(ID_FILE_OPEN, &CAthletic_Doc::OnFileOpen)
	ON_COMMAND(ID_CPU_VR, &CAthletic_Doc::OnCpuVR)
	ON_COMMAND(ID_GPU_VR, &CAthletic_Doc::OnGpuVR)
	ON_COMMAND(ID_GPU_AO, &CAthletic_Doc::OnGpuVR_AO)
END_MESSAGE_MAP()


// CAthletic_Doc ����/�Ҹ�

CAthletic_Doc::CAthletic_Doc()
{
	//// TODO: ���⿡ ��ȸ�� ���� �ڵ带 �߰��մϴ�.
	//m_render_cpu= Cpu_VR();
	//m_render_gpu= Gpu_VR();
	
	FILE *infile;
	infile = fopen("data/Athletics.bmp", "rb");
	if(infile==NULL) {printf("No Image File"); return;}

	BITMAPFILEHEADER hf;
	BITMAPINFOHEADER hInfo;
	fread(&hf, sizeof(BITMAPFILEHEADER),1,infile);
	
	if(hf.bfType!=0x4D42) exit(1);
	fread(&hInfo,sizeof(BITMAPINFOHEADER),1,infile);
	if(hInfo.biBitCount!=24) {printf("Bad File Format!!"); return;}

	m_imageBuffer = new unsigned char[hInfo.biSizeImage];
	fread(m_imageBuffer, sizeof(unsigned char), hInfo.biSizeImage, infile);
	fclose(infile);

	m_ResultImgSize[0] = hInfo.biWidth;
	m_ResultImgSize[1] = hInfo.biHeight;
	
	m_Viewing[0] = 0.f;
	m_Viewing[1] = 1.f;
	m_Viewing[2] = 0.f;

	m_Volcenter[0] = 0.f;
	m_Volcenter[1] = 0.f;
	m_Volcenter[2] = 0.f;
	
	m_CurrentRenderType=-1;

}

CAthletic_Doc::~CAthletic_Doc()
{
	delete[] m_imageBuffer;
}

BOOL CAthletic_Doc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: ���⿡ ���ʱ�ȭ �ڵ带 �߰��մϴ�.
	// SDI ������ �� ������ �ٽ� ����մϴ�.

	return TRUE;
}




// CAthletic_Doc serialization

void CAthletic_Doc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: ���⿡ ���� �ڵ带 �߰��մϴ�.
	}
	else
	{
		// TODO: ���⿡ �ε� �ڵ带 �߰��մϴ�.
	}
}

#ifdef SHARED_HANDLERS

// ����� �׸��� �����մϴ�.
void CAthletic_Doc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// ������ �����͸� �׸����� �� �ڵ带 �����Ͻʽÿ�.
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// �˻� ó���⸦ �����մϴ�.
void CAthletic_Doc::InitializeSearchContent()
{
	CString strSearchContent;
	// ������ �����Ϳ��� �˻� �������� �����մϴ�.
	// ������ �κ��� ";"�� ���еǾ�� �մϴ�.

	// ��: strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void CAthletic_Doc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CAthletic_Doc ����

#ifdef _DEBUG
void CAthletic_Doc::AssertValid() const
{
	CDocument::AssertValid();
}

void CAthletic_Doc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CAthletic_Doc ���

static int CALLBACK BrowseCallbackProc( HWND hWnd, UINT uMsg, LPARAM lParam,
										LPARAM lpData )
{
	switch( uMsg )
	{
	case BFFM_INITIALIZED:		// ���� ���� ��ȭ���ڸ� �ʱ�ȭ �� ��, �ʱ� ��� ����
		{
			::SendMessage( hWnd, BFFM_SETSELECTION, TRUE, (LPARAM)lpData );
		}
		break;

	// BROWSEINFO ����ü�� ulFlags ���� BIF_STATUSTEXT �� ������ ��� ȣ��
	// ��, BIF_NEWDIALOGSTYLE �� �����Ǿ� ���� ��� ȣ����� ����
	case BFFM_SELCHANGED:		// ����ڰ� ������ ������ ��� ��ȭ���ڿ� ���õ� ��� ǥ��
		{
			TCHAR szPath[ MAX_PATH ] = { 0, };

			::SHGetPathFromIDList( (LPCITEMIDLIST)lParam, szPath );
			::SendMessage( hWnd, BFFM_SETSTATUSTEXT, 0, (LPARAM)szPath );
		}
		break;

	// BROWSEINFO ����ü�� ulFlags ���� BIF_VALIDATE �� ������ ��� ȣ��
	// BIF_EDITBOX �� ���� ������ ��츸 ȣ���
	case BFFM_VALIDATEFAILED:	// ������ ��Ʈ�ѿ��� ���� �̸��� �߸� �Է��� ��� ȣ��
		{
			::MessageBox( hWnd, _T( "�ش� ������ ã�� �� �����ϴ�." ), _T( "����" ),
				MB_ICONERROR | MB_OK );
		}
		break;
	}

	return 0;
}
void CAthletic_Doc::OnFileOpen()
{
	// TODO: ���⿡ ��� ó���� �ڵ带 �߰��մϴ�.
	CWnd *pWnd = AfxGetMainWnd();
	HWND hWnd = pWnd->m_hWnd;

	BROWSEINFO bi;

	TCHAR szTemp[ MAX_PATH ] = { 0, };

	TCHAR * pszPath = _T( "C:\\" );
	
	::ZeroMemory( &bi, sizeof( BROWSEINFO ) );

	bi.hwndOwner	= hWnd;
	bi.lpszTitle	= _T( "���� ��θ� �������ּ���." );
	bi.ulFlags		= BIF_NEWDIALOGSTYLE | BIF_EDITBOX | BIF_RETURNONLYFSDIRS
						| BIF_STATUSTEXT | BIF_VALIDATE;
	bi.lpfn			= BrowseCallbackProc;
	bi.lParam		= (LPARAM)pszPath;

	LPITEMIDLIST pItemIdList = ::SHBrowseForFolder( &bi );

	if( !::SHGetPathFromIDList( pItemIdList, szTemp ) )
		return;

	vtkSmartPointer<vtkImageData> input = vtkSmartPointer<vtkImageData>::New();
	
	char charPath[MAX_PATH] = {0};
	WideCharToMultiByte(CP_ACP, 0, szTemp, MAX_PATH, charPath, MAX_PATH, NULL, NULL);

	//�̹��� �о����
	printf("Run vtkDICOMImageReader.. \n"); 
	vtkSmartPointer<vtkDICOMImageReader> dicomReader = vtkSmartPointer<vtkDICOMImageReader>::New();
	dicomReader->SetDirectoryName(charPath);
	dicomReader->SetDataScalarTypeToShort();
	dicomReader->Update();
	input->DeepCopy(dicomReader->GetOutput());
	
	int dim[3];
	input->GetDimensions(dim);
	
	if(dim[0] < 2 || dim[1] < 2 || dim[2] < 2){
		return;
	}else{
		printf("load Volume data size (x,y,z) : %d, %d, %d\n", dim[0], dim[1], dim[2]);
	}

	short *h_volume = (short*)input->GetScalarPointer();

	double *range =input->GetScalarRange();
	printf("-min density : %.1f\n-max density : %.1f\n", range[0], range[1]);

	double *spacing = input->GetSpacing();
	printf("-voxel spacing : %f %f %f\n", spacing[0], spacing[1], spacing[2]);


	CMainFrame *pFrame = (CMainFrame *)AfxGetMainWnd();
	CChildFrame *pChild = (CChildFrame *)pFrame->GetActiveFrame();
	CAthletic_View *pView = (CAthletic_View *)pChild->GetActiveView();

	//pView->SetImageSize(dim[0]+200, dim[2]+200);
	
	m_ResultImgSize[0] = dim[0]+200;
	m_ResultImgSize[1] = dim[2]+200;
	float firstView[3] = {(float)dim[0]/2.f, (float)dim[1], (float)dim[2]/2.f};
	m_Viewing[0] = firstView[0]; 
	m_Viewing[1] = firstView[1]; 
	m_Viewing[2] = firstView[2];
	firstView[1] /= 2.f;
	m_Volcenter[0] = firstView[0]; 
	m_Volcenter[1] = firstView[1]; 
	m_Volcenter[2] = firstView[2];

	m_vol.SetVolume(h_volume, dim, range, spacing);
	m_TF.SetTF(4096);

	printf("\n[Volume Load Complete]\n\n");
	
	MessageBox( hWnd, szTemp, _T("���� �ε� Ȯ��"), MB_OK );

}

void CAthletic_Doc::GLinit(void)
{    
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB); // ���� ���۸�
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClearDepth(1.0f);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);

	glGenTextures(1, &m_texName);
	glBindTexture (GL_TEXTURE_2D, m_texName);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, 3, m_ResultImgSize[0], m_ResultImgSize[1], 
				 0, GL_RGB, GL_UNSIGNED_BYTE, m_imageBuffer);

	glEnable(GL_TEXTURE_2D);
	glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	
}

void CAthletic_Doc::GLRenderScene(void)
{
	GLinit();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

	glBindTexture(GL_TEXTURE_2D, m_texName);

	glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0); glVertex2f(-2.8, -2.8);   
		glTexCoord2f(1.0, 0.0); glVertex2f(2.8, -2.8);
		glTexCoord2f(1.0, 1.0); glVertex2f(2.8, 2.8);
		glTexCoord2f(0.0, 1.0); glVertex2f(-2.8, 2.8);
	glEnd();
	glFlush();
	glDisable(GL_TEXTURE_2D);
	
}

//CPU�޴� -> basic (0)
void CAthletic_Doc::OnCpuVR()
{
	CWnd *pWnd = AfxGetMainWnd();
	HWND hWnd = pWnd->m_hWnd;

	CMainFrame *pFrame = (CMainFrame *)AfxGetMainWnd();
	CChildFrame *pChild = (CChildFrame *)pFrame->GetActiveFrame();
	CAthletic_View *pView = (CAthletic_View *)pChild->GetActiveView();

	// TODO: ���⿡ ��� ó���� �ڵ带 �߰��մϴ�.
	
	//���â ����
	if(m_vol.m_density == NULL) {
		printf("������ �����ϴ�.\n");
		MessageBox( hWnd, _T("������ �����ϴ�."), _T("���"), MB_OK );
		return;
	}
	if(m_TF.GetTFData() == NULL) {
		MessageBox( hWnd, _T("��ȯ�Լ��� �����ϴ�."), _T("���"), MB_OK );
		return;
	}
	
	//�̹��� ���
	uchar *image = m_render_cpu.VR_basic(&m_vol, &m_TF, 
		m_ResultImgSize, m_Viewing);
	if(image == NULL) return;
	delete[] m_imageBuffer;

	m_imageBuffer = image;

	//���� Ÿ���� ���� �亯ȯ
	m_CurrentRenderType = 0;
	
	pView->Invalidate(TRUE);
}

//Volume* CAthletic_Doc::GetVolume()
//{
//	return &m_vol;
//}

//GPU�޴� -> basic (100)
void CAthletic_Doc::OnGpuVR()
{
	CWnd *pWnd = AfxGetMainWnd();
	HWND hWnd = pWnd->m_hWnd;

	CMainFrame *pFrame = (CMainFrame *)AfxGetMainWnd();
	CChildFrame *pChild = (CChildFrame *)pFrame->GetActiveFrame();
	CAthletic_View *pView = (CAthletic_View *)pChild->GetActiveView();

	// TODO: ���⿡ ��� ó���� �ڵ带 �߰��մϴ�.
	
	//���â ����
	if(m_vol.m_density == NULL) {
		printf("������ �����ϴ�. \n");
		MessageBox( hWnd, _T("������ �����ϴ�."), _T("���"), MB_OK );
		return;
	}
	if(m_TF.GetTFData() == NULL) {
		MessageBox( hWnd, _T("��ȯ�Լ��� �����ϴ�."), _T("���"), MB_OK );
		return;
	}
	
	//�̹��� ���
	uchar *image = m_render_gpu.VR_basic(&m_vol, &m_TF, 
		m_ResultImgSize, m_Viewing);
	
	if(image == NULL) return;
	delete[] m_imageBuffer;

	m_imageBuffer = image;

	//���� Ÿ���� ���� �亯ȯ
	m_CurrentRenderType = 100;

	pView->Invalidate(TRUE);
}

//GPU�޴� -> AO (101)
void CAthletic_Doc::OnGpuVR_AO()
{
	CWnd *pWnd = AfxGetMainWnd();
	HWND hWnd = pWnd->m_hWnd;

	CMainFrame *pFrame = (CMainFrame *)AfxGetMainWnd();
	CChildFrame *pChild = (CChildFrame *)pFrame->GetActiveFrame();
	CAthletic_View *pView = (CAthletic_View *)pChild->GetActiveView();

	// TODO: ���⿡ ��� ó���� �ڵ带 �߰��մϴ�.

	//���â ����
	if(m_vol.m_density == NULL) {
		printf("������ �����ϴ�. \n");
		MessageBox( hWnd, _T("������ �����ϴ�."), _T("���"), MB_OK );
		return;
	}
	if(m_TF.GetTFData() == NULL) {
		MessageBox( hWnd, _T("��ȯ�Լ��� �����ϴ�."), _T("���"), MB_OK );
		return;
	}

	//�̹��� ���
	uchar *image = m_render_gpu.VR_AmbientOcclusion(&m_vol, &m_TF, 
		m_ResultImgSize, m_Viewing);
	if(image == NULL){
		MessageBox( hWnd, _T("����� ���� �ϼ���."), _T("����"), MB_OK );
		return;
	}
	
	if(image == NULL) return;
	delete[] m_imageBuffer;

	m_imageBuffer = image;

	//���� Ÿ���� ���� �亯ȯ
	m_CurrentRenderType = 101;

	pView->Invalidate(TRUE);
}

const float r = 200.0f;
const float PI = 3.1415926536f;
const double degree = PI/180.0;
float fMove_x=90.f, fMove_z=0.f;

void CAthletic_Doc::MoveUp(void)
{
	//m_Viewing[2] += 10.f;
	fMove_z += 2.f;
	m_Viewing[1] = (float)(r*cos(degree*fMove_z) + m_Volcenter[1]);
	m_Viewing[2] = (float)(r*sin(degree*fMove_z) + m_Volcenter[2]);
}
	
void CAthletic_Doc::MoveDown(void)
{
	//m_Viewing[2] -= 10.f;
	fMove_z -= 2.f;
	m_Viewing[1] = (float)(r*cos(degree*fMove_z) + m_Volcenter[1]);
	m_Viewing[2] = (float)(r*sin(degree*fMove_z) + m_Volcenter[2]);
}

void CAthletic_Doc::MoveRight(void)
{
	//m_Viewing[0] += 10.f;
	fMove_x -= 2.f;
	m_Viewing[0] = (float)(r*cos(degree*fMove_x) + m_Volcenter[0]);
	m_Viewing[1] = (float)(r*sin(degree*fMove_x) + m_Volcenter[1]);
}

void CAthletic_Doc::MoveLeft(void)
{
	//m_Viewing[0] -= 10.f;
	fMove_x += 2.f;
	m_Viewing[0] = (float)(r*cos(degree*fMove_x) + m_Volcenter[0]);
	m_Viewing[1] = (float)(r*sin(degree*fMove_x) + m_Volcenter[1]);
}


void CAthletic_Doc::RenderType()
{
	
	uchar *image = NULL;

	CMainFrame *pFrame = (CMainFrame *)AfxGetMainWnd();
	CChildFrame *pChild = (CChildFrame *)pFrame->GetActiveFrame();
	CAthletic_View *pView = (CAthletic_View *)pChild->GetActiveView();

	//CAthletic_View *pView = this->GetView();
	//printf("%d %d\n", m_ResultImgSize[0], m_ResultImgSize[1]);
	if(m_CurrentRenderType == 0)
		image = m_render_cpu.VR_basic(&m_vol, &m_TF, m_ResultImgSize, m_Viewing);
	else if(m_CurrentRenderType == 100)
		image = m_render_gpu.VR_basic(&m_vol, &m_TF, m_ResultImgSize, m_Viewing);
	else if(m_CurrentRenderType == 101)
		image = m_render_gpu.VR_AmbientOcclusion(&m_vol, &m_TF, m_ResultImgSize, m_Viewing);
	if(image == NULL) return;
	
	delete[] m_imageBuffer;
	m_imageBuffer = image;
	pView->Invalidate(TRUE);
}
