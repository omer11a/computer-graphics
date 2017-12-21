#include "StdAfx.h"
#include "Renderer.h"
#include "CG_skel_w_MFC.h"
#include "InitShader.h"
#include "GL\freeglut.h"

#define INDEX(width,x,y,c) (x+y*width)*3+c

Renderer::Renderer() : m_width(512), m_height(512), m_zBuffer(NULL),
m_cTransform(), m_projection(), m_oTransform(), m_nTransform(), m_camera_multiply()
{
	InitOpenGLRendering();
	CreateBuffers(512, 512);
}
Renderer::Renderer(int width, int height) : m_width(width), m_height(height), m_zBuffer(NULL),
m_cTransform(), m_projection(), m_oTransform(), m_nTransform()
{
	InitOpenGLRendering();
	CreateBuffers(width, height);
}

Renderer::~Renderer(void)
{
	DestroyBuffers();
}

void Renderer::CreateBuffers(int width, int height)
{
	m_width = width;
	m_height = height;
	CreateOpenGLBuffer(); //Do not remove this line.
	m_outBuffer = new float[3 * m_width * m_height];
	m_zBuffer = new float[m_width * m_height];
	//m_paintBuffer = NULL;
}

//void Renderer::CreateLocalBuffer()
//{
//	if (m_paintBuffer != NULL) {
//		delete[] m_paintBuffer;
//	}
//	m_paintBuffer = new bool[m_width * m_height];
//	for (int i = 0; i < m_width * m_height; ++i) {
//		m_paintBuffer[i] = false;
//	}
//}


void Renderer::DestroyBuffers()
{
	delete[] m_outBuffer;
	if (m_zBuffer != NULL) {
		delete[] m_zBuffer;
	}
}

vec4 Renderer::applyCameraTransformation(const vec3& p, const vec3& n) const {
	vec4 pTransformed;
	vec4 nTransformed;

	pTransformed = m_oTransform * p;
	if (length(n) != 0) {
		nTransformed = normalize(m_nTransform * n);
		nTransformed.w = 0;
	}

	return m_cTransform * (pTransformed + nTransformed);
}

vec3 Renderer::applyProjection(const vec4& p) const {
	vec4 result = m_projection * p;
	result = result / result.w;
	return vec3(result.x, result.y, result.z);
}

void Renderer::clip(float x0, float x1, float xmin, float xmax, float& t1, float& t2) const {
	float p1 = x0 - x1;
	float p2 = -p1;
	float q1 = x0 - xmin;
	float q2 = xmax - x0;

	if (p1 == 0) {
		// line is parallel to clipping window
		if ((q1 < 0) || (q2 < 0)) {
			// line is outside of the clipping window
			t1 = 0;
			t2 = 0;
		}

		return;
	}
	
	float u1 = 0;
	float u2 = 0;
	float r1 = q1 / p1;
	float r2 = q2 / p2;
	if (p1 < 0) {
		u1 = r1;
		u2 = r2;
	} else {
		u1 = r2;
		u2 = r1;
	}

	t1 = max(t1, u1);
	t2 = min(t2, u2);
}

bool Renderer::clipLine(const vec3& v1, const vec3& n1, const vec3& v2, const vec3& n2, vec3& start, vec3& end) const {
	vec4 p1 = applyCameraTransformation(v1, n1);
	vec4 p2 = applyCameraTransformation(v2, n2);

	float t1 = 0;
	float t2 = 1;
	clip(p1.z, p2.z, -zFar, -zNear, t1, t2);
	if (t1 >= t2) {
		return false;
	}

	vec4 dp = p2 - p1;
	vec3 q1 = applyProjection(p1 + t1 * dp);
	vec3 q2 = applyProjection(p1 + t2 * dp);
	
	t1 = 0;
	t2 = 1;
	clip(q1.x, q2.x, -1, 1, t1, t2);
	clip(q1.y, q2.y, -1, 1, t1, t2);
	if (t1 >= t2) {
		return false;
	}

	vec3 dq = q2 - q1;
	start = q1 + t1 * dq;
	end = q1 + t2 * dq;
	return true;
}

vec3 Renderer::convertToScreen(const vec3& p) const {
	static const double min_size = min(m_height, m_width) * 0.5;
	return vec3(round(m_width * 0.5 + min_size * (p.x)), round(m_height * 0.5 + min_size * (p.y)), p.z);
}

