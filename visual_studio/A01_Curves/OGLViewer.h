#pragma once
#ifndef __OGLVIEWER__
#define __OGLVIEWER__

#include "GL/glew.h"

#include <QOpenGLWidget>
#include <QKeyEvent>
#include <QTimer>
#include <QTime>
//#include <QGLFunctions>
#include <vector>
#include "OpenGL_Utils/GLSLProgram.h"
#include "Math/MathUtil.h"
#include "Math/CGVector.h"
#include "Geometry/Mesh.h"
//#include "Math/Matrix4D.h"
#include "Camera/Camera.h"
#include "Accel/KdTreeAccel.h"

static int point_proj_mat_loc;// Porjection matrix location
static int curve_proj_mat_loc;// Porjection matrix location
static GLfloat proj_mat[16] = {
	1,0,0,0,
	0,1,0,0,
	0,0,1,0,
	0,0,0,1
};

static GLfloat* points_verts;// vertices vbo
static GLfloat* points_colors;// Texture coordinates vbo

//static int box_vbo_size;// Triangle face numbers
static Matrix4D matrix;// Transform matrix

static GLSLProgram* curve_shader;
static GLSLProgram* points_shader;// OpenGL shader program
static int vertex_position;// Uniform matrix location
static int vertex_colour;// Uniform matrix location
//////////////////////////////////////////////////////////////////////////
enum OPERATION_MODE
{
	DRAWING_MODE = 0,
	EDIT_MODE = 1,
	VIEW_MODE = 2
};
// OpenGL Window in Qt
class OGLViewer : public QOpenGLWidget
{
	Q_OBJECT
public:
	//OGLViewer();
	OGLViewer(QWidget *parent = nullptr);
	~OGLViewer();

	//void update();
public slots:
	//void resetCamera();
	void initParas();
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
public:
	vector<Point3D*> ctrl_points;
protected:
	//orthoCamera *view_cam;
private:
	int fps;

	QTime process_time;
	int m_lastMousePos[2];
private:
	int display_mode = 0;
	int cv_op_mode = DRAWING_MODE;
	friend class MainWindow;
};

#endif // __OGLVIEWER__