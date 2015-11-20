#pragma once
#ifndef __OGLVIEWER__
#define __OGLVIEWER__

#include "GL/glew.h"

#include <QOpenGLWidget>
#include <QKeyEvent>
#include <QTimer>
#include <QTime>
#include <QString>
#include <QFileDialog>
#include <QOpenGLFramebufferObject>
//#include <QOpenGLVertexArrayObject>
//#include <QGLFunctions>

#include "OpenGL_Utils/GLSLProgram.h"
#include "Math/MathUtil.h"
#include "Image/ImageData.h"
#include "Geometry/Mesh.h"
//#include "Math/Matrix4D.h"
#include "Camera/Camera.h"
#include "Accel/KdTreeAccel.h"

static Mesh *model_mesh;
static GLfloat* model_verts;// vertices vbo
static GLfloat* model_uvs;// Texture coordinates vbo
static GLfloat* model_norms;// Normal coordinates vbo
static GLint* model_uids;
static int model_vbo_size;// Triangle face numbers

static Mesh *box_mesh;// Display object
static GLfloat* box_verts;// vertices vbo
static GLfloat* box_uvs;// Texture coordinates vbo
static GLfloat* box_norms;// Normal coordinates vbo
static GLint* box_idxs;
static int box_vbo_size;// Triangle face numbers
static GLSLProgram* shader;// OpenGL shader program
static GLSLProgram* shader_uid;// OpenGL shader program
static Matrix4D matrix;// Transform matrix

static int model_mat_loc;// Uniform matrix location
static GLfloat model_mat[16];// Uniform matrix buffer
static int view_mat_loc;// Uniform matrix location
static GLfloat view_mat[16];
static int proj_mat_loc;// Porjection matrix location
static GLfloat proj_mat[16];
static int sel_id_loc;
//////////////////////////////////////////////////////////////////////////
// Acceleration
//////////////////////////////////////////////////////////////////////////
static vector<Shape*> triangleList;
static KdTreeAccel *mytree;
static ImageData* img;
//////////////////////////////////////////////////////////////////////////
// Sphere Attribute
//////////////////////////////////////////////////////////////////////////

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
	//OGLViewer();
	OGLViewer(QWidget *parent = nullptr);
	~OGLViewer();

	//void update();
public slots:
	void resetCamera();
	void initParas();
protected:
	void initializeGL() Q_DECL_OVERRIDE;
	void paintGL() Q_DECL_OVERRIDE;
	void resizeGL(int w, int h) Q_DECL_OVERRIDE;
	void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;

	void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
	void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
	void mouseReleaseEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
	void mouseMoveEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
private:
	void bindBox();
	void bindMesh();
	void renderUidBuffer();
	void saveFrameBuffer();
public:
	double process_fps;
protected:
	perspCamera *view_cam;
	vector<Shape*> triangleList;
	KdTreeAccel *mytree;
	GLint m_Select;
private:
	int fps;
	int tcount;

	QTime process_time;
	int m_lastMousePos[2];
	int m_selectMode;
private: // OpenGL variables
	int display_mode = 0;
	vector<GLuint> vao_handles;

	friend class MainWindow;
};

#endif // __OGLVIEWER__