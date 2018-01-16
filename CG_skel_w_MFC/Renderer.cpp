
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
using namespace glm;

#include "stdafx.h"
#include "Renderer.h"
#include "Polygon.h"
#include "CG_skel_w_MFC.h"
#include "InitShader.h"
#include "GL/freeglut.h"

#include <functional>

#define INDEX(width,x,y,c) ((x)+(y)*(width))*3+(c)

Renderer::Renderer() : BaseRenderer(512, 512), m_zBuffer(NULL),
m_cTransform(1), m_projection(1), m_oTransform(1), m_nTransform(1), m_cnTransform(1), shader(NULL), is_wire_mode(false)
{
	default_shader = new FlatShader();
	InitOpenGLRendering();
	anti_factor = 1;
	CreateBuffers(512, 512);
}
Renderer::Renderer(int width, int height, Shader * shader) : BaseRenderer(width, height), m_zBuffer(NULL),
m_cTransform(1), m_projection(1), m_oTransform(1), m_nTransform(1), m_cnTransform(1), shader(shader), is_wire_mode(false)
{
	default_shader = new FlatShader();
	InitOpenGLRendering();
	anti_factor = 1;
	CreateBuffers(width, height);
}

Renderer::~Renderer(void)
{
	delete default_shader;
	delete shader;
	DestroyBuffers();
}

void Renderer::CreateBuffers(int width, int height)
{
	m_width = anti_factor * width;
	m_height = anti_factor * height;
	m_screen_width = width;
	m_screen_height = height;
	min_size = min(m_width, m_height) / 2;
	CreateOpenGLBuffer(); //Do not remove this line.
	m_outBuffer = new float[3 * m_width * m_height];
	m_screenBuffer = new float[3 * m_screen_width * m_screen_height];
	m_zBuffer = new float[m_width * m_height];
	m_paintBuffer = NULL;
}

void Renderer::CreateLocalBuffer()
{
	if (m_paintBuffer != NULL) {
		delete[] m_paintBuffer;
	}
	m_paintBuffer = new bool[m_width * m_height];
	for (int i = 0; i < m_width * m_height; ++i) {
		m_paintBuffer[i] = false;
	}
}

void Renderer::DestroyBuffers()
{
	delete[] m_outBuffer;
	delete[] m_screenBuffer;
	if (m_zBuffer != NULL) {
		delete[] m_zBuffer;
	}
}

