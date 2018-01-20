// CG_skel_w_MFC.cpp : Defines the entry point for the console application.
//
/*
keyboard commands:	(keys	->	action)
general:
ESC/q	->	exit
DEL		-> clear screen

modes:
w/W		->	enter world mode
m/M		->	enter model mode
v/V		->	enter camera view mode
c/C		->	enter camera world mode


transformations:
+/-		->	control scaling in current mode
r/R		->	control rotation in current mode
t/T		->	control translation in current mode

active objects:
</>		->	move between cameras
l		->	set camera look at
P/p		->	set camera perspective-Horizontal / perspective-Vertical
o/f		->	set camera orthogonal / frustum
Z/z		->	set zoom in/out

*		->	change active light
s		->	set camera/light visibility

TAB		->	change active models
b		->	switch model bounding box visibility
n		->	switch model normal visibility
N		->	switch model face normal visibility
y		->	switch wire view

PrimMeshModels:
1-9		-> add model
*/
#include "stdafx.h"
#include "CG_skel_w_MFC.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// The one and only application object

#include "GL/glew.h"
#include "GL/freeglut.h"
#include "GL/freeglut_ext.h"
#include "vec.h"
#include "mat.h"
#include "InputDialog.h"
#include "Shader.h"
#include "Renderer.h"
#include "Scene.h"
#include <ctime>
#include <string>

#define BUFFER_OFFSET( offset )   ((GLvoid*) (offset))

// model menu
#define NEW_ITEM 1
#define EDIT_ITEM 2
#define ADD_TEXTURE 3
#define ADD_NORMAL_MAP 4
#define DEL_NORMAL_MAP 5
#define ADD_COLOR_ANIMATION 6
#define DEL_COLOR_ANIMATION 7
#define ADD_VERTEX_ANIMATION 8
#define DEL_VERTEX_ANIMATION 9
#define ADD_TOON_SHADING 10
#define DEL_TOON_SHADING 11
#define ADD_WOOD_TEXTURE 12
#define DEL_WOOD_TEXTURE 13


// light extra menu
#define AMBIENT 3

// shaders menu
#define FLAT 1
#define GOURAUD 2
#define PHONG 3
#define FOG 4
#define FOG_DISABLE 5


#define ADD_CAMERA 1

// main menu
#define MAIN_DEMO 1
#define MAIN_HELP 2
#define MAIN_ABOUT 3

// setting menu
#define SETTING_SCALING		1
#define SETTING_ROTATION	2
#define SETTING_MOVEMENT	3
#define SETTING_ZOOM		4
#define SETTING_AA			5

#define MODEL_OBJECT	'm'
#define MODEL_WORLD		'w'
#define CAMERA_OBJECT	'v'
#define CAMERA_WORLD	'c'
#define LIGHT_OBJECT	'x'
#define LIGHT_WORLD		'd'

typedef struct configuration_s {
	unsigned char mode;	// 0 - none
	vec3 scaling;	// must be greater or equal to 1	(decreasing scaling is 1/scaling)
	vec3 translating;
	vec3 rotating;
	float zoom;
	bool is_demo;
} configuration_t;

Scene *scene;
Renderer *renderer;
configuration_t config;

int last_x,last_y;
bool lb_down,rb_down,mb_down;
unsigned char transformation_mode;
clock_t last_time;
//----------------------------------------------------------------------------
// Callbacks

void animation()
{
	clock_t t = clock();
	float delta = ((t - last_time) * 1.0f) / CLOCKS_PER_SEC;
	if (delta > 2) {// to avoid jumps after pausing for dialogs
		delta = 0.1;
	}
	if (delta > 0.05) {
		//cout << "hi" << endl;
		last_time = t;
		scene->stepAnimations(delta);
		glutPostRedisplay();
	}
}

void display(void)
{
	//Call the scene and ask it to draw itself
	redraw();
}

void reshape(int width, int height)
{
	//update the renderer's buffers
	renderer->UpdateBuffers(width, height);
	redraw();
}

