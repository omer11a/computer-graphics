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
#define IDC_RADIO1_EDIT		301
#define IDC_RADIO2_EDIT		302
#define IDC_RADIO3_EDIT		303

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
vec3 ColorToVec(COLORREF color) {
	return vec3(color & 0xFF, (color & 0xFF00) >> 8, (color & 0xFF0000) >> 16) / 255;
}

COLORREF VecToColor(const vec3& color) {

	return ((color.x) + (static_cast<int>(color.y) << 8) + (static_cast<int>(color.z) << 16)) * 255;
}

void CLightDialog::choose_color() {
	CColorDialog dlg;
	if (dlg.DoModal() == IDOK) {
		color = dlg.GetColor();
	}
}

void CLightDialog::radio_pressed() {
	is_point = point_radio.GetCheck();
}

CLightDialog::CLightDialog(CString title)
	: CInputDialog(title), coordinates(), is_point(true)
{ }

CLightDialog::~CLightDialog()
{ }

vec3 CLightDialog::GetColor() const
{
	return ColorToVec(color);
}

vec3 CLightDialog::GetLightCoordinates() const
{
	return coordinates;
}

bool CLightDialog::IsPoint() const
{
	return is_point;
}

void CLightDialog::DoDataExchange(CDataExchange* pDX)
{
	CInputDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_X_EDIT, coordinates.x);
	DDX_Text(pDX, IDC_Y_EDIT, coordinates.y);
	DDX_Text(pDX, IDC_Z_EDIT, coordinates.z);
}

// CLightDialog message handlers
BEGIN_MESSAGE_MAP(CLightDialog, CInputDialog)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_COLOR_EDIT, choose_color)
	ON_BN_CLICKED(IDC_RADIO2_EDIT, radio_pressed)
	ON_BN_CLICKED(IDC_RADIO1_EDIT, radio_pressed)
END_MESSAGE_MAP()

int CLightDialog::OnCreate(LPCREATESTRUCT lpcs)
{
	int height = 110;
	int lsx = 220, lex = 300;

	point_radio.Create("Point", BS_AUTORADIOBUTTON | WS_CHILD | WS_VISIBLE | WS_TABSTOP,
		CRect(200, 30, 280, 45), this, IDC_RADIO1_EDIT);
	point_radio.SetCheck(true);
	parallel_radio.Create("Parallel", BS_AUTORADIOBUTTON | WS_CHILD | WS_VISIBLE | WS_TABSTOP,
		CRect(300, 30, 380, 45), this, IDC_RADIO2_EDIT);

	coordinatesxEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
		CRect(lsx, height, lex, height + 20), this, IDC_X_EDIT);
	height += 40;

	coordinatesyEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
		CRect(lsx, height, lex, height + 20), this, IDC_Y_EDIT);
	height += 40;

	coordinateszEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
		CRect(lsx, height, lex, height + 20), this, IDC_Z_EDIT);
	height += 50;

	colorEdit.Create("Choose...", BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
		CRect(220, height, 300, height + 20), this, IDC_COLOR_EDIT);

	return 0;
}

void CLightDialog::OnPaint()
{
	CPaintDC dc(this);
	dc.SetBkMode(TRANSPARENT);
	int height = 32;

	CRect rect(100, height, 250, height + 18);
	dc.DrawText("Light Type:", -1, &rect, DT_SINGLELINE);

	rect.bottom += 50;
	rect.top += 50;
	dc.DrawText("Location / Direction:", -1, &rect, DT_SINGLELINE);
	
	rect.bottom += 30;
	rect.top += 30;
	dc.DrawText("X:", -1, &rect, DT_SINGLELINE);

	rect.bottom += 40;
	rect.top += 40; 
	dc.DrawText("Y:", -1, &rect, DT_SINGLELINE);

	rect.bottom += 40;
	rect.top += 40;
	dc.DrawText("Z:", -1, &rect, DT_SINGLELINE);

	rect.bottom += 50;
	rect.top += 50;
	dc.DrawText("Color:", -1, &rect, DT_SINGLELINE);

	coordinatesxEdit.SetFocus();
}

// ----------------------
//    Class CEditModelDialog
// ----------------------
void CEditModelDialog::set_ambient() {
	CColorDialog dlg;
	if (dlg.DoModal() == IDOK) {
		ambient = dlg.GetColor();
	}
}

void CEditModelDialog::set_specular() {
	CColorDialog dlg;
	if (dlg.DoModal() == IDOK) {
		specular = dlg.GetColor();
	}
}

void CEditModelDialog::set_diffuse() {
	CColorDialog dlg;
	if (dlg.DoModal() == IDOK) {
		diffuse = dlg.GetColor();
	}
}

void CEditModelDialog::update_random() {
	should_random = randomEdit.GetCheck();
}

