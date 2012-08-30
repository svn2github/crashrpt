
// MFCDemoView.cpp : implementation of the CMFCDemoView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "MFCDemo.h"
#endif

#include "MFCDemoDoc.h"
#include "MFCDemoView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMFCDemoView

IMPLEMENT_DYNCREATE(CMFCDemoView, CView)

BEGIN_MESSAGE_MAP(CMFCDemoView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CMFCDemoView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

// CMFCDemoView construction/destruction

CMFCDemoView::CMFCDemoView()
{
	// TODO: add construction code here

}

CMFCDemoView::~CMFCDemoView()
{
}

BOOL CMFCDemoView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CMFCDemoView drawing

void CMFCDemoView::OnDraw(CDC* /*pDC*/)
{
	CMFCDemoDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
}


// CMFCDemoView printing


void CMFCDemoView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CMFCDemoView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CMFCDemoView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CMFCDemoView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CMFCDemoView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CMFCDemoView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CMFCDemoView diagnostics

#ifdef _DEBUG
void CMFCDemoView::AssertValid() const
{
	CView::AssertValid();
}

void CMFCDemoView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CMFCDemoDoc* CMFCDemoView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMFCDemoDoc)));
	return (CMFCDemoDoc*)m_pDocument;
}
#endif //_DEBUG


// CMFCDemoView message handlers