void keyboard(unsigned char key, int x, int y)
{
	bool should_redraw = false;
	switch (key) {
	case 'q':
	case 033:
		exit(EXIT_SUCCESS);
		break;
	case 'a': // secret conf
	case 'A':
		if (key == 'a') {
			modelMenu(NEW_ITEM);
		} else {
			fileMenu(ADD_CAMERA);
		}
		break;
	case 127:
		// clear screen
		scene->clear();
		config.is_demo = false;
		should_redraw = true;
		break;
	case '\b':
		// clear active object
		should_redraw = delete_active_object();
		break;
	case '\t':
		change_active_model();
		break;
	case '*':
		change_active_light();
		break;

	// camera operations
	case '>':
		scene->nextCamera();
		cout << "switched active camera" << endl;
		should_redraw = true;
		break;
	case '<':
		scene->prevCamera();
		cout << "switched active camera" << endl;
		should_redraw = true;
		break;
	case 'y':
		renderer->SwitchWire();
		should_redraw = true;
		break;
	case 'l':
		should_redraw = set_lookat();
		break;
	case 'o':
		should_redraw = set_ortho();
		break;
	case 'f':
		should_redraw = set_frustum();
		break;
	case 'p':
	case 'P':
		should_redraw = set_perspective();
		break;
	case 'z':
	case 'Z':
		should_redraw = zoom(key);
		break;

	// switch modes
	case MODEL_OBJECT:
	case 'M':
	case MODEL_WORLD:
	case 'W':
	case CAMERA_OBJECT:
	case 'V':
	case CAMERA_WORLD:
	case 'C':
	case LIGHT_OBJECT:
	case 'X':
	case LIGHT_WORLD:
	case 'D':
		config.mode = tolower(key);
		cout << "switched mode to " << key << endl;
		break;

	// scaling
	case '-':
	case '+':
		should_redraw = scale(key);
		break;

	// rotating
	case 'r':
	case 'R':
		should_redraw = rotation(key);
		break;

	// translating
	case 't':
	case 'T':
		should_redraw = translate(key);
		break;

	// visibility options
	case 's':
		scene->getActiveCamera()->switchVisibility();
		should_redraw = true;
		break;
	case 'b':
		if (scene->getNumberOfModels() > 0) {
			scene->getActiveModel()->switchBoundingBoxVisibility();
			cout << "switched bounding box visibility of active model." << endl;
			should_redraw = true;
		}
		break;
	case 'n':
		if (scene->getNumberOfModels() > 0) {
			scene->getActiveModel()->switchVertexNormalsVisibility();
			cout << "switched vertex normals visibility of active model." << endl;
			should_redraw = true;
		}
		break;
	case 'N':
		if (scene->getNumberOfModels() > 0) {
			scene->getActiveModel()->switchFaceNormalsVisibility();
			cout << "switched face normals visibility of active model." << endl;
			should_redraw = true;
		}
		break;

	case '4':
		scene->addPrimitive(key - '0');
		cout << "added primitive object " << key << " with model id #" << scene->getNumberOfModels() << endl;
		should_redraw = true;
		break;
	}

	redraw(should_redraw);
}

void mouse(int button, int state, int x, int y)
{
	//button = {GLUT_LEFT_BUTTON, GLUT_MIDDLE_BUTTON, GLUT_RIGHT_BUTTON}
	//state = {GLUT_DOWN,GLUT_UP}
	
	//set down flags
	switch(button) {
		case GLUT_LEFT_BUTTON:
			lb_down = (state==GLUT_UP)?0:1;
			break;
		case GLUT_RIGHT_BUTTON:
			rb_down = (state==GLUT_UP)?0:1;
			break;
		case GLUT_MIDDLE_BUTTON:
			mb_down = (state==GLUT_UP)?0:1;	
			break;
	}

	// add your code
}

void motion(int x, int y)
{
	// calc difference in mouse movement
	int dx=x-last_x;
	int dy=y-last_y;
	// update last x,y
	last_x=x;
	last_y=y;
}

void help()
{
	AfxMessageBox(
		"GENERAL:\n"
		"ESC/q\t\texit\n"
		"DEL\t\tclear screen\n"
		"BS\t\tdelete active object\n"
		"\n"
		"MODES:\n"
		"* w/W\t\tenter world mode\n"
		"* m/M\t\tenter model mode\n"
		"* v/V\t\tenter camera view mode\n"
		"* c/C\t\tenter camera world mode\n"
		"\n"
		"TRANSFORMATIONS:\n"
		"* +/-\t\tcontrol scaling in current mode\n"
		"* r/R\t\tcontrol rotation in current mode\n"
		"* t/T\t\tcontrol translation in current mode\n"
		"\n"
		"ACTIVE OBJECTS:\n"
		"* </>\t\tmove between cameras\n"
		"* l\t\tset camera look at\n"
		"* P/p\t\tset camera perspective - Horizontal / perspective - Vertical\n"
		"* o/f\t\tset camera orthogonal / frustum\n"
		"* Z/z\t\tset zoom in / out\n"
		"\n"
		"* *\t\tchange active light\n"
		"* s\t\tset camera/light visibility\n"
		"\n"
		"* TAB\t\tchange active model\n"
		"* b\t\tswitch model bounding box visibility\n"
		"* n\t\tswitch model normal visibility\n"
		"* N\t\tswitch model face normal visibility\n"
		"\n"
		"PrimMeshModels:\n"
		"* 1-9\t\tadd model", MB_ICONINFORMATION);
}