CEditModelDialog::CEditModelDialog(CString title)
	: CInputDialog(title), ambient(0xFFFFFF), specular(0), diffuse(0), shininess(1), should_random(false)
{ }

CEditModelDialog::~CEditModelDialog()
{ }

vec3 CEditModelDialog::GetAmbientColor() const
{
	return ColorToVec(ambient);
}

vec3 CEditModelDialog::GetSpecularColor() const
{
	return ColorToVec(specular);
}

vec3 CEditModelDialog::GetDiffuseColor() const
{
	return ColorToVec(diffuse);
}

float CEditModelDialog::GetShininess() const
{
	return shininess;
}

bool CEditModelDialog::ShouldRandom() const
{
	return should_random;
}

void CEditModelDialog::DoDataExchange(CDataExchange* pDX)
{
	CInputDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_VALUE_EDIT, shininess);
}

// CEditModelDialog message handlers
BEGIN_MESSAGE_MAP(CEditModelDialog, CInputDialog)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_COLOR_EDIT, set_ambient)
	ON_BN_CLICKED(IDC_COLOR_EDIT + 1, set_specular)
	ON_BN_CLICKED(IDC_COLOR_EDIT + 2, set_diffuse)
	ON_BN_CLICKED(IDC_VALUE_EDIT + 1, update_random)
END_MESSAGE_MAP()

int CEditModelDialog::OnCreate(LPCREATESTRUCT lpcs)
{
	int height = 70;
	int sx = 220, ex = 300;

	ambientEdit.Create("Choose...", BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
		CRect(sx, height, ex, height + 20), this, IDC_COLOR_EDIT);
	height += 40;

	specularEdit.Create("Choose...", BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
		CRect(sx, height, ex, height + 20), this, IDC_COLOR_EDIT + 1);
	height += 40;

	diffuseEdit.Create("Choose...", BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
		CRect(sx, height, ex, height + 20), this, IDC_COLOR_EDIT + 2);
	height += 40;

	shininessEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
		CRect(sx, height, ex, height + 20), this, IDC_VALUE_EDIT);
	height += 40;

	randomEdit.Create("Choose...", BS_AUTOCHECKBOX | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
		CRect(sx, height, ex, height + 20), this, IDC_VALUE_EDIT + 1);

	return 0;
}

void CEditModelDialog::OnPaint()
{
	CPaintDC dc(this);
	dc.SetBkMode(TRANSPARENT);
	int height = 72;

	CRect rect(50, height, 200, height + 18);
	dc.DrawText("Ambient:", -1, &rect, DT_SINGLELINE);

	rect.bottom += 40;
	rect.top += 40;
	dc.DrawText("Specular:", -1, &rect, DT_SINGLELINE);

	rect.bottom += 40;
	rect.top += 40;
	dc.DrawText("Diffuse:", -1, &rect, DT_SINGLELINE);

	rect.bottom += 40;
	rect.top += 40;
	dc.DrawText("Shininess:", -1, &rect, DT_SINGLELINE);

	rect.bottom += 40;
	rect.top += 40;
	dc.DrawText("Use Random Materials:", -1, &rect, DT_SINGLELINE);

	ambientEdit.SetFocus();
}

// ----------------------
//    Class Cv2c1Dialog
// ----------------------
void Cv2c1Dialog::choose_color() {
	CColorDialog dlg;
	if (dlg.DoModal() == IDOK) {
		color = dlg.GetColor();
	}
}

Cv2c1Dialog::Cv2c1Dialog(CString title, CString v1_title, CString v2_title, CString c_title, const float v1_default, const float v2_default)
	: CInputDialog(title), v1_t(v1_title), v2_t(v2_title), v1(v1_default), v2(v2_default)
{ }

Cv2c1Dialog::~Cv2c1Dialog()
{ }

vec3 Cv2c1Dialog::GetColor() const
{
	return ColorToVec(color);
}

float Cv2c1Dialog::GetValue1() const
{
	return v1;
}

float Cv2c1Dialog::GetValue2() const
{
	return v2;
}

void Cv2c1Dialog::DoDataExchange(CDataExchange* pDX)
{
	CInputDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_X_EDIT, v1);
	DDX_Text(pDX, IDC_Y_EDIT, v2);
}

// Cv2c1Dialog message handlers
BEGIN_MESSAGE_MAP(Cv2c1Dialog, CInputDialog)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_COLOR_EDIT, choose_color)
END_MESSAGE_MAP()

int Cv2c1Dialog::OnCreate(LPCREATESTRUCT lpcs)
{
	int height = 80;
	int lsx = 250, lex = 330;

	v1Edit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
		CRect(lsx, height, lex, height + 20), this, IDC_X_EDIT);
	height += 60;

	v2Edit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
		CRect(lsx, height, lex, height + 20), this, IDC_Y_EDIT);
	height += 60;

	colorEdit.Create("Choose...", BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
		CRect(lsx, height, lex, height + 20), this, IDC_COLOR_EDIT);

	return 0;
}

