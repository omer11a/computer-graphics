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

#define IDC_FOV_EDIT		210
#define IDC_ASPECT_EDIT		211
#define IDC_VALUE_EDIT		212

#define IDC_COLOR_EDIT		300
#define IDC_POINT_EDIT		301
#define IDC_PARALLEL_EDIT	302

#define CMD_EDIT_TITLE "Command"
#define X_EDIT_TITLE "X:"
#define Y_EDIT_TITLE "Y:"
#define Z_EDIT_TITLE "Z:"

#define LEFT_EDIT_TITLE "Left:"
#define RIGHT_EDIT_TITLE "Right:"
#define BOTTOM_EDIT_TITLE "Bottom:"
#define TOP_EDIT_TITLE "Top:"
#define NEAR_EDIT_TITLE "Near:"
#define FAR_EDIT_TITLE "Far:"



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
	int start_loc = 50;
	leftEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
		CRect(110, start_loc, 230, start_loc + 20), this, IDC_LEFT_EDIT);
	
	rightEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
		CRect(330, start_loc, 450, start_loc + 20), this, IDC_RIGHT_EDIT);

	start_loc += 50;
	bottomEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
		CRect(110, start_loc, 230, start_loc + 20), this, IDC_BOTTOM_EDIT);
	
	topEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
		CRect(330, start_loc, 450, start_loc + 20), this, IDC_TOP_EDIT);

	start_loc += 50;
	nearEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
		CRect(110, start_loc, 230, start_loc + 20), this, IDC_NEAR_EDIT);
	
	farEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
		CRect(330, start_loc, 450, start_loc + 20), this, IDC_FAR_EDIT);

	return 0;
}

void COrthoDialog::OnPaint()
{
	int start_loc = 52;
	CPaintDC dc(this);
	dc.SetBkMode(TRANSPARENT);

	CRect left_rect(40, start_loc, 170, start_loc + 18);
	dc.DrawText(CString(LEFT_EDIT_TITLE), -1, &left_rect, DT_SINGLELINE);

	CRect right_rect(260, start_loc, 410, start_loc + 18);
	dc.DrawText(CString(RIGHT_EDIT_TITLE), -1, &right_rect, DT_SINGLELINE);

	start_loc += 50;
	CRect bottom_rect(40, start_loc, 170, start_loc + 18);
	dc.DrawText(CString(BOTTOM_EDIT_TITLE), -1, &bottom_rect, DT_SINGLELINE);

	CRect top_rect(260, start_loc, 410, start_loc + 18);
	dc.DrawText(CString(TOP_EDIT_TITLE), -1, &top_rect, DT_SINGLELINE);

	start_loc += 50;
	CRect near_rect(40, start_loc, 170, start_loc + 18);
	dc.DrawText(CString(NEAR_EDIT_TITLE), -1, &near_rect, DT_SINGLELINE);

	CRect far_rect(260, start_loc, 410, start_loc + 18);
	dc.DrawText(CString(FAR_EDIT_TITLE), -1, &far_rect, DT_SINGLELINE);

	leftEdit.SetFocus();
}

// ----------------------
//    Class CValueDialog
// ----------------------

CValueDialog::CValueDialog(CString title, CString field, float default_value)
	: CInputDialog(title), name(field), value(default_value)
{ }

CValueDialog::~CValueDialog()
{ }

float CValueDialog::GetValue() const
{
	return value;
}

void CValueDialog::DoDataExchange(CDataExchange* pDX)
{
	CInputDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_VALUE_EDIT, value);
}

// CValueDialog message handlers
BEGIN_MESSAGE_MAP(CValueDialog, CInputDialog)
	ON_WM_CREATE()
	ON_WM_PAINT()
END_MESSAGE_MAP()

int CValueDialog::OnCreate(LPCREATESTRUCT lpcs)
{
	valueEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
		CRect(200, 140, 400, 160), this, IDC_VALUE_EDIT);

	return 0;
}

