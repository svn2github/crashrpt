// MFCDemoDoc.h : interface of the CMFCDemoDoc class
//


#pragma once


class CMFCDemoDoc : public CDocument
{
protected: // create from serialization only
	CMFCDemoDoc();
	DECLARE_DYNCREATE(CMFCDemoDoc)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// Implementation
public:
	virtual ~CMFCDemoDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
};


