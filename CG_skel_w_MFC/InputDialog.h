
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

// ----------------------
//    Class CValueDialog
// ----------------------

class CValueDialog : public CInputDialog
{
public:
	CValueDialog(CString title = "Input Dialog", CString field = "Value:", float default_value = 0);
	virtual ~CValueDialog();

	float GetValue() const;

protected:
	float value;
	CString name;
	CEdit valueEdit;

	virtual void DoDataExchange(CDataExchange* pDX);

	afx_msg int OnCreate(LPCREATESTRUCT lpcs);
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
};

// ----------------------
//    Class CPerspectiveDialog
// ----------------------

class CPerspectiveDialog : public CInputDialog
{
public:
	CPerspectiveDialog(CString title = "Perspective Setting", bool is_horizontal = true);
	virtual ~CPerspectiveDialog();

	float GetFov() const;
	float GetAspect() const;
	float GetNear() const;
	float GetFar() const;

protected:

	bool is_horizontal;
	float fov;
	float aspect;
	float zNear;
	float zFar;
	CEdit fovEdit;
	CEdit aspectEdit;
	CEdit nearEdit;
	CEdit farEdit;

	virtual void DoDataExchange(CDataExchange* pDX);

	afx_msg int OnCreate(LPCREATESTRUCT lpcs);
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
};

// ----------------------
//    Class CLightDialog
// ----------------------
vec3 ColorToVec(COLORREF color);
COLORREF VecToColor(const vec3& color);

class CLightDialog : public CInputDialog {
public:
	CLightDialog(CString title = "Add light Source");
	virtual ~CLightDialog();

	vec3 GetColor() const;
	vec3 GetLightCoordinates() const;
	bool IsPoint() const;
protected:

	bool is_point;
	COLORREF color;
	vec3 coordinates;

	CButton point_radio;
	CButton parallel_radio;
	CEdit coordinatesxEdit;
	CEdit coordinatesyEdit;
	CEdit coordinateszEdit;
	CButton colorEdit;

	afx_msg void radio_pressed();
	afx_msg void choose_color();
	virtual void DoDataExchange(CDataExchange* pDX);
	
	afx_msg int OnCreate(LPCREATESTRUCT lpcs);
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()

};

// ----------------------
//    Class CEditModelDialog
// ----------------------
class CEditModelDialog : public CInputDialog {
public:
	CEditModelDialog(CString title = "Edit Model Colors");
	virtual ~CEditModelDialog();

	vec3 GetAmbientColor() const;
	vec3 GetSpecularColor() const;
	vec3 GetDiffuseColor() const;
	float GetShininess() const;
	bool ShouldRandom() const;
protected:

	COLORREF ambient;
	COLORREF specular;
	COLORREF diffuse;
	float shininess;
	bool should_random;
	
	CEdit shininessEdit;
	CButton ambientEdit;
	CButton specularEdit;
	CButton diffuseEdit;
	CButton randomEdit;

	afx_msg void set_ambient();
	afx_msg void set_specular();
	afx_msg void set_diffuse();
	afx_msg void update_random();
	virtual void DoDataExchange(CDataExchange* pDX);

	afx_msg int OnCreate(LPCREATESTRUCT lpcs);
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()

};

// ----------------------
//    Class Cv2c1Dialog
// ----------------------
class Cv2c1Dialog : public CInputDialog {
public:
	Cv2c1Dialog(CString title = "Add Fog", CString v1_title = "Value 1:", CString v2_title = "Value 2:", CString c_title = "Color:",const float v1_default = 0.004, const float v2_default= 0.008);
	virtual ~Cv2c1Dialog();

	float GetValue1() const;
	float GetValue2() const;
	vec3 GetColor() const;

protected:

	CString v1_t, v2_t, c_t;
	float v1;
	float v2;
	COLORREF color;

	CEdit v1Edit;
	CEdit v2Edit;
	CButton colorEdit;

	afx_msg void choose_color();
	virtual void DoDataExchange(CDataExchange* pDX);

	afx_msg int OnCreate(LPCREATESTRUCT lpcs);
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()

};

// ----------------------
//    Class CTextureDialog
// ----------------------
class CTextureDialog : public CInputDialog {
public:
	CTextureDialog(CString title = "Set Texture");
	virtual ~CTextureDialog();

	vec3 GetAmbientColor() const;
	vec3 GetSpecularColor() const;
	CString GetTexturePath() const;
	float GetShininess() const;
protected:

	COLORREF ambient;
	COLORREF specular;
	CString path;
	float shininess;

	CButton ambientEdit;
	CButton specularEdit;
	CButton pathEdit;
	CEdit shininessEdit;

	afx_msg void set_ambient();
	afx_msg void set_specular();
	afx_msg void set_path();
	virtual void DoDataExchange(CDataExchange* pDX);

	afx_msg int OnCreate(LPCREATESTRUCT lpcs);
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
};

// ----------------------
//    Class CAnimationDialog
// ----------------------
class CAnimationDialog : public CInputDialog {
public:
	enum AnimationType {
		HUE=0, SAT=1, LUM=2
	};
	CAnimationDialog(CString title = "Add Animation", const bool isColorAnimation = true);
	virtual ~CAnimationDialog();

	AnimationType GetAnimationType() const;
	GLfloat GetDuration() const;
	GLfloat GetSpeed() const;
protected:

	bool is_color_animation;
	AnimationType animation_type;
	GLfloat duration;
	GLfloat speed;
	
	CButton hue_radio;
	CButton sat_radio;
	CButton lum_radio;
	CEdit durationEdit;
	CEdit speedEdit;

	afx_msg void radio_pressed();
	virtual void DoDataExchange(CDataExchange* pDX);

	afx_msg int OnCreate(LPCREATESTRUCT lpcs);
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()

};
