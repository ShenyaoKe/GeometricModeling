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
#include "hds_mesh.h"
#include "HairMesh.h"


static Matrix4D matrix;// Transform matrix

static GLfloat model_mat[16];// Uniform matrix buffer
static GLfloat view_mat[16];
static GLfloat proj_mat[16];
static int sel_id_loc;
//////////////////////////////////////////////////////////////////////////
// Acceleration
//////////////////////////////////////////////////////////////////////////
/*
static vector<Shape*> triangleList;
static KdTreeAccel *mytree;
static ImageData* img;*/

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
	GLuint bindCharacter();
	GLuint bindHairMesh();

	void renderUidBuffer();
	void saveFrameBuffer();

	void generateHairCage();
public:
	double process_fps;
protected:
	perspCamera *view_cam;
	vector<Shape*> triangleList;
	KdTreeAccel *mytree;
	GLint m_Select;
private:// OpenGL variables
	friend class MainWindow;
	int fps;
	int tcount;

	QTime process_time;
	int m_lastMousePos[2];
	int m_selectMode;
	int display_mode = 0;
private://Scene data
	HDS_Mesh* charMesh;
	vector<GLfloat> char_verts;
	vector<GLuint> char_idxs;
	GLuint char_pts_vbo;
	GLuint char_elb;
	GLuint char_vao;

	HairMesh* hairMesh;
	vector<GLfloat> hmsh_verts;
	vector<GLuint> hmsh_idxs;
	vector<GLuint> hmsh_offset;
	GLuint hmsh_pts_vbo;
	GLuint hmsh_elb;
	GLuint hmsh_vao;
	int curLayerID = 0;
	int curStrokeID = 0;

	// Bind VAO

	vector<GLuint> vao_handles;

	GLSLProgram* shader_obj;// Character Shader
	GLSLProgram* shader_hairmesh;// Hair Mesh Shader
	GLSLProgram* shader_uid;// Picking Shader

};

#endif // __OGLVIEWER__