void lightMenu(int id)
{
	bool should_redraw = false;
	CColorDialog cdlg;
	CLightDialog ldlg;
	switch (id) {
		case NEW_ITEM:
			if (ldlg.DoModal() == IDOK) {
				if (ldlg.IsPoint()) {
					PointLightSource pls(ldlg.GetColor(), vec3());
					pls.transformInWorld(Translate(ldlg.GetLightCoordinates()));
					scene->addLight(pls);
					cout << "point light was loaded with ID #" << scene->getNumberOfLights() - 1 << endl;
					config.is_demo = false;
					should_redraw = true;
				} else {
					// parallel
					ParallelLightSource pls(ldlg.GetColor(), ldlg.GetLightCoordinates());
					scene->addLight(pls);
					cout << "parallel light was loaded with ID #" << scene->getNumberOfLights() - 1 << endl;
					config.is_demo = false;
					should_redraw = true;
				}
			}
			break;
		case EDIT_ITEM:
			if (scene->getNumberOfLights() > 0) {
				if (cdlg.DoModal() == IDOK) {
					vec3 color = ColorToVec(cdlg.GetColor());
					scene->getActiveLight()->setIntensity(color);
					cout << "Set light color to " << color << endl;
					config.is_demo = false;
					should_redraw = true;
				}
			}
			break;
		case AMBIENT:
			if (cdlg.DoModal() == IDOK) {
				vec3 color = ColorToVec(cdlg.GetColor());
				AmbientLight al(color);
				scene->setAmbientLight(al);
				cout << "Set Ambient color to " << color << endl;
				config.is_demo = false;
				should_redraw = true;
			}
			break;
	}
	redraw(should_redraw);
}

void shaderMenu(int id)
{
	bool should_redraw = true;
	Cv2c1Dialog fdlg("Add Fog", "Extinction Coefficient:", "Scattering Coefficient:");
	switch (id) {
	case FLAT:
		renderer->SetBaseShader(Renderer::ShaderType::Flat);
		cout << "set flat shader" << endl;
		break;
	case GOURAUD:
		renderer->SetBaseShader(Renderer::ShaderType::Gouraud);
		cout << "set gourued shader" << endl;
		break;
	case PHONG:
		renderer->SetBaseShader(Renderer::ShaderType::Phong);
		cout << "set phong shader" << endl;
		break;
	case FOG:
		if (fdlg.DoModal() == IDOK) {
			float extinction = fdlg.GetValue1();
			float scattering = fdlg.GetValue2();
			if ((extinction >= 0) && (extinction <= 1) && (scattering >= 0) && (scattering <= 1)) {
			renderer->SetFog(fdlg.GetColor(), extinction, scattering);
			cout << "set fog" << endl;
			} else {
				cout << "fog error! parameters must be in [0,1] range." << endl;
			}
		} else {
			should_redraw = false;
		}
		break;
	case FOG_DISABLE:
		renderer->DisableFog();
		break;
	}

	redraw(should_redraw);
}

