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
s		->	set camera visibility

TAB		->	move between models
b		->	switch model bounding box visibility
n		->	switch model normal visibility
N		->	switch model face normal visibility

PrimMeshModels:
1-9		-> add model
*/

/*
TODO:
camera changes: lookat
*/

#include "stdafx.h"
#include "CG_skel_w_MFC.h"
#include "InputDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// The one and only application object

#include "GL/glew.h"
#include "GL/freeglut.h"
#include "GL/freeglut_ext.h"
#include "vec.h"
#include "mat.h"
#include "InitShader.h"
#include "Scene.h"
#include "Renderer.h"
#include <string>

#define BUFFER_OFFSET( offset )   ((GLvoid*) (offset))

#define FILE_OPEN 1
#define ADD_CAMERA 2

#define MAIN_DEMO 1
#define MAIN_HELP 2
#define MAIN_ABOUT 3

#define SETTING_SCALING		1
#define SETTING_ROTATION	2
#define SETTING_MOVEMENT	3
#define SETTING_ZOOM	4

typedef struct configuration_s {
	unsigned char mode;	// 0 - none, 'm' - model, 'w' - world, 'v' - view
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

//----------------------------------------------------------------------------
// Callbacks

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
		fileMenu((key == 'a') ? FILE_OPEN : ADD_CAMERA);
		break;
	case 127:
		// clear screen
		scene->clear();
		config.is_demo = false;
		should_redraw = true;
		break;
	case '\t':
		change_active_model();
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
		renderer->switchWire();
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
		should_redraw = set_perspective(key);
		break;
	case 'z':
	case 'Z':
		should_redraw = zoom(key);
		break;

	// switch modes
	case 'm': // model mode
	case 'M':
	case 'w': // model world mode
	case 'W':
	case 'v': // camera view mode
	case 'V':
	case 'c': // camera world mode
	case 'C':
		config.mode = key;
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
			cout << "switched face normals visibility of active model." << endl;
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
		"* s\t\tset camera visibility"
		"\n"
		"* TAB\t\tmove between models\n"
		"* b\t\tswitch model bounding box visibility\n"
		"* n\t\tswitch model normal visibility\n"
		"* N\t\tswitch model face normal visibility\n"
		"\n"
		"PrimMeshModels:\n"
		"* 1-9\t\tadd model", MB_ICONINFORMATION);
}