//vec3 Renderer::PixelToPoint(const vec3 & p) const
//{
//	static const double min_size = min(m_height, m_width) * 0.5;
//	float x = (p.x - m_width * 0.5) / min_size;
//	float y = (p.y - m_height * 0.5) / min_size;
//	return vec3(x, y, p.z);
//}

// TODO: check if extra bool is still important...
bool Renderer::pointToScreen(const vec3& p, const vec3& n, vec3& q, bool should_screen) const
{
	vec4 transformed = applyCameraTransformation(p, n);
	if ((transformed.z > -zNear) || (transformed.z < -zFar)) {
		return false;
	}

	vec3 result = applyProjection(transformed);
	if ((result.x < -1) || (result.x > 1) || (result.y < -1) || (result.y > 1)) {
		return false;
	}

	q = should_screen ? convertToScreen(result) : result;
	return true;
}

bool Renderer::lineToScreen(const vec3& p1, const vec3& n1, const vec3& p2, const vec3& n2, vec3& q1, vec3& q2) const {
	bool shouldDraw = clipLine(p1, n1, p2, n2, q1, q2);
	if (!shouldDraw) {
		return false;
	}

	q1 = convertToScreen(q1);
	q2 = convertToScreen(q2);
	return true;
}

bool Renderer::PlotPixel(const int x, const int y, const float z, const vec3& color)
{
	if ((x >= m_width) || (x < 0) || (y >= m_height) || (y < 0)) {
		return false;
	}

	if (z > m_zBuffer[y * m_width + x]) {
		m_outBuffer[INDEX(m_width, x, y, 0)] = color.x;
		m_outBuffer[INDEX(m_width, x, y, 1)] = color.y;
		m_outBuffer[INDEX(m_width, x, y, 2)] = color.z;
		m_zBuffer[y * m_width + x] = z;
	}
	//if (m_paintBuffer != NULL) {
	//	m_paintBuffer[y * m_width + x] = true;
	//}
	return true;
}

vec3 Renderer::GetCenterMass(const vec3& p1, const vec3& p2, const vec3& p3) const {
	return vec3((p1.x + p2.x + p3.x) / 3, (p1.y + p2.y + p3.y) / 3, (p1.z + p2.z + p3.z) / 3);
}

//void Renderer::UpdateBCPoint(const vec3& p1, const vec3& p2, const vec3& p3, vec3& p) const
//{
//	vec3 first, second, third;
//	if ((p1.x <= p.x) && (p1.y <= p.y)) {
//		first = p1;
//		if ((p2.x >= p.x) && (p2.y >= p.y)) {
//			second = p2;
//			third = p3;
//		} else {
//			second = p3;
//			third = p2;
//		}
//	} else if ((p2.x <= p.x) && (p2.y <= p.y)) {
//		first = p2;
//		if ((p1.x >= p.x) && (p1.y >= p.y)) {
//			second = p1;
//			third = p3;
//		} else {
//			second = p3;
//			third = p1;
//		}
//	} else if ((p3.x <= p.x) && (p3.y <= p.y)) {
//		first = p3;
//		if ((p2.x >= p.x) && (p2.y >= p.y)) {
//			second = p2;
//			third = p1;
//		} else {
//			second = p1;
//			third = p2;
//		}
//	}
//
//	vec2 bc = inverse(mat2(first.x, second.x, first.y, second.y)) * vec2(p.x, p.y);
//
//	p = transpose(mat3(first, second, third)) * vec3(bc, 1 - bc.x - bc.y);
//	//return vec3(bc, 1 - bc.x - bc.y);
//}

void Renderer::DrawLine(const vec3& p1, const vec3& n1, const vec3& p2, const vec3& n2, const vec3& c1, const vec3& c2)
{
	vec3 newP1, newP2;
	if (!lineToScreen(p1, n1, p2, n2, newP1, newP2)) {
		return;
	}

	//TODO: recalculate colors
	if (abs(newP1.y - newP2.y) > abs(newP1.x - newP2.x)) {
		this->DrawSteepLine(newP1, newP2, c1, c2);
	} else {
		this->DrawModerateLine(newP1, newP2, c1, c2);
	}
}

