#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "SARibbonMainWindow.h"

#include <QTreeWidget>
#include <QStatusBar>

#include <STEPCAFControl_Reader.hxx>
#include <STEPControl_Writer.hxx>
#include <IGESCAFControl_Reader.hxx>
#include <IGESControl_Writer.hxx>
#include <IGESControl_Controller.hxx>
#include <Interface_Static.hxx>
#include <StlAPI.hxx>
#include <BRep_Builder.hxx>
#include <BRepTools.hxx>
#include <Quantity_Color.hxx>

#include "occ/occview.h"
#include "occ/modelmaker.h"
#include "occ/RLAPI_Reader.h"
#include "occ/Robot.h"
#include "page/constructionwidget.h"
#include "page/dialogPosition.h"
#include "page/MeasureWidget.h"

#include "Method/calibration.h"
#include "Method/coordinate.h"
#include "Method/uniqueid.h"
#include "Method/explanation.h"
#include "fileInOut/importfile.h"
#include "fileInOut/importcoefile.h"
#include "fileInOut/importstructfile.h"
#include "Decode/decode.h"
#include "commondata.h"
#include "serial.h"

class SARibbonCategory;
enum class GeometryType;

class MainWindow : public SARibbonMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *par = nullptr);
    ~MainWindow();

    void initFunction();
    void initRibbon();
    void initToolBar();
    void initDockWidget();
    void initRobot();

private:
    void createCategoryProject(SARibbonCategory *page);
    void createCategorySetting(SARibbonCategory *page);
    void createCategoryMeasure(SARibbonCategory *page);
    void createCategoryTolerance(SARibbonCategory *page);
    void createCategoryConstruct(SARibbonCategory *page);
    void createCategoryReport(SARibbonCategory *page);
    void createCategoryVision(SARibbonCategory *page);
    void createCategoryHelp(SARibbonCategory *page);

private slots:
    void on_doConstruct(int constructType);
    void on_setItemColor();
    void on_deleteSelected();
    void on_importModel();
    void on_exportModel();
    void on_addMeasurePnt(const gp_Pnt &pnt);
    void on_measueAction();
    void on_showReport();

protected:
    void closeEvent(QCloseEvent *event);

private:
    occView *occWidget;
    constructionWidget *constructWidget;
    DialogPosition *posDlg;    
    MeasureWidget *measWid;
    QDockWidget *measDock;
    GeometryType currentType;
    Robot *robot;

    calibration * mCalibration;
    coordinate * mCoordinate;
    uniqueID * mUniqueID;
    commonData * mCommonData;
    explanation * mExplanation;
    importFile * mImportFile;
    Serial * mSerial;

    QTreeWidget *itemTree;
    QStatusBar *resultStatus;

    bool shadedModel=true;
    QList <Handle(AIS_InteractiveObject)> m_displayedShapes;
    QList <MeasureItem*> mDMISItems;

    void addItem(QString itemName);
    void addItem(MeasureItem *aItm);
    void addResult(QString resultType,QString result);
    QString typeName(int TopoShape);
    void SetRobotJoint();

    void chooseConnectModel();
    void initialForm();
    void initialMethod();
    void setMethod(Method * arg);
    void decodeModelChoose();
private slots:
    void readStructSlots();
    void readCoeffSlots();

    void test();
};

#endif // MAINWINDOW_H
