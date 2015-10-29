#pragma once
#ifndef __OGLVIEWER__
#define __OGLVIEWER__

#include "GL/glew.h"

#include <QOpenGLWidget>
#include <QKeyEvent>
#include <QTimer>
#include <QTime>
#include <QPixmap>
#include <QImage>
#include <QPainter>
//#include <QGLFunctions>
#include "common.h"

#include "OpenGL_Utils/GLSLProgram.h"
#include "Math/MathUtil.h"

//#include "Math/CGVector.h"
//#include "Geometry/Mesh.h"
//#include "Math/Matrix4D.h"
//#include "Accel/BBox.h"
//#include "Image/ImageData.h"
#include "ImageScalor.h"
#include "Bezier.h"
//#include "curveIntersection.h"
//#include "Camera/Camera.h"
//#include "Accel/KdTreeAccel.h"

static int point_color_loc;
static int point_size_loc;
static int point_view_mat_loc;// Porjection matrix location
static int point_proj_mat_loc;// Porjection matrix location
static int cv_flc_loc;
static int cv_frc_loc;
static int cv_elc_loc;
static int cv_erc_loc;

static int curve_proj_mat_loc;// Porjection matrix location
static int curve_view_mat_loc;// Porjection matrix location
static GLint win_size_loc;
static GLfloat proj_mat[16] = {
	1,0,0,0,
	0,1,0,0,
	0,0,1,0,
	0,0,0,1
};
static GLfloat sel_pt_color[3] = { 1, 0, 0 };
static GLfloat uns_pt_color[3] = { 0.6, 0.4, 0.88 };


static GLint points_size;
static GLfloat* points_verts = nullptr;// vertices vbo
static GLfloat* points_colors = nullptr;// Texture coordinates vbo

//static int box_vbo_size;// Triangle face numbers
//static Matrix4D matrix;// Transform matrix

static GLSLProgram* points_shader;// OpenGL shader program
static GLSLProgram* curve_shaders;
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
	//void changeCurveType(int new_cv_type = 0);
	void setDegree(int val = 1);
	void setSegment(int val = 20);

	void writePoints(const char *filename);
	void readPoints(const char *filename);
	void exportSVG(const char *filename);
	void setDispCtrlPts(bool mode);
	void setDispCurves(bool mode);

	//
	void generateDiffusionCurve();
	void saveFrameBuffer();
signals:
	void selectionChanged();
protected:
	void initializeGL() Q_DECL_OVERRIDE;
	void paintGL() Q_DECL_OVERRIDE;
	void resizeGL(int w, int h) Q_DECL_OVERRIDE;
	void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;

	void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
	void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
	void mouseMoveEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
private:
	void clearImageBuffers();
	//void exportPointVBO(GLfloat* &ptsVBO);
	void selectPoint(const QVector3D *cursor);
private:// Points
	vector<QVector3D*> ctrl_points;
	vector<Bezier*> curves;

	vector<QImage*> ds_imgs;
	vector<QImage*> us_imgs;
protected:
private:// Viewport configurations
	int m_lastMousePos[2];
	GLfloat viewScale;
	GLfloat viewTx, viewTy; // View translation
	// Display Options
	bool drawCtrlPts;
	bool drawCurves;
	bool drawImage;
private:
	bool cv_closed;
	int cv_op_mode;// draw edit view
	GLint curve_degree;
	GLint curve_degree_loc;
	GLint curve_seg;
	GLint curve_seg_loc;

	QVector3D* current_point;
	Bezier* current_curve;
	friend class MainWindow;
};

#endif // __OGLVIEWER__