void Cv2c1Dialog::OnPaint()
{
	CPaintDC dc(this);
	dc.SetBkMode(TRANSPARENT);
	int height = 82;

	CRect rect(50, height, 250, height + 18);
	dc.DrawText(v1_t, -1, &rect, DT_SINGLELINE);

	rect.bottom += 60;
	rect.top += 60;
	dc.DrawText(v2_t, -1, &rect, DT_SINGLELINE);

	rect.bottom += 60;
	rect.top += 60;
	dc.DrawText(c_t, -1, &rect, DT_SINGLELINE);

	v1Edit.SetFocus();
}

// ----------------------
//    Class CTextureDialog
// ----------------------
void CTextureDialog::set_ambient() {
	CColorDialog dlg;
	if (dlg.DoModal() == IDOK) {
		ambient = dlg.GetColor();
	}
}

void CTextureDialog::set_specular() {
	CColorDialog dlg;
	if (dlg.DoModal() == IDOK) {
		specular = dlg.GetColor();
	}
}

void CTextureDialog::set_path() {
	CFileDialog tfdlg(TRUE, _T(".png"), NULL, NULL, _T("*.png|*.*"));
	if (tfdlg.DoModal() == IDOK) {
		path = tfdlg.GetPathName();
	}
}

CTextureDialog::CTextureDialog(CString title)
	: CInputDialog(title), ambient(0xFFFFFF), specular(0), path(""), shininess(1)
{ }

CTextureDialog::~CTextureDialog()
{ }

vec3 CTextureDialog::GetAmbientColor() const
{
	return ColorToVec(ambient);
}

vec3 CTextureDialog::GetSpecularColor() const
{
	return ColorToVec(specular);
}

CString CTextureDialog::GetTexturePath() const
{
	return path;
}

float CTextureDialog::GetShininess() const
{
	return shininess;
}

void CTextureDialog::DoDataExchange(CDataExchange* pDX)
{
	CInputDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_VALUE_EDIT, shininess);
}

// CTextureDialog message handlers
BEGIN_MESSAGE_MAP(CTextureDialog, CInputDialog)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_COLOR_EDIT, set_ambient)
	ON_BN_CLICKED(IDC_COLOR_EDIT + 1, set_specular)
	ON_BN_CLICKED(IDC_COLOR_EDIT + 2, set_path)
END_MESSAGE_MAP()

int CTextureDialog::OnCreate(LPCREATESTRUCT lpcs)
{
	int height = 70;
	int sx = 220, ex = 300;

	ambientEdit.Create("Choose...", BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
		CRect(sx, height, ex, height + 20), this, IDC_COLOR_EDIT);
	height += 40;

	specularEdit.Create("Choose...", BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
		CRect(sx, height, ex, height + 20), this, IDC_COLOR_EDIT + 1);
	height += 40;

	pathEdit.Create("Browse...", BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
		CRect(sx, height, ex, height + 20), this, IDC_COLOR_EDIT + 2);
	height += 40;

	shininessEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
		CRect(sx, height, ex, height + 20), this, IDC_VALUE_EDIT);
	height += 40;

	return 0;
}

void CTextureDialog::OnPaint()
{
	CPaintDC dc(this);
	dc.SetBkMode(TRANSPARENT);
	int height = 72;

	CRect rect(50, height, 200, height + 18);
	dc.DrawText("Ambient:", -1, &rect, DT_SINGLELINE);

	rect.bottom += 40;
	rect.top += 40;
	dc.DrawText("Specular:", -1, &rect, DT_SINGLELINE);

	rect.bottom += 40;
	rect.top += 40;
	dc.DrawText("Texture Path:", -1, &rect, DT_SINGLELINE);

	rect.bottom += 40;
	rect.top += 40;
	dc.DrawText("Shininess:", -1, &rect, DT_SINGLELINE);

	ambientEdit.SetFocus();
}

// ----------------------
//    Class CAnimationDialog
// ----------------------

void CAnimationDialog::radio_pressed() {
	if (hue_radio.GetCheck()) {
		animation_type = HUE;
	} else if (sat_radio.GetCheck()) {
		animation_type = SAT;
	} else {
		animation_type = LUM;
	}
}

CAnimationDialog::CAnimationDialog(CString title, const bool isColorAnimation)
	: CInputDialog(title), is_color_animation(isColorAnimation), animation_type(HUE), duration(0), speed(0)
{ }

CAnimationDialog::~CAnimationDialog()
{ }

CAnimationDialog::AnimationType CAnimationDialog::GetAnimationType() const
{
	return animation_type;
}

GLfloat CAnimationDialog::GetDuration() const
{
	return duration;
}

