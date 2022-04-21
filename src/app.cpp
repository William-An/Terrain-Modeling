#include <iostream>
#include <QApplication>
#include <QBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QButtonGroup>
#include <QScrollArea>
#include <QScrollBar>
#include <QDir>
#include <QSurfaceFormat>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "app.hpp"
#include "terrain.hpp"

// Constructor
App::App(std::string configFile, QWidget* parent) : QWidget(parent) {
	setWindowTitle(QString("Procedural Modeling Terrain"));
	dist = std::uniform_int_distribution<int>(0, INT_MAX);
	rd   = std::mt19937();
	initLayout(configFile);
}

// Respond to keyboard events
void App::keyReleaseEvent(QKeyEvent* e) {
	// Quit when user presses and releases ESC
	switch (e->key()) {
	case Qt::Key_Escape:
		QApplication::quit();
		break;
	default:
		QWidget::keyReleaseEvent(e);
		break;
	}
}

// Setup window layouts and child widgets
void App::initLayout(std::string configFile) {
	// Top-level horizontal layout
	QHBoxLayout* topLayout = new QHBoxLayout;
	topLayout->setContentsMargins(0, 0, 0, 0);
	setLayout(topLayout);

	// Surfaces layout
	surfacesLayout = new QVBoxLayout;

	// Lights layout
	QVBoxLayout* lightLayout = new QVBoxLayout;
	lightLayout->setContentsMargins(10, 10, 10, 10);

	// Control panel widget
	QWidget* controlWidget = new QWidget(this);
	QVBoxLayout* controlLayout = new QVBoxLayout;
	controlLayout->setContentsMargins(10, 10, 10, 10);
	controlWidget->setLayout(controlLayout);

	// General control widget group
	// TODO add callback
	QGroupBox* generalControl = new QGroupBox("General Control", this);
	QGridLayout* generalLayout = new QGridLayout;
	generalControl->setLayout(generalLayout);
	controlLayout->addWidget(generalControl);

	// Random seed selector
	QLabel* seedLbl = new QLabel("RNG seed:", this);
	randomSeedSpin = new QSpinBox(this);
	randomSeedSpin->setRange(0, 2147483647);
	generalLayout->addWidget(seedLbl, 0, 0);
	generalLayout->addWidget(randomSeedSpin, 0, 1);

	// Terrain name
	QLabel* terrainNameLbl = new QLabel("Terrain Name:", this);
	terrainName = new QLineEdit(this);
	terrainName->setText(QString("Terrain"));
	generalLayout->addWidget(terrainNameLbl, 1, 0);
	generalLayout->addWidget(terrainName, 1, 1);

	// Terrain size control
	QHBoxLayout* terrainSizeControlLayout = new QHBoxLayout;
	terrainWidth = new QSpinBox(this);
	terrainLength = new QSpinBox(this);
	terrainWidth->setRange(0, 4096);
	terrainWidth->setValue(20);
	terrainLength->setRange(0, 4096);
	terrainLength->setValue(20);
	terrainSizeControlLayout->addWidget(new QLabel("Width:"));
	terrainSizeControlLayout->addWidget(terrainWidth);
	terrainSizeControlLayout->addWidget(new QLabel("Length:"));
	terrainSizeControlLayout->addWidget(terrainLength);
	generalLayout->addLayout(terrainSizeControlLayout, 2, 0, 1, 2);

	// Randomized seed and generate terrain button
	QHBoxLayout* terrainGenerationLayout = new QHBoxLayout;
	randomizedBtn = new QPushButton("Random", this);
	generateTerrainBtn = new QPushButton("Generate", this);
	terrainGenerationLayout->addWidget(randomizedBtn);
	terrainGenerationLayout->addWidget(generateTerrainBtn);
	generalLayout->addLayout(terrainGenerationLayout, 3, 0, 1, 2);

	// Surface control buttons
	QHBoxLayout* surfaceControlLayout = new QHBoxLayout;
	addSurfaceBtn = new QPushButton("Add a surface", this);
	clearSurfaceBtn = new QPushButton("Clear all surfaces", this);
	surfaceControlLayout->addWidget(addSurfaceBtn);
	surfaceControlLayout->addWidget(clearSurfaceBtn);
	generalLayout->addLayout(surfaceControlLayout, 4, 0, 1, 2);

	// Load and dump config buttons
	QHBoxLayout* configLoadSaveControlLayout = new QHBoxLayout;
	loadTerrainConfigBtn = new QPushButton("Load", this);
	dumpTerrainConfigBtn = new QPushButton("Save", this);
	configLoadSaveControlLayout->addWidget(loadTerrainConfigBtn);
	configLoadSaveControlLayout->addWidget(dumpTerrainConfigBtn);
	generalLayout->addLayout(configLoadSaveControlLayout, 5, 0, 1, 2);

	// Normal mode selection
	QHBoxLayout* normalsLayout = new QHBoxLayout;
	QLabel* normalsLbl = new QLabel("Normals:", this);
	normalsLayout->addWidget(normalsLbl);
	QButtonGroup* normalsGroup = new QButtonGroup(this);
	smoothNormalsRadio = new QRadioButton("Smooth", this);
	normalsLayout->addWidget(smoothNormalsRadio);
	normalsGroup->addButton(smoothNormalsRadio);
	faceNormalsRadio = new QRadioButton("Flat", this);
	normalsLayout->addWidget(faceNormalsRadio);
	normalsGroup->addButton(faceNormalsRadio);
	generalLayout->addLayout(normalsLayout, 6, 0, 1, 2);

	// Shading mode selection
	QHBoxLayout* shadingLayout = new QHBoxLayout;
	QLabel* shadingLbl = new QLabel("Shading:", this);
	shadingLayout->addWidget(shadingLbl);
	QButtonGroup* shadingGroup = new QButtonGroup(this);
	phongShadingRadio = new QRadioButton("Phong", this);
	shadingLayout->addWidget(phongShadingRadio);
	shadingGroup->addButton(phongShadingRadio);
	normalsShadingRadio = new QRadioButton("Normals", this);
	shadingLayout->addWidget(normalsShadingRadio);
	shadingGroup->addButton(normalsShadingRadio);
	generalLayout->addLayout(shadingLayout, 7, 0, 1, 2);
	// End of general control

	// Material properties
	QGroupBox* materialGroup = new QGroupBox("Material properties", this);
	// controlLayout->addWidget(materialGroup);
	QGridLayout* materialLayout = new QGridLayout;
	materialGroup->setLayout(materialLayout);
	// Ambient strength
	QLabel* ambStrLbl = new QLabel("Ambient", this);
	materialLayout->addWidget(ambStrLbl, 0, 0);
	ambStrSlider = new QSlider(Qt::Horizontal, this);
	ambStrSlider->setRange(0, 50);
	materialLayout->addWidget(ambStrSlider, 0, 1);
	QLabel* ambStrValLbl = new QLabel("0.05", this);
	materialLayout->addWidget(ambStrValLbl, 0, 2);
	// Diffuse strength
	QLabel* diffStrLbl = new QLabel("Diffuse", this);
	materialLayout->addWidget(diffStrLbl, 1, 0);
	diffStrSlider = new QSlider(Qt::Horizontal, this);
	diffStrSlider->setRange(0, 100);
	materialLayout->addWidget(diffStrSlider, 1, 1);
	QLabel* diffStrValLbl = new QLabel("1.00", this);
	materialLayout->addWidget(diffStrValLbl, 1, 2);
	// Specular strength
	QLabel* specStrLbl = new QLabel("Specular", this);
	materialLayout->addWidget(specStrLbl, 2, 0);
	specStrSlider = new QSlider(Qt::Horizontal, this);
	specStrSlider->setRange(0, 100);
	materialLayout->addWidget(specStrSlider, 2, 1);
	QLabel* specStrValLbl = new QLabel("0.50", this);
	materialLayout->addWidget(specStrValLbl, 2, 2);
	// Specular exponent
	QLabel* specExpLbl = new QLabel("Exponent", this);
	materialLayout->addWidget(specExpLbl, 3, 0);
	specExpSlider = new QSlider(Qt::Horizontal, this);
	specExpSlider->setRange(0, 20);
	materialLayout->addWidget(specExpSlider, 3, 1);
	QLabel* specExpValLbl = new QLabel("9999.99", this);
	materialLayout->addWidget(specExpValLbl, 3, 2);
	materialLayout->setColumnMinimumWidth(2, specExpValLbl->sizeHint().width());
	// Object color
	QHBoxLayout* objColorLayout = new QHBoxLayout;
	materialLayout->addLayout(objColorLayout, 4, 0, 1, 3);
	QLabel* objColorLbl = new QLabel("Color (RGB):", this);
	objColorLayout->addWidget(objColorLbl);
	objColorRSpin = new QSpinBox(this);
	objColorRSpin->setRange(0, 255);
	objColorLayout->addWidget(objColorRSpin);
	objColorGSpin = new QSpinBox(this);
	objColorGSpin->setRange(0, 255);
	objColorLayout->addWidget(objColorGSpin);
	objColorBSpin = new QSpinBox(this);
	objColorBSpin->setRange(0, 255);
	objColorLayout->addWidget(objColorBSpin);

	// Surfaces Layout
	// Add an init surface
	surfaces = new std::vector<App::SurfaceWidgetGroup*>();
	addLayer();

	controlLayout->addLayout(surfacesLayout);
	controlLayout->addStretch();

	// Control layout done

	// Scrollable area
	scrollArea = new QScrollArea(this);
	scrollArea->setWidgetResizable(true);
	scrollArea->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
	scrollArea->setWidget(controlWidget);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scrollArea->setMinimumWidth(controlLayout->sizeHint().width() +
		scrollArea->verticalScrollBar()->sizeHint().width());
	topLayout->addWidget(scrollArea);

	// Set OpenGL context format
	QSurfaceFormat format;
	format.setVersion(3, 3);
	format.setProfile(QSurfaceFormat::CoreProfile);
	format.setDepthBufferSize(24);
	format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
	QSurfaceFormat::setDefaultFormat(format);

	// OpenGL viewing window
	glView = new GLView(this);
	topLayout->addWidget(glView);

	// Put Lights here
	QButtonGroup* lightPosGroup = new QButtonGroup(this);
	// Light 1 controls
	QGroupBox* light1Group = new QGroupBox("Light 1", this);
	lightLayout->addWidget(light1Group);
	QVBoxLayout* light1Layout = new QVBoxLayout;
	light1Group->setLayout(light1Layout);
	QHBoxLayout* light1TypeLayout = new QHBoxLayout;
	light1Layout->addLayout(light1TypeLayout);
	light1EnabledCB = new QCheckBox("Enabled", this);
	light1TypeLayout->addWidget(light1EnabledCB);
	light1PointRadio = new QRadioButton("Point", this);
	light1TypeLayout->addWidget(light1PointRadio);
	light1DirRadio = new QRadioButton("Directional", this);
	light1TypeLayout->addWidget(light1DirRadio);
	QGridLayout* light1ColorPosLayout = new QGridLayout;
	light1Layout->addLayout(light1ColorPosLayout);
	QLabel* light1ColorLbl = new QLabel("Color (RGB):", this);
	light1ColorPosLayout->addWidget(light1ColorLbl, 0, 0);
	light1RSpin = new QSpinBox(this);
	light1RSpin->setRange(0, 255);
	light1ColorPosLayout->addWidget(light1RSpin, 0, 1);
	light1GSpin = new QSpinBox(this);
	light1GSpin->setRange(0, 255);
	light1ColorPosLayout->addWidget(light1GSpin, 0, 2);
	light1BSpin = new QSpinBox(this);
	light1BSpin->setRange(0, 255);
	light1ColorPosLayout->addWidget(light1BSpin, 0, 3);
	light1PosRadio = new QRadioButton("Position:", this);
	lightPosGroup->addButton(light1PosRadio);
	light1PosRadio->setChecked(true);
	light1ColorPosLayout->addWidget(light1PosRadio, 1, 0);
	QLabel* light1XLbl = new QLabel("0.0", this);
	light1ColorPosLayout->addWidget(light1XLbl, 1, 1);
	QLabel* light1YLbl = new QLabel("0.0", this);
	light1ColorPosLayout->addWidget(light1YLbl, 1, 2);
	QLabel* light1ZLbl = new QLabel("0.0", this);
	light1ColorPosLayout->addWidget(light1ZLbl, 1, 3);

	// Light 2 controls
	QGroupBox* light2Group = new QGroupBox("Light 2", this);
	lightLayout->addWidget(light2Group);
	QVBoxLayout* light2Layout = new QVBoxLayout;
	light2Group->setLayout(light2Layout);
	QHBoxLayout* light2TypeLayout = new QHBoxLayout;
	light2Layout->addLayout(light2TypeLayout);
	light2EnabledCB = new QCheckBox("Enabled", this);
	light2TypeLayout->addWidget(light2EnabledCB);
	light2PointRadio = new QRadioButton("Point", this);
	light2TypeLayout->addWidget(light2PointRadio);
	light2DirRadio = new QRadioButton("Directional", this);
	light2TypeLayout->addWidget(light2DirRadio);
	QGridLayout* light2ColorPosLayout = new QGridLayout;
	light2Layout->addLayout(light2ColorPosLayout);
	QLabel* light2ColorLbl = new QLabel("Color (RGB):", this);
	light2ColorPosLayout->addWidget(light2ColorLbl, 0, 0);
	light2RSpin = new QSpinBox(this);
	light2RSpin->setRange(0, 255);
	light2ColorPosLayout->addWidget(light2RSpin, 0, 1);
	light2GSpin = new QSpinBox(this);
	light2GSpin->setRange(0, 255);
	light2ColorPosLayout->addWidget(light2GSpin, 0, 2);
	light2BSpin = new QSpinBox(this);
	light2BSpin->setRange(0, 255);
	light2ColorPosLayout->addWidget(light2BSpin, 0, 3);
	light2PosRadio = new QRadioButton("Position:", this);
	lightPosGroup->addButton(light2PosRadio);
	light2ColorPosLayout->addWidget(light2PosRadio, 1, 0);
	QLabel* light2XLbl = new QLabel("0.0", this);
	light2ColorPosLayout->addWidget(light2XLbl, 1, 1);
	QLabel* light2YLbl = new QLabel("0.0", this);
	light2ColorPosLayout->addWidget(light2YLbl, 1, 2);
	QLabel* light2ZLbl = new QLabel("0.0", this);
	light2ColorPosLayout->addWidget(light2ZLbl, 1, 3);

	// Light 3 controls
	QGroupBox* light3Group = new QGroupBox("Light 3", this);
	lightLayout->addWidget(light3Group);
	QVBoxLayout* light3Layout = new QVBoxLayout;
	light3Group->setLayout(light3Layout);
	QHBoxLayout* light3TypeLayout = new QHBoxLayout;
	light3Layout->addLayout(light3TypeLayout);
	light3EnabledCB = new QCheckBox("Enabled", this);
	light3TypeLayout->addWidget(light3EnabledCB);
	light3PointRadio = new QRadioButton("Point", this);
	light3TypeLayout->addWidget(light3PointRadio);
	light3DirRadio = new QRadioButton("Directional", this);
	light3TypeLayout->addWidget(light3DirRadio);
	QGridLayout* light3ColorPosLayout = new QGridLayout;
	light3Layout->addLayout(light3ColorPosLayout);
	QLabel* light3ColorLbl = new QLabel("Color (RGB):", this);
	light3ColorPosLayout->addWidget(light3ColorLbl, 0, 0);
	light3RSpin = new QSpinBox(this);
	light3RSpin->setRange(0, 255);
	light3ColorPosLayout->addWidget(light3RSpin, 0, 1);
	light3GSpin = new QSpinBox(this);
	light3GSpin->setRange(0, 255);
	light3ColorPosLayout->addWidget(light3GSpin, 0, 2);
	light3BSpin = new QSpinBox(this);
	light3BSpin->setRange(0, 255);
	light3ColorPosLayout->addWidget(light3BSpin, 0, 3);
	light3PosRadio = new QRadioButton("Position:", this);
	lightPosGroup->addButton(light3PosRadio);
	light3ColorPosLayout->addWidget(light3PosRadio, 1, 0);
	QLabel* light3XLbl = new QLabel("0.0", this);
	light3ColorPosLayout->addWidget(light3XLbl, 1, 1);
	QLabel* light3YLbl = new QLabel("0.0", this);
	light3ColorPosLayout->addWidget(light3YLbl, 1, 2);
	QLabel* light3ZLbl = new QLabel("0.0", this);
	light3ColorPosLayout->addWidget(light3ZLbl, 1, 3);

	// Light 4 controls
	QGroupBox* light4Group = new QGroupBox("Light 4", this);
	lightLayout->addWidget(light4Group);
	QVBoxLayout* light4Layout = new QVBoxLayout;
	light4Group->setLayout(light4Layout);
	QHBoxLayout* light4TypeLayout = new QHBoxLayout;
	light4Layout->addLayout(light4TypeLayout);
	light4EnabledCB = new QCheckBox("Enabled", this);
	light4TypeLayout->addWidget(light4EnabledCB);
	light4PointRadio = new QRadioButton("Point", this);
	light4TypeLayout->addWidget(light4PointRadio);
	light4DirRadio = new QRadioButton("Directional", this);
	light4TypeLayout->addWidget(light4DirRadio);
	QGridLayout* light4ColorPosLayout = new QGridLayout;
	light4Layout->addLayout(light4ColorPosLayout);
	QLabel* light4ColorLbl = new QLabel("Color (RGB):", this);
	light4ColorPosLayout->addWidget(light4ColorLbl, 0, 0);
	light4RSpin = new QSpinBox(this);
	light4RSpin->setRange(0, 255);
	light4ColorPosLayout->addWidget(light4RSpin, 0, 1);
	light4GSpin = new QSpinBox(this);
	light4GSpin->setRange(0, 255);
	light4ColorPosLayout->addWidget(light4GSpin, 0, 2);
	light4BSpin = new QSpinBox(this);
	light4BSpin->setRange(0, 255);
	light4ColorPosLayout->addWidget(light4BSpin, 0, 3);
	light4PosRadio = new QRadioButton("Position:", this);
	lightPosGroup->addButton(light4PosRadio);
	light4ColorPosLayout->addWidget(light4PosRadio, 1, 0);
	QLabel* light4XLbl = new QLabel("0.0", this);
	light4ColorPosLayout->addWidget(light4XLbl, 1, 1);
	QLabel* light4YLbl = new QLabel("0.0", this);
	light4ColorPosLayout->addWidget(light4YLbl, 1, 2);
	QLabel* light4ZLbl = new QLabel("0.0", this);
	light4ColorPosLayout->addWidget(light4ZLbl, 1, 3);
	lightLayout->addStretch();

	topLayout->addLayout(lightLayout);

	// Conntect random seed spiner to glstate
	connect(randomSeedSpin, QOverload<int>::of(&QSpinBox::valueChanged), [=] {
		setRandomSeed();});

	// Set name
	connect(terrainName, &QLineEdit::textChanged, [=](const QString &text) {
		glView->getGLState().terrain->setName(text.toStdString()); });

	// Change size
	connect(terrainWidth, QOverload<int>::of(&QSpinBox::valueChanged), [=] {
		setTerrainSize();});
	connect(terrainLength, QOverload<int>::of(&QSpinBox::valueChanged), [=] {
		setTerrainSize();});

	// Random button
	connect(randomizedBtn, &QPushButton::clicked, [=] {
		int rand_seed = dist(rd);
		randomSeedSpin->setValue(rand_seed);
		generateLayers();});

	connect(generateTerrainBtn, &QPushButton::clicked, [=] {
		generateLayers();});

	// Add surface
	connect(addSurfaceBtn, &QPushButton::clicked, [=] {
		addLayer();
		
		// Scroll to the bottom	
		scrollControlToEnd();
	});
	
	// Clear all surfaces
	connect(clearSurfaceBtn, &QPushButton::clicked, [=] {
		clearLayers();});

	// Update normal mode
	connect(faceNormalsRadio, &QRadioButton::clicked, [=](bool checked) {
			if (!checked) return;
			glView->getGLState().setNormalMode(GLState::NORMALMODE_FACE);
			glView->update();
		});
	connect(smoothNormalsRadio, &QRadioButton::clicked, [=](bool checked) {
			if (!checked) return;
			glView->getGLState().setNormalMode(GLState::NORMALMODE_SMOOTH);
			glView->update();
		});

	// Update shading mode
	connect(normalsShadingRadio, &QRadioButton::clicked, [=](bool checked) {
			if (!checked) return;
			glView->getGLState().setShadingMode(GLState::SHADINGMODE_NORMALS);
			glView->update();
		});
	connect(phongShadingRadio, &QRadioButton::clicked, [=](bool checked) {
			if (!checked) return;
			glView->getGLState().setShadingMode(GLState::SHADINGMODE_PHONG);
			glView->update();
		});

	// Light update lambdas
	auto updateLightEnabled = [=](int idx, bool enabled) {
			Light& light = glView->getGLState().getLight(idx);
			light.setEnabled(enabled);
			glView->update();
		};
	auto updateLightType = [=](int idx, Light::LightType type) {
			Light& light = glView->getGLState().getLight(idx);
			light.setType(type);
			glView->update();
		};
	auto updateLightColor = [=](int idx, QSpinBox* r, QSpinBox* g, QSpinBox* b) {
			glm::vec3 color;
			color.r = r->value() / 255.0f;
			color.g = g->value() / 255.0f;
			color.b = b->value() / 255.0f;
			Light& light = glView->getGLState().getLight(idx);
			light.setColor(color);
			glView->update();
		};

	// Update light 1
	connect(light1EnabledCB, &QCheckBox::clicked, [=](bool enabled) {
		updateLightEnabled(0, enabled); });
	connect(light1PointRadio, &QRadioButton::clicked, [=](bool checked) {
		if (!checked) return;
		updateLightType(0, Light::POINT); });
	connect(light1DirRadio, &QRadioButton::clicked, [=](bool checked) {
		if (!checked) return;
		updateLightType(0, Light::DIRECTIONAL); });
	connect(light1RSpin, QOverload<int>::of(&QSpinBox::valueChanged), [=]() {
		updateLightColor(0, light1RSpin, light1GSpin, light1BSpin); });
	connect(light1GSpin, QOverload<int>::of(&QSpinBox::valueChanged), [=]() {
		updateLightColor(0, light1RSpin, light1GSpin, light1BSpin); });
	connect(light1BSpin, QOverload<int>::of(&QSpinBox::valueChanged), [=]() {
		updateLightColor(0, light1RSpin, light1GSpin, light1BSpin); });
	connect(light1PosRadio, &QRadioButton::clicked, [=](bool checked) {
		if (!checked) return;
		glView->setActiveLight(0); });

	// Update light 2
	connect(light2EnabledCB, &QCheckBox::clicked, [=](bool enabled) {
		updateLightEnabled(1, enabled); });
	connect(light2PointRadio, &QRadioButton::clicked, [=](bool checked) {
		if (!checked) return;
		updateLightType(1, Light::POINT); });
	connect(light2DirRadio, &QRadioButton::clicked, [=](bool checked) {
		if (!checked) return;
		updateLightType(1, Light::DIRECTIONAL); });
	connect(light2RSpin, QOverload<int>::of(&QSpinBox::valueChanged), [=]() {
		updateLightColor(1, light2RSpin, light2GSpin, light2BSpin); });
	connect(light2GSpin, QOverload<int>::of(&QSpinBox::valueChanged), [=]() {
		updateLightColor(1, light2RSpin, light2GSpin, light2BSpin); });
	connect(light2BSpin, QOverload<int>::of(&QSpinBox::valueChanged), [=]() {
		updateLightColor(1, light2RSpin, light2GSpin, light2BSpin); });
	connect(light2PosRadio, &QRadioButton::clicked, [=](bool checked) {
		if (!checked) return;
		glView->setActiveLight(1); });

	// Update light 3
	connect(light3EnabledCB, &QCheckBox::clicked, [=](bool enabled) {
		updateLightEnabled(2, enabled); });
	connect(light3PointRadio, &QRadioButton::clicked, [=](bool checked) {
		if (!checked) return;
		updateLightType(2, Light::POINT); });
	connect(light3DirRadio, &QRadioButton::clicked, [=](bool checked) {
		if (!checked) return;
		updateLightType(2, Light::DIRECTIONAL); });
	connect(light3RSpin, QOverload<int>::of(&QSpinBox::valueChanged), [=]() {
		updateLightColor(2, light3RSpin, light3GSpin, light3BSpin); });
	connect(light3GSpin, QOverload<int>::of(&QSpinBox::valueChanged), [=]() {
		updateLightColor(2, light3RSpin, light3GSpin, light3BSpin); });
	connect(light3BSpin, QOverload<int>::of(&QSpinBox::valueChanged), [=]() {
		updateLightColor(2, light3RSpin, light3GSpin, light3BSpin); });
	connect(light3PosRadio, &QRadioButton::clicked, [=](bool checked) {
		if (!checked) return;
		glView->setActiveLight(2); });

	// Update light 4
	connect(light4EnabledCB, &QCheckBox::clicked, [=](bool enabled) {
		updateLightEnabled(3, enabled); });
	connect(light4PointRadio, &QRadioButton::clicked, [=](bool checked) {
		if (!checked) return;
		updateLightType(3, Light::POINT); });
	connect(light4DirRadio, &QRadioButton::clicked, [=](bool checked) {
		if (!checked) return;
		updateLightType(3, Light::DIRECTIONAL); });
	connect(light4RSpin, QOverload<int>::of(&QSpinBox::valueChanged), [=]() {
		updateLightColor(3, light4RSpin, light4GSpin, light4BSpin); });
	connect(light4GSpin, QOverload<int>::of(&QSpinBox::valueChanged), [=]() {
		updateLightColor(3, light4RSpin, light4GSpin, light4BSpin); });
	connect(light4BSpin, QOverload<int>::of(&QSpinBox::valueChanged), [=]() {
		updateLightColor(3, light4RSpin, light4GSpin, light4BSpin); });
	connect(light4PosRadio, &QRadioButton::clicked, [=](bool checked) {
		if (!checked) return;
		glView->setActiveLight(3); });

	// Read OpenGL state after it's been initialized
	connect(glView, &GLView::initialized, [=]() {
			const GLState& glState = glView->getGLState();

			// Normals mode
			if (glState.getNormalMode() == GLState::NORMALMODE_FACE)
				faceNormalsRadio->setChecked(true);
			else if (glState.getNormalMode() == GLState::NORMALMODE_SMOOTH)
				smoothNormalsRadio->setChecked(true);

			// Shading mode
			if (glState.getShadingMode() == GLState::SHADINGMODE_NORMALS)
				normalsShadingRadio->setChecked(true);
			else if (glState.getShadingMode() == GLState::SHADINGMODE_PHONG)
				phongShadingRadio->setChecked(true);
			else if (glState.getShadingMode() == GLState::SHADINGMODE_GOURAUD)
				gouraudShadingRadio->setChecked(true);

			// Read the initial config file
			glView->readConfigFile(configFile);

			// Set terrain to the setting
			setTerrainSize();
			setRandomSeed();
			generateLayers();
		});
	// Update configuration when a config file is read
	connect(glView, &GLView::configChanged, [=]() {
			const GLState& glState = glView->getGLState();

			// Ambient strength
			float ambStr = glState.getAmbientStrength();
			int ambStrVal = int(glm::pow(ambStr, 1.0f / 3.0f) *
				(ambStrSlider->maximum() - ambStrSlider->minimum()) +
				ambStrSlider->minimum());
			ambStrSlider->setValue(ambStrVal);
			ambStrValLbl->setText(QString::number(ambStr, 'f', 3));

			// Diffuse strength
			float diffStr = glState.getDiffuseStrength();
			int diffStrVal = diffStr *
				(diffStrSlider->maximum() - diffStrSlider->minimum()) +
				diffStrSlider->minimum();
			diffStrSlider->setValue(diffStrVal);
			diffStrValLbl->setText(QString::number(diffStr, 'f', 2));

			// Specular strength
			float specStr = glState.getSpecularStrength();
			int specStrVal = specStr *
				(specStrSlider->maximum() - specStrSlider->minimum()) +
				specStrSlider->minimum();
			specStrSlider->setValue(specStrVal);
			specStrValLbl->setText(QString::number(specStr, 'f', 2));

			// Specular exponent
			float minPow2 = 0.0f, maxPow2 = 10.0f;
			float specExp = glState.getSpecularExponent();
			int specExpVal = int((glm::log2(specExp) - minPow2) / (maxPow2 - minPow2) *
				(specExpSlider->maximum() - specExpSlider->minimum()) +
				specExpSlider->minimum());
			specExpSlider->setValue(specExpVal);
			specExpValLbl->setText(QString::number(specExp, 'f', 2));

			// Object color
			glm::vec3 objColor = glState.getObjectColor();
			objColorRSpin->setValue(objColor.r * 255);
			objColorGSpin->setValue(objColor.g * 255);
			objColorBSpin->setValue(objColor.b * 255);

			unsigned int numLights = glState.getNumLights();

			// Light 1
			if (numLights >= 1) {
				const Light& light1 = glState.getLight(0);
				light1EnabledCB->setChecked(light1.getEnabled());
				if (light1.getType() == Light::POINT)
					light1PointRadio->setChecked(true);
				else if (light1.getType() == Light::DIRECTIONAL)
					light1DirRadio->setChecked(true);
				glm::vec3 light1Color = light1.getColor();
				light1RSpin->setValue(int(light1Color.r * 255));
				light1GSpin->setValue(int(light1Color.g * 255));
				light1BSpin->setValue(int(light1Color.b * 255));
				glm::vec3 light1Pos = light1.getPos();
				light1XLbl->setText(QString::number(light1Pos.x, 'f', 3));
				light1YLbl->setText(QString::number(light1Pos.y, 'f', 3));
				light1ZLbl->setText(QString::number(light1Pos.z, 'f', 3));
			} else
				light1Group->setDisabled(true);

			// Light 2
			if (numLights >= 2) {
				const Light& light2 = glState.getLight(1);
				light2EnabledCB->setChecked(light2.getEnabled());
				if (light2.getType() == Light::POINT)
					light2PointRadio->setChecked(true);
				else if (light2.getType() == Light::DIRECTIONAL)
					light2DirRadio->setChecked(true);
				glm::vec3 light2Color = light2.getColor();
				light2RSpin->setValue(int(light2Color.r * 255));
				light2GSpin->setValue(int(light2Color.g * 255));
				light2BSpin->setValue(int(light2Color.b * 255));
				glm::vec3 light2Pos = light2.getPos();
				light2XLbl->setText(QString::number(light2Pos.x, 'f', 3));
				light2YLbl->setText(QString::number(light2Pos.y, 'f', 3));
				light2ZLbl->setText(QString::number(light2Pos.z, 'f', 3));
			} else
				light2Group->setDisabled(true);

			// Light 3
			if (numLights >= 3) {
				const Light& light3 = glState.getLight(2);
				light3EnabledCB->setChecked(light3.getEnabled());
				if (light3.getType() == Light::POINT)
					light3PointRadio->setChecked(true);
				else if (light3.getType() == Light::DIRECTIONAL)
					light3DirRadio->setChecked(true);
				glm::vec3 light3Color = light3.getColor();
				light3RSpin->setValue(int(light3Color.r * 255));
				light3GSpin->setValue(int(light3Color.g * 255));
				light3BSpin->setValue(int(light3Color.b * 255));
				glm::vec3 light3Pos = light3.getPos();
				light3XLbl->setText(QString::number(light3Pos.x, 'f', 3));
				light3YLbl->setText(QString::number(light3Pos.y, 'f', 3));
				light3ZLbl->setText(QString::number(light3Pos.z, 'f', 3));
			} else
				light3Group->setDisabled(true);

			// Light 4
			if (numLights >= 4) {
				const Light& light4 = glState.getLight(3);
				light4EnabledCB->setChecked(light4.getEnabled());
				if (light4.getType() == Light::POINT)
					light4PointRadio->setChecked(true);
				else if (light4.getType() == Light::DIRECTIONAL)
					light4DirRadio->setChecked(true);
				glm::vec3 light4Color = light4.getColor();
				light4RSpin->setValue(int(light4Color.r * 255));
				light4GSpin->setValue(int(light4Color.g * 255));
				light4BSpin->setValue(int(light4Color.b * 255));
				glm::vec3 light4Pos = light4.getPos();
				light4XLbl->setText(QString::number(light4Pos.x, 'f', 3));
				light4YLbl->setText(QString::number(light4Pos.y, 'f', 3));
				light4ZLbl->setText(QString::number(light4Pos.z, 'f', 3));
			} else
				light4Group->setDisabled(true);
		});
	connect(glView, &GLView::normalModeChanged, [=]() {
			GLState::NormalMode nm = glView->getGLState().getNormalMode();
			if (nm == GLState::NORMALMODE_FACE)
				faceNormalsRadio->setChecked(true);
			else if (nm == GLState::NORMALMODE_SMOOTH)
				smoothNormalsRadio->setChecked(true);
		});
	connect(glView, &GLView::shadingModeChanged, [=]() {
			GLState::ShadingMode sm = glView->getGLState().getShadingMode();
			if (sm == GLState::SHADINGMODE_NORMALS)
				normalsShadingRadio->setChecked(true);
			else if (sm == GLState::SHADINGMODE_PHONG)
				phongShadingRadio->setChecked(true);
			else if (sm == GLState::SHADINGMODE_GOURAUD)
				gouraudShadingRadio->setChecked(true);
		});
	connect(glView, &GLView::ambientStrengthChanged, [=](float ambStr) {
			int ambStrVal = int(glm::pow(ambStr, 1.0f / 3.0f) *
				(ambStrSlider->maximum() - ambStrSlider->minimum()) +
				ambStrSlider->minimum());
			ambStrSlider->setValue(ambStrVal);
			ambStrValLbl->setText(QString::number(ambStr, 'f', 3));
		});
	connect(glView, &GLView::diffuseStrengthChanged, [=](float diffStr) {
			int diffStrVal = diffStr *
				(diffStrSlider->maximum() - diffStrSlider->minimum()) +
				diffStrSlider->minimum();
			diffStrSlider->setValue(diffStrVal);
			diffStrValLbl->setText(QString::number(diffStr, 'f', 2));
		});
	connect(glView, &GLView::specularStrengthChanged, [=](float specStr) {
			int specStrVal = specStr *
				(specStrSlider->maximum() - specStrSlider->minimum()) +
				specStrSlider->minimum();
			specStrSlider->setValue(specStrVal);
			specStrValLbl->setText(QString::number(specStr, 'f', 2));
		});
	connect(glView, &GLView::specularExponentChanged, [=](float specExp) {
			float minPow2 = 0.0f, maxPow2 = 10.0f;
			int specExpVal = int((glm::log2(specExp) - minPow2) / (maxPow2 - minPow2) *
				(specExpSlider->maximum() - specExpSlider->minimum()) +
				specExpSlider->minimum());
			specExpSlider->setValue(specExpVal);
			specExpValLbl->setText(QString::number(specExp, 'f', 2));
		});

	
	// Update active light radio button
	connect(glView, &GLView::activeLightChanged, [=](int index) {
			switch (index) {
			case 0:
				light1PosRadio->setChecked(true);
				break;
			case 1:
				light2PosRadio->setChecked(true);
				break;
			case 2:
				light3PosRadio->setChecked(true);
				break;
			case 3:
				light4PosRadio->setChecked(true);
				break;
			default:
				// Uncheck currently checked light (if any)
				if (lightPosGroup->checkedButton() != nullptr) {
					lightPosGroup->setExclusive(false);
					lightPosGroup->checkedButton()->setChecked(false);
					lightPosGroup->setExclusive(true);
				}
				break;
			}
		});
	connect(glView, &GLView::lightEnabledDisabled, [=](int index) {
			bool enabled = glView->getGLState().getLight(index).getEnabled();
			switch (index) {
			case 0:
				light1EnabledCB->setChecked(enabled);
				break;
			case 1:
				light2EnabledCB->setChecked(enabled);
				break;
			case 2:
				light3EnabledCB->setChecked(enabled);
				break;
			case 3:
				light4EnabledCB->setChecked(enabled);
				break;
			default: break;
			}
		});
	connect(glView, &GLView::lightTypeChanged, [=](int index) {
			Light::LightType type = glView->getGLState().getLight(index).getType();
			switch (index) {
			case 0:
				if (type == Light::POINT)
					light1PointRadio->setChecked(true);
				else if (type == Light::DIRECTIONAL)
					light1DirRadio->setChecked(true);
				break;
			case 1:
				if (type == Light::POINT)
					light2PointRadio->setChecked(true);
				else if (type == Light::DIRECTIONAL)
					light2DirRadio->setChecked(true);
				break;
			case 2:
				if (type == Light::POINT)
					light3PointRadio->setChecked(true);
				else if (type == Light::DIRECTIONAL)
					light3DirRadio->setChecked(true);
				break;
			case 3:
				if (type == Light::POINT)
					light4PointRadio->setChecked(true);
				else if (type == Light::DIRECTIONAL)
					light4DirRadio->setChecked(true);
				break;
			}
		});
	connect(glView, &GLView::lightPosChanged, [=](int index) {
			glm::vec3 lightPos = glView->getGLState().getLight(index).getPos();
			switch (index) {
			case 0:
				light1XLbl->setText(QString::number(lightPos.x, 'f', 3));
				light1YLbl->setText(QString::number(lightPos.y, 'f', 3));
				light1ZLbl->setText(QString::number(lightPos.z, 'f', 3));
				break;
			case 1:
				light2XLbl->setText(QString::number(lightPos.x, 'f', 3));
				light2YLbl->setText(QString::number(lightPos.y, 'f', 3));
				light2ZLbl->setText(QString::number(lightPos.z, 'f', 3));
				break;
			case 2:
				light3XLbl->setText(QString::number(lightPos.x, 'f', 3));
				light3YLbl->setText(QString::number(lightPos.y, 'f', 3));
				light3ZLbl->setText(QString::number(lightPos.z, 'f', 3));
				break;
			case 3:
				light4XLbl->setText(QString::number(lightPos.x, 'f', 3));
				light4YLbl->setText(QString::number(lightPos.y, 'f', 3));
				light4ZLbl->setText(QString::number(lightPos.z, 'f', 3));
				break;
			default: break;
			}
		});
}

