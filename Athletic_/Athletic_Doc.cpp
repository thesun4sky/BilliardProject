
// Athletic_Doc.cpp : CAthletic_Doc 클래스의 구현
//

#include "stdafx.h"
// SHARED_HANDLERS는 미리 보기, 축소판 그림 및 검색 필터 처리기를 구현하는 ATL 프로젝트에서 정의할 수 있으며
// 해당 프로젝트와 문서 코드를 공유하도록 해 줍니다.
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


// CAthletic_Doc 생성/소멸

CAthletic_Doc::CAthletic_Doc()
{
	//// TODO: 여기에 일회성 생성 코드를 추가합니다.
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

	// TODO: 여기에 재초기화 코드를 추가합니다.
	// SDI 문서는 이 문서를 다시 사용합니다.

	return TRUE;
}




// CAthletic_Doc serialization

void CAthletic_Doc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: 여기에 저장 코드를 추가합니다.
	}
	else
	{
		// TODO: 여기에 로딩 코드를 추가합니다.
	}
}

#ifdef SHARED_HANDLERS

// 축소판 그림을 지원합니다.
void CAthletic_Doc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// 문서의 데이터를 그리려면 이 코드를 수정하십시오.
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

// 검색 처리기를 지원합니다.
void CAthletic_Doc::InitializeSearchContent()
{
	CString strSearchContent;
	// 문서의 데이터에서 검색 콘텐츠를 설정합니다.
	// 콘텐츠 부분은 ";"로 구분되어야 합니다.

	// 예: strSearchContent = _T("point;rectangle;circle;ole object;");
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

// CAthletic_Doc 진단

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


// CAthletic_Doc 명령

static int CALLBACK BrowseCallbackProc( HWND hWnd, UINT uMsg, LPARAM lParam,
										LPARAM lpData )
{
	switch( uMsg )
	{
	case BFFM_INITIALIZED:		// 폴더 선택 대화상자를 초기화 할 때, 초기 경로 설정
		{
			::SendMessage( hWnd, BFFM_SETSELECTION, TRUE, (LPARAM)lpData );
		}
		break;

	// BROWSEINFO 구조체의 ulFlags 값에 BIF_STATUSTEXT 가 설정된 경우 호출
	// 단, BIF_NEWDIALOGSTYLE 가 설정되어 있을 경우 호출되지 않음
	case BFFM_SELCHANGED:		// 사용자가 폴더를 선택할 경우 대화상자에 선택된 경로 표시
		{
			TCHAR szPath[ MAX_PATH ] = { 0, };

			::SHGetPathFromIDList( (LPCITEMIDLIST)lParam, szPath );
			::SendMessage( hWnd, BFFM_SETSTATUSTEXT, 0, (LPARAM)szPath );
		}
		break;

	// BROWSEINFO 구조체의 ulFlags 값에 BIF_VALIDATE 가 설정된 경우 호출
	// BIF_EDITBOX 와 같이 설정된 경우만 호출됨
	case BFFM_VALIDATEFAILED:	// 에디터 콘트롤에서 폴더 이름을 잘못 입력한 경우 호출
		{
			::MessageBox( hWnd, _T( "해당 폴더를 찾을 수 없습니다." ), _T( "오류" ),
				MB_ICONERROR | MB_OK );
		}
		break;
	}

	return 0;
}
void CAthletic_Doc::OnFileOpen()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
	CWnd *pWnd = AfxGetMainWnd();
	HWND hWnd = pWnd->m_hWnd;

	BROWSEINFO bi;

	TCHAR szTemp[ MAX_PATH ] = { 0, };

	TCHAR * pszPath = _T( "C:\\" );
	
	::ZeroMemory( &bi, sizeof( BROWSEINFO ) );

	bi.hwndOwner	= hWnd;
	bi.lpszTitle	= _T( "파일 경로를 선택해주세요." );
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

	//이미지 읽어오기
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
	
	MessageBox( hWnd, szTemp, _T("볼륨 로드 확인"), MB_OK );

}

void CAthletic_Doc::GLinit(void)
{    
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB); // 더블 버퍼링
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

