#include "stdafx.h"
#include "CG_skel_w_MFC.h"
#include "InputDialog.h"

#define IDC_CMD_EDIT 200
#define IDC_X_EDIT 201
#define IDC_Y_EDIT 202
#define IDC_Z_EDIT 203

#define IDC_LEFT_EDIT		204
#define IDC_RIGHT_EDIT		205
#define IDC_BOTTOM_EDIT		206
#define IDC_TOP_EDIT		207
#define IDC_NEAR_EDIT		208
#define IDC_FAR_EDIT		209

#define CMD_EDIT_TITLE "Command"
#define X_EDIT_TITLE "X ="
#define Y_EDIT_TITLE "Y ="
#define Z_EDIT_TITLE "Z ="

#define LEFT_EDIT_TITLE "Left ="
#define RIGHT_EDIT_TITLE "Right ="
#define BOTTOM_EDIT_TITLE "Bottom ="
#define TOP_EDIT_TITLE "Top ="
#define NEAR_EDIT_TITLE "Near ="
#define FAR_EDIT_TITLE "Far ="



// ------------------------
//    Class CInputDialog
// ------------------------

IMPLEMENT_DYNAMIC(CInputDialog, CDialog)

CInputDialog::CInputDialog(CString title)
	: CDialog(CInputDialog::IDD, NULL), mTitle(title)
{ }

CInputDialog::~CInputDialog()
{ }

BOOL CInputDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetWindowText(mTitle);

	return TRUE;
}

void CInputDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

// ----------------------
//    Class CCmdDialog
// ----------------------

CCmdDialog::CCmdDialog(CString title)
	: CInputDialog(title), mCmd("")
{ }

CCmdDialog::~CCmdDialog()
{ }

string CCmdDialog::GetCmd()
{
	return ((LPCTSTR)mCmd);
}

void CCmdDialog::DoDataExchange(CDataExchange* pDX)
{
	CInputDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_CMD_EDIT, mCmd);
}

// CCmdDialog message handlers
BEGIN_MESSAGE_MAP(CCmdDialog, CInputDialog)
	ON_WM_CREATE()
	ON_WM_PAINT()
END_MESSAGE_MAP()

int CCmdDialog::OnCreate(LPCREATESTRUCT lpcs)
{
	mCmdEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
		CRect(10, 30, 450, 100), this, IDC_CMD_EDIT);

	return 0;
}

void CCmdDialog::OnPaint()
{
	CPaintDC dc(this);
	dc.SetBkMode(TRANSPARENT);

	CRect cmd_rect(10, 10, 450, 30);
	dc.DrawText(CString(CMD_EDIT_TITLE), -1, &cmd_rect, DT_SINGLELINE);

	mCmdEdit.SetFocus();
}

// ----------------------
//    Class CXyzDialog
// ----------------------

CXyzDialog::CXyzDialog(CString title, vec3 default_values)
	: CInputDialog(title), mX(default_values.x), mY(default_values.y), mZ(default_values.z)
{ }

CXyzDialog::~CXyzDialog()
{ }

vec3 CXyzDialog::GetXYZ()
{
	return vec3(mX, mY, mZ);
}

void CXyzDialog::DoDataExchange(CDataExchange* pDX)
{
	CInputDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_X_EDIT, mX);
	DDX_Text(pDX, IDC_Y_EDIT, mY);
	DDX_Text(pDX, IDC_Z_EDIT, mZ);
}

// CXyzDialog message handlers
BEGIN_MESSAGE_MAP(CXyzDialog, CInputDialog)
	ON_WM_CREATE()
	ON_WM_PAINT()
END_MESSAGE_MAP()

int CXyzDialog::OnCreate(LPCREATESTRUCT lpcs)
{
	mXEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
		CRect(130, 70, 340, 90), this, IDC_X_EDIT);

	mYEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
		CRect(130, 140, 340, 160), this, IDC_Y_EDIT);

	mZEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
		CRect(130, 210, 340, 230), this, IDC_Z_EDIT);

	return 0;
}

void CXyzDialog::OnPaint()
{
	CPaintDC dc(this);
	dc.SetBkMode(TRANSPARENT);

	CRect x_rect(100, 72, 450, 90);
	dc.DrawText(CString(X_EDIT_TITLE), -1, &x_rect, DT_SINGLELINE);

	CRect y_rect(100, 142, 450, 160);
	dc.DrawText(CString(Y_EDIT_TITLE), -1, &y_rect, DT_SINGLELINE);

	CRect z_rect(100, 212, 450, 230);
	dc.DrawText(CString(Z_EDIT_TITLE), -1, &z_rect, DT_SINGLELINE);

	mXEdit.SetFocus();
}

// -------------------------
//    Class CCmdXyzDialog
// -------------------------

CCmdXyzDialog::CCmdXyzDialog(CString title)
	: CInputDialog(title), mCmd(""), mX(0.0), mY(0.0), mZ(0.0)
{ }

CCmdXyzDialog::~CCmdXyzDialog()
{ }

string CCmdXyzDialog::GetCmd()
{
	return ((LPCTSTR)mCmd);
}

vec3 CCmdXyzDialog::GetXYZ()
{
	return vec3(mX, mY, mZ);
}

void CCmdXyzDialog::DoDataExchange(CDataExchange* pDX)
{
	CInputDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_CMD_EDIT, mCmd);
	DDX_Text(pDX, IDC_X_EDIT, mX);
	DDX_Text(pDX, IDC_Y_EDIT, mY);
	DDX_Text(pDX, IDC_Z_EDIT, mZ);
}

// CCmdXyzDialog message handlers
BEGIN_MESSAGE_MAP(CCmdXyzDialog, CInputDialog)
	ON_WM_CREATE()
	ON_WM_PAINT()
END_MESSAGE_MAP()

int CCmdXyzDialog::OnCreate(LPCREATESTRUCT lpcs)
{
	mCmdEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
		CRect(10, 30, 450, 100), this, IDC_CMD_EDIT);

	mXEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
		CRect(40, 135, 250, 155), this, IDC_X_EDIT);

	mYEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
		CRect(40, 190, 250, 210), this, IDC_Y_EDIT);

	mZEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
		CRect(40, 245, 250, 265), this, IDC_Z_EDIT);

	return 0;
}

void CCmdXyzDialog::OnPaint()
{
	CPaintDC dc(this);
	dc.SetBkMode(TRANSPARENT);

	CRect cmd_rect(10, 10, 450, 30);
	dc.DrawText(CString(CMD_EDIT_TITLE), -1, &cmd_rect, DT_SINGLELINE);

	CRect x_rect(10, 137, 450, 155);
	dc.DrawText(CString(X_EDIT_TITLE), -1, &x_rect, DT_SINGLELINE);

	CRect y_rect(10, 192, 450, 210);
	dc.DrawText(CString(Y_EDIT_TITLE), -1, &y_rect, DT_SINGLELINE);

	CRect z_rect(10, 247, 450, 265);
	dc.DrawText(CString(Z_EDIT_TITLE), -1, &z_rect, DT_SINGLELINE);

	mCmdEdit.SetFocus();
}


// ----------------------
//    Class COrthoDialog
// ----------------------

COrthoDialog::COrthoDialog(CString title)
	: CInputDialog(title), left(0), right(0), bottom(0), top(0), zNear(0), zFar(0)
{ }

COrthoDialog::~COrthoDialog()
{ }

float COrthoDialog::GetLeft() const
{
	return left;
}

float COrthoDialog::GetRight() const
{
	return right;
}

float COrthoDialog::GetBottom() const
{
	return bottom;
}

float COrthoDialog::GetTop() const
{
	return top;
}

float COrthoDialog::GetNear() const
{
	return zNear;
}

float COrthoDialog::GetFar() const
{
	return zFar;
}

void COrthoDialog::DoDataExchange(CDataExchange* pDX)
{
	CInputDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_LEFT_EDIT, left);
	DDX_Text(pDX, IDC_RIGHT_EDIT, right);
	DDX_Text(pDX, IDC_BOTTOM_EDIT, bottom);
	DDX_Text(pDX, IDC_TOP_EDIT, top);
	DDX_Text(pDX, IDC_NEAR_EDIT, zNear);
	DDX_Text(pDX, IDC_FAR_EDIT, zFar);
}

// COrthoDialog message handlers
BEGIN_MESSAGE_MAP(COrthoDialog, CInputDialog)
	ON_WM_CREATE()
	ON_WM_PAINT()
END_MESSAGE_MAP()

int COrthoDialog::OnCreate(LPCREATESTRUCT lpcs)
{
	int start_loc = 30;
	leftEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
		CRect(90, start_loc, 250, start_loc + 20), this, IDC_LEFT_EDIT);

	start_loc += 50;
	rightEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
		CRect(90, start_loc, 250, start_loc + 20), this, IDC_RIGHT_EDIT);

	start_loc += 50;
	bottomEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
		CRect(90, start_loc, 250, start_loc + 20), this, IDC_BOTTOM_EDIT);

	start_loc += 50;
	topEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
		CRect(90, start_loc, 250, start_loc + 20), this, IDC_TOP_EDIT);

	start_loc += 50;
	nearEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
		CRect(90, start_loc, 250, start_loc + 20), this, IDC_NEAR_EDIT);

	start_loc += 50;
	farEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
		CRect(90, start_loc, 250, start_loc + 20), this, IDC_FAR_EDIT);

	return 0;
}

void COrthoDialog::OnPaint()
{
	int start_loc = 32;
	CPaintDC dc(this);
	dc.SetBkMode(TRANSPARENT);

	CRect left_rect(50, start_loc, 350, start_loc + 18);
	dc.DrawText(CString(LEFT_EDIT_TITLE), -1, &left_rect, DT_SINGLELINE);

	start_loc += 50;
	CRect right_rect(50, start_loc, 350, start_loc + 18);
	dc.DrawText(CString(RIGHT_EDIT_TITLE), -1, &right_rect, DT_SINGLELINE);

	start_loc += 50;
	CRect bottom_rect(50, start_loc, 350, start_loc + 18);
	dc.DrawText(CString(BOTTOM_EDIT_TITLE), -1, &bottom_rect, DT_SINGLELINE);

	start_loc += 50;
	CRect top_rect(50, start_loc, 350, start_loc + 18);
	dc.DrawText(CString(TOP_EDIT_TITLE), -1, &top_rect, DT_SINGLELINE);

	start_loc += 50;
	CRect near_rect(50, start_loc, 350, start_loc + 18);
	dc.DrawText(CString(NEAR_EDIT_TITLE), -1, &near_rect, DT_SINGLELINE);

	start_loc += 50;
	CRect far_rect(50, start_loc, 450, start_loc + 18);
	dc.DrawText(CString(FAR_EDIT_TITLE), -1, &far_rect, DT_SINGLELINE);

	leftEdit.SetFocus();
}
