
#include "stdafx.h"
#include "Renderer.h"
#include "Polygon.h"
#include "CG_skel_w_MFC.h"
#include "InitShader.h"
#include "GL\freeglut.h"
#include <functional>

#define INDEX(width,x,y,c) (x+y*width)*3+c

Renderer::Renderer() : BaseRenderer(512, 512), m_zBuffer(NULL),
m_cTransform(), m_projection(), m_oTransform(), m_nTransform(), m_cnTransform(), shader(NULL), is_wire_mode(false)
{
	InitOpenGLRendering();
	CreateBuffers(512, 512);
}
Renderer::Renderer(int width, int height, Shader * shader) : BaseRenderer(width, height), m_zBuffer(NULL),
m_cTransform(), m_projection(), m_oTransform(), m_nTransform(), m_cnTransform(), shader(shader), is_wire_mode(false)
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
	vec3 v;
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

	//TODO: recalculate colors
	std::cout << "newP1" << newP1 << "newP2" << newP2 << std::endl;
	if (abs(newP1.y - newP2.y) > abs(newP1.x - newP2.x)) {
		this->DrawSteepLine(newP1, newP2, c1);
	} else {
		this->DrawModerateLine(newP1, newP2, c1);
	}

	return true;
}

void Renderer::DrawLine(const vec3& p1, const vec3& p2, const vec3& c)
{
	vec3 q1 = convertToScreen(p1);
	vec3 q2 = convertToScreen(p2);

	if (abs(q1.y - q2.y) > abs(q1.x - q2.x)) {
		this->DrawSteepLine(q1, q2, c);
	} else {
		this->DrawModerateLine(q1, q2, c);
	}
}

void Renderer::DrawSteepLine(const vec3& p1, const vec3& p2, const vec3& c) {
	vec3 start, end;

	if (p1.y > p2.y) {
		start = p2;
		end = p1;
	} else {
		start = p1;
		end = p2;
	}

	int x = start.x;
	int y = start.y;
	float z = start.z;
	
	int dx = abs(end.x - start.x);
	int dy = end.y - start.y;
	float dz = (end.z - start.z) / dy;

	int de = 2 * dx;
	int d = 2 * dx - dy;
	int dne = 2 * dx - 2 * dy;
	int x_change = (end.x > start.x) ? 1 : -1;

	PlotPixel(x, y, z, c);
	for (int y = start.y; y < end.y; ++y) {
		if (d < 0) {
			d += de;
		} else {
			x += x_change;
			d += dne;
		}
		z += dz;
		PlotPixel(x, y, z, c);
	}
}

void Renderer::DrawModerateLine(const vec3& p1, const vec3& p2, const vec3& c) {
	vec3 start, end;

	if (p1.x > p2.x) {
		start = p2;
		end = p1;
	} else {
		start = p1;
		end = p2;
	}

	int x = start.x;
	int y = start.y;
	float z = start.z;

	int dx = end.x - start.x;
	int dy = abs(end.y - start.y);
	float dz = (end.z - start.z) / dx;

	int de = 2 * dy;
	int d = 2 * dy - dx;
	int dne = 2 * dy - 2 * dx;
	int y_change = (end.y > start.y) ? 1 : -1;

	PlotPixel(x, y, z, c);
	for (int x = start.x; x < end.x; ++x) {
		if (d < 0) {
			d += de;
		} else {
			y += y_change;
			d += dne;
		}

		z += dz;
		PlotPixel(x, y, z, c);
	}
}

