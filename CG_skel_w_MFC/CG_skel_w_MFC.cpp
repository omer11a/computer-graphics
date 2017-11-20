// CG_skel_w_MFC.cpp : Defines the entry point for the console application.
//
/*

keyboard commands:	(keys	->	action)
general:
ESC/q	->	exit
DEL		-> clear screen

modes:
w		->	enter world mode
m		->	enter active model mode
v		->	enter active camera mode

transformations:
+/-		->	control scaling in current mode
r/R		->	control rotation in current mode
t/T		->	control translation in current mode

active objects:
</>		->	move between cameras
l		->	set camera look at
o/p/P	->	set camera orthogonal / perspective-Horizontal / perspective-Vertical
Z/z		->	set zoom in/out

<TBD>		->	move between models
b		->	switch model bounding box visibility
n		->	switch model normal visibility
f		->	switch model face normal visibility

PrimMeshModels:
1-9		-> add model
*/

/*
TODO:
camera changes: lookat, projection(ortho, perspective), zoom in/out
DONE:	visible: normals, face-normals, cameras
change active model
add models
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
#define MAIN_ABOUT 2

#define SETTING_SCALING		1
#define SETTING_ROTATION	2
#define SETTING_MOVEMENT	3

typedef struct configuration_s {
	unsigned char mode;	// 0 - none, 'm' - model, 'w' - world, 'v' - view
	vec3 scaling;	// must be greater or equal to 1	(decreasing scaling is 1/scaling)
	vec3 translating;
	vec3 rotating;
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
	//scene->drawDemo();
}

void reshape(int width, int height)
{
	//update the renderer's buffers
	renderer->UpdateBuffers(width, height);
	// TODO: draw actual objects
	if (config.is_demo) {
		scene->drawDemo();
	} else {
		clear_buffers();
		scene->draw();
	}
}

void keyboard(unsigned char key, int x, int y)
{
	bool should_redraw = false;
	switch (key) {
	case 'q':
	case 033:
		exit(EXIT_SUCCESS);
		break;
	case 127:
		// clear screen
		scene->clear();
		clear_buffers();
		config.is_demo = false;
		break;

	// camera operations
	case '>':
		scene->nextCamera();
		should_redraw = true;
		break;
	case '<':
		scene->prevCamera();
		should_redraw = true;
		break;
	case 'l':
		should_redraw = set_lookat();
		break;
	case 'o':
		should_redraw = set_ortho();
		break;
	case 'p':
		//scene->getActiveCamera()->perspectiveHorizontal();
		break;
	case 'z':
	case 'Z':
		should_redraw = set_zoom(key);
		break;

	// switch modes
	case 'm': // model mode
	case 'w': // model world mode
	case 'v': // camera view mode
	case 'c': // camera world mode
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
	case 'f':
		if (scene->getNumberOfModels() > 0) {
			scene->getActiveModel()->switchFaceNormalsVisibility();
			cout << "switched face normals visibility of active model." << endl;
			should_redraw = true;
		}
		break;
	}

	if (should_redraw) {
		if (config.is_demo) {
			scene->drawDemo();
		} else {
			clear_buffers();
			scene->draw();
		};
	}
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
	if (should_redraw) {
		if (config.is_demo) {
			scene->drawDemo();
		} else {
			clear_buffers();
			scene->draw();
		};
	}
}

void settingMenu(int id)
{
	CString s = "";
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
	}
}

void mainMenu(int id)
{
	switch (id)
	{
	case MAIN_DEMO:
		scene->drawDemo();
		config.is_demo = true;
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

	// add sub menu
	//int menuAdd = glutCreateMenu(fileMenu);
	//glutAddMenuEntry("Model", ADD_MODEL);

	// setting sub menu
	int menuSetting = glutCreateMenu(settingMenu);
	glutAddMenuEntry("Scaling...", SETTING_SCALING);
	glutAddMenuEntry("Rotation...", SETTING_ROTATION);
	glutAddMenuEntry("Movement...", SETTING_MOVEMENT);

	glutCreateMenu(mainMenu);
	glutAddSubMenu("File",menuFile);
	glutAddSubMenu("Setting", menuSetting);
	glutAddMenuEntry("Demo",MAIN_DEMO);
	glutAddMenuEntry("About",MAIN_ABOUT);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}
//----------------------------------------------------------------------------

void clear_buffers()
{
	renderer->ClearColorBuffer();
	renderer->ClearDepthBuffer();
	renderer->SwapBuffers();
}

bool set_ortho()
{
	COrthoDialog dlg;
	if (dlg.DoModal() == IDOK) {
		scene->getActiveCamera()->ortho(
			dlg.GetLeft(), 
			dlg.GetRight(),
			dlg.GetBottom(),
			dlg.GetTop(),
			dlg.GetNear(),
			dlg.GetFar()
		);
		return true;
	}
	return false;
}

bool set_lookat()
{
	/*CLookAtDialog dlg;
	if (dlg.DoModal() == IDOK) {
		scene->getActiveCamera()->lookAt(
			dlg.GetEye(),
			dlg.GetAt(),
			dlg.GetUp()
		);
		return true;
	}*/
	return false;
}

bool set_zoom(char type)
{
	CZoomDialog dlg;
	if (dlg.DoModal() == IDOK) {
		switch (type) {
		case 'Z':
			scene->getActiveCamera()->zoomIn(dlg.GetZ());
			return true;
		case 'z':
			scene->getActiveCamera()->zoomOut(dlg.GetZ());
			return true;
		}
	}
	return false;
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
	scene = new Scene(renderer);
	config = { 0, vec3(1), vec3(), vec3(), false};
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