void CValueDialog::OnPaint()
{
	CPaintDC dc(this);
	dc.SetBkMode(TRANSPARENT);

	CRect value_rect(0, 142, 190, 160);
	dc.DrawText(name, -1, &value_rect, DT_SINGLELINE | DT_RIGHT);

	valueEdit.SetFocus();
}

// ----------------------
//    Class CPerspectiveDialog
// ----------------------

CPerspectiveDialog::CPerspectiveDialog(CString title, bool is_horizontal)
	: CInputDialog(title), is_horizontal(is_horizontal), fov(0), aspect(0), zNear(0), zFar(0)
{ }

CPerspectiveDialog::~CPerspectiveDialog()
{ }

float CPerspectiveDialog::GetFov() const
{
	return fov;
}

float CPerspectiveDialog::GetAspect() const
{
	return aspect;
}

float CPerspectiveDialog::GetNear() const
{
	return zNear;
}

float CPerspectiveDialog::GetFar() const
{
	return zFar;
}

void CPerspectiveDialog::DoDataExchange(CDataExchange* pDX)
{
	CInputDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_FOV_EDIT, fov);
	DDX_Text(pDX, IDC_ASPECT_EDIT, aspect);
	DDX_Text(pDX, IDC_NEAR_EDIT, zNear);
	DDX_Text(pDX, IDC_FAR_EDIT, zFar);
}

// CPerspectiveDialog message handlers
BEGIN_MESSAGE_MAP(CPerspectiveDialog, CInputDialog)
	ON_WM_CREATE()
	ON_WM_PAINT()
END_MESSAGE_MAP()

int CPerspectiveDialog::OnCreate(LPCREATESTRUCT lpcs)
{
	int start_loc = 50;
	fovEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
		CRect(130, start_loc, 240, start_loc + 20), this, IDC_FOV_EDIT);


	aspectEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
		CRect(350, start_loc, 450, start_loc + 20), this, IDC_ASPECT_EDIT);

	start_loc += 80;
	nearEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
		CRect(130, start_loc, 240, start_loc + 20), this, IDC_NEAR_EDIT);


	farEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
		CRect(350, start_loc, 450, start_loc + 20), this, IDC_FAR_EDIT);

	return 0;
}

void CPerspectiveDialog::OnPaint()
{
	int start_loc = 52;
	CPaintDC dc(this);
	dc.SetBkMode(TRANSPARENT);

	CRect fov_rect(0, start_loc, 120, start_loc + 18);
	dc.DrawText((is_horizontal ? "Horizontal FOV:" : "Vertical FOV:"), -1, &fov_rect, DT_SINGLELINE | DT_RIGHT);

	CRect aspect_rect(290, start_loc, 340, start_loc + 18);
	dc.DrawText("Aspect:", -1, &aspect_rect, DT_SINGLELINE | DT_RIGHT);

	start_loc += 80;
	CRect near_rect(40, start_loc, 120, start_loc + 18);
	dc.DrawText(CString(NEAR_EDIT_TITLE), -1, &near_rect, DT_SINGLELINE | DT_RIGHT);

	CRect far_rect(290, start_loc, 340, start_loc + 18);
	dc.DrawText(CString(FAR_EDIT_TITLE), -1, &far_rect, DT_SINGLELINE | DT_RIGHT);

	fovEdit.SetFocus();
}

// ----------------------
//    Class CLightDialog
// ----------------------

vec3 color2vec(COLORREF color) {
	return vec3();
}

void CLightDialog::choose_color() {
	CColorDialog dlg;
	if (dlg.DoModal() == IDOK) {
		color = dlg.GetColor();
	}
}

CLightDialog::CLightDialog(CString title)
	: CInputDialog(title), l_location(), l_size(), is_point(true)
{ }

CLightDialog::~CLightDialog()
{ }

vec3 CLightDialog::GetColor() const
{
	return color2vec(color);
}

vec3 CLightDialog::GetLLocation() const
{
	return l_location;
}

vec3 CLightDialog::GetLSize() const
{
	return l_size;
}

bool CLightDialog::IsPoint() const
{
	return is_point;
}

