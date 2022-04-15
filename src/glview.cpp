#define NOMINMAX
#include "glview.hpp"
#include <QTimer>
#include <iostream>

// Constructor
GLView::GLView(QWidget* parent) :
	QOpenGLWidget(parent),
	activeLight(0) {

	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	setFocusPolicy(Qt::ClickFocus);

	// Uncomment these lines to enable continuous redrawing
	//QTimer* timer = new QTimer(this);
	//connect(timer, &QTimer::timeout, this, QOverload<>::of(&QOpenGLWidget::update));
	//timer->start(0);
}

// Destructor
GLView::~GLView() {
	makeCurrent();		// Make sure OpenGL context is active
}

GLState& GLView::getGLState() {
	if (!glState.isInit())
		throw std::runtime_error(
			"Tried to access GLState before context was initialized");
	// Make sure OpenGL context is active before accessing state
	makeCurrent();
	return glState;
}

// Set the light that will be affected by rotations
void GLView::setActiveLight(unsigned int light) {
	if (light >= glState.getNumLights()) return;
	getGLState().getLight(activeLight).endRotate();
	activeLight = light;
	emit activeLightChanged(activeLight);
}

// Read configuration file
void GLView::readConfigFile(std::string configFilename) {
	setActiveLight(0);
	getGLState().readConfig(configFilename);
	emit configChanged();
}

// Mouse button clicked
void GLView::mousePressEvent(QMouseEvent* e) {
	if (e->button() == Qt::LeftButton)
		// Rotate light if holding shift
		if (e->modifiers() & Qt::ShiftModifier) {
			float scale = glm::min(width(), height());
			getGLState().getLight(activeLight).beginRotate(
				glm::vec2(e->x() / scale, e->y() / scale));

		// Rotate camera if not holding shift
		} else
			getGLState().beginCameraRotate(glm::vec2(e->x(), e->y()));
	else
		e->ignore();
}

// Mouse button released
void GLView::mouseReleaseEvent(QMouseEvent* e) {
	// Stop rotating camera or light
	if (e->button() == Qt::LeftButton) {
		getGLState().endCameraRotate();
		getGLState().getLight(activeLight).endRotate();
	} else
		e->ignore();
}

// Mouse cursor moved
void GLView::mouseMoveEvent(QMouseEvent* e) {
	GLState& state = getGLState();

	// Perform camera rotation
	if (state.isCamRotating()) {
		state.rotateCamera(glm::vec2(e->x(), e->y()));
		update();

	// Perform light rotation
	} else if (state.getLight(activeLight).isRotating()) {
		float scale = glm::min(width(), height());
		state.getLight(activeLight).rotateLight(
			glm::vec2(e->x() / scale, e->y() / scale));
		emit lightPosChanged(activeLight);		// Notify App of position change
		update();

	} else
		e->ignore();
}

// Mouse wheel scrolled
void GLView::wheelEvent(QWheelEvent* e) {
	float offset = e->angleDelta().y() / (8 * 15) * -0.1f;

	// Offset light if holding shift
	if (e->modifiers() & Qt::ShiftModifier) {
		getGLState().getLight(activeLight).offsetLight(offset);
		emit lightPosChanged(activeLight);
		update();

	// Offset camera if not holding shift
	} else {
		getGLState().offsetCamera(offset);
		update();
	}
}

