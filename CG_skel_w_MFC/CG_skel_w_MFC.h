#pragma once

#include "resource.h"

void display(void);
void reshape(int width, int height);
void keyboard(unsigned char key, int x, int y);
void mouse(int button, int state, int x, int y);
void help();
void lightMenu(int id);
void modelMenu(int id);
void fileMenu(int id);
void settingMenu(int id);
void mainMenu(int id);
void initMenu();


void redraw(bool should_redraw=true);
bool set_ortho();
bool set_frustum();
bool set_perspective(char type);
bool set_lookat();
void change_active_model();
void change_active_light();
bool scale(unsigned char type);
bool rotation(unsigned char direction);
bool translate(unsigned char direction);
bool zoom(unsigned char type);
void set_scale_vector();
void set_rotation_vector();
void set_translation_vector();
void set_zoom_value();