// Search the models/ directory for any .obj files and adds them to the combo box
void App::findModels() {
	QDir modelsDir("models/");
	QFileInfoList modelsList = modelsDir.entryInfoList({ "*.obj" });
	for (auto& f : modelsList) {
		meshSelectCombo->addItem(f.filePath());
	}
}

// Program entry point
int main(int argc, char** argv) {
	QApplication a(argc, argv);

	try {
		std::string configFile = "config.txt";
		if (argc > 1)
			configFile = std::string(argv[1]);

		// Create and show the application window
		App app(configFile);
		app.show();

		// Enter the event loop
		return a.exec();

	} catch (const std::exception& e) {
		std::cerr << "Fatal error: " << e.what() << std::endl;
		return -1;
	}
}

App::SurfaceWidgetGroup::SurfaceWidgetGroup(int index, int randomNum, QWidget *parent): QGroupBox(parent) {
	surface_index = index;

	// TODO Constructing group box layout
	// Use Grid layout
	surfaceLayout = new QVBoxLayout(this);

	// Layer number
	this->setTitle(QString("Surface No. %1:").arg(index));

	// Phong Model parameters
	QGridLayout* paramGridLayout = new QGridLayout();
	QLabel* ambLbl = new QLabel("Ambient", this);
	ambLbl->sizePolicy().setHorizontalStretch(1);
	ambStrSpin = new QDoubleSpinBox(this);
	ambStrSpin->setRange(0, 1);
	ambStrSpin->setSingleStep(0.05);
	ambStrSpin->setDecimals(3);
	ambStrSpin->sizePolicy().setHorizontalStretch(1);
	ambStrSpin->setValue(1.0 / (50 + randomNum % 50));
	paramGridLayout->addWidget(ambLbl, 0, 0);
	paramGridLayout->addWidget(ambStrSpin, 0, 1);

	QLabel* diffLbl = new QLabel("Diffuse", this);
	diffLbl->sizePolicy().setHorizontalStretch(1);
	diffStrSpin = new QDoubleSpinBox(this);
	diffStrSpin->setRange(0, 1);
	diffStrSpin->setSingleStep(0.05);
	diffStrSpin->setDecimals(3);
	diffStrSpin->sizePolicy().setHorizontalStretch(1);
	diffStrSpin->setValue(1.0 / (2 + randomNum % 3));
	paramGridLayout->addWidget(diffLbl, 0, 2);
	paramGridLayout->addWidget(diffStrSpin, 0, 3);

	QLabel* specLbl = new QLabel("Specular", this);
	specLbl->sizePolicy().setHorizontalStretch(1);
	specStrSpin = new QDoubleSpinBox(this);
	specStrSpin->setRange(0, 1);
	specStrSpin->setSingleStep(0.05);
	specStrSpin->setDecimals(3);
	specStrSpin->setValue(1);
	specStrSpin->sizePolicy().setHorizontalStretch(1);
	paramGridLayout->addWidget(specLbl, 1, 0);
	paramGridLayout->addWidget(specStrSpin, 1, 1);

	QLabel* expLbl = new QLabel("Exponent", this);
	expLbl->sizePolicy().setHorizontalStretch(1);
	specExpSpin = new QDoubleSpinBox(this);
	specExpSpin->setRange(0, 1024);
	specExpSpin->setDecimals(2);
	specExpSpin->setValue(8);
	specExpSpin->setStepType(QAbstractSpinBox::StepType::AdaptiveDecimalStepType);
	specExpSpin->sizePolicy().setHorizontalStretch(1);
	paramGridLayout->addWidget(expLbl, 1, 2);
	paramGridLayout->addWidget(specExpSpin, 1, 3);

	QLabel* objColorLbl = new QLabel("Color (RGB):", this);
	objColorRSpin = new QSpinBox(this);
	objColorRSpin->setRange(0, 255);
	objColorRSpin->sizePolicy().setHorizontalStretch(1);
	objColorRSpin->setValue(randomNum % 200 + 20);
	objColorGSpin = new QSpinBox(this);
	objColorGSpin->setRange(0, 255);
	objColorGSpin->sizePolicy().setHorizontalStretch(1);
	objColorGSpin->setValue(randomNum % 150 + 40);
	objColorBSpin = new QSpinBox(this);
	objColorBSpin->setRange(0, 255);
	objColorBSpin->sizePolicy().setHorizontalStretch(1);
	objColorBSpin->setValue(randomNum % 100 + 30);
	paramGridLayout->addWidget(objColorLbl, 2, 0);
	paramGridLayout->addWidget(objColorRSpin, 2, 1);
	paramGridLayout->addWidget(objColorGSpin, 2, 2);
	paramGridLayout->addWidget(objColorBSpin, 2, 3);

	colorPickerBtn = new QPushButton("Pick a Color", this);
	colorSelected = new QLabel(this);
	colorPicker = new QColorDialog(this);
	paramGridLayout->addWidget(colorPickerBtn, 3, 0, 1, 2);
	paramGridLayout->addWidget(colorSelected, 3, 2, 1, 2);
	paramGridLayout->addWidget(colorPicker);

	surfaceLayout->addLayout(paramGridLayout);

	// Surface functions config buttons
	QHBoxLayout* controlBtnsLayout = new QHBoxLayout;
	addSurfaceFuncBtn = new QPushButton("Add a sub layer", this);
	clearSurfaceBtn = new QPushButton("Delete all sub layers", this);
	removeSurfaceBtn = new QPushButton("Delete this layer", this);
	controlBtnsLayout->addWidget(addSurfaceFuncBtn);
	controlBtnsLayout->addWidget(clearSurfaceBtn);
	controlBtnsLayout->addWidget(removeSurfaceBtn);
	surfaceLayout->addLayout(controlBtnsLayout);

	// Control checkboxs
	QHBoxLayout* controlCBLayout = new QHBoxLayout;
	enableSurfaceCB = new QCheckBox("Enable Surface", this);
	drawSurfaceCB = new QCheckBox("Draw Surface", this);
	enableSurfaceCB->setCheckState(Qt::CheckState::Checked);
	drawSurfaceCB->setCheckState(Qt::CheckState::Checked);
	controlCBLayout->addWidget(enableSurfaceCB);
	controlCBLayout->addWidget(drawSurfaceCB);
	surfaceLayout->addLayout(controlCBLayout);

	// Surface functions control
	subSurfaceFuncs = new std::vector<SubSurfaceFunc*>();
	subSurfaceFuncsLayout = new QVBoxLayout;
	addSurfaceFunc();

	surfaceLayout->addLayout(subSurfaceFuncsLayout);

	auto changeColorSelected = [=] {
		QColor color(objColorRSpin->value(), objColorGSpin->value(), objColorBSpin->value());
		QPalette palette = colorSelected->palette();
		palette.setColor(colorSelected->backgroundRole(), color);    
		colorSelected->setAutoFillBackground(true);
		colorSelected->setPalette(palette);
	};

	// Control buttons
	connect(addSurfaceFuncBtn, &QPushButton::clicked, [=] {
		addSurfaceFunc();});
	connect(clearSurfaceBtn, &QPushButton::clicked, [=] {
		clearAllSubSurfaces();});
	connect(removeSurfaceBtn, &QPushButton::clicked, [=] {
		((App*) parent)->removeLayer(this);});

	// Color selection
	connect(colorPickerBtn, &QAbstractButton::clicked, [=] {
		colorPicker->open();
	});
	connect(colorPicker, &QColorDialog::colorSelected, [=] (const QColor& color) {
		int r, g, b;
		color.getRgb(&r, &g, &b);
		objColorRSpin->setValue(r);
		objColorGSpin->setValue(g);
		objColorBSpin->setValue(b);
	});
	connect(objColorRSpin, QOverload<int>::of(&QSpinBox::valueChanged), changeColorSelected);
	connect(objColorGSpin, QOverload<int>::of(&QSpinBox::valueChanged), changeColorSelected);
	connect(objColorBSpin, QOverload<int>::of(&QSpinBox::valueChanged), changeColorSelected);

	// Set initial color
	changeColorSelected();
}