void Renderer::DrawSteepLine(const vec3& p1, const vec3& p2, const vec3& c1, const vec3& c2) {
	vec3 start, start_c, end, end_c;

	if (p1.y > p2.y) {
		start = p2;
		start_c = c2;
		end = p1;
		end_c = c1;
	} else {
		start = p1;
		start_c = c1;
		end = p2;
		end_c = c2;
	}

	int x = start.x;
	int y = start.y;
	float z = start.z;
	vec3 color = start_c;
	
	int dx = abs(end.x - start.x);
	int dy = end.y - start.y;
	float dz = (end.z - start.z) / dy;
	vec3 dc = (end_c - start_c) / dy;

	int de = 2 * dx;
	int d = 2 * dx - dy;
	int dne = 2 * dx - 2 * dy;
	int x_change = (end.x > start.x) ? 1 : -1;

	PlotPixel(x, y, z, start_c);
	for (int y = start.y; y < end.y; ++y) {
		if (d < 0) {
			d += de;
		} else {
			x += x_change;
			d += dne;
		}
		z += dz;
		color += dc;
		PlotPixel(x, y, z, color);
	}
}

void Renderer::DrawModerateLine(const vec3& p1, const vec3& p2, const vec3& c1, const vec3& c2) {
	vec3 start, start_c, end, end_c;

	if (p1.x > p2.x) {
		start = p2;
		start_c = c2;
		end = p1;
		end_c = c1;
	} else {
		start = p1;
		start_c = c1;
		end = p2;
		end_c = c2;
	}

	int x = start.x;
	int y = start.y;
	float z = start.z;
	vec3 color = start_c;

	int dx = end.x - start.x;
	int dy = abs(end.y - start.y);
	float dz = (end.z - start.z) / dx;
	vec3 dc = (end_c - start_c) / dx;

	int de = 2 * dy;
	int d = 2 * dy - dx;
	int dne = 2 * dy - 2 * dx;
	int y_change = (end.y > start.y) ? 1 : -1;

	PlotPixel(x, y, z, start_c);
	for (int x = start.x; x < end.x; ++x) {
		if (d < 0) {
			d += de;
		} else {
			y += y_change;
			d += dne;
		}

		z += dz;
		color += dc;
		PlotPixel(x, y, z, color);
	}
}

//void Renderer::PaintTriangle(const vec3& p1, const vec3& p2, const vec3& p3, const vec3& c1)
//{
//	vec3 n_p1, n_p2, n_p3;
//	if (
//		!pointToScreen(p1, vec3(), n_p1, false) &
//		!pointToScreen(p2, vec3(), n_p2, false) &
//		!pointToScreen(p3, vec3(), n_p3, false)) {
//		return;
//	}
//
//	vec3 n_cm = convertToScreen(GetCenterMass(n_p1, n_p2, n_p3));
//
//	PaintTriangleRecursive(n_p1, n_p2, n_p3, n_cm, c1);
//}
//
//void Renderer::PaintTriangleRecursive(const vec3& p1, const vec3& p2, const vec3& p3, const vec3& p, const vec3& c1)
//{
//	//if already painted
//	if (m_paintBuffer[(int)(p.y * m_width + p.x)]) {
//		return;
//	}
//
//	//Set the color of node to replacement - color.
//	vec3 np = PixelToPoint(p);
//	UpdateBCPoint(p1, p2, p3, np);
//
//	if (!PlotPixel(p.x, p.y, np.z, c1)) {
//		return;
//	}
//
//	PaintTriangleRecursive(p1, p2, p3, p + vec3(1, 0, 0), c1);
//	PaintTriangleRecursive(p1, p2, p3, p + vec3(-1, 0, 0), c1);
//	PaintTriangleRecursive(p1, p2, p3, p + vec3(0, 1, 0), c1);
//	PaintTriangleRecursive(p1, p2, p3, p + vec3(-1, 0, 0), c1);
//}


