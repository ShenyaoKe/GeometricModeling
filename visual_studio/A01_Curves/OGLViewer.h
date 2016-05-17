#pragma once
#ifndef __OGLVIEWER__
#define __OGLVIEWER__

#include "GL/glew.h"
#include "common.h"
#include <QOpenGLWidget>
#include <QKeyEvent>
#include <QTimer>
#include <QTime>

#include "OpenGL_Utils/GLSLProgram.h"
#include "Math/MathUtil.h"
#include "Math/Geometry.h"
#include "Math/Matrix4x4.h"
#include "Accel/Bounds.h"
#include "curveIntersection.h"

static GLfloat proj_mat[16] = {
	1,0,0,0,
	0,1,0,0,
	0,0,1,0,
	-1,1,0,1
};

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

	void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
	void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
	void mouseMoveEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
private:
	void exportPointVBO(vector<GLfloat> &ptsVBO);
	void updateBufferData();
	// Intersection functions
	// box-box
	bool intersect(const vector<Point2f*> &cv0,
		const vector<Point2f*> &cv1);
	bool intersect(const vector<Point2f*> cv,
		const Point2f* p0, const Point2f* p1);
	bool intersect(const Point2f* p0, const Point2f* p1,
		const Point2f* q0, const Point2f* q1);
	bool internalIntersect(const vector<Point2f*> &cv);
public:
	vector<Point2f *> ctrl_pts;
	vector<Point2f *> intersects;// intersections
protected:
private:
	int lastMousePos[2];
	GLfloat viewScale;
	// Display Options
	bool drawCtrlPts;
	bool drawCurves;
	bool drawIntx;// draw intersection
private:
	int cv_op_mode;// draw edit view
	int cv_type;
	GLint curve_degree;
	//GLint curve_degree_loc;
	GLint curve_seg;
	//GLint curve_seg_loc;

	GLuint pts_vbo, pts_vao, curve_vao;
	GLuint intx_vbo, intx_vao;

	Point2f* curPoint;
	vector<GLfloat> points_verts, intx_verts;// vertices vbo
	unique_ptr<GLSLProgram> points_shader;// OpenGL shader program
	unique_ptr<GLSLProgram> intx_shader;
	shared_ptr<GLSLProgram> cvShaders[5];
	shared_ptr<GLSLProgram> cvShader;

	friend class MainWindow;
};

#endif // __OGLVIEWER__