void App::SurfaceWidgetGroup::addSurfaceFunc() {
	int index = subSurfaceFuncs->size();
	SubSurfaceFunc* subSurf = new SubSurfaceFunc(index, this);
	subSurfaceFuncs->push_back(subSurf);
	subSurfaceFuncsLayout->addWidget(subSurf);
	printf("%s:%s:%d adding sub surface index %d\n", __FILE__, __func__, __LINE__, index);
}

void App::SurfaceWidgetGroup::removeSubSurfaceFunc(QWidget* subSurf) {
	auto remove_it = subSurfaceFuncs->begin();

	// Find the item to delete
	for (; remove_it < subSurfaceFuncs->end(); remove_it++) {
		if (*remove_it == subSurf)
			break;
	}

	// Not found
	if (remove_it == subSurfaceFuncs->end())
		return;

	// Reset index
	int remove_indx = (*remove_it)->index;
	for (auto it = remove_it + 1; it < subSurfaceFuncs->end(); it++) {
		(*it)->setIndex((*it)->index - 1);
	}

	// Remove from layout first
	subSurfaceFuncsLayout->removeWidget(*remove_it);
	(*remove_it)->setVisible(false);

	// Then destory it
	subSurfaceFuncs->erase(remove_it);

	printf("%s:%s:%d removing sub surface index %d\n", __FILE__, __func__, __LINE__, remove_indx);
}

