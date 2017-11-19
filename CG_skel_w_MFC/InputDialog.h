
#pragma once

#include <string>
using std::string;

#include "vec.h"

// ------------------------
//    Class CInputDialog
// ------------------------

class CInputDialog : public CDialog
{
	DECLARE_DYNAMIC(CInputDialog)

public:
	CInputDialog(CString title = "Input Dialog");
	virtual ~CInputDialog();

	virtual BOOL OnInitDialog();

	enum { IDD = IDD_INPUTDIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

private:
	CString mTitle;
};

// ----------------------
//    Class CCmdDialog
// ----------------------

class CCmdDialog : public CInputDialog
{
public:
	CCmdDialog(CString title = "Input Dialog");
	virtual ~CCmdDialog();

	string GetCmd();

protected:
	CString mCmd;
	CEdit mCmdEdit;

	virtual void DoDataExchange(CDataExchange* pDX);

	afx_msg int OnCreate(LPCREATESTRUCT lpcs);
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
};

// ----------------------
//    Class CXyzDialog
// ----------------------

class CXyzDialog : public CInputDialog
{
public:
	CXyzDialog(CString title = "Input Dialog", vec3 default_values=vec3());
	virtual ~CXyzDialog();

	vec3 GetXYZ();

protected:
	float mX;
	float mY;
	float mZ;
	CEdit mXEdit;
	CEdit mYEdit;
	CEdit mZEdit;

	virtual void DoDataExchange(CDataExchange* pDX);

	afx_msg int OnCreate(LPCREATESTRUCT lpcs);
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
};

// -------------------------
//    Class CCmdXyzDialog
// -------------------------

class CCmdXyzDialog : public CInputDialog
{
public:
	CCmdXyzDialog(CString title = "Input Dialog");
	virtual ~CCmdXyzDialog();

	string GetCmd();
	vec3 GetXYZ();

protected:
	CString mCmd;
	float mX;
	float mY;
	float mZ;
	CEdit mCmdEdit;
	CEdit mXEdit;
	CEdit mYEdit;
	CEdit mZEdit;

	virtual void DoDataExchange(CDataExchange* pDX);

	afx_msg int OnCreate(LPCREATESTRUCT lpcs);
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
};


// ----------------------
//    Class COrthoDialog
// ----------------------

class COrthoDialog : public CInputDialog
{
public:
	COrthoDialog(CString title = "Orthogonal Setting");
	virtual ~COrthoDialog();

	float GetLeft() const;
	float GetRight() const;
	float GetBottom() const;
	float GetTop() const;
	float GetNear() const;
	float GetFar() const;

protected:
	
	float left;
	float right;
	float bottom;
	float top;
	float zNear; 
	float zFar;
	CEdit leftEdit;
	CEdit rightEdit;
	CEdit bottomEdit;
	CEdit topEdit;
	CEdit nearEdit;
	CEdit farEdit;

	virtual void DoDataExchange(CDataExchange* pDX);

	afx_msg int OnCreate(LPCREATESTRUCT lpcs);
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
};