void Renderer::PaintTriangle2(const vec3& p1, const vec3& p2, const vec3& p3, const vec3& c1, const vec3& c2, const vec3& c3)
{
	vec3 top, middle, bottom;
	//sort points by height
	if ((p1.y >= p2.y) && (p1.y >= p3.y)) {
		// p1 is top point
		top = p1;
		middle = (p2.y > p3.y) ? p2 : p3;
		bottom = (p2.y > p3.y) ? p3 : p2;
	} else if ((p2.y >= p1.y) && (p2.y >= p3.y)) {
		// p1 is top point
		top = p2;
		middle = (p1.y > p3.y) ? p1 : p3;
		bottom = (p1.y > p3.y) ? p3 : p1;
	} else if ((p3.y >= p2.y) && (p3.y >= p1.y)) {
		// p1 is top point
		top = p3;
		middle = (p2.y > p1.y) ? p2 : p1;
		bottom = (p2.y > p1.y) ? p1 : p2;
	}

	vec3 s_top, s_middle, s_bottom;
	pointToScreen(top, vec3(), s_top);
	pointToScreen(middle, vec3(), s_middle);
	pointToScreen(bottom, vec3(), s_bottom);

	float t1, t2;
	int step_number = 2*(s_middle.y - s_bottom.y);
	if (step_number != 0) {
		vec3 dp1 = (middle - bottom) / step_number;
		vec3 dp2 = (top - bottom) / (s_top.y - s_bottom.y);
		for (int i = 0; i < step_number; ++i) {
			t1 = 0, t2 = 1;
			clip(bottom.y, middle.y, bottom.y, bottom.y + i * dp1.y, t1, t2);
			vec3 end_line = bottom + t2 * (middle - bottom);

			//t1 = 0, t2 = 1;
			t2 = (end_line.y - bottom.y) / (top.y - bottom.y);
			//clip(bottom.y, top.y, bottom.y, bottom.y + i * dp1.y, t1, t2);
			vec3 start_line = bottom + t2 * (top - bottom);

			DrawLine(start_line, vec3(), end_line, vec3(), c1, c2);
		}
	}

	
	
}

void Renderer::DrawTriangles(const vector<vec3>* vertices, const vector<vec3>* vertexNormals, const vector<vec3>* faceNormals) {
	vec3 white(1);
	vec3 yellow(1, 1, 0);
	vec3 pink(1, 140 / 255., 1);
	int fn_index = 0;
	int vn_index = 0;
	vec3 v_normal, f_normal;

	// assuming that vertices size is a multiplication of 3
	auto i = vertices->begin();
	while (i != vertices->end()) {
		vec3 a = *(i++);
		vec3 b = *(i++);
		vec3 c = *(i++);
		vec3 cm = GetCenterMass(a, b, c);
		//CreateLocalBuffer();
		DrawLine(a, vec3(), b, vec3(), white, white);
		DrawLine(b, vec3(), c, vec3(), white, white);
		DrawLine(c, vec3(), a, vec3(), white, white);
		PaintTriangle2(a, b, c, white, white, white);

		if (faceNormals != NULL) {
			f_normal = faceNormals->at(fn_index++);
			DrawLine(cm, vec3(), cm, f_normal, pink, pink);
		}

		if (vertexNormals != NULL) {
			v_normal = vertexNormals->at(vn_index++);
			DrawLine(a, vec3(), a, v_normal, yellow, yellow);
			v_normal = vertexNormals->at(vn_index++);
			DrawLine(b, vec3(), b, v_normal, yellow, yellow);
			v_normal = vertexNormals->at(vn_index++);
			DrawLine(c, vec3(), c, v_normal, yellow, yellow);
		}
	}
}

void Renderer::DrawSquare(const vec3& p1, const vec3& p2, const vec3& p3, const vec3& p4, const vec3& color)
{
	DrawLine(p1, vec3(), p2, vec3(), color, color);
	DrawLine(p2, vec3(), p3, vec3(), color, color);
	DrawLine(p3, vec3(), p4, vec3(), color, color);
	DrawLine(p4, vec3(), p1, vec3(), color, color);
}

void Renderer::DrawBox(const vec3& minValues, const vec3& maxValues)
{
	vec3 color(0.9);
	// front
	DrawSquare(vec3(minValues.x, minValues.y, minValues.z), vec3(maxValues.x, minValues.y, minValues.z),
		vec3(maxValues.x, maxValues.y, minValues.z), vec3(minValues.x, maxValues.y, minValues.z), color);
	// back
	DrawSquare(vec3(minValues.x, minValues.y, maxValues.z), vec3(maxValues.x, minValues.y, maxValues.z),
		vec3(maxValues.x, maxValues.y, maxValues.z), vec3(minValues.x, maxValues.y, maxValues.z), color);
	// top
	DrawSquare(vec3(minValues.x, maxValues.y, minValues.z), vec3(maxValues.x, maxValues.y, minValues.z),
		vec3(maxValues.x, maxValues.y, maxValues.z), vec3(minValues.x, maxValues.y, maxValues.z), color);
	// bottom
	DrawSquare(vec3(minValues.x, minValues.y, minValues.z), vec3(maxValues.x, minValues.y, minValues.z),
		vec3(maxValues.x, minValues.y, maxValues.z), vec3(minValues.x, minValues.y, maxValues.z), color);
	// left
	DrawSquare(vec3(minValues.x, minValues.y, minValues.z), vec3(minValues.x, minValues.y, maxValues.z),
		vec3(minValues.x, maxValues.y, maxValues.z), vec3(minValues.x, maxValues.y, minValues.z), color);
	// right
	DrawSquare(vec3(maxValues.x, minValues.y, minValues.z), vec3(maxValues.x, minValues.y, maxValues.z),
		vec3(maxValues.x, maxValues.y, maxValues.z), vec3(maxValues.x, maxValues.y, minValues.z), color);
}