void modelMenu(int id)
{
	bool should_redraw = false;
	CFileDialog fdlg(TRUE, _T(".obj"), NULL, NULL, _T("*.obj|*.*"));

	switch (id) {
	case NEW_ITEM:
		if (fdlg.DoModal() == IDOK) {
			std::string s((LPCTSTR)fdlg.GetPathName());
			scene->loadOBJModel((LPCTSTR)fdlg.GetPathName());
			cout << "model " << s << " was loaded with ID #" << scene->getNumberOfModels() - 1 << endl;
			config.is_demo = false;
			should_redraw = true;
		}
		break;
	case EDIT_ITEM:
		if (scene->getNumberOfModels() > 0) {
			CEditModelDialog emdlg;
			if (emdlg.DoModal() == IDOK) {
				if (emdlg.ShouldRandom()) {
					scene->getActiveModel()->setRandomMaterial();
				} else {
					Material m = {
						emdlg.GetAmbientColor(),
						emdlg.GetSpecularColor(),
						emdlg.GetDiffuseColor(),
						emdlg.GetShininess()
					};
					if (m.shininess < 0) {
						cout << "model properties error: shininess must be non-negative." << endl;
						break;
					}
					scene->getActiveModel()->setUniformMaterial(m);
				}
				cout << "set active model properties" << endl;
				should_redraw = true;
			}
		}
		break;
	case ADD_TEXTURE:
		if (scene->getNumberOfModels() > 0) {
			CTextureDialog tdlg;
			if (tdlg.DoModal() == IDOK) {
				if (tdlg.GetShininess() < 0) {
					cout << "model properties error: shininess must be non-negative." << endl;
					break;
				}
				scene->getActiveModel()->setTextures(
					tdlg.GetAmbientColor(),
					tdlg.GetSpecularColor(),
					(LPCTSTR)tdlg.GetTexturePath(),
					tdlg.GetShininess());
				cout << "loaded texture file" << endl;
				should_redraw = true;
			}
		}
		break;
	case ADD_NORMAL_MAP:
		if (scene->getNumberOfModels() > 0) {
			CFileDialog fdlg(TRUE, _T(".png"), NULL, NULL, _T("*.png|*.*"));
			if (fdlg.DoModal() == IDOK) {
				scene->getActiveModel()->enableNormalMap(
					(LPCTSTR)fdlg.GetPathName());
				cout << "loaded normal map file" << endl;
				should_redraw = true;
			}
		}
		break;
	case DEL_NORMAL_MAP:
		if (scene->getNumberOfModels() > 0) {
			scene->getActiveModel()->disableNormalMap();
			cout << "unloaded normal map file" << endl;
			should_redraw = true;
		}
		break;
	case ADD_COLOR_ANIMATION:
		if (scene->getNumberOfModels() > 0) {
			CAnimationDialog adlg("Add Color Animation");
			if (adlg.DoModal() == IDOK) {
				scene->getActiveModel()->startColorAnimation(
					int(adlg.GetAnimationType()),
					adlg.GetSpeed(),
					adlg.GetDuration()
				);
				cout << "enabled color animation" << endl;
				should_redraw = true;
			}
		}
		break;
	case DEL_COLOR_ANIMATION:
		if (scene->getNumberOfModels() > 0) {
			scene->getActiveModel()->stopColorAnimation();
			cout << "disabled color animation" << endl;
			should_redraw = true;
		}
		break;
	case ADD_VERTEX_ANIMATION:
		if (scene->getNumberOfModels() > 0) {
			CAnimationDialog adlg("Add Vertex Animation", false);
			if (adlg.DoModal() == IDOK) {
				scene->getActiveModel()->startVertexAnimation(
					adlg.GetSpeed(),
					adlg.GetDuration()
				);
				cout << "enabled vertex animation" << endl;
				should_redraw = true;
			}
		}
		break;
	case DEL_VERTEX_ANIMATION:
		if (scene->getNumberOfModels() > 0) {
			scene->getActiveModel()->stopVertexAnimation();
			cout << "disabled vertex animation" << endl;
			should_redraw = true;
		}
		break;
	case ADD_TOON_SHADING:
		if (scene->getNumberOfModels() > 0) {
			Cv2c1Dialog tdlg("Addd Toon Shading", "Color Quantization Coefficient", "Silhouette Thickness", "color", 0.1, 4);
			if (tdlg.DoModal() == IDOK) {
				int cqc = tdlg.GetValue1(); // > 0
				float st = tdlg.GetValue2(); // >= 0
				if ((cqc <= 0) || (st < 0)) {
					cout << "toon shading: invalid parameters" << endl;
				} else {
					scene->getActiveModel()->enableToonShading(cqc, st, tdlg.GetColor());
					cout << "enabled toon shading" << endl;
					should_redraw = true;
				}
			}
		}
		break;
	case DEL_TOON_SHADING:
		if (scene->getNumberOfModels() > 0) {
			scene->getActiveModel()->disableToonShading();
			cout << "disabled toon shading" << endl;
			should_redraw = true;
		}
		break;
	case ADD_WOOD_TEXTURE:
		if (scene->getNumberOfModels() > 0) {
			Cc2Dialog c2dlg("Add Wood Texture");
			if (c2dlg.DoModal() == IDOK) {
				scene->getActiveModel()->enableWoodTexture(c2dlg.GetColor1(), c2dlg.GetColor2());
				cout << "enabled wood texture" << endl;
				should_redraw = true;
			}
		}
		break;
	case DEL_WOOD_TEXTURE:
		if (scene->getNumberOfModels() > 0) {
			scene->getActiveModel()->disableWoodTexture();
			cout << "disabled wood texture" << endl;
			should_redraw = true;
		}
		break;
	}
	redraw(should_redraw);
}

void fileMenu(int id)
{
	bool should_redraw = false;
	CLightDialog ldlg;

	switch (id) {
		case ADD_CAMERA:
			scene->addCamera();
			cout << "camera was added with ID #" << scene->getNumberOfCameras() - 1 << endl;
			should_redraw = true;
			break;
	}
	redraw(should_redraw);
}

void settingMenu(int id)
{
	CValueDialog dlg("Anti-Aliasing Setting", "Factor", 1);
	switch (id) {
	case SETTING_SCALING:
		set_scale_vector();
		break;
	case SETTING_ROTATION:
		set_rotation_vector();
		break;
	case SETTING_MOVEMENT:
		set_translation_vector();
		break;
	case SETTING_ZOOM:
		set_zoom_value();
		break;
	case SETTING_AA:
		if (dlg.DoModal() == IDOK) {
			int factor = dlg.GetValue();
			if (factor >= 1) {
				renderer->SetAntiAliasing(factor);
				cout << "set anti aliasing with factor " << factor << endl;
				redraw();
			}
		}
		break;
	}
}

void mainMenu(int id)
{
	switch (id)
	{
	case MAIN_DEMO:
		config.is_demo = true;
		redraw();
		break;
	case MAIN_HELP:
		help();
		break;
	case MAIN_ABOUT:
		AfxMessageBox(_T("Computer Graphics"));
		break;
	}
}