void Renderer::PaintTriangle(const vec3& p1, const vec3& p2, const vec3& p3, const vec3& c1)
{
	vec3 n_p1, n_p2, n_p3;
	//if (
	//	!pointToScreen(p1, vec3(), n_p1, false) &
	//	!pointToScreen(p2, vec3(), n_p2, false) &
	//	!pointToScreen(p3, vec3(), n_p3, false)) {
	//	//whole triangle is outside the clipping area.
	//	return;
	//}

	pointToScreen(p1, vec3(), n_p1, false);
	pointToScreen(p2, vec3(), n_p2, false);
	pointToScreen(p3, vec3(), n_p3, false);

	vec3 sp1 = convertToScreen(n_p1);
	vec3 sp2 = convertToScreen(n_p2);
	vec3 sp3 = convertToScreen(n_p3);
	if (((sp2.x - sp1.x) * (sp3.y - sp1.y)) - ((sp3.x - sp1.x) * (sp2.y - sp1.y)) == 0) {
		// the 3 points perform a stright line.
		return;
	}
	
	vec3 n_cm = convertToScreen(GetCenterMass(n_p1, n_p2, n_p3));
	std::cout << "p1=" << p1 << "\tp2=" << p2 << "\tp3=" << p3 << "\tp=" << n_cm << std::endl;
	std::cout << "p1=" << n_p1 << "\tp2=" << n_p2 << "\tp3=" << n_p3 << "\tp=" << n_cm << std::endl;
	std::cout << "p1=" << convertToScreen(n_p1) << "\tp2=" << convertToScreen(n_p2) << "\tp3=" << convertToScreen(n_p3) << "\tp=" << n_cm << std::endl;

	PaintTriangleFloodFill(n_p1, n_p2, n_p3, n_cm);
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

///<summary>
/// Determine whether a point P is inside the triangle ABC. Note, this function
/// assumes that P is coplanar with the triangle.
/// from https://blogs.msdn.microsoft.com/rezanour/2011/08/07/barycentric-coordinates-and-point-in-triangle-tests/
///</summary>
///<returns>True if the point is inside, false if it is not.</returns>
bool PointInTriangle(const vec3& A, const vec3& B, const vec3& C, const vec3& P)
{
	// Prepare our barycentric variables
	vec3 u = B - A;
	vec3 v = C - A;
	vec3 w = P - A;
	vec3 vCrossW = cross(v, w);
	vec3 vCrossU = cross(v, u);

	// Test sign of r
	if (dot(vCrossW, vCrossU) < 0)
		return false;

	vec3 uCrossW = cross(u, w);
	vec3 uCrossV = cross(u, v);

	// Test sign of t
	if (dot(uCrossW, uCrossV) < 0)
		return false;

	// At this point, we know that r and t and both > 0.
	// Therefore, as long as their sum is <= 1, each must be less <= 1
	float denom = length(uCrossV);
	float r = length(vCrossW) / denom;
	float t = length(uCrossW) / denom;
	return (r + t <= 1);
}

bool Renderer::PixelToPoint(const vec3& p1, const vec3& p2, const vec3& p3, const vec3& p, vec3& newP) const {

	float x = (p.x - m_width * 0.5) / min_size;
	float y = (p.y - m_height * 0.5) / min_size;
	vec2 np(x, y);

	mat2 a(np.x - p1.x, p2.x - p3.x,
		np.y - p1.y, p2.y - p3.y);
	vec2 tk = inverse(a) * vec2(p2.x - p1.x, p2.y - p1.y);
	// p4 is the intersection of lines: p1<->p, p2<->p3
	vec3 p4 = p2 + tk[1] * (p3 - p2);

	//return p1 + (p4 - p1) / tk[0];
	newP = p1 + (p4 - p1) / tk[0];
	newP.x = np.x;
	newP.y = np.y;

	if (!PointInTriangle(p1, p2, p3, newP))
		return false;
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
	
	// TODO: duplicate the polygon to store the camera world coordinates.
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
		DrawLine(a, b);
		DrawLine(b, c);
		DrawLine(c, a);

		vec3 cm = convertToScreen(GetCenterMass(a, b, c));
		PaintTriangleFloodFill(a, b, c, cm);
		//PaintTriangleScanLines(a, b, c, vec3(1)); // last is color
	}
	
	while (!triangles.empty()) {
		ConvexPolygon * polygon = triangles.back();
		triangles.pop_back();
		delete polygon;
	}
}

