
// Athletic_View.cpp : CAthletic_View 클래스의 구현
//

#include "stdafx.h"
// SHARED_HANDLERS는 미리 보기, 축소판 그림 및 검색 필터 처리기를 구현하는 ATL 프로젝트에서 정의할 수 있으며
// 해당 프로젝트와 문서 코드를 공유하도록 해 줍니다.
#ifndef SHARED_HANDLERS
#include "Athletic_.h"
#endif
#include "MainFrm.h"
#include "ChildFrm.h"
#include "Athletic_Doc.h"
#include "Athletic_View.h"
#include "math.h"
#include <windows.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAthletic_View

IMPLEMENT_DYNCREATE(CAthletic_View, CView)

BEGIN_MESSAGE_MAP(CAthletic_View, CView)
	// 표준 인쇄 명령입니다.
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CAthletic_View::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()

// CAthletic_View 생성/소멸

CAthletic_View::CAthletic_View()
{
	// TODO: 여기에 생성 코드를 추가합니다.

	//m_vol = Volume();
	//m_TF = TFManager();
	//0 : CPU-Basic
	//1 : 
	//100 : GPU-Basic
	//101 : GPU AO
	//102 :
}

CAthletic_View::~CAthletic_View()
{
}

BOOL CAthletic_View::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: CREATESTRUCT cs를 수정하여 여기에서
	//  Window 클래스 또는 스타일을 수정합니다.

	return CView::PreCreateWindow(cs);
}

// CAthletic_View 그리기

void CAthletic_View::OnDraw(CDC* /*pDC*/)
{
	CAthletic_Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: 여기에 원시 데이터에 대한 그리기 코드를 추가합니다.
	wglMakeCurrent(m_hDC, m_hRC);

	pDoc->GLRenderScene();
	SwapBuffers(m_hDC);

	wglMakeCurrent(m_hDC, NULL);
}


// CAthletic_View 인쇄


void CAthletic_View::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CAthletic_View::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 기본적인 준비
	return DoPreparePrinting(pInfo);
}

void CAthletic_View::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 인쇄하기 전에 추가 초기화 작업을 추가합니다.
}

void CAthletic_View::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 인쇄 후 정리 작업을 추가합니다.
}

void CAthletic_View::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CAthletic_View::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CAthletic_View 진단

#ifdef _DEBUG
void CAthletic_View::AssertValid() const
{
	CView::AssertValid();
}

void CAthletic_View::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CAthletic_Doc* CAthletic_View::GetDocument() const // 디버그되지 않은 버전은 인라인으로 지정됩니다.
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CAthletic_Doc)));
	return (CAthletic_Doc*)m_pDocument;
}
#endif //_DEBUG


// CAthletic_View 메시지 처리기


void CAthletic_View::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	VERIFY(wglMakeCurrent(m_hDC, m_hRC));

	GLResize(cx, cy);

	VERIFY(wglMakeCurrent(NULL, NULL));
}

void CAthletic_View::GLResize(int cx, int cy)
{
	glViewport (0, 0, (GLsizei) cx, (GLsizei) cy);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	gluPerspective( 50.0f, (GLdouble) cx/cy, 1.0f, 150.0f );
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt (0.0, 0.0, 5.0 , 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}

void CAthletic_View::OnDestroy()
{
	wglDeleteContext(m_hRC);
	::ReleaseDC(m_hWnd, m_hDC);
	
	CView::OnDestroy();
	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
}


int CAthletic_View::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  여기에 특수화된 작성 코드를 추가합니다.
	int nPixelFormat;
	m_hDC = ::GetDC(m_hWnd);

	static PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW|
		PFD_SUPPORT_OPENGL|
		PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		24,
		0,0,0,0,0,0,
		0,0,
		0,0,0,0,0,
		32,
		0,
		0,
		PFD_MAIN_PLANE,
		0,
		0,0,0
	};

	nPixelFormat = ChoosePixelFormat(m_hDC, &pfd);
	VERIFY(SetPixelFormat(m_hDC,nPixelFormat, &pfd));
	m_hRC = wglCreateContext(m_hDC);
	VERIFY(wglMakeCurrent(m_hDC,m_hRC));
	wglMakeCurrent(NULL,NULL);

	return 0;
}

void CAthletic_View::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	
	CMainFrame *pFrame = (CMainFrame *)AfxGetMainWnd();
	CAthletic_Doc *pDoc = (CAthletic_Doc *)pFrame->GetActiveDocument();

	CView::OnKeyDown(nChar, nRepCnt, nFlags);
	switch(nChar)
	{
	case VK_UP:		//상
		//printf("상\n");
		pDoc->MoveUp();
		break;

	case VK_DOWN:	//하
		//printf("하\n");
		pDoc->MoveDown();
		break;

	case VK_RIGHT:	//우
		//printf("우\n");
		pDoc->MoveRight();
		break;

	case VK_LEFT:	//좌
		//printf("좌\n");
		pDoc->MoveLeft();
		break;
	}

	pDoc->RenderType();
	/*
	//printf("%d %d\n", m_ResultImgSize[0], m_ResultImgSize[1]);
	if(m_CurrentRenderType == 0)
		image = pDoc->m_render_cpu.VR_basic(&m_vol, &m_TF, pDoc->m_ResultImgSize, m_Viewing);
	else if(m_CurrentRenderType == 100)
		image = pDoc->m_render_gpu.VR_basic(&m_vol, &m_TF, pDoc->m_ResultImgSize, m_Viewing);
	else if(m_CurrentRenderType == 101)
		image = pDoc->m_render_gpu.VR_AmbientOcclusion(&m_vol, &m_TF, pDoc->m_ResultImgSize, m_Viewing);
	if(image == NULL) return;
		delete[] pDoc->m_imageBuffer;

		pDoc->m_imageBuffer = image;
	Invalidate(TRUE);
	*/
}