void App::SurfaceWidgetGroup::clearAllSubSurfaces() {
	// Remove all widget first
	for (auto it = subSurfaceFuncs->begin(); it < subSurfaceFuncs->end(); it++) {
		subSurfaceFuncsLayout->removeWidget(*it);
		auto widget = *it;
		widget->setVisible(false);
	}
	subSurfaceFuncs->clear();

	printf("%s:%s:%d removing all sub surfaces\n", __FILE__, __func__, __LINE__);
}

void App::removeLayer(SurfaceWidgetGroup* layer) {
	auto remove_it = std::find(surfaces->begin(), surfaces->end(), layer);
	if (remove_it == surfaces->end())
		return;

	// Reset index
	int remove_index = (*remove_it)->surface_index;
	for (auto it = remove_it + 1; it < surfaces->end(); it++) {
		(*it)->setIndex((*it)->surface_index - 1);
	}

	// Remove from layout first and set to invisible
	auto surface = *remove_it;
	surfacesLayout->removeWidget(surface);
	surface->setVisible(false);

	// Destory it
	surfaces->erase(remove_it);

	printf("%s:%s:%d removing layer index %d\n", __FILE__, __func__, __LINE__, remove_index);
};

void App::addLayer() {
	int index = surfaces->size();
	int randomNum = dist(rd);
	SurfaceWidgetGroup* layer = new SurfaceWidgetGroup(index, randomNum, this);
	surfaces->push_back(layer);
	surfacesLayout->addWidget(layer);
	// todo Resize after adding one
	// surfacesLayout->
	printf("%s:%s:%d adding layer index %d\n", __FILE__, __func__, __LINE__, index);
}

