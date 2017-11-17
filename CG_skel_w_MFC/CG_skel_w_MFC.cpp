// CG_skel_w_MFC.cpp : Defines the entry point for the console application.
//


/*
+/-		control scaling of current mode
w		enter world mode
m		enter active model mode
v		enter active camera mode

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
#define MAIN_DEMO 1
#define MAIN_ABOUT 2

#define SETTING_SCALING		1
#define SETTING_ROTATION	2
#define SETTING_MOVEMENT	3

typedef struct configuration_s {
	unsigned char mode;	// 0 - none, 'm' - model, 'w' - world, 'v' - view
	vec3 scaling;	// must be greater or equal to 1	(decreasing scaling is 1/scaling)
	vec3 shifting;
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
	case 033:
		exit(EXIT_SUCCESS);
		break;
	case 'c':
		// clear screen
		clear_buffers();
		config.is_demo = false;
		break;
	// switch modes
	case 'm': // model mode
	case 'w': // world mode
	case 'v': // view mode (camera mode)
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
	case 'b':
		if (config.mode == 'm') {
			if (scene->getNumberOfModels() > 0) {
				scene->getActiveModel()->switchBoundingBoxVisibility();
				cout << "switched bounding box visibility of active model." << endl;
				should_redraw = true;
			}
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
	switch (id)
	{
		case FILE_OPEN:
			CFileDialog dlg(TRUE,_T(".obj"),NULL,NULL,_T("*.obj|*.*"));
			if (dlg.DoModal() == IDOK) {
				std::string s((LPCTSTR)dlg.GetPathName());
				scene->loadOBJModel((LPCTSTR)dlg.GetPathName());
				scene->draw();
				config.is_demo = false;
			}
			break;
	}
}

void settingMenu(int id)
{
	CString s = "";
	switch (id) {
	case SETTING_SCALING:
		s = "Scaling Setting";
		break;
	case SETTING_ROTATION:
		s = "Rotation Setting";
		break;
	case SETTING_MOVEMENT:
		s = "Movement Setting";	
		break;
	}

	CXyzDialog dlg(s);
	vec3 result;
	if (dlg.DoModal() == IDOK) {
		switch (id) {
		case SETTING_SCALING:
			result = dlg.GetXYZ();
			if ((result.x >= 1) && (result.y >= 1) && (result.z >= 1)) {
				config.scaling = result;
			} else {
				cout << "scaling setting mustn't be lower than 1." << endl;
			}
			break;
		case SETTING_ROTATION:
			result = dlg.GetXYZ();
			if (((result.x > 0) && (result.y == 0) && (result.z == 0)) ||
				((result.x == 0) && (result.y > 0) && (result.z == 0)) ||
				((result.x == 0) && (result.y == 0) && (result.z > 0))) {
				config.rotating = result;
			} else {
				cout << "rotation setting: must rotate in one axis at a time and to positive direction." << endl;
			}
			break;
		case SETTING_MOVEMENT:
			result = dlg.GetXYZ();
			if ((result.x >= 0) && (result.y >= 0) && (result.z >= 0)) {
				config.shifting = dlg.GetXYZ();
			} else {
				cout << "scaling setting mustn't be lower than 1." << endl;
			}
			break;
		}
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

bool scale(unsigned char key)
{
	bool should_redraw = false;
	mat4 scaling_mat = Scale(config.scaling);
	if (key == '-') {
		scaling_mat = Scale(vec3(1 / config.scaling.x, 1 / config.scaling.y, 1 / config.scaling.z));
	}

	switch (config.mode) {
	case 'm':
		if (scene->getNumberOfModels() > 0) {
			scene->getActiveModel()->transformInModel(scaling_mat);
			cout << "scalling model with " << scaling_mat << endl;
			should_redraw = true;
		}
		break;
	case 'w':
		if (scene->getNumberOfModels() > 0) {
			scene->getActiveModel()->transformInWorld(scaling_mat);
			cout << "scalling world with " << scaling_mat << endl;
			should_redraw = true;
		}
		break;
	case 'v':
		if (scene->getNumberOfCameras() > 0) {
			scene->getActiveCamera()->transformInView(scaling_mat);
			cout << "scalling camera with " << scaling_mat << endl;
			should_redraw = true;
		}
		break;
	default:
		return should_redraw;
	}
	return should_redraw;
}

bool rotation(unsigned char direction)
{
	bool should_redraw = false;
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
			should_redraw = true;
		}
		break;
	case 'w':
		if (scene->getNumberOfModels() > 0) {
			if (rotation_vec.x != 0) scene->getActiveModel()->transformInWorld(RotateX(rotation_vec.x));
			if (rotation_vec.y != 0) scene->getActiveModel()->transformInWorld(RotateY(rotation_vec.y));
			if (rotation_vec.z != 0) scene->getActiveModel()->transformInWorld(RotateZ(rotation_vec.z));
			should_redraw = true;
		}
		break;
	case 'v':
		if (scene->getNumberOfCameras() > 0) {
			cout << "scalling: with " << rotation_vec << endl;
			if (rotation_vec.x != 0) scene->getActiveCamera()->transformInView(RotateX(rotation_vec.x));
			if (rotation_vec.y != 0) scene->getActiveCamera()->transformInView(RotateY(rotation_vec.y));
			if (rotation_vec.z != 0) scene->getActiveCamera()->transformInView(RotateZ(rotation_vec.z));
			should_redraw = true;
		}
		break;
	default:
		return should_redraw;
	}
	
	return should_redraw;
}

bool translate(unsigned char direction)
{
	return false;
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
	config = { 0, vec3(1.1,1.1,1), vec3(), false};
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