void Renderer::PaintTriangleFloodFill(const vec3& p1, const vec3& p2, const vec3& p3, const vec3& p)
{
	//if already painted
	if (m_paintBuffer[(int)(p.y * m_width + p.x)]) {
		return;
	}

	//	3. Set Q to the empty queue.
	vector<vec3> q;
	float abc_area = calculateArea(p1, p2, p3);

	//	4. Set the color of node to replacement - color.
	vec3 newP = p;
	vec3 c = CalculatePointColor(p1, p2, p3, abc_area, newP);
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

void Renderer::PaintTriangleScanLines(const vec3& p1, const vec3& p2, const vec3& p3, const vec3& c1)
{
	vec3 s_p1 = convertToScreen(p1);
	vec3 s_p2 = convertToScreen(p2);
	vec3 s_p3 = convertToScreen(p3);
	// check all in line

	vec3 top, middle, bottom;
	//sort points by height
	if ((s_p1.y >= s_p2.y) && (s_p1.y >= s_p3.y)) {
		// s_p1 is top point
		top = s_p1;
		middle = (s_p2.y > s_p3.y) ? s_p2 : s_p3;
		bottom = (s_p2.y > s_p3.y) ? s_p3 : s_p2;
	} else if ((s_p2.y >= s_p1.y) && (s_p2.y >= s_p3.y)) {
		// s_p2 is top point
		top = s_p2;
		middle = (s_p1.y > s_p3.y) ? s_p1 : s_p3;
		bottom = (s_p1.y > s_p3.y) ? s_p3 : s_p1;
	} else if ((s_p3.y >= s_p2.y) && (s_p3.y >= s_p1.y)) {
		// s_p3 is top point
		top = s_p3;
		middle = (s_p2.y > s_p1.y) ? s_p2 : s_p1;
		bottom = (s_p2.y > s_p1.y) ? s_p1 : s_p2;
	}

	for (float y = bottom.y; y < top.y; ++y) {
		vec3 q;
		float minx, maxx;
		minx = INFINITY;
		maxx = -INFINITY;

		if (GetIntersectionPoint(p1, p2, y, q)) {
			minx = min(minx, q.x);
			maxx = max(maxx, q.x);
		}
		if (GetIntersectionPoint(p2, p3, y, q)) {
			minx = min(minx, q.x);
			maxx = max(maxx, q.x);
		}
		if (GetIntersectionPoint(p1, p3, y, q)) {
			minx = min(minx, q.x);
			maxx = max(maxx, q.x);
		}

		minx = floor(minx);
		maxx = ceil(maxx);

		vec3 minx_camera, maxx_camera;
		PixelToPoint(p1, p2, p3, vec3(minx, y, 0), minx_camera);
		PixelToPoint(p1, p2, p3, vec3(maxx, y, 0), maxx_camera);

		for (float x = minx; x <= maxx; ++x) {
			vec3 newP;
			if ((!m_paintBuffer[(int)(y * m_width + x)]) &&
				(PixelToPoint(p1, p2, p3, vec3(x, y, 0), newP))) {
				PlotPixel(x, y, newP.z, c1);
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
	vec3 pink(1, 140 / 255., 1);
	int fn_index = 0;
	int vn_index = 0;
	vec3 v_normal, f_normal;

	// assuming that vertices size is a multiplication of 3
	auto i = vertices->begin();
	auto m = materials->begin();
	while (i != vertices->end()) {
		// ----- new version ----
		vector<vec3> tri_vertices;
		tri_vertices.push_back(*(i++));
		tri_vertices.push_back(*(i++));
		tri_vertices.push_back(*(i++));
		vec3 cm = GetCenterMass(&tri_vertices);

		// TODO: get materials from scene?
		vector<Material> materials;
		materials.push_back(*(m++));
		materials.push_back(*(m++));
		materials.push_back(*(m++));

		vector<vec3> v_normals;
		if ((vertexNormals != NULL) && (vertexNormals->size() != 0)) {
			v_normals.push_back(vertexNormals->at(vn_index++));
			v_normals.push_back(vertexNormals->at(vn_index++));
			v_normals.push_back(vertexNormals->at(vn_index++));
		}

		if ((faceNormals != NULL) && (faceNormals->size() != 0)) {
			f_normal = faceNormals->at(fn_index++);
		}

		if (is_wire_mode) {
			std::cout << "line drawing" << std::endl;
			DrawLine(tri_vertices[0], vec3(), tri_vertices[1], vec3(), white);
			DrawLine(tri_vertices[1], vec3(), tri_vertices[2], vec3(), white);
			DrawLine(tri_vertices[2], vec3(), tri_vertices[0], vec3(), white);
		} else {
			vec3 transform_f_normal = normalize(m_cnTransform * m_nTransform * f_normal);
			PaintTriangle(&tri_vertices, &materials, &v_normals, transform_f_normal);
		}

		if ((allowVertexNormals) && (v_normals.size() > 0)) {
			DrawLine(tri_vertices[0], vec3(), tri_vertices[0], v_normals[0], yellow);
			DrawLine(tri_vertices[1], vec3(), tri_vertices[1], v_normals[1], yellow);
			DrawLine(tri_vertices[2], vec3(), tri_vertices[2], v_normals[2], yellow);
		}

		if ((allowFaceNormals) && (faceNormals != NULL)) {
			DrawLine(cm, vec3(), cm, f_normal, pink);
		}
	}
}

void Renderer::DrawSquare(const vec3& p1, const vec3& p2, const vec3& p3, const vec3& p4, const vec3& color)
{
	DrawLine(p1, vec3(), p2, vec3(), color);
	DrawLine(p2, vec3(), p3, vec3(), color);
	DrawLine(p3, vec3(), p4, vec3(), color);
	DrawLine(p4, vec3(), p1, vec3(), color);
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
	
	/*vector<vec3> vertices;
	vertices.push_back(camera_location + vec3(-10, 0, 0));
	vertices.push_back(camera_location + vec3(10, 0, 0));
	vertices.push_back(camera_location + vec3(0, 25, 0));
	
	DrawLine(vertices[0], vertices[1], color);
	DrawLine(vertices[1], vertices[2], color);
	DrawLine(vertices[2], vertices[0], color);
	*/
	for (int i = -5; i < 5; ++i) {
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
	
	for (int i = -5; i <= 5; ++i) {
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
}

void Renderer::SetCameraTransform(const mat4 & cTransform)
{
	m_cTransform = cTransform;
	m_cnTransform = convert4dTo3d(m_cTransform);
	shader->setTransform(m_cTransform * m_oTransform);//is this a must?
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

	shader->setTransform(m_cTransform * m_oTransform);
}

void Renderer::SetDemoBuffer()
{
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

void Renderer::SwitchWire()
{
	is_wire_mode = !is_wire_mode;
}

void Renderer::SetBaseShader(Shader * s) {
	if (shader != NULL) {
		delete shader;
	}

	shader = s;
}

void Renderer::SetFog(const vec3& color)
{
	if (shader != NULL) {
		shader = new FogShader(shader, color);
	} else {
		shader = new FogShader(new FlatShader(), color);
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
	for (int i = 0; i < m_width * m_height * 3; ++i) {
		m_outBuffer[i] = 0;
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