void initMenu()
{
	// model sub menu
	int menuModel = glutCreateMenu(modelMenu);
	glutAddMenuEntry("New", NEW_ITEM);
	glutAddMenuEntry("Edit", EDIT_ITEM);
	glutAddMenuEntry("Add Texture", ADD_TEXTURE);
	glutAddMenuEntry("Enable Normal Mapping", ADD_NORMAL_MAP);
	glutAddMenuEntry("Disable Normal Mapping", DEL_NORMAL_MAP);
	glutAddMenuEntry("Enable Color Animation", ADD_COLOR_ANIMATION);
	glutAddMenuEntry("Disable Color Animation", DEL_COLOR_ANIMATION);
	glutAddMenuEntry("Enable Vertex Animation", ADD_VERTEX_ANIMATION);
	glutAddMenuEntry("Disable Vertex Animation", DEL_VERTEX_ANIMATION);
	glutAddMenuEntry("Enable Toon Shading", ADD_TOON_SHADING);
	glutAddMenuEntry("Disable Toon Shading", DEL_TOON_SHADING);
	glutAddMenuEntry("Enable Wood Texture", ADD_WOOD_TEXTURE);
	glutAddMenuEntry("Disable Wood Texture", DEL_WOOD_TEXTURE);

	// light sub menu
	int menuLight = glutCreateMenu(lightMenu);
	glutAddMenuEntry("New", NEW_ITEM);
	glutAddMenuEntry("Edit", EDIT_ITEM);
	glutAddMenuEntry("Ambient", AMBIENT);

	// shader sub menu
	int menuShader = glutCreateMenu(shaderMenu);
	glutAddMenuEntry("Flat", FLAT);
	glutAddMenuEntry("Gouraud", GOURAUD);
	glutAddMenuEntry("Phong", PHONG);
	glutAddMenuEntry("Fog", FOG);
	glutAddMenuEntry("Disable Fog", FOG_DISABLE);
	
	// file sub menu
	int menuFile = glutCreateMenu(fileMenu);
	glutAddSubMenu("Model", menuModel);
	glutAddMenuEntry("Camera", ADD_CAMERA);
	glutAddSubMenu("Shader", menuShader);
	glutAddSubMenu("Light", menuLight);

	// setting sub menu
	int menuSetting = glutCreateMenu(settingMenu);
	glutAddMenuEntry("Scaling...", SETTING_SCALING);
	glutAddMenuEntry("Rotation...", SETTING_ROTATION);
	glutAddMenuEntry("Movement...", SETTING_MOVEMENT);
	glutAddMenuEntry("Zoom...", SETTING_ZOOM);
	glutAddMenuEntry("Anti-Aliasing...", SETTING_AA);

	glutCreateMenu(mainMenu);
	glutAddSubMenu("Add/Set", menuFile);
	glutAddSubMenu("Setting", menuSetting);
	glutAddMenuEntry("Demo",MAIN_DEMO);
	glutAddMenuEntry("Help", MAIN_HELP);
	glutAddMenuEntry("About",MAIN_ABOUT);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}
//----------------------------------------------------------------------------

void redraw(bool should_redraw)
{
	if (should_redraw) {
		if (config.is_demo) {
			scene->drawDemo();
		} else {
			renderer->ClearColorBuffer();
			scene->draw();
		};
	}
}

bool set_ortho()
{
	COrthoDialog dlg;
	if (dlg.DoModal() == IDOK) {
		if ((dlg.GetLeft() >= dlg.GetRight()) ||
			(dlg.GetBottom() >= dlg.GetTop()) ||
			(dlg.GetNear() >= dlg.GetFar())) {
			cout << "ortho setting: invalid parameters (" <<
				dlg.GetLeft() << ", " << dlg.GetRight() << ", " <<
				dlg.GetBottom() << ", " << dlg.GetTop() << ", " <<
				dlg.GetNear() << ", " << dlg.GetFar() << ")" << endl;
			return false;
		}
		scene->getActiveCamera()->ortho(
			dlg.GetLeft(), dlg.GetRight(),
			dlg.GetBottom(), dlg.GetTop(),
			dlg.GetNear(), dlg.GetFar()
		);
		return true;
	}
	return false;
}

bool set_frustum()
{
	COrthoDialog dlg("Frustum Setting");
	if (dlg.DoModal() == IDOK) {
		if ((dlg.GetLeft() >= dlg.GetRight()) ||
			(dlg.GetBottom() >= dlg.GetTop()) ||
			(dlg.GetNear() >= dlg.GetFar())) {
			cout << "frumstum setting: invalid parameters (" <<
				dlg.GetLeft() << ", " << dlg.GetRight() << ", " <<
				dlg.GetBottom() << ", " << dlg.GetTop() << ", " <<
				dlg.GetNear() << ", " << dlg.GetFar() << ")" << endl;
			return false;
		}
		scene->getActiveCamera()->frustum(
			dlg.GetLeft(), dlg.GetRight(),
			dlg.GetBottom(), dlg.GetTop(),
			dlg.GetNear(), dlg.GetFar()
		);
		return true;
	}
	return false;
}

