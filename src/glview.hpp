#ifndef GLVIEW_HPP
#define GLVIEW_HPP

#include "glstate.hpp"
#include <QOpenGLWidget>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>

// OpenGL widget: creates a context and window to draw on
class GLView : public QOpenGLWidget {
	Q_OBJECT
public:
	GLView(QWidget* parent = NULL);
	~GLView();

	QSize sizeHint() const { return QSize(800, 600); }

	GLState& getGLState();

	// Set the active light
	void setActiveLight(unsigned int light);
	void readConfigFile(std::string configFilename);

signals:
	void initialized();
	void configChanged();
	void normalModeChanged();
	void shadingModeChanged();
	void ambientStrengthChanged(float ambStr);
	void diffuseStrengthChanged(float diffStr);
	void specularStrengthChanged(float specStr);
	void specularExponentChanged(float specExp);
	void activeLightChanged(unsigned int light);
	void lightEnabledDisabled(unsigned int light);
	void lightTypeChanged(unsigned int light);
	void lightPosChanged(unsigned int light);

protected:
	// All OpenGL logic happens in this object (see glstate.hpp/cpp)
	GLState glState;

	unsigned int activeLight;	// Which light is affected by rotations

	// Event callbacks
	void mousePressEvent(QMouseEvent* e);
	void mouseReleaseEvent(QMouseEvent* e);
	void mouseMoveEvent(QMouseEvent* e);
	void wheelEvent(QWheelEvent* e);
	void keyPressEvent(QKeyEvent* e);

	// OpenGL callbacks
	void initializeGL();
	void paintGL();
	void resizeGL(int w, int h);
};

#endif