GLfloat CAnimationDialog::GetSpeed() const
{
	return speed;
}



void CAnimationDialog::DoDataExchange(CDataExchange* pDX)
{
	CInputDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_VALUE_EDIT, duration);
	DDX_Text(pDX, IDC_VALUE_EDIT + 1, speed);
}

// CAnimationDialog message handlers
BEGIN_MESSAGE_MAP(CAnimationDialog, CInputDialog)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_RADIO2_EDIT, radio_pressed)
	ON_BN_CLICKED(IDC_RADIO1_EDIT, radio_pressed)
END_MESSAGE_MAP()

int CAnimationDialog::OnCreate(LPCREATESTRUCT lpcs)
{
	int height = 140;
	int lsx = 220, lex = 300;

	if (is_color_animation) {
		hue_radio.Create("Hue", BS_AUTORADIOBUTTON | WS_CHILD | WS_VISIBLE | WS_TABSTOP,
			CRect(200, 60, 260, 75), this, IDC_RADIO1_EDIT);
		hue_radio.SetCheck(true);
		sat_radio.Create("Sat", BS_AUTORADIOBUTTON | WS_CHILD | WS_VISIBLE | WS_TABSTOP,
			CRect(265, 60, 315, 75), this, IDC_RADIO2_EDIT);
		lum_radio.Create("Lum", BS_AUTORADIOBUTTON | WS_CHILD | WS_VISIBLE | WS_TABSTOP,
			CRect(320, 60, 380, 75), this, IDC_RADIO3_EDIT);
	}
	durationEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
		CRect(lsx, height, lex, height + 20), this, IDC_VALUE_EDIT);
	height += 40;

	speedEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
		CRect(lsx, height, lex, height + 20), this, IDC_VALUE_EDIT + 1);
	height += 40;

	return 0;
}

void CAnimationDialog::OnPaint()
{
	CPaintDC dc(this);
	dc.SetBkMode(TRANSPARENT);
	int height = 62;

	CRect rect(90, height, 250, height + 18);
	if (is_color_animation) {
		dc.DrawText("Animation Type:", -1, &rect, DT_SINGLELINE);
	}

	rect.bottom += 50;
	rect.top += 50;
	dc.DrawText("Animation Parameters:", -1, &rect, DT_SINGLELINE);

	rect.bottom += 30;
	rect.top += 30;
	dc.DrawText("Duration:", -1, &rect, DT_SINGLELINE);

	rect.bottom += 40;
	rect.top += 40;
	dc.DrawText("Speed:", -1, &rect, DT_SINGLELINE);

	durationEdit.SetFocus();
}

// ----------------------
//    Class Cc2Dialog
// ----------------------
void Cc2Dialog::set_c1() {
	CColorDialog dlg;
	if (dlg.DoModal() == IDOK) {
		c1 = dlg.GetColor();
	}
}

void Cc2Dialog::set_c2() {
	CColorDialog dlg;
	if (dlg.DoModal() == IDOK) {
		c2 = dlg.GetColor();
	}
}

Cc2Dialog::Cc2Dialog(const CString title, const CString c1_title, const CString c2_title)
	: CInputDialog(title), color1_title(c1_title), c1(0xFFFFFF), c2(0)
{ }

Cc2Dialog::~Cc2Dialog()
{ }

vec3 Cc2Dialog::GetColor1() const
{
	return ColorToVec(c1);
}

vec3 Cc2Dialog::GetColor2() const
{
	return ColorToVec(c2);
}

void Cc2Dialog::DoDataExchange(CDataExchange* pDX)
{
	CInputDialog::DoDataExchange(pDX);
	
}

// CEditModelDialog message handlers
BEGIN_MESSAGE_MAP(Cc2Dialog, CInputDialog)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_COLOR_EDIT, set_c1)
	ON_BN_CLICKED(IDC_COLOR_EDIT + 1, set_c2)
END_MESSAGE_MAP()

int Cc2Dialog::OnCreate(LPCREATESTRUCT lpcs)
{
	int height = 70;
	int sx = 220, ex = 300;

	c1Edit.Create("Choose...", BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
		CRect(sx, height, ex, height + 20), this, IDC_COLOR_EDIT);
	height += 40;

	c2Edit.Create("Choose...", BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
		CRect(sx, height, ex, height + 20), this, IDC_COLOR_EDIT + 1);
	height += 40;

	return 0;
}

void Cc2Dialog::OnPaint()
{
	CPaintDC dc(this);
	dc.SetBkMode(TRANSPARENT);
	int height = 72;

	CRect rect(50, height, 200, height + 18);
	dc.DrawText(color1_title, -1, &rect, DT_SINGLELINE);

	rect.bottom += 40;
	rect.top += 40;
	dc.DrawText(color2_title, -1, &rect, DT_SINGLELINE);

	c1Edit.SetFocus();
}