bool set_perspective()
{
	bool is_horizontal = false;
	CString title = is_horizontal ? "Horizontal" : "Vertical";
	CPerspectiveDialog dlg(title + " Perspective Setting", is_horizontal);
	if (dlg.DoModal() == IDOK) {
		if ((dlg.GetFov() <= 0) || (dlg.GetFov() >= 180) ||
			(dlg.GetAspect() <= 0) ||
			(dlg.GetNear() >= dlg.GetFar())) {
			cout << "perspective setting: invalid parameters (" <<
				dlg.GetFov() << ", " << dlg.GetAspect() << ", " <<
				dlg.GetNear() << ", " << dlg.GetFar() << ")" << endl;
			return false;
		}
		scene->getActiveCamera()->perspectiveVertical(
			dlg.GetFov(), dlg.GetAspect(),
			dlg.GetNear(), dlg.GetFar()
		);
		
		return true;
	}
	return false;
}

bool set_lookat()
{
	Camera * cam = scene->getActiveCamera();
	mat4 tc = inverse(cam->getInverseTransform());
	// TODO - validate this
	vec4 eye = tc * vec4(vec3(), 1);
	eye = eye / eye.w;
	vec4 up = glm::normalize(tc * vec4(0, 1, 0, 0));

	cout << "start from : " << eye << " with up in " << up << endl;
	if (scene->getNumberOfModels() > 0) {
		vec4 at = scene->getActiveModel()->getLocation();
		at = at / at.w;
		cam->lookAt(eye, at, up);
		cout << "look at: looking at " << at << endl;
		tc = inverse(cam->getInverseTransform());
		//cout << "end at: " << (tc * vec3()) << " with up in " << (tc * vec3(0, 1, 0)) << endl;
		return true;
	} else {
		cout << "look at: no model to look at..." << endl;
	}
	return false;
}

void change_active_model()
{
	int max_id = scene->getNumberOfModels();
	if (max_id > 0) {
		CValueDialog dlg("Active Model", "Model ID:", max_id - 1);
		if (dlg.DoModal() == IDOK) {
			int v = dlg.GetValue();
			if ((v < 0) || (v >= max_id)) {
				cout << "invalid model ID #" << v << endl;
			} else {
				scene->setActiveModel(v);
				cout << "active model changed to #" << v << endl;
			}
		}
	} else {
		cout << "no models in system" << endl;
	}
}

void change_active_light()
{
	int max_id = scene->getNumberOfLights();
	if (max_id > 0) {
		CValueDialog dlg("Active Light", "Light ID:", max_id - 1);
		if (dlg.DoModal() == IDOK) {
			int v = dlg.GetValue();
			if ((v < 0) || (v >= max_id)) {
				cout << "invalid light ID #" << v << endl;
			} else {
				scene->setActiveLight(v);
				cout << "active light changed to #" << v << endl;
			}
		}
	} else {
		cout << "no lights in system" << endl;
	}
}

bool delete_active_object()
{
	bool should_redraw = false;
	switch (config.mode) {
	case MODEL_OBJECT:
	case MODEL_WORLD:
		if (scene->getNumberOfModels() > 0) {
			scene->removeActiveModel();
			cout << "deleted active model" << endl;
			should_redraw = true;
		}
		break;
	case CAMERA_OBJECT:
	case CAMERA_WORLD:
		if (scene->getNumberOfCameras() > 1) {
			scene->removeActiveCamera();
			cout << "deleted active camera" << endl;
			should_redraw = true;
		}
		break;
	case LIGHT_OBJECT:
	case LIGHT_WORLD:
		if (scene->getNumberOfLights() > 0) {
			scene->removeActiveLight();
			cout << "deleted active light" << endl;
			should_redraw = true;
		}
		break;
	}
	return should_redraw;
}


bool scale(unsigned char key)
{
	bool should_redraw = false;
	if ((config.scaling.x == 1) && (config.scaling.y == 1) && (config.scaling.z == 1)) {
		set_scale_vector();
	}
	mat4 scaling_mat = Scale(config.scaling);
	if (key == '-') {
		scaling_mat = Scale(vec3(1 / config.scaling.x, 1 / config.scaling.y, 1 / config.scaling.z));
	}

	switch (config.mode) {
	case MODEL_OBJECT:
		if (scene->getNumberOfModels() > 0) {
			scene->getActiveModel()->transformInModel(scaling_mat);
			//cout << "scalling model frame with " << scaling_mat << endl;
			should_redraw = true;
		}
		break;
	case MODEL_WORLD:
		if (scene->getNumberOfModels() > 0) {
			scene->getActiveModel()->transformInWorld(scaling_mat);
			//cout << "scalling model world frame with " << scaling_mat << endl;
			should_redraw = true;
		}
		break;
	case CAMERA_OBJECT:
		if (scene->getNumberOfCameras() > 0) {
			scene->getActiveCamera()->transformInView(scaling_mat);
			//cout << "scalling camera frame with " << scaling_mat << endl;
			should_redraw = true;
		}
		break;
	case CAMERA_WORLD:
		if (scene->getNumberOfCameras() > 0) {
			scene->getActiveCamera()->transformInWorld(scaling_mat);
			//cout << "scalling camera world with " << scaling_mat << endl;
			should_redraw = true;
		}
		break;
	default:
		cout << "scaling error: not in mode" << endl;
		return should_redraw;
	}
	return should_redraw;
}