vec4 Renderer::applyCameraTransformation(const vec3& p, const vec3& n) const {
	// TODO: do we need this?!
	//vec4 pTransformed;
	//vec4 nTransformed;

	//pTransformed = m_oTransform * p;
	//if (length(n) != 0) {
	//	nTransformed = normalize(m_nTransform * n);
	//	nTransformed.w = 0;
	//}

	//return m_cTransform * (pTransformed + nTransformed);
	return vec4();
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

bool Renderer::clipLine(
	const vec3& v1,
	const vec3& n1,
	const vec3& v2,
	const vec3& n2,
	vec3& start,
	vec3& end
) const {
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

bool Renderer::clipLine(
	const vec3& v1,
	const vec3& v2,
	vec3& start,
	vec3& end
) const {
	float t1 = 0;
	float t2 = 1;
	clip(v1.x, v2.x, -1, 1, t1, t2);
	clip(v1.y, v2.y, -1, 1, t1, t2);
	clip(v1.z, v2.z, -1, 1, t1, t2);
	if (t1 >= t2) {
		return false;
	}

	vec3 dv = v2 - v1;
	start = v1 + t1 * dv;
	end = v1 + t2 * dv;
	return true;
}

vec3 Renderer::convertToScreen(const vec3& p) const {
	return vec3(round(m_width * 0.5 + min_size * (p.x)), round(m_height * 0.5 + min_size * (p.y)), p.z);
}

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
	if (m_paintBuffer != NULL) {
		m_paintBuffer[y * m_width + x] = true;
	}
	return true;
}

vec3 Renderer::GetCenterMass(const vec3& p1, const vec3& p2, const vec3& p3) const {
	return vec3((p1.x + p2.x + p3.x) / 3, (p1.y + p2.y + p3.y) / 3, (p1.z + p2.z + p3.z) / 3);
}

vec3 Renderer::GetCenterMass(vector<vec3> const * const vertices) const
{
	vec3 v(0);
	if ((vertices == NULL) || (vertices->size() == 0)) {
		return v;
	}
	for (auto i = vertices->begin(); i != vertices->end(); ++i) {
		v += *i;
	}
	return v / vertices->size();
}

bool Renderer::DrawLine(const vec3& p1, const vec3& n1, const vec3& p2, const vec3& n2, const vec3& c1)
{
	vec3 newP1, newP2;
	if (!lineToScreen(p1, n1, p2, n2, newP1, newP2)) {
		std::cout << "skipped drawing lines" << std::endl;
		return false;
	}

	if (abs(newP1.y - newP2.y) > abs(newP1.x - newP2.x)) {
		this->DrawSteepLine(newP1, newP2, c1);
	} else {
		this->DrawModerateLine(newP1, newP2, c1);
	}

	return true;
}

void Renderer::DrawLine(const vec3& p1, const vec3& p2, const vec3& c, const int p1_idx, const int p2_idx)
{
	vec3 q1 = convertToScreen(p1);
	vec3 q2 = convertToScreen(p2);

	if (abs(q1.y - q2.y) > abs(q1.x - q2.x)) {
		this->DrawSteepLine(q1, q2, c, p1_idx, p2_idx);
	} else {
		this->DrawModerateLine(q1, q2, c, p1_idx, p2_idx);
	}
}

void Renderer::DrawSteepLine(const vec3& p1, const vec3& p2, const vec3& c, const int p1_idx, const int p2_idx) {
	vec3 start, end;
	int start_idx, end_idx;
	vec3 b_c, shader_color;

	if (p1.y > p2.y) {
		start = p2;
		start_idx = p2_idx;
		end = p1;
		end_idx = p1_idx;
	} else {
		start = p1;
		start_idx = p1_idx;
		end = p2;
		end_idx = p2_idx;
	}

	int x = start.x;
	int y = start.y;
	float z = start.z;
	
	int dx = abs(end.x - start.x);
	int dy = end.y - start.y;
	float dz = (end.z - start.z) / dy;
	float t = dy;

	int de = 2 * dx;
	int d = 2 * dx - dy;
	int dne = 2 * dx - 2 * dy;
	int x_change = (end.x > start.x) ? 1 : -1;

	if (c.x == -1) {
		b_c[start_idx] = t / dy;	// dy can't be 0
		b_c[end_idx] = 1 - b_c[start_idx];
		shader_color = shader->getColor(b_c);
	}
	PlotPixel(x, y, z, ((c.x == -1) ? shader_color : c));
	for (int y = start.y; y < end.y; ++y) {
		if (d < 0) {
			d += de;
		} else {
			x += x_change;
			d += dne;
		}
		z += dz;
		--t;
		if (c.x == -1) {
			b_c[start_idx] = t / dy;	// dy can't be 0
			b_c[end_idx] = 1 - b_c[start_idx];
			shader_color = shader->getColor(b_c);
		}
		PlotPixel(x, y, z, ((c.x == -1) ? shader_color : c));
	}
}

void Renderer::DrawModerateLine(const vec3& p1, const vec3& p2, const vec3& c, const int p1_idx, const int p2_idx) {
	vec3 start, end;
	int start_idx, end_idx;
	vec3 b_c, shader_color;

	if (p1.x > p2.x) {
		start = p2;
		start_idx = p2_idx;
		end = p1;
		end_idx = p1_idx;
	} else {
		start = p1;
		start_idx = p1_idx;
		end = p2;
		end_idx = p2_idx;
	}

	int x = start.x;
	int y = start.y;
	float z = start.z;

	int dx = end.x - start.x;
	int dy = abs(end.y - start.y);
	float dz = (end.z - start.z) / dx;
	float t = dx;

	int de = 2 * dy;
	int d = 2 * dy - dx;
	int dne = 2 * dy - 2 * dx;
	int y_change = (end.y > start.y) ? 1 : -1;

	if (c.x == -1) {
		b_c[start_idx] = t / dx;	// dx can't be 0
		b_c[end_idx] = 1 - b_c[start_idx];
		shader_color = shader->getColor(b_c);
	}
	PlotPixel(x, y, z, ((c.x == -1) ? shader_color : c));
	PlotPixel(x, y, z, c);
	for (int x = start.x; x < end.x; ++x) {
		if (d < 0) {
			d += de;
		} else {
			y += y_change;
			d += dne;
		}

		z += dz;
		--t;
		if (c.x == -1) {
			b_c[start_idx] = t / dx;	// dx can't be 0
			b_c[end_idx] = 1 - b_c[start_idx];
			shader_color = shader->getColor(b_c);
		}
		PlotPixel(x, y, z, ((c.x == -1) ? shader_color : c));
	}
}

void Renderer::SetPolygonToShader(const ConvexPolygon * shader_polygon, const vec3& f_normal) {
	vector<vec4> tri_vertex = shader_polygon->getVertices();
	mat3 vertex(convert4dTo3d(tri_vertex[0]),
		convert4dTo3d(tri_vertex[1]),
		convert4dTo3d(tri_vertex[2]));

	vector<vec3> tri_normals = shader_polygon->getNormals();
	mat3 normals(0);
	if (tri_normals.size() != 0) {
		normals = mat3(tri_normals[0],
			tri_normals[1],
			tri_normals[2]);
	}

	vector<Material> tri_materials = shader_polygon->getMaterials();
	PolygonMaterial pm = {tri_materials[0],
		tri_materials[1],
		tri_materials[2]
	};

	shader->setPolygon(vertex, pm, normals, f_normal);
}

vec3 Renderer::CalculatePointColor(const vec3& p1, const vec3& p2, const vec3& p3, const float abc_area, const vec3& p) {
	vec3 b_c = calculateBarycentricCoordinates(p1, p2, p3, p, abc_area);
	return shader->getColor(b_c);
}

bool Renderer::PixelToPoint(const vec3& p1, const vec3& p2, const vec3& p3, const vec3& p, vec3& newP) const {

	float x = (p.x - m_width * 0.5) / min_size;
	float y = (p.y - m_height * 0.5) / min_size;
	vec2 np(x, y);

	mat2 a(np.x - p1.x, p2.x - p3.x,
		np.y - p1.y, p2.y - p3.y);
	if (glm::determinant(a) == 0) {
		return false;
	}
	vec2 tk = inverse(a) * vec2(p2.x - p1.x, p2.y - p1.y);
	// p4 is the intersection of lines: p1<->p, p2<->p3
	vec3 p4 = p2 + tk[1] * (p3 - p2);

	newP = p1 + (p4 - p1) / tk[0];
	newP.x = np.x;
	newP.y = np.y;

	if ((newP.x < -1) || (newP.x > 1) ||
		(newP.y < -1) || (newP.y > 1) ||
		(newP.z < -1) || (newP.z > 1)) {
		return false;
	} else {
		return true;
	}
}

void Renderer::PaintTriangle(const vector<vec3> * vertices, const vector<Material> * materials, const vector<vec3>* v_normals, const vec3& f_normal)
{
	ConvexPolygon p(*vertices, *materials, *v_normals);
	p.transform(m_cTransform * m_oTransform, m_cnTransform * m_nTransform);
	p.clip(2, -zNear, std::less_equal<float>());
	p.clip(2, -zFar, std::greater_equal<float>());
	
	ConvexPolygon shader_polygon = p;
	
	p.transform(m_projection);
	p.divide();

	// x coordinate
	p.clip(0, 1, less_equal<float>(), &shader_polygon);
	p.clip(0, -1, greater_equal<float>(), &shader_polygon);
	// y coordinate
	p.clip(1, 1, std::less_equal<float>(), &shader_polygon);
	p.clip(1, -1, greater_equal<float>(), &shader_polygon);

	vector<ConvexPolygon*> triangles;
	vector<ConvexPolygon*> shader_triangles;
	p.getTriangles(triangles);
	shader_polygon.getTriangles(shader_triangles);
	auto j = shader_triangles.begin();

	for (auto i = triangles.begin(); (i != triangles.end()) && (j != shader_triangles.end()); ++i, ++j) {
		vector<vec4> tri_vertex = (*i)->getVertices();
		vec3 a = convert4dTo3d(tri_vertex[0]);
		vec3 b = convert4dTo3d(tri_vertex[1]);
		vec3 c = convert4dTo3d(tri_vertex[2]);

		SetPolygonToShader(*j, f_normal);

		CreateLocalBuffer();
		DrawLine(a, b, vec3(-1), 0, 1);
		DrawLine(b, c, vec3(-1), 1, 2);
		DrawLine(c, a, vec3(-1), 2, 0);

		vec3 cm = GetCenterMass(a, b, c);
		//PaintTriangleFloodFill(a, b, c, cm);
		PaintTriangleScanLines(a, b, c);
	}
	
	while (!triangles.empty()) {
		ConvexPolygon * polygon = triangles.back();
		triangles.pop_back();
		delete polygon;
	}

	while (!shader_triangles.empty()) {
		ConvexPolygon * polygon = shader_triangles.back();
		shader_triangles.pop_back();
		delete polygon;
	}
}

void Renderer::PaintTriangleFloodFill(const vec3& p1, const vec3& p2, const vec3& p3, const vec3& p)
{
	float abc_area = calculateArea(p1, p2, p3);
	vec3 c = CalculatePointColor(p1, p2, p3, abc_area, p);
	vec3 newP = convertToScreen(p);
	
	//if already painted
	if (m_paintBuffer[(int)(newP.y * m_width + newP.x)]) {
		return;
	}

	//	3. Set Q to the empty queue.
	vector<vec3> q;

	//	4. Set the color of node to replacement - color.
	PlotPixel(newP.x, newP.y, newP.z, c);

	//	5. Add node to the end of Q.
	q.push_back(newP);

	//	6. While Q is not empty:
	while (!q.empty()) {
	//	7.     Set n equal to the first element of Q.
		vec3 n = q.front();
	//	8.     Remove first element from Q.
		q.erase(q.begin());
		
	//	9.     If the color of the node to the west of n is target - color,
		if ((n.x > 0) &&
			(!m_paintBuffer[(int)((n.y) * m_width + (n.x - 1))]) &&
			(PixelToPoint(p1, p2, p3, n - vec3(1, 0, 0), newP))) {
	//	set the color of that node to replacement - color and add that node to the end of Q.
			c = CalculatePointColor(p1, p2, p3, abc_area, newP);
			newP = convertToScreen(newP);
			PlotPixel(newP.x, newP.y, newP.z, c);
			q.push_back(n - vec3(1, 0, 0));
		}
	//	10.     If the color of the node to the east of n is target - color,
		if ((n.x + 1 < m_width) &&
			(!m_paintBuffer[(int)((n.y) * m_width + (n.x + 1))]) &&
			(PixelToPoint(p1, p2, p3, n + vec3(1, 0, 0), newP))) {
	//	set the color of that node to replacement - color and add that node to the end of Q.
			c = CalculatePointColor(p1, p2, p3, abc_area, newP);
			newP = convertToScreen(newP);
			PlotPixel(newP.x, newP.y, newP.z, c);
			q.push_back(n + vec3(1, 0, 0));
		}
	
	//	11.    If the color of the node to the north of n is target - color,
		if ((n.y > 0) &&
			(!m_paintBuffer[(int)((n.y - 1) * m_width + (n.x))]) &&
			(PixelToPoint(p1, p2, p3, n - vec3(0, 1, 0), newP))) {
	//	set the color of that node to replacement - color and add that node to the end of Q.
			c = CalculatePointColor(p1, p2, p3, abc_area, newP);
			newP = convertToScreen(newP);
			PlotPixel(newP.x, newP.y, newP.z, c);
			q.push_back(n - vec3(0, 1, 0));
		}

	//	12.    If the color of the node to the south of n is target - color,
		if ((n.y + 1 < m_height) &&
			(!m_paintBuffer[(int)((n.y + 1) * m_width + (n.x))]) &&
			(PixelToPoint(p1, p2, p3, n + vec3(0, 1, 0), newP))) {
	//	set the color of that node to replacement - color and add that node to the end of Q.
			c = CalculatePointColor(p1, p2, p3, abc_area, newP);
			newP = convertToScreen(newP);
			PlotPixel(newP.x, newP.y, newP.z, c);
			q.push_back(n + vec3(0, 1, 0));
		}
	
	//	13. Continue looping until Q is exhausted.
	}
	//	14. Return.
}

bool GetIntersectionPoint(const vec3& p1, const vec3& p2, const float y, vec3& p) {
	float s = p2.y - p1.y;

	if (s == 0) {
		return false;
	}
	float t = (y - p1.y) / s;
	if ((t < 0) || (t > 1)) {
		return false;
	}

	p = p1 + t * (p2 - p1);
	return true;
}

void Renderer::PaintTriangleScanLines(const vec3& p1, const vec3& p2, const vec3& p3)
{
	if ((length(p1 - p2) == 0) ||
		(length(p2 - p3) == 0) || 
		(length(p3 - p1) == 0)) {
		return;
	}

	const vec3 * points3d[3] = { &p1, &p2, &p3 };
	vec3 s_p1 = convertToScreen(p1);
	vec3 s_p2 = convertToScreen(p2);
	vec3 s_p3 = convertToScreen(p3);
	const vec3 * points2d[3] = { &s_p1, &s_p2, &s_p3 };
	float abc_area = calculateArea(p1, p2, p3);

	int top_idx, bottom_idx;
	//sort points by height
	if ((s_p1.y >= s_p2.y) && (s_p1.y >= s_p3.y)) {
		// s_p1 is top point
		top_idx = 0;
		bottom_idx = (s_p2.y > s_p3.y) ? 2 : 1;
	} else if ((s_p2.y >= s_p1.y) && (s_p2.y >= s_p3.y)) {
		// s_p2 is top point
		top_idx = 1;
		bottom_idx = (s_p1.y > s_p3.y) ? 2 : 0;
	} else if ((s_p3.y >= s_p2.y) && (s_p3.y >= s_p1.y)) {
		// s_p3 is top point
		top_idx = 2;
		bottom_idx = (s_p2.y > s_p1.y) ? 0 : 1;
	}

	for (float y = points2d[bottom_idx]->y; y < points2d[top_idx]->y; ++y) {
		vec3 q;
		float minx, maxx;
		minx = INFINITY;
		maxx = -INFINITY;

		if (GetIntersectionPoint(s_p1, s_p2, y, q)) {
			minx = min(minx, q.x);
			maxx = max(maxx, q.x);
		}
		if (GetIntersectionPoint(s_p2, s_p3, y, q)) {
			minx = min(minx, q.x);
			maxx = max(maxx, q.x);
		}
		if (GetIntersectionPoint(s_p1, s_p3, y, q)) {
			minx = min(minx, q.x);
			maxx = max(maxx, q.x);
		}

		minx = floor(minx);
		maxx = ceil(maxx);

		for (float x = minx; x < maxx; ++x) {
			vec3 newP;
			if ((!m_paintBuffer[(int)(y * m_width + x)]) &&
				(PixelToPoint(p1, p2, p3, vec3(x, y, 0), newP))) {
				vec3 c = CalculatePointColor(p1, p2, p3, abc_area, newP);
				PlotPixel(x, y, newP.z, c);
			}

		}
		//DrawLine(p1, vec3(), p2, vec3(), vec3(1), vec3(1));
	}
}

void Renderer::DrawTriangles(
	const vector<vec3>* vertices,
	const vector<Material>* materials,
	const vector<vec3>* vertexNormals,
	const vector<vec3>* faceNormals,
	const bool allowVertexNormals,
	const bool allowFaceNormals)
{
	vec3 white(1);
	vec3 yellow(1, 1, 0);
	vec3 pink(1, 140 / 255.0f, 1);
	int fn_index = 0;
	int vn_index = 0;
	vec3 v_normal, f_normal;

	// if there are no normals, use the flat shader
	Shader * temp = NULL;
	if ((vertexNormals == NULL) || (vertexNormals->size() == 0) ||
		(faceNormals == NULL) || (faceNormals->size() == 0)) {
		temp = shader;
		shader = default_shader;
	}

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	/*m_cTransform[0].x = -0.599999964;
	m_cTransform[0].y =- 0.411596596;
	m_cTransform[0].z =	0.685994387;
	m_cTransform[0].w =	0.000000000;
	m_cTransform[1].x = 0.000000000;
	m_cTransform[1].y = 0.857492924;
	m_cTransform[1].z = 0.514495790;
	m_cTransform[1].w = 0.000000000;
	m_cTransform[2].x = -0.799999952;
	m_cTransform[2].y = 0.308697462;
	m_cTransform[2].z = -0.514495790;
	m_cTransform[2].w = 0.000000000;
	m_cTransform[3].x = - 0.00000000;
	m_cTransform[3].y = - 0.00000000;
	m_cTransform[3].z = - 5.83095264;
	m_cTransform[3].w = 1.00000000;

	m_projection[0].x = 1.81066012;
	m_projection[0].y = 0.000000000;
	m_projection[0].z = 0.000000000;
	m_projection[0].w = 0.000000000;
	m_projection[1].x = 0.000000000;
	m_projection[1].y = 2.41421342;
	m_projection[1].z = 0.000000000;
	m_projection[1].w = 0.000000000;
	m_projection[2].x = 0.000000000;
	m_projection[2].y = 0.000000000;
	m_projection[2].z = -1.00200200;
	m_projection[2].w = -1.00000000;
	m_projection[3].x =	0.000000000;
	m_projection[3].y =	0.000000000;
	m_projection[3].z = -0.200200200;
	m_projection[3].w = 0.000000000;
	*/
	//mat4 mul1 = m_cTransform * m_oTransform;

	//glm::mat4 MVP2; //= m_projection * m_cTransform * m_oTransform;
	//mat4 MVP = mul1 * m_projection;
	//std::cout << m_oTransform << std::endl;
	//std::cout << m_cTransform << std::endl;
	//std::cout << m_projection << std::endl;
	//std::cout << mul1 << std::endl;
	//std::cout << MVP << std::endl;
	mat4 MVP = m_projection * m_cTransform * m_oTransform;
	//MVP[0].x = -1.08639598;
	//MVP[0].y = -0.993682027;
	//MVP[0].z = -0.687367737;
	//MVP[0].w = -0.685994387;
	//MVP[1].x = 0.000000000;
	//MVP[1].y = 2.07017088;
	//MVP[1].z = -0.515525818;
	//MVP[1].w = -0.514495790;
	//MVP[2].x = -1.44852805;
	//MVP[2].y = 0.745261550;
	//MVP[2].z = 0.515525818;
	//MVP[2].w = 0.514495790;
	//MVP[3].x = 0.000000000;
	//MVP[3].y = 0.000000000;
	//MVP[3].z = 5.64242601;
	//MVP[3].w = 5.83095264;
	//MVP = transpose(MVP);
	//std::cout << MVP << std::endl;
	//vector<glm::vec3> vertices_copy;
	vector<vec3> vertices_copy;
	vector<vec4> vertices_copy2;
	for (auto i = vertices->begin(); i != vertices->end(); ++i) {
		//vertices_copy.push_back(glm::vec3((*i).x, (*i).y, (*i).z));
		vertices_copy.push_back(vec3((*i).x, (*i).y, (*i).z));
		//vertices_copy2.push_back(MVP * vec4((*i), 1));
	}

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	//glBufferData(GL_ARRAY_BUFFER, vertices_copy.size() * sizeof(glm::vec3), &vertices_copy[0], GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, vertices_copy.size() * sizeof(vec3), &vertices_copy[0], GL_STATIC_DRAW);

	


	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

	// first attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);

	// Draw the triangle !
	if (is_wire_mode) {
		glDrawArrays(GL_LINES, 0, vertices_copy.size());
	} else {
		glDrawArrays(GL_TRIANGLES, 0, vertices_copy.size());
	}

	glDisableVertexAttribArray(0);


	if (temp != NULL) {
		shader = temp;
	}
}

void Renderer::DrawSquare(const vec3& p1, const vec3& p2, const vec3& p3, const vec3& p4, const vec3& color)
{
	DrawLine(p1, vec3(0), p2, vec3(0), color);
	DrawLine(p2, vec3(0), p3, vec3(0), color);
	DrawLine(p3, vec3(0), p4, vec3(0), color);
	DrawLine(p4, vec3(0), p1, vec3(0), color);
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
	
	for (int i = -5 * anti_factor; i < 5 * anti_factor; ++i) {
		PlotPixel(camera_location.x + i, camera_location.y, camera_location.z, color);
		PlotPixel(camera_location.x, camera_location.y + i, camera_location.z, color);
	}
}

void Renderer::DrawLight(const vec3& color, const vec3& position)
{
	vec3 light_location;
	bool in_sight = pointToScreen(position, vec3(), light_location);
	if (!in_sight) {
		return;
	}
	
	for (int i = -5 * anti_factor; i <= 5 * anti_factor; ++i) {
		PlotPixel(light_location.x + i, light_location.y + i, light_location.z, color);	// \ 
		PlotPixel(light_location.x + i, light_location.y - i, light_location.z, color);	// /
		PlotPixel(light_location.x, light_location.y + i, light_location.z, color);		// |
		PlotPixel(light_location.x + i, light_location.y, light_location.z, color);		// -
	}
	/*
	DrawLine(light_location + vec3(-10, -10, 0), light_location + vec3(10, 10, 0), color);
	DrawLine(light_location + vec3(-10, 10, 0), light_location + vec3(10, -10, 0), color);
	DrawLine(light_location + vec3(-10,0,0), light_location + vec3(10,0,0), color);
	DrawLine(light_location + vec3(0,-10,0), light_location + vec3(0,10,0), color);
	*/
}

void Renderer::SetLights(const vector<Light *> * lights) {
	shader->setLights(lights);
	default_shader->setLights(lights);
}

void Renderer::SetCameraTransform(const mat4 & cTransform)
{
	m_cTransform = cTransform;
	m_cnTransform = convert4dTo3d(m_cTransform);
	shader->setTransform(m_cTransform);
	default_shader->setTransform(m_cTransform);
}

void Renderer::SetProjection(const mat4 & projection)
{
	m_projection = projection;
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
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	mat4 MVP;
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

	static const GLfloat g_vertex_buffer_data[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		0.0f,  1.0f, 0.0f,
	};

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	// first attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // dont normalized
		0,                  // stride
		(void*)0            // array buffer offset
	);

	glDrawArrays(GL_TRIANGLES, 0, 3); // 3 indices starting at 0 -> 1 triangle
	glDisableVertexAttribArray(0);
}