//CPU메뉴 -> basic (0)
void CAthletic_Doc::OnCpuVR()
{
	CWnd *pWnd = AfxGetMainWnd();
	HWND hWnd = pWnd->m_hWnd;

	CMainFrame *pFrame = (CMainFrame *)AfxGetMainWnd();
	CChildFrame *pChild = (CChildFrame *)pFrame->GetActiveFrame();
	CAthletic_View *pView = (CAthletic_View *)pChild->GetActiveView();

	// TODO: 여기에 명령 처리기 코드를 추가합니다.
	
	//경고창 생성
	if(m_vol.m_density == NULL) {
		printf("볼륨이 없습니다.\n");
		MessageBox( hWnd, _T("볼륨이 없습니다."), _T("경고"), MB_OK );
		return;
	}
	if(m_TF.GetTFData() == NULL) {
		MessageBox( hWnd, _T("변환함수가 없습니다."), _T("경고"), MB_OK );
		return;
	}
	
	//이미지 출력
	uchar *image = m_render_cpu.VR_basic(&m_vol, &m_TF, 
		m_ResultImgSize, m_Viewing);
	if(image == NULL) return;
	delete[] m_imageBuffer;

	m_imageBuffer = image;

	//렌더 타입을 통한 뷰변환
	m_CurrentRenderType = 0;
	
	pView->Invalidate(TRUE);
}

//Volume* CAthletic_Doc::GetVolume()
//{
//	return &m_vol;
//}

//GPU메뉴 -> basic (100)
void CAthletic_Doc::OnGpuVR()
{
	CWnd *pWnd = AfxGetMainWnd();
	HWND hWnd = pWnd->m_hWnd;

	CMainFrame *pFrame = (CMainFrame *)AfxGetMainWnd();
	CChildFrame *pChild = (CChildFrame *)pFrame->GetActiveFrame();
	CAthletic_View *pView = (CAthletic_View *)pChild->GetActiveView();

	// TODO: 여기에 명령 처리기 코드를 추가합니다.
	
	//경고창 생성
	if(m_vol.m_density == NULL) {
		printf("볼륨이 없습니다. \n");
		MessageBox( hWnd, _T("볼륨이 없습니다."), _T("경고"), MB_OK );
		return;
	}
	if(m_TF.GetTFData() == NULL) {
		MessageBox( hWnd, _T("변환함수가 없습니다."), _T("경고"), MB_OK );
		return;
	}
	
	//이미지 출력
	uchar *image = m_render_gpu.VR_basic(&m_vol, &m_TF, 
		m_ResultImgSize, m_Viewing);
	
	if(image == NULL) return;
	delete[] m_imageBuffer;

	m_imageBuffer = image;

	//렌더 타입을 통한 뷰변환
	m_CurrentRenderType = 100;

	pView->Invalidate(TRUE);
}

//GPU메뉴 -> AO (101)
void CAthletic_Doc::OnGpuVR_AO()
{
	CWnd *pWnd = AfxGetMainWnd();
	HWND hWnd = pWnd->m_hWnd;

	CMainFrame *pFrame = (CMainFrame *)AfxGetMainWnd();
	CChildFrame *pChild = (CChildFrame *)pFrame->GetActiveFrame();
	CAthletic_View *pView = (CAthletic_View *)pChild->GetActiveView();

	// TODO: 여기에 명령 처리기 코드를 추가합니다.

	//경고창 생성
	if(m_vol.m_density == NULL) {
		printf("볼륨이 없습니다. \n");
		MessageBox( hWnd, _T("볼륨이 없습니다."), _T("경고"), MB_OK );
		return;
	}
	if(m_TF.GetTFData() == NULL) {
		MessageBox( hWnd, _T("변환함수가 없습니다."), _T("경고"), MB_OK );
		return;
	}

	//이미지 출력
	uchar *image = m_render_gpu.VR_AmbientOcclusion(&m_vol, &m_TF, 
		m_ResultImgSize, m_Viewing);
	if(image == NULL){
		MessageBox( hWnd, _T("디버깅 시작 하세요."), _T("실패"), MB_OK );
		return;
	}
	
	if(image == NULL) return;
	delete[] m_imageBuffer;

	m_imageBuffer = image;

	//렌더 타입을 통한 뷰변환
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