bool rotation(unsigned char direction)
{
	bool should_redraw = false;
	if (length(config.rotating) == 0) {
		set_rotation_vector();
	}

	vec3 rotation_vec = config.rotating;
	if (direction == 'R') {
		rotation_vec *= -1;
	}
	
	switch (config.mode) {
	case MODEL_OBJECT:
		if (scene->getNumberOfModels() > 0) {
			if (rotation_vec.x != 0) scene->getActiveModel()->transformInModel(RotateX(rotation_vec.x));
			if (rotation_vec.y != 0) scene->getActiveModel()->transformInModel(RotateY(rotation_vec.y));
			if (rotation_vec.z != 0) scene->getActiveModel()->transformInModel(RotateZ(rotation_vec.z));
			cout << "rotating model frame with " << rotation_vec << endl;
			should_redraw = true;
		}
		break;
	case MODEL_WORLD:
		if (scene->getNumberOfModels() > 0) {
			if (rotation_vec.x != 0) scene->getActiveModel()->transformInWorld(RotateX(rotation_vec.x));
			if (rotation_vec.y != 0) scene->getActiveModel()->transformInWorld(RotateY(rotation_vec.y));
			if (rotation_vec.z != 0) scene->getActiveModel()->transformInWorld(RotateZ(rotation_vec.z));
			cout << "rotating model world frame with " << rotation_vec << endl;
			should_redraw = true;
		}
		break;
	case CAMERA_OBJECT:
		if (scene->getNumberOfCameras() > 0) {
			if (rotation_vec.x != 0) scene->getActiveCamera()->transformInView(RotateX(rotation_vec.x));
			if (rotation_vec.y != 0) scene->getActiveCamera()->transformInView(RotateY(rotation_vec.y));
			if (rotation_vec.z != 0) scene->getActiveCamera()->transformInView(RotateZ(rotation_vec.z));
			cout << "rotating camera frame with " << rotation_vec << endl;
			should_redraw = true;
		}
		break;
	case CAMERA_WORLD:
		if (scene->getNumberOfCameras() > 0) {
			if (rotation_vec.x != 0) scene->getActiveCamera()->transformInWorld(RotateX(rotation_vec.x));
			if (rotation_vec.y != 0) scene->getActiveCamera()->transformInWorld(RotateY(rotation_vec.y));
			if (rotation_vec.z != 0) scene->getActiveCamera()->transformInWorld(RotateZ(rotation_vec.z));
			cout << "rotating camera world frame with " << rotation_vec << endl;
			should_redraw = true;
		}
		break;
	case LIGHT_OBJECT:
		if (scene->getNumberOfLights() > 0) {
			if (rotation_vec.x != 0) scene->getActiveLight()->transformInModel(RotateX(rotation_vec.x));
			if (rotation_vec.y != 0) scene->getActiveLight()->transformInModel(RotateY(rotation_vec.y));
			if (rotation_vec.z != 0) scene->getActiveLight()->transformInModel(RotateZ(rotation_vec.z));
			cout << "rotating light frame with " << rotation_vec << endl;
			should_redraw = true;
		}
		break;
	case LIGHT_WORLD:
		if (scene->getNumberOfLights() > 0) {
			if (rotation_vec.x != 0) scene->getActiveLight()->transformInWorld(RotateX(rotation_vec.x));
			if (rotation_vec.y != 0) scene->getActiveLight()->transformInWorld(RotateY(rotation_vec.y));
			if (rotation_vec.z != 0) scene->getActiveLight()->transformInWorld(RotateZ(rotation_vec.z));
			cout << "rotating light world frame with " << rotation_vec << endl;
			should_redraw = true;
		}
		break;
	default:
		cout << "rotation error: not in mode" << endl;
		return should_redraw;
	}
	
	return should_redraw;
}

