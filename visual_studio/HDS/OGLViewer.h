#pragma once
#ifndef __OGLVIEWER__
#define __OGLVIEWER__

#include "GL/glew.h"
#include "common.h"

#include <QOpenGLWidget>
#include <QKeyEvent>
#include <QTimer>
#include <QTime>
#include <QString>
#include <QFileDialog>
#include <QMessageBox>
#include <QOpenGLVertexArrayObject>
//#include <QGLFunctions>

#include "OpenGL_Utils/GLSLProgram.h"
#include "Math/MathUtil.h"
#include "Math/Matrix4x4.h"
#include "Image/ImageData.h"
#include "Geometry/TriangleMesh.h"
#include "Camera/perspCamera.h"
#include "hds_mesh.h"

static GLSLProgram* triangle_shader;// OpenGL shader program
//static Matrix4D matrix;// Transform matrix

static GLfloat model_mat[16];// Uniform matrix buffer
static GLfloat view_mat[16];
static GLfloat proj_mat[16];

const double eps = 5e-4;


// OpenGL Window in Qt
class OGLViewer : public QOpenGLWidget
{
	Q_OBJECT
public:
	enum Select_Mode
	{
		OBJECT_SELECT,
		COMPONENT_SELECT,
		FACE_COMPONENT_SELECT
	};
	OGLViewer(QWidget *parent = nullptr);
	~OGLViewer();

	//void update();
	public slots:
	void resetCamera();
	void initParas();
	/*void loadOBJ();
	void saveOBJ();*/
signals:
	void levelChanged(int level);
protected:
	void initializeGL() Q_DECL_OVERRIDE;
	void paintGL() Q_DECL_OVERRIDE;
	void resizeGL(int w, int h) Q_DECL_OVERRIDE;

	void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
	void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
	void mouseReleaseEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
	void mouseMoveEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
private:
	void bindMesh();
	void saveFrameBuffer();
protected:
	perspCamera *view_cam;
private:
	int fps;
	int tcount;

	QTime process_time;
	int m_lastMousePos[2];
	int m_selectMode;
private: // OpenGL variables
	int display_mode = 0;
	int subd_lv;
	int offset = 0;
	bool showPiece = false;
	bool drawPoint = false;
	bool drawWireFrame = false;

	MeshLoader loader;

	vector<GLfloat> mesh_verts;
	vector<GLuint> mesh_idxs;

	vector<GLuint> vao_handles;
	GLuint mesh_vbo;
	GLuint mesh_ibo;
	GLuint mesh_vao;

	friend class MainWindow;
};

#endif // __OGLVIEWER__