void fileMenu(int id)
{
	bool should_redraw = false;
	CFileDialog dlg(TRUE, _T(".obj"), NULL, NULL, _T("*.obj|*.*"));
	switch (id)
	{
		case FILE_OPEN:
			if (dlg.DoModal() == IDOK) {
				std::string s((LPCTSTR)dlg.GetPathName());
				scene->loadOBJModel((LPCTSTR)dlg.GetPathName());
				cout << "model " << s << " was loaded with ID #" << scene->getNumberOfModels() - 1 << endl;
				config.is_demo = false;
				should_redraw = true;
			}
			break;
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
	// file sub menu
	int menuFile = glutCreateMenu(fileMenu);
	glutAddMenuEntry("Open...", FILE_OPEN);
	glutAddMenuEntry("Camera", ADD_CAMERA);

	// setting sub menu
	int menuSetting = glutCreateMenu(settingMenu);
	glutAddMenuEntry("Scaling...", SETTING_SCALING);
	glutAddMenuEntry("Rotation...", SETTING_ROTATION);
	glutAddMenuEntry("Movement...", SETTING_MOVEMENT);
	glutAddMenuEntry("Zoom...", SETTING_ZOOM);

	glutCreateMenu(mainMenu);
	glutAddSubMenu("File",menuFile);
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
			renderer->ClearDepthBuffer();
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

bool set_perspective(char type)
{
	bool is_horizontal = (type == 'P');
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
		if (is_horizontal) {
			scene->getActiveCamera()->perspectiveHorizontal(
				dlg.GetFov(), dlg.GetAspect(),
				dlg.GetNear(), dlg.GetFar()
			);
		} else {
			scene->getActiveCamera()->perspectiveVertical(
				dlg.GetFov(), dlg.GetAspect(),
				dlg.GetNear(), dlg.GetFar()
			);
		}
		return true;
	}
	return false;
}

bool set_lookat()
{
	Camera * cam = scene->getActiveCamera();
	mat4 tc = inverse(cam->getInverseTransform());
	vec4 eye = tc * vec3();
	eye = eye / eye.w;
	vec4 up = tc * vec3(0, 1, 0);
	up = up / up.w;

	cout << "start from : " << eye << " with up in " << up << endl;
	if (scene->getNumberOfModels() > 0) {
		vec4 at = scene->getActiveModel()->getLocation();
		at = at / at.w;
		cam->lookAt(eye, at, up);
		cout << "look at: looking at " << at << endl;
		tc = inverse(cam->getInverseTransform());
		cout << "end at: " << (tc * vec3()) << " with up in " << (tc * vec3(0, 1, 0)) << endl;
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
			if ((v < 0) || (v >= scene->getNumberOfModels())) {
				cout << "invalid model ID #" << v << endl;
			} else {
				scene->setActiveModel(v);
				cout << "active model changed to #" << v << endl;
			}
		}
	} else {
		cout << "no model in system" << endl;
	}
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
	case 'm':
		if (scene->getNumberOfModels() > 0) {
			scene->getActiveModel()->transformInModel(scaling_mat);
			cout << "scalling model frame with " << scaling_mat << endl;
			should_redraw = true;
		}
		break;
	case 'w':
		if (scene->getNumberOfModels() > 0) {
			scene->getActiveModel()->transformInWorld(scaling_mat);
			cout << "scalling model world frame with " << scaling_mat << endl;
			should_redraw = true;
		}
		break;
	case 'v':
		if (scene->getNumberOfCameras() > 0) {
			scene->getActiveCamera()->transformInView(scaling_mat);
			cout << "scalling camera frame with " << scaling_mat << endl;
			should_redraw = true;
		}
		break;
	case 'c':
		if (scene->getNumberOfCameras() > 0) {
			scene->getActiveCamera()->transformInWorld(scaling_mat);
			cout << "scalling camera world with " << scaling_mat << endl;
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
	case 'm':
		if (scene->getNumberOfModels() > 0) {
			if (rotation_vec.x != 0) scene->getActiveModel()->transformInModel(RotateX(rotation_vec.x));
			if (rotation_vec.y != 0) scene->getActiveModel()->transformInModel(RotateY(rotation_vec.y));
			if (rotation_vec.z != 0) scene->getActiveModel()->transformInModel(RotateZ(rotation_vec.z));
			cout << "rotating model frame with " << rotation_vec << endl;
			should_redraw = true;
		}
		break;
	case 'w':
		if (scene->getNumberOfModels() > 0) {
			if (rotation_vec.x != 0) scene->getActiveModel()->transformInWorld(RotateX(rotation_vec.x));
			if (rotation_vec.y != 0) scene->getActiveModel()->transformInWorld(RotateY(rotation_vec.y));
			if (rotation_vec.z != 0) scene->getActiveModel()->transformInWorld(RotateZ(rotation_vec.z));
			cout << "rotating model world frame with " << rotation_vec << endl;
			should_redraw = true;
		}
		break;
	case 'v':
		if (scene->getNumberOfCameras() > 0) {
			if (rotation_vec.x != 0) scene->getActiveCamera()->transformInView(RotateX(rotation_vec.x));
			if (rotation_vec.y != 0) scene->getActiveCamera()->transformInView(RotateY(rotation_vec.y));
			if (rotation_vec.z != 0) scene->getActiveCamera()->transformInView(RotateZ(rotation_vec.z));
			cout << "rotating camera frame with " << rotation_vec << endl;
			should_redraw = true;
		}
		break;
	case 'c':
		if (scene->getNumberOfCameras() > 0) {
			if (rotation_vec.x != 0) scene->getActiveCamera()->transformInWorld(RotateX(rotation_vec.x));
			if (rotation_vec.y != 0) scene->getActiveCamera()->transformInWorld(RotateY(rotation_vec.y));
			if (rotation_vec.z != 0) scene->getActiveCamera()->transformInWorld(RotateZ(rotation_vec.z));
			cout << "rotating camera world frame with " << rotation_vec << endl;
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
	case 'm':
		if (scene->getNumberOfModels() > 0) {
			scene->getActiveModel()->transformInModel(Translate(translation_vec));
			cout << "translating model frame with " << translation_vec << endl;
			should_redraw = true;
		}
		break;
	case 'w':
		if (scene->getNumberOfModels() > 0) {
			scene->getActiveModel()->transformInWorld(Translate(translation_vec));
			cout << "translating model world frame with " << translation_vec << endl;
			should_redraw = true;
		}
		break;
	case 'v':
		if (scene->getNumberOfCameras() > 0) {
			scene->getActiveCamera()->transformInView(Translate(translation_vec));
			cout << "translating camera frame with " << translation_vec << endl;
			should_redraw = true;
		}
		break;
	case 'c':
		if (scene->getNumberOfCameras() > 0) {
			scene->getActiveCamera()->transformInWorld(Translate(translation_vec));
			cout << "translating camera world frame with " << translation_vec << endl;
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
		scene->getActiveCamera()->zoom(delta);
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

	
	
	renderer = new Renderer(512,512);
	scene = new Scene(renderer, vec3(0, 5, 15));
	config = { 0, vec3(1), vec3(), vec3(), 1, false};
	//----------------------------------------------------------------------------
	// Initialize Callbacks

	glutDisplayFunc( display );
	glutKeyboardFunc( keyboard );
	glutMouseFunc( mouse );
	glutMotionFunc ( motion );
	glutReshapeFunc( reshape );
	initMenu();
	

	glutMainLoop();
	delete scene;
	delete renderer;
	return 0;
}

CWinApp theApp;

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