bool translate(unsigned char direction)
{
	bool should_redraw = false;
	if (length(config.translating) == 0) {
		set_translation_vector();
	}
	vec3 translation_vec = config.translating;
	if (direction == 'T') {
		translation_vec *= -1;
	}

	switch (config.mode) {
	case MODEL_OBJECT:
		if (scene->getNumberOfModels() > 0) {
			scene->getActiveModel()->transformInModel(Translate(translation_vec));
			cout << "translating model frame with " << translation_vec << endl;
			should_redraw = true;
		}
		break;
	case MODEL_WORLD:
		if (scene->getNumberOfModels() > 0) {
			scene->getActiveModel()->transformInWorld(Translate(translation_vec));
			cout << "translating model world frame with " << translation_vec << endl;
			should_redraw = true;
		}
		break;
	case CAMERA_OBJECT:
		if (scene->getNumberOfCameras() > 0) {
			scene->getActiveCamera()->transformInView(Translate(translation_vec));
			cout << "translating camera frame with " << translation_vec << endl;
			should_redraw = true;
		}
		break;
	case CAMERA_WORLD:
		if (scene->getNumberOfCameras() > 0) {
			scene->getActiveCamera()->transformInWorld(Translate(translation_vec));
			cout << "translating camera world frame with " << translation_vec << endl;
			should_redraw = true;
		}
		break;
	case LIGHT_OBJECT:
		if (scene->getNumberOfLights() > 0) {
			scene->getActiveLight()->transformInModel(Translate(translation_vec));
			cout << "translating light object frame with " << translation_vec << endl;
			should_redraw = true;
		}
		break;
	case LIGHT_WORLD:
		if (scene->getNumberOfLights() > 0) {
			scene->getActiveLight()->transformInWorld(Translate(translation_vec));
			cout << "translating light world frame with " << translation_vec << endl;
			should_redraw = true;
		}
		break;
	default:
		cout << "translation error: not in mode" << endl;
		return should_redraw;
	}

	return should_redraw;
}

bool zoom(unsigned char type)
{
	int delta = (type == 'z') ? config.zoom : -config.zoom;
	Camera * cam = scene->getActiveCamera();
	if ((cam->getNear() + delta > 0) && (cam->getNear() + delta > 0)) {
		cam->zoom(delta);
		return true;
	}
	cout << "zoom is out of range, far=" << cam->getFar() << ", near=" << cam->getNear() << ", zoom=" << delta << endl;
	return false;
}

void set_scale_vector()
{
	CXyzDialog dlg("Scaling Setting", vec3(1));
	if (dlg.DoModal() == IDOK) {
		vec3 result = dlg.GetXYZ();
		if ((result.x != 0) && (result.y != 0) && (result.z != 0)) {
			config.scaling = result;
		} else {
			cout << "scaling setting: can't scale to zero." << endl;
		}
	}
}

void set_rotation_vector()
{
	CXyzDialog dlg("Rotation Setting");
	if (dlg.DoModal() == IDOK) {
		vec3 result = dlg.GetXYZ();
		if (((result.x > 0) && (result.y == 0) && (result.z == 0)) ||
			((result.x == 0) && (result.y > 0) && (result.z == 0)) ||
			((result.x == 0) && (result.y == 0) && (result.z > 0))) {
			config.rotating = result;
		} else {
			cout << "rotation setting: must rotate in one axis at a time and to positive direction." << endl;
		}
	}
}

void set_translation_vector()
{
	CXyzDialog dlg("Movement Setting");
	if (dlg.DoModal() == IDOK) {
		config.translating = dlg.GetXYZ();
	}
}

void set_zoom_value()
{
	CValueDialog dlg("Zoom Dialog", "Z:", 1);
	if (dlg.DoModal() == IDOK) {
		config.zoom = dlg.GetValue();
	}
}

int my_main( int argc, char **argv )
{
	//----------------------------------------------------------------------------
	// Initialize window
	glutInit( &argc, argv );
	glutInitDisplayMode( GLUT_RGBA| GLUT_DOUBLE);
	glutInitWindowSize( 512, 512 );
	glutInitContextVersion( 3, 2 );
	glutInitContextProfile( GLUT_CORE_PROFILE );
	glutCreateWindow( "CG" );
	glewExperimental = GL_TRUE;
	glewInit();
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		/*		...*/
	}
	fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));

	
	renderer = new Renderer(512, 512);
	scene = new Scene(renderer, vec3(0, 5, 15));
	config = { 0, vec3(1), vec3(), vec3(), 1, false};
	last_time = clock();
	//----------------------------------------------------------------------------
	// Initialize Callbacks

	glutDisplayFunc( display );
	glutKeyboardFunc( keyboard );
	glutMouseFunc( mouse );
	glutMotionFunc ( motion );
	glutReshapeFunc( reshape );
	glutIdleFunc( animation );
	initMenu();
	

	glutMainLoop();
	delete scene;
	delete renderer;
	return 0;
}

using namespace std;

int main( int argc, char **argv )
{
	int nRetCode = 0;
	
	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
		_tprintf(_T("Fatal Error: MFC initialization failed\n"));
		nRetCode = 1;
	}
	else
	{
		my_main(argc, argv );
	}
	
	return nRetCode;
}
