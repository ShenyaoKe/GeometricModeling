#pragma once
#ifndef __OGLVIEWER__
#define __OGLVIEWER__

#include "GL/glew.h"

#include <QOpenGLWidget>
#include <QKeyEvent>
#include <QTimer>
#include <QTime>
//#include <QGLFunctions>
#include <cstdio>
#include <cstdlib>
//#include <crtdefs>
#include <vector>
#include "OpenGL_Utils/GLSLProgram.h"
#include "Math/MathUtil.h"
#include "Math/CGVector.h"
//#include "Geometry/Mesh.h"
#include "Math/Matrix4D.h"
#include "Accel/BBox.h"
#include "curveIntersection.h"
//#include "Camera/Camera.h"
//#include "Accel/KdTreeAccel.h"

static int point_proj_mat_loc;// Porjection matrix location
static int curve_proj_mat_loc;// Porjection matrix location
static GLint win_size_loc;
static GLfloat proj_mat[16] = {
	1,0,0,0,
	0,1,0,0,
	0,0,1,0,
	0,0,0,1
};

static GLfloat* points_verts = nullptr;// vertices vbo
static GLfloat* points_colors;// Texture coordinates vbo

//static int box_vbo_size;// Triangle face numbers
static Matrix4D matrix;// Transform matrix

static GLSLProgram* points_shader;// OpenGL shader program
static GLSLProgram* intersection_shader;
static GLSLProgram* curve_shaders[5];
//static GLSLProgram* lagrange_shader;// OpenGL shader program
static int vertex_position;// Uniform matrix location
static int vertex_colour;// Uniform matrix location
//////////////////////////////////////////////////////////////////////////
enum OPERATION_MODE
{
	DRAWING_MODE = 0,
	EDIT_MODE = 1,
	VIEW_MODE = 2
};
enum CURVE_TYPE
{
	LAGRANGE_CURVE = 0,
	BEZIER_CURVE = 1,
	B_SPLINE = 2,
	CATMULL_ROM_CURVE = 3
};
// OpenGL Window in Qt
class OGLViewer : public QOpenGLWidget
{
	Q_OBJECT
public:
	//OGLViewer();
	OGLViewer(QWidget *parent = nullptr);
	~OGLViewer();

public slots:
	void updateCamera();
	void initParas();
	void clearVertex();
	void changeOperation(int val = EDIT_MODE);
	void changeCurveType(int new_cv_type = 0);
	void setDegree(int val = 1);
	void setSegment(int val = 20);
	void findIntersections();
	void writePoints(const char *filename);
	void readPoints(const char *filename);
	void exportSVG(const char *filename);
	void setDispCtrlPts(bool mode);
	void setDispCurves(bool mode);
	void setDispIntersections(bool mode);
protected:
	void initializeGL() Q_DECL_OVERRIDE;
	void paintGL() Q_DECL_OVERRIDE;
	void resizeGL(int w, int h) Q_DECL_OVERRIDE;
	void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;

	void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
	void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
	void mouseMoveEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
private:
	void exportPointVBO(GLfloat* &ptsVBO);
	// Intersection functions
	// box-box
	bool intersect(const vector<Point3D*> &cv0,
		const vector<Point3D*> &cv1);
	bool intersect(const vector<Point3D*> cv,
		const Point3D* p0, const Point3D* p1);
	bool intersect(const Point3D* p0, const Point3D* p1,
		const Point3D* q0, const Point3D* q1);
	bool internalIntersect(const vector<Point3D*> &cv);
	void subdivBezier(const vector<Point3D*> &cvs,
		vector<Point3D*> &lf_cvs, vector<Point3D*> &rt_cvs);
public:
	vector<Point3D *> ctrl_points;
	vector<Point3D *> intersections;
protected:
private:
	int m_lastMousePos[2];
	GLfloat viewScale;
	// Display Options
	bool drawCtrlPts;
	bool drawCurves;
	bool drawIntersection;
private:
	int cv_op_mode;// draw edit view
	int cv_type;
	GLint curve_degree;
	GLint curve_degree_loc;
	GLint curve_seg;
	GLint curve_seg_loc;

	Point3D* curPoint;
	friend class MainWindow;
};

#endif // __OGLVIEWER__