void Renderer::SwitchWire()
{
	is_wire_mode = !is_wire_mode;
}

void Renderer::SetAntiAliasing(int new_factor)
{
	if (new_factor < 1) return;

	anti_factor = new_factor;
	DestroyBuffers();
	CreateBuffers(m_screen_width, m_screen_height);
}

void Renderer::SetBaseShader(Shader * s) {
	if (shader != NULL) {
		delete shader;
	}

	shader = s;
}

void Renderer::SetFog(const vec3& color, const float extinction, const float scattering)
{
	if (shader != NULL) {
		shader = new FogShader(shader, color, extinction, scattering);
	} else {
		shader = new FogShader(new FlatShader(), color, extinction, scattering);
	}
}

/////////////////////////////////////////////////////
//OpenGL stuff. Don't touch.

void Renderer::InitOpenGLRendering()
{
	int a = glGetError();
	//a = glGetError();
	//glGenTextures(1, &gScreenTex);
	//a = glGetError();
	glGenVertexArrays(1, &gScreenVtc);
	glBindVertexArray(gScreenVtc);
	
	//const GLfloat vtc[] = {
	//	-1, -1,
	//	1, -1,
	//	-1, 1,
	//	-1, 1,
	//	1, -1,
	//	1, 1
	//};
	//const GLfloat tex[] = {
	//	0,0,
	//	1,0,
	//	0,1,
	//	0,1,
	//	1,0,
	//	1,1 };
	//
	//GLuint buffer;
	//glGenBuffers(1, &buffer);
	//glBindBuffer(GL_ARRAY_BUFFER, buffer);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(vtc) + sizeof(tex), NULL, GL_STATIC_DRAW);

	//glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vtc), vtc);
	//glBufferSubData(GL_ARRAY_BUFFER, sizeof(vtc), sizeof(tex), tex);

	//GLuint program = InitShader("vshader.glsl", "fshader.glsl");
	//glUseProgram(program);
	//GLint  vPosition = glGetAttribLocation(program, "vPosition");

	//glEnableVertexAttribArray(vPosition);
	//glVertexAttribPointer(vPosition, 2, GL_FLOAT, GL_FALSE, 0,
	//	0);

	//GLint  vTexCoord = glGetAttribLocation(program, "vTexCoord");
	//glEnableVertexAttribArray(vTexCoord);
	//glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,
	//	(GLvoid *) sizeof(vtc));
	//glProgramUniform1i(program, glGetUniformLocation(program, "texture"), 0);
	//a = glGetError();

	// shai's code
	// Create and compile our GLSL program from the shaders
	programID = InitShader("vshader_transform.glsl", "fshader_transform.glsl");
	//programID = InitShader("vshader_basic.glsl", "fshader_basic.glsl");
	// Use our shader
	glUseProgram(programID);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	
	/*glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);*/
}

