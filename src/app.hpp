#ifndef APP_HPP
#define APP_HPP

#include <random>
#include <QWidget>
#include <QComboBox>
#include <QRadioButton>
#include <QPushButton>
#include <QCheckBox>
#include <QSlider>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QScrollArea>
#include <QScrollBar>
#include "glview.hpp"

// Application window
class App : public QWidget {
	Q_OBJECT
public:
	App(std::string configFile, QWidget* parent = NULL);
	
	// Surface group
	class SurfaceWidgetGroup : public QGroupBox {
		public:
			class SubSurfaceFunc : public QWidget {
				public:
					int index;
					QLabel* subSurfaceLbl;
					QLineEdit* subSurfaceLine;
					QPushButton* subSurfaceRemoveBtn;
					QHBoxLayout* subSurfaceLayout;

					SubSurfaceFunc(int index, QWidget *parent = nullptr): QWidget(parent) {
						this->index = index;

						// Layout control
						subSurfaceLayout = new QHBoxLayout(this);
						this->setLayout(subSurfaceLayout);

						subSurfaceLbl = new QLabel(QString("Sub layer %1: ").arg(index));
						subSurfaceLbl->sizePolicy().setHorizontalStretch(3);
						subSurfaceLine = new QLineEdit("normal(x,y, 0.5, 0.5)");
						subSurfaceLine->sizePolicy().setHorizontalStretch(10);
						subSurfaceRemoveBtn = new QPushButton(QString("Delete"));
						subSurfaceRemoveBtn->sizePolicy().setHorizontalStretch(2);

						subSurfaceLayout->addWidget(subSurfaceLbl);
						subSurfaceLayout->addWidget(subSurfaceLine);
						subSurfaceLayout->addWidget(subSurfaceRemoveBtn);

						// Connect delete button to parent remove
						connect(subSurfaceRemoveBtn, &QPushButton::clicked, [=] {
							((SurfaceWidgetGroup *)parent)->removeSubSurfaceFunc(this);
						});
					};

					~SubSurfaceFunc() {
						subSurfaceLayout->removeWidget(subSurfaceLbl);
						subSurfaceLayout->removeWidget(subSurfaceLine);
						subSurfaceLayout->removeWidget(subSurfaceRemoveBtn);
						delete subSurfaceLbl;
						delete subSurfaceLine;
						delete subSurfaceRemoveBtn;
						delete subSurfaceLayout;
					}

					void setIndex(int i) {
						this->index = i;
						subSurfaceLbl->setText(QString("Sub layer %1: ").arg(index));
					}
			};
			int surface_index;
			std::vector<SubSurfaceFunc*>* subSurfaceFuncs;
			QDoubleSpinBox* ambStrSpin;
			QDoubleSpinBox* diffStrSpin;
			QDoubleSpinBox* specStrSpin;
			QDoubleSpinBox* specExpSpin;
			QSpinBox* objColorRSpin;
			QSpinBox* objColorGSpin;
			QSpinBox* objColorBSpin;
			QPushButton* addSurfaceFuncBtn;		// Add a line of the func
			QPushButton* clearSurfaceBtn;		// Remove all functions
			QPushButton* removeSurfaceBtn;		// Delete this widget group
			QVBoxLayout* surfaceLayout;
			QVBoxLayout* subSurfaceFuncsLayout;	// Handle to add and remove func sublayer

			// TODO Should have a view returned to construct in top level
			SurfaceWidgetGroup(int index, QWidget *parent = nullptr);

			// Add a sub surface func input and add to surfaceFuncsLayout
			void addSurfaceFunc();

			// Remove a sub surface func with the pointer val 
			void removeSubSurfaceFunc(QWidget* subSurf);

			// Remove all sub surfaces
			void clearAllSubSurfaces();

			// Set index
			void setIndex(int i) {
				surface_index = i;
				this->setTitle(QString("Surface No. %1:").arg(i));
			}
	};

	void addLayer();
	// remove a surface layer 
	void removeLayer(SurfaceWidgetGroup* layer);
	void clearLayers();
	void scrollControlToEnd() {
		int end = scrollArea->verticalScrollBar()->maximum();
		printf("Maximu: %d\n", end);
		scrollArea->verticalScrollBar()->setValue(end);
	};
	void generateLayers();


protected:
	// Event handlers
	void keyReleaseEvent(QKeyEvent* e);

private:
	// Initialization
	void initLayout(std::string configFile);
	void findModels();

	// Random device
	std::uniform_int_distribution<int> dist;
	std::mt19937 rd;

	// Widgets
	GLView* glView;
	QScrollArea* scrollArea;
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
	QVBoxLayout* surfacesLayout;
	std::vector<App::SurfaceWidgetGroup*>* surfaces;

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
