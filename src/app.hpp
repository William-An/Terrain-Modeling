#ifndef APP_HPP
#define APP_HPP

#include <QWidget>
#include <QComboBox>
#include <QRadioButton>
#include <QPushButton>
#include <QCheckBox>
#include <QSlider>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include "glview.hpp"

// Application window
class App : public QWidget {
	Q_OBJECT
public:
	App(std::string configFile, QWidget* parent = NULL);

protected:
	// Event handlers
	void keyReleaseEvent(QKeyEvent* e);

private:
	// Initialization
	void initLayout(std::string configFile);
	void findModels();

	// Widgets
	GLView* glView;
	QComboBox* meshSelectCombo;
	QRadioButton* faceNormalsRadio;
	QRadioButton* smoothNormalsRadio;
	QRadioButton* normalsShadingRadio;
	QRadioButton* phongShadingRadio;
	QRadioButton* gouraudShadingRadio;
	QPushButton* presetGoldBtn;
	QPushButton* presetObsidianBtn;
	QPushButton* presetPearlBtn;
	QSlider* ambStrSlider;
	QSlider* diffStrSlider;
	QSlider* specStrSlider;
	QSlider* specExpSlider;
	QSpinBox* objColorRSpin;
	QSpinBox* objColorGSpin;
	QSpinBox* objColorBSpin;

	// General control
	QSpinBox* randomSeedSpin;			// Random seed dialog
	QLineEdit* terrainName;
	QPushButton* randomizedBtn;			// Roll for a new seed
	QPushButton* generateTerrainBtn;	// Generate terrain based on current config
	QPushButton* addSurfaceBtn;			// Add a surface control
	QPushButton* clearSurfaceBtn;		// Remove all surface funcs
	QPushButton* loadTerrainConfigBtn;
	QPushButton* dumpTerrainConfigBtn;

	// Surface control
	class SurfaceWidgetGroup {
		public:
			std::vector<std::pair<QLabel*, QLineEdit*>> surfaceFuncs;
			QDoubleSpinBox* ambStrSpin;
			QDoubleSpinBox* diffStrSpin;
			QDoubleSpinBox* specStrSpin;
			QDoubleSpinBox* specExpSpin;
			QSpinBox* objColorRSpin;
			QSpinBox* objColorGSpin;
			QSpinBox* objColorBSpin;
			QPushButton* removeSurfaceBtn;	// Delete this widget group
			QPushButton* addSurfaceFuncBtn;	// Add a line of the func
			QPushButton* clearSurfaceBtn;	// Remove all functions

			// TODO Should have a view returned to construct in top level
			SurfaceWidgetGroup();
	};

	std::vector<App::SurfaceWidgetGroup*> surfaces;

	// Light 1 controls
	QCheckBox* light1EnabledCB;
	QRadioButton* light1PointRadio;
	QRadioButton* light1DirRadio;
	QSpinBox* light1RSpin;
	QSpinBox* light1GSpin;
	QSpinBox* light1BSpin;
	QRadioButton* light1PosRadio;

	// Light 2 controls
	QCheckBox* light2EnabledCB;
	QRadioButton* light2PointRadio;
	QRadioButton* light2DirRadio;
	QSpinBox* light2RSpin;
	QSpinBox* light2GSpin;
	QSpinBox* light2BSpin;
	QRadioButton* light2PosRadio;

	// Light 3 controls
	QCheckBox* light3EnabledCB;
	QRadioButton* light3PointRadio;
	QRadioButton* light3DirRadio;
	QSpinBox* light3RSpin;
	QSpinBox* light3GSpin;
	QSpinBox* light3BSpin;
	QRadioButton* light3PosRadio;

	// Light 4 controls
	QCheckBox* light4EnabledCB;
	QRadioButton* light4PointRadio;
	QRadioButton* light4DirRadio;
	QSpinBox* light4RSpin;
	QSpinBox* light4GSpin;
	QSpinBox* light4BSpin;
	QRadioButton* light4PosRadio;
};

#endif