// Key press event
void GLView::keyPressEvent(QKeyEvent* e) {
	makeCurrent();
	switch (e->key()) {
	// Toggle normals type (flat vs. smooth)
	case Qt::Key_N: {
		GLState::NormalMode nm = glState.getNormalMode();
		if (nm == GLState::NORMALMODE_FACE) {
			glState.setNormalMode(GLState::NORMALMODE_SMOOTH);
			std::cout << "Showing smoothed normals" << std::endl;
		} else if (nm == GLState::NORMALMODE_SMOOTH) {
			glState.setNormalMode(GLState::NORMALMODE_FACE);
			std::cout << "Showing flat normals" << std::endl;
		}
		emit normalModeChanged();
		update();
		break; }
	// Toggle shading mode (normals vs. Phong)
	case Qt::Key_L: {
		GLState::ShadingMode sm = glState.getShadingMode();
		if (e->modifiers() & Qt::ShiftModifier) {
			if (sm == GLState::SHADINGMODE_NORMALS) {
				glState.setShadingMode(GLState::SHADINGMODE_GOURAUD);
				std::cout << "Showing Gouraud shading" << std::endl;
			} else if (sm == GLState::SHADINGMODE_PHONG) {
				glState.setShadingMode(GLState::SHADINGMODE_NORMALS);
				std::cout << "Showing normals as colors" << std::endl;
			} else if (sm == GLState::SHADINGMODE_GOURAUD) {
				glState.setShadingMode(GLState::SHADINGMODE_PHONG);
				std::cout << "Showing Phong shading & illumination" << std::endl;
			}
		} else {
			if (sm == GLState::SHADINGMODE_NORMALS) {
				glState.setShadingMode(GLState::SHADINGMODE_PHONG);
				std::cout << "Showing Phong shading & illumination" << std::endl;
			} else if (sm == GLState::SHADINGMODE_PHONG) {
				glState.setShadingMode(GLState::SHADINGMODE_GOURAUD);
				std::cout << "Showing Gouraud shading" << std::endl;
			} else if (sm == GLState::SHADINGMODE_GOURAUD) {
				glState.setShadingMode(GLState::SHADINGMODE_NORMALS);
				std::cout << "Showing normals as colors" << std::endl;
			}
		}
		emit shadingModeChanged();
		update();
		break; }
	// Increase / decrease ambient strength
	case Qt::Key_A: {
		float ambStr = glState.getAmbientStrength();
		if (e->modifiers() & Qt::ShiftModifier) {
			ambStr = glm::min(1.0f, ambStr + 0.02f);
		} else {
			ambStr = glm::max(0.0f, ambStr - 0.02f);
		}
		glState.setAmbientStrength(ambStr);
		std::cout << "Set ambient strength to " << ambStr << std::endl;
		emit ambientStrengthChanged(ambStr);
		update();
		break; }
	// Increase / decrease diffuse strength
	case Qt::Key_D: {
		float diffStr = glState.getDiffuseStrength();
		if (e->modifiers() & Qt::ShiftModifier) {
			diffStr = glm::min(1.0f, diffStr + 0.1f);
		} else {
			diffStr = glm::max(0.0f, diffStr - 0.1f);
		}
		glState.setDiffuseStrength(diffStr);
		std::cout << "Set diffuse strength to " << diffStr << std::endl;
		emit diffuseStrengthChanged(diffStr);
		update();
		break; }
	// Increase / decrease specular strength
	case Qt::Key_S: {
		float specStr = glState.getSpecularStrength();
		if (e->modifiers() & Qt::ShiftModifier) {
			specStr = glm::min(1.0f, specStr + 0.1f);
		} else {
			specStr = glm::max(0.0f, specStr - 0.1f);
		}
		glState.setSpecularStrength(specStr);
		std::cout << "Set specular strength to " << specStr << std::endl;
		emit specularStrengthChanged(specStr);
		update();
		break; }
	// Increase / decrease specular exponent
	case Qt::Key_X: {
		float specExp = glState.getSpecularExponent();
		if (e->modifiers() & Qt::ShiftModifier) {
			specExp = glm::min(1024.0f, specExp * 2.0f);
		} else {
			specExp = glm::max(1.0f, specExp / 2.0f);
		}
		glState.setSpecularExponent(specExp);
		std::cout << "Set specular exponent to " << specExp << std::endl;
		emit specularExponentChanged(specExp);
		update();
		break; }
	// Switch active light source
	case Qt::Key_1:
		setActiveLight(0);
		std::cout << "Active light: " << activeLight+1 << std::endl;
		break;
	case Qt::Key_2:
		setActiveLight(1);
		std::cout << "Active light: " << activeLight+1 << std::endl;
		break;
	case Qt::Key_3:
		setActiveLight(2);
		std::cout << "Active light: " << activeLight+1 << std::endl;
		break;
	case Qt::Key_4:
		setActiveLight(3);
		std::cout << "Active light: " << activeLight+1 << std::endl;
		break;
	case Qt::Key_5:
		setActiveLight(4);
		std::cout << "Active light: " << activeLight+1 << std::endl;
		break;
	case Qt::Key_6:
		setActiveLight(5);
		std::cout << "Active light: " << activeLight+1 << std::endl;
		break;
	case Qt::Key_7:
		setActiveLight(6);
		std::cout << "Active light: " << activeLight+1 << std::endl;
		break;
	case Qt::Key_8:
		setActiveLight(7);
		std::cout << "Active light: " << activeLight+1 << std::endl;
		break;
	// Enable / disable active light
	case Qt::Key_E: {
		bool enabled = glState.getLight(activeLight).getEnabled();
		enabled = !enabled;
		glState.getLight(activeLight).setEnabled(enabled);
		std::cout << (enabled ? "Enabled" : "Disabled") << " light " << activeLight+1 << std::endl;
		emit lightEnabledDisabled(activeLight);
		update();
		break; }
	// Toggle active light type
	case Qt::Key_T: {
		Light::LightType type = glState.getLight(activeLight).getType();
		if (type == Light::POINT) {
			glState.getLight(activeLight).setType(Light::DIRECTIONAL);
			std::cout << "Set light " << activeLight+1 << " to directional light" << std::endl;
		} else if (type == Light::DIRECTIONAL) {
			glState.getLight(activeLight).setType(Light::POINT);
			std::cout << "Set light " << activeLight+1 << " to point light" << std::endl;
		}
		emit lightTypeChanged(activeLight);
		update();
		break; }
	default:
		QOpenGLWidget::keyPressEvent(e);
		break;
	}
}