void Renderer::CreateOpenGLBuffer()
{
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, gScreenTex);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, m_screen_width, m_screen_height, 0, GL_RGB, GL_FLOAT, NULL);
	//glViewport(0, 0, m_screen_width, m_screen_height);
}

void Renderer::UpdateBuffers(int width, int height)
{
	DestroyBuffers();
	CreateBuffers(width, height);
}

void Renderer::SwapBuffers()
{
	//FillAntiAliasingBuffer();
	int a = glGetError();
	//glActiveTexture(GL_TEXTURE0);
	//a = glGetError();
	//glBindTexture(GL_TEXTURE_2D, gScreenTex);
	//a = glGetError();
	//glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_screen_width, m_screen_height, GL_RGB, GL_FLOAT, m_screenBuffer);
	//glGenerateMipmap(GL_TEXTURE_2D);
	//a = glGetError();

	//glBindVertexArray(gScreenVtc);
	//a = glGetError();
	//glDrawArrays(GL_TRIANGLES, 0, 6);
	//a = glGetError();
	glutSwapBuffers();
	a = glGetError();
}

void Renderer::FillAntiAliasingBuffer() {
	int sq_factor = anti_factor * anti_factor;

	for (int x = 0; x < m_screen_width; ++x) {
		for (int y = 0; y < m_screen_height; ++y) {
			float r_sum = 0, g_sum = 0, b_sum = 0;

			int starting_x = anti_factor * x;
			int starting_y = anti_factor * y;
			for (int i = 0; i < anti_factor; ++i) {
				for (int j = 0; j < anti_factor; ++j) {
					r_sum += m_outBuffer[INDEX(m_width, starting_x + i, starting_y + j, 0)];
					m_outBuffer[INDEX(m_width, starting_x + i, starting_y + j, 0)] = 0;
					g_sum += m_outBuffer[INDEX(m_width, starting_x + i, starting_y + j, 1)];
					m_outBuffer[INDEX(m_width, starting_x + i, starting_y + j, 1)] = 0;
					b_sum += m_outBuffer[INDEX(m_width, starting_x + i, starting_y + j, 2)];
					m_outBuffer[INDEX(m_width, starting_x + i, starting_y + j, 2)] = 0;
				}
			}

			m_screenBuffer[INDEX(m_screen_width, x, y, 0)] = r_sum / sq_factor;
			m_screenBuffer[INDEX(m_screen_width, x, y, 1)] = g_sum / sq_factor;
			m_screenBuffer[INDEX(m_screen_width, x, y, 2)] = b_sum / sq_factor;
		}
	}
}

void Renderer::ClearColorBuffer()
{
	glClear(GL_COLOR_BUFFER_BIT);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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