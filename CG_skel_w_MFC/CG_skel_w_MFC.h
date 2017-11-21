#pragma once

#include "resource.h"

void display(void);
void reshape(int width, int height);
void keyboard(unsigned char key, int x, int y);
void mouse(int button, int state, int x, int y);
void fileMenu(int id);
void mainMenu(int id);
void initMenu();


void clear_buffers();
bool set_ortho();
bool set_frustum();
bool set_perspective(char type);
bool set_lookat();
bool set_zoom(char type);
void change_active_model();
bool scale(unsigned char type);
bool rotation(unsigned char direction);
bool translate(unsigned char direction);
void set_scale_vector();
void set_rotation_vector();
void set_translation_vector();