void Renderer::DrawCamera()
{
	vec3 color(1, 140 / 255, 1);
	vec3 camera_location;
	bool in_sight = pointToScreen(vec3(), vec3(), camera_location);
	if (!in_sight) {
		return;
	}
	vector<vec3> vertices;
	vertices.push_back(camera_location + vec3(-10, 0.0f, 20));
	vertices.push_back(camera_location + vec3(10, 0, 30));
	vertices.push_back(camera_location + vec3(0, 25, 0));
	/*
	DrawLine(vertices[0], vertices[1], color);
	DrawLine(vertices[1], vertices[2], color);
	DrawLine(vertices[2], vertices[0], color);*/
	
	for (int i = -5; i < 5; ++i) {
		PlotPixel(camera_location.x + i, camera_location.y, camera_location.z, color);
		PlotPixel(camera_location.x, camera_location.y + i, camera_location.z, color);
	}
}

void Renderer::SetCameraTransform(const mat4 & cTransform)
{
	m_cTransform = cTransform;
	m_camera_multiply = m_projection * m_cTransform;
}

void Renderer::SetProjection(const mat4 & projection)
{
	m_projection = projection;
	m_camera_multiply = m_projection * m_cTransform;
}

void Renderer::SetZRange(float zNear, float zFar)
{
	this->zNear = zNear;
	this->zFar = zFar;
}

void Renderer::SetObjectMatrices(const mat4 & oTransform, const mat3 & nTransform)
{
	m_oTransform = oTransform;
	m_nTransform = nTransform;
}

void Renderer::SetDemoBuffer()
{
	/*for (int i = 50; i <= 150; i += 10) {
	DrawLine(vec3(100, 100, 0), vec3(i, 150, 0), vec3(1, 0, 0));
	}
	for (int i = 50; i <= 150; i += 10) {
	DrawLine(vec3(100, 100, 0), vec3(150, i, 0), vec3(0, 1, 0));
	}
	for (int i = 50; i <= 150; i += 10) {
	DrawLine(vec3(100, 100, 0), vec3(i, 50, 0), vec3(0, 0, 1));
	}
	for (int i = 50; i <= 150; i += 10) {
	DrawLine(vec3(100, 100, 0), vec3(50, i, 0), vec3(1, 1, 1));
	}
	;
	vec3 t = this->PointToScreen(vec3(0, 0, 0));

	for (float i = -1.0f; i <= 1; i += 0.1f) {
	vec3 t2 = PointToScreen(vec3(i, 1, 0));
	DrawLine(t, t2, vec3(0, abs(i), abs(i*i)));
	}*/

	DrawLine(vec3(3.05f, 0.35, 0.05), vec3(), vec3(-3.30f, 7.70f, 0.9f), vec3(), vec3(1, 0, 0.5f), vec3(0, 1, 0.5f));
	//CreateLocalBuffer();
	vector<vec3> vertices0;
	vertices0.push_back(vec3(-3.2f, 7.2f, 9));
	vertices0.push_back(vec3(0.5f, 5.6f, 8));
	vertices0.push_back(vec3(5.8f, 7.7f, 7));
	this->DrawTriangles(&vertices0);

	vector<vec3> vertices;
	vertices.push_back(vec3(0.2f, 0.2f, 20));
	vertices.push_back(vec3(0.5f, 0.6f, 30));
	vertices.push_back(vec3(0.8f, 0.7f, 0));

	vertices.push_back(vec3(-0.05f, -0.4f, 0));
	vertices.push_back(vec3(-0.15f, -0.4f, 0));
	vertices.push_back(vec3(-0.1f, -0.3f, 0));

	vertices.push_back(vec3(-0.05f, -0.32f, 0));
	vertices.push_back(vec3(-0.15f, -0.32f, 0));
	vertices.push_back(vec3(-0.1f, -0.42f, 0));

	vertices.push_back(vec3(-1.0f, -1.0f, 0));
	vertices.push_back(vec3(-1.0f, 1.0f, 0));
	vertices.push_back(vec3(1.0f, -1.0f, 0));

	vertices.push_back(vec3(1.0f, 1.0f, 0));
	vertices.push_back(vec3(-1.0f, 1.0f, 0));
	vertices.push_back(vec3(1.0f, -1.0f, 0));
	//this->DrawTriangles(&vertices);

	//DrawBox(vec3(1), vec3(3));

	//vertical line
	for (int i = 0; i < m_height; i++) {

		m_outBuffer[INDEX(m_width, m_width / 2, i, 0)] = 1;
		m_outBuffer[INDEX(m_width, m_width / 2, i, 1)] = 0;
		m_outBuffer[INDEX(m_width, m_width / 2, i, 2)] = 0;
	}
	//horizontal line
	for (int i = 0; i < m_width; i++) {
		m_outBuffer[INDEX(m_width, i, 256, 0)] = 1;
		m_outBuffer[INDEX(m_width, i, 256, 1)] = 0;
		m_outBuffer[INDEX(m_width, i, 256, 2)] = 1;
	}
}





