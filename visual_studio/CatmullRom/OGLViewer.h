#pragma once
#ifndef __OGLVIEWER__
#define __OGLVIEWER__

#include "GL/glew.h"
#include "common.h"
#include <QOpenGLWidget>
#include <QKeyEvent>
#include <QTimer>
#include <QTime>
#include <QVector2D>
#include "OpenGL_Utils/GLSLProgram.h"
#include "Math/MathUtil.h"
//#include "Math/Matrix4x4.h"


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
	void setSegment(int val = 20);
	void setDispCtrlPts(bool mode);
	void setDispCurves(bool mode);
protected:
	void initializeGL() Q_DECL_OVERRIDE;
	void paintGL() Q_DECL_OVERRIDE;
	void resizeGL(int w, int h) Q_DECL_OVERRIDE;

	void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
	void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
	void mouseMoveEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
private:
	void addCurvePatch();
	void updateBufferData();
public:
	vector<QVector2D> ctrl_pts;
	vector<GLuint> ctrl_idx;
protected:
private:
	int lastMousePos[2];
	GLfloat viewScale;
	// Display Options
	bool drawCtrlPts;
	bool drawCurves;
private:
	int cv_op_mode;// draw edit view
	GLint curve_degree;
	GLint curve_seg;

	GLuint pts_vbo, pts_vao, curve_ibo, curve_vao;

	QVector2D* curPoint;
	unique_ptr<GLSLProgram> point_shader;// OpenGL shader program
	unique_ptr<GLSLProgram> curve_shader;

	friend class MainWindow;
};

#endif // __OGLVIEWER__