// Called when OpenGL context is created
void GLView::initializeGL() {
	glState.initializeGL();		// Initialize the state

	std::cout << "Mouse controls:" << std::endl;
	std::cout << "  Left click + drag to rotate camera" << std::endl;
	std::cout << "  Scroll wheel to zoom in/out" << std::endl;
	std::cout << "  SHIFT + left click + drag to rotate active light source" << std::endl;
	std::cout << "  SHIFT + scroll wheel to change active light distance" << std::endl;
	std::cout << "Keyboard controls:" << std::endl;
	std::cout << "  1-8:  Change active light source" << std::endl;
	std::cout << "  e:    Enable/disable active light source" << std::endl;
	std::cout << "  t:    Toggle active light type (point vs. directional)" << std::endl;
	std::cout << "  a,A:  Decrease/increase ambient strength" << std::endl;
	std::cout << "  d,D:  Decrease/increase diffuse strength" << std::endl;
	std::cout << "  s,S:  Decrease/increase specular strength" << std::endl;
	std::cout << "  x,X:  Decrease/increase specular exponent" << std::endl;
	std::cout << "  n:    Toggle normals type (flat vs. smooth)" << std::endl;
	std::cout << "  l,L:  Toggle shading type (Phong vs. Gouraud vs. colored normals)" << std::endl;
	std::cout << std::endl;
	std::cout << "Active light: " << activeLight+1 << std::endl;

	// Tell App that OpenGL has been setup
	emit initialized();
}

// Called when the screen is redrawn
void GLView::paintGL() {
	glState.paintGL();
}

// Called when the screen is resized
void GLView::resizeGL(int w, int h) {
	glState.resizeGL(w, h);
}