/////////////////////////////////////////////////////
//OpenGL stuff. Don't touch.

void Renderer::InitOpenGLRendering()
{
	int a = glGetError();
	a = glGetError();
	glGenTextures(1, &gScreenTex);
	a = glGetError();
	glGenVertexArrays(1, &gScreenVtc);
	GLuint buffer;
	glBindVertexArray(gScreenVtc);
	glGenBuffers(1, &buffer);
	const GLfloat vtc[] = {
		-1, -1,
		1, -1,
		-1, 1,
		-1, 1,
		1, -1,
		1, 1
	};
	const GLfloat tex[] = {
		0,0,
		1,0,
		0,1,
		0,1,
		1,0,
		1,1 };
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vtc) + sizeof(tex), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vtc), vtc);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vtc), sizeof(tex), tex);

	GLuint program = InitShader("vshader.glsl", "fshader.glsl");
	glUseProgram(program);
	GLint  vPosition = glGetAttribLocation(program, "vPosition");

	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 2, GL_FLOAT, GL_FALSE, 0,
		0);

	GLint  vTexCoord = glGetAttribLocation(program, "vTexCoord");
	glEnableVertexAttribArray(vTexCoord);
	glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,
		(GLvoid *) sizeof(vtc));
	glProgramUniform1i(program, glGetUniformLocation(program, "texture"), 0);
	a = glGetError();
}

void Renderer::CreateOpenGLBuffer()
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gScreenTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, m_width, m_height, 0, GL_RGB, GL_FLOAT, NULL);
	glViewport(0, 0, m_width, m_height);
}

void Renderer::UpdateBuffers(int width, int height)
{
	DestroyBuffers();
	CreateBuffers(width, height);
}

void Renderer::SwapBuffers()
{

	int a = glGetError();
	glActiveTexture(GL_TEXTURE0);
	a = glGetError();
	glBindTexture(GL_TEXTURE_2D, gScreenTex);
	a = glGetError();
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, GL_RGB, GL_FLOAT, m_outBuffer);
	glGenerateMipmap(GL_TEXTURE_2D);
	a = glGetError();

	glBindVertexArray(gScreenVtc);
	a = glGetError();
	glDrawArrays(GL_TRIANGLES, 0, 6);
	a = glGetError();
	glutSwapBuffers();
	a = glGetError();
}

void Renderer::ClearColorBuffer()
{
	vec3 black(0);

	for (int x = 0; x < m_width; ++x) {
		for (int y = 0; y < m_height; ++y) {
			PlotPixel(x, y, INFINITY, black);
		}
	}
}

void Renderer::ClearDepthBuffer()
{
	if (m_zBuffer != NULL) {
		for (int x = 0; x < m_width; ++x) {
			for (int y = 0; y < m_height; ++y) {
				m_zBuffer[m_width * y + x] = -INFINITY;//can be zFar
			}
		}
	}
}