void App::clearLayers() {
	// Remove all widget first
	for (auto it = surfaces->begin(); it < surfaces->end(); it++) {
		surfacesLayout->removeWidget(*it);
		auto widget = *it;
		widget->setVisible(false);
	}
	surfaces->clear();

	printf("%s:%s:%d removing all layers\n", __FILE__, __func__, __LINE__);
}

void App::generateLayers() {
	// Create the layer configurations to be passed into the glstate
	GLState& state = glView->getGLState();
	state.clearTerrainLayers();
	
	// Pushing functions
	for (auto group_it = surfaces->begin(); group_it < surfaces->end(); group_it++) {
		// Each layer
		App::SurfaceWidgetGroup* surfaceGroup = *group_it;

		// Get phong config
		Terrain::PhongConfig config;
		config.ambient = surfaceGroup->ambStrSpin->value();
		config.diffuse = surfaceGroup->diffStrSpin->value();
		config.specular = surfaceGroup->specStrSpin->value();
		config.exponent = surfaceGroup->specExpSpin->value();
		config.color.r = surfaceGroup->objColorRSpin->value();
		config.color.g = surfaceGroup->objColorGSpin->value();
		config.color.b = surfaceGroup->objColorBSpin->value();

		// TODO create the checkbox for these config
		config.enable = surfaceGroup->enableSurfaceCB->checkState() == Qt::CheckState::Checked;
		config.drawSurface =surfaceGroup->drawSurfaceCB->checkState() == Qt::CheckState::Checked;
		config.coverBottom = false;

		std::vector<std::string> funcStrings;
		std::vector<SurfaceWidgetGroup::SubSurfaceFunc*>* funcsWidget = surfaceGroup->subSurfaceFuncs;

		// Each func
		for (auto surface_it = funcsWidget->begin(); surface_it < funcsWidget->end(); surface_it++) {
			SurfaceWidgetGroup::SubSurfaceFunc* func_widget = *surface_it;
			funcStrings.push_back(func_widget->subSurfaceLine->text().toStdString());
		}

		// Make sure we don't pass empty funcs vector
		if (!funcStrings.empty())
			state.pushTerrainLayer(std::pair(funcStrings, config));
	}

	state.evaluateTerrain();
	state.generateTerrain();
	state.paintGL();
}

void App::setRandomSeed() {
	int value = randomSeedSpin->value();
	glView->getGLState().terrain->setSeed(value);
}

void App::setTerrainSize() {
	int width = terrainWidth->value();
	int length = terrainLength->value();
	printf("W: %d L: %d\n", width, length);
	glView->getGLState().terrain->setSize(width, length);
}