void CLightDialog::DoDataExchange(CDataExchange* pDX)
{
	CInputDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_X_EDIT, l_location.x);
	DDX_Text(pDX, IDC_Y_EDIT, l_location.y);
	DDX_Text(pDX, IDC_Z_EDIT, l_location.z);
	DDX_Text(pDX, IDC_X_EDIT + 50, l_size.x);
	DDX_Text(pDX, IDC_Y_EDIT + 50, l_size.y);
	DDX_Text(pDX, IDC_Z_EDIT + 50, l_size.z);
}

// CLightDialog message handlers
BEGIN_MESSAGE_MAP(CLightDialog, CInputDialog)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_COLOR_EDIT, choose_color)
END_MESSAGE_MAP()

int CLightDialog::OnCreate(LPCREATESTRUCT lpcs)
{
	int height = 110;
	int lsx = 100, lex = 150;
	int ssx = 300, sex = 350;

	point_radio.Create("Point", BS_AUTORADIOBUTTON | WS_CHILD | WS_VISIBLE | WS_TABSTOP,
		CRect(150, 10, 230, 40), this, IDC_PARALLEL_EDIT);
	parallel_radio.Create("Parallel", BS_AUTORADIOBUTTON | WS_CHILD | WS_VISIBLE | WS_TABSTOP,
		CRect(350, 10, 420, 40), this, IDC_POINT_EDIT);

	locxEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
		CRect(lsx, height, lex, height + 20), this, IDC_X_EDIT);
	sizexEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
		CRect(ssx, height, sex, height + 20), this, IDC_X_EDIT + 50);
	height += 40;

	locyEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
		CRect(lsx, height, lex, height + 20), this, IDC_Y_EDIT);
	sizeyEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
		CRect(ssx, height, sex, height + 20), this, IDC_Y_EDIT + 50);
	height += 40;

	loczEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
		CRect(lsx, height, lex, height + 20), this, IDC_Z_EDIT);
	sizezEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
		CRect(ssx, height, sex, height + 20), this, IDC_Z_EDIT + 50);
	height += 60;

	colorEdit.Create("Choose...", BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
		CRect(120, height, 200, height + 20), this, IDC_COLOR_EDIT);

	return 0;
}

void CLightDialog::OnPaint()
{
	CPaintDC dc(this);
	dc.SetBkMode(TRANSPARENT);
	int height = 72;

	CRect loc_rect(50, height, 150, height + 18);
	dc.DrawText("Location:", -1, &loc_rect, DT_SINGLELINE | DT_CENTER);
	CRect size_rect(250, height, 350, height + 18);
	dc.DrawText("Size:", -1, &size_rect, DT_SINGLELINE | DT_CENTER);
	height += 40;

	CRect locx_rect(50, height, 80, height + 18);
	dc.DrawText("X:", -1, &locx_rect, DT_SINGLELINE | DT_RIGHT);
	CRect sizex_rect(250, height, 280, height + 18);
	dc.DrawText("X:", -1, &sizex_rect, DT_SINGLELINE | DT_RIGHT);
	height += 40;

	CRect locy_rect(50, height, 80, height + 18);
	dc.DrawText("Y:", -1, &locy_rect, DT_SINGLELINE | DT_RIGHT);
	CRect sizey_rect(250, height, 280, height + 18);
	dc.DrawText("Y:", -1, &sizey_rect, DT_SINGLELINE | DT_RIGHT);
	height += 40;

	CRect locz_rect(50, height, 80, height + 18);
	dc.DrawText("Z:", -1, &locz_rect, DT_SINGLELINE | DT_RIGHT);
	CRect sizez_rect(250, height, 280, height + 18);
	dc.DrawText("Z:", -1, &sizez_rect, DT_SINGLELINE | DT_RIGHT);
	height += 60;

	CRect color_rect(50, height, 100, height + 18);
	dc.DrawText("Color:", -1, &color_rect, DT_SINGLELINE | DT_RIGHT);

	locxEdit.SetFocus();
}