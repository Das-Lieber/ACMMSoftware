#include "mainwindow.h"

#include <QToolBar>
#include <QDockWidget>
#include <QHeaderView>
#include <QColorDialog>
#include <QMessageBox>
#include <QLabel>
#include <QFileDialog>
#include <QFileInfo>
#include <QRandomGenerator>

#include <Prs3d_PointAspect.hxx>

#include "SARibbonBar.h"
#include "SARibbonCategory.h"
#include "SARibbonPannel.h"
#include "SARibbonMenu.h"
#include "SARibbonApplicationButton.h"

#include "occ/MeasureItem.h"
#include "occ/LabelWidget.h"
#include "occ/GeneralTools.h"
#include "occ/RobotLink.h"
#include "page/DMISReport.h"
#include "page/ReportViewer.h"

#define MEASURE_TEST

MainWindow::MainWindow(QWidget *par) : SARibbonMainWindow(par)
{
    setWindowTitle("ACMM测量软件");
    this->setFont(QFont("Microsoft Yahei UI"));
}

MainWindow::~MainWindow()
{
    delete robot;
    for(int i=0;i<mDMISItems.size();++i)
    {
        delete mDMISItems[i];
    }
}

void MainWindow::initFunction()
{
    occWidget = new occView(this);
    setCentralWidget(occWidget);
    connect(occWidget,&occView::newMeasureItem,this,[=](MeasureItem *aItm) {
        addItem(aItm);
    });

#ifdef MEASURE_TEST
    connect(occWidget,&occView::pickPixel,this,[=](int Xp, int Yp) {
        if(currentType == GeometryType::UNKNOWN)
            return;

        gp_Pnt ResultPoint;
        //1、获取投影点和投影方向,作眼射线
        double X,Y,Z,VX,VY,VZ;
        occWidget->getView()->ConvertWithProj(Xp,Yp,X,Y,Z,VX,VY,VZ);//二维平面点、三维投影点、投影方向
        ResultPoint.SetCoord(X,Y,Z);//设置三维点坐标
        gp_Lin eyeLin(ResultPoint,gp_Dir(VX,VY,VZ));//眼射线
        //2、计算眼射线和shape交点
        //面
        if(currentType == GeometryType::PLANE ||
                currentType == GeometryType::SPHERE ||
                currentType == GeometryType::CYLINDER ||
                currentType == GeometryType::CONE ||
                currentType == GeometryType::DISTANCE)
        {
            double minD = -1;//最小距离
            gp_Pnt minP;//最小距离的点
            IntCurvesFace_ShapeIntersector ICFSI;//用来确定线和曲面上的交点
            if(occWidget->getContext()->SelectedShape().IsNull())
                return;
            ICFSI.Load(occWidget->getContext()->SelectedShape(),Precision::Confusion());//Precision::Confusion()精度
            ICFSI.Perform(eyeLin,0,100000);
            if (ICFSI.IsDone())
            {
                for (int i=1;i<=ICFSI.NbPnt();i++)
                {
                    gp_Pnt Pi = ICFSI.Pnt(i);
                    double Dis = ResultPoint.Distance(Pi);
                    if (minD == -1 || minD>Dis)
                    {
                        minD = Dis;
                        minP = Pi;
                    }
                }
            }
            if (minD !=-1)
            {
                ResultPoint.SetCoord(minP.X(),minP.Y(),minP.Z());
            }
            else return;
        }
        //线
        else if(currentType == GeometryType::LINE ||
                currentType == GeometryType::CIRCLE)
        {
            if(occWidget->getContext()->SelectedShape().IsNull())
                return;
            TopoDS_Edge targetEdge = TopoDS::Edge(occWidget->getContext()->SelectedShape());
            Standard_Real first, last;
            Handle(Geom_Curve) aCurve  = BRep_Tool::Curve(targetEdge,first,last);
            TopoDS_Edge eyeEdge = BRepBuilderAPI_MakeEdge(eyeLin);//通过射线构造边
            Handle(Geom_Curve) eCurve  = BRep_Tool::Curve(eyeEdge,first,last);

            GeomAPI_ExtremaCurveCurve extCC(eCurve,aCurve);
            if(extCC.NbExtrema() == 0)
                return ;
            gp_Pnt pt1,pt2;
            extCC.NearestPoints(pt1,pt2);//求解曲线间最近点
            ResultPoint = pt2;
        }
        else return;

        TopoDS_Shape aPt = ModelMaker::make_point(ResultPoint);
        Handle(AIS_Shape) ptAIS = new AIS_Shape(aPt);
        ptAIS->Attributes()->SetPointAspect(new Prs3d_PointAspect(Aspect_TOM_O_STAR,Quantity_NOC_GREEN,1));
        occWidget->getContext()->Display(ptAIS,Standard_True);
        if(!measDock->isHidden()) {
            measWid->AppendPnt(ResultPoint);
            measWid->AppendVirtualPnt(ResultPoint);
        }
    });
#endif

    resultStatus = new QStatusBar(this);
    QLabel *copyright  = new QLabel("Copyright@合肥工业大学 仪器科学与光电工程学院    ");
    QFont ft("Microsoft Yahei UI",10);
    copyright->setFont(ft);
    resultStatus->addPermanentWidget(copyright);
    this->setStatusBar(resultStatus);

    constructWidget = new constructionWidget();
    QObject::connect(constructWidget,&constructionWidget::doConstruct,this,&MainWindow::on_doConstruct);

    mUniqueID = new uniqueID(this);
    mCalibration = new calibration(this);
    connect(mCalibration,&calibration::readyClose,this,[=]() {
        setMethod(posDlg);
    });
    mCoordinate = new coordinate(this);
    connect(mCoordinate,&coordinate::readyClose,this,[=]() {
        setMethod(posDlg);
    });
    mCommonData = new commonData(this);
    mExplanation = new explanation(this);
    connect(mExplanation,&explanation::connectDone,this,[=](){
        setMethod(posDlg);
    });
    mSerial = new Serial(this);
    mImportFile = nullptr;
}

void MainWindow::initRibbon()
{
    SARibbonBar *ribbon = ribbonBar();
    QFont ft = ribbon->font();
    ft.setPointSize(14);
    ribbon->setFont(ft);

    ribbon->applitionButton()->setVisible(false);

    QFont f = ribbon->font();
    f.setFamily("Microsoft YaHei UI");
    ribbon->setFont(f);

    SARibbonCategory* categoryProject = ribbon->addCategoryPage("工程");
    createCategoryProject(categoryProject);

    SARibbonCategory* categorySetting = ribbon->addCategoryPage("设置");
    createCategorySetting(categorySetting);

    SARibbonCategory* categoryMeasure = ribbon->addCategoryPage("测量");
    createCategoryMeasure(categoryMeasure);

    SARibbonCategory* categoryConstruct = ribbon->addCategoryPage("构建");
    createCategoryConstruct(categoryConstruct);

    SARibbonCategory* categoryTolerance = ribbon->addCategoryPage("公差");
    createCategoryTolerance(categoryTolerance);

    SARibbonCategory* categoryReport = ribbon->addCategoryPage("报告");
    createCategoryReport(categoryReport);

    SARibbonCategory* categoryVision = ribbon->addCategoryPage("视图");
    createCategoryVision(categoryVision);

    SARibbonCategory* categoryHelp = ribbon->addCategoryPage("帮助");
    createCategoryHelp(categoryHelp);
}

void MainWindow::initToolBar()
{
    QToolBar *toolBar_view = new QToolBar("视图",this);
    toolBar_view->setAllowedAreas(Qt::TopToolBarArea);

    QAction *act = new QAction(QIcon(":/res/toolBar/Left.png"),"左视图",this);
    connect(act,&QAction::triggered,this,[=](){
        occWidget->getView()->SetProj(V3d_Xneg);
        occWidget->getView()->FitAll();
        addResult(tr("Success"),tr("已显示左视图"));
    });
    toolBar_view->addAction(act);
    act = new QAction(QIcon(":/res/toolBar/Right.png"),"右视图",this);
    connect(act,&QAction::triggered,this,[=](){
        occWidget->getView()->SetProj(V3d_Xpos);
        occWidget->getView()->FitAll();
        addResult(tr("Success"),tr("已显示右视图"));
    });
    toolBar_view->addAction(act);
    act = new QAction(QIcon(":/res/toolBar/Front.png"),"前视图",this);
    connect(act,&QAction::triggered,this,[=](){
        occWidget->getView()->SetProj(V3d_Yneg);
        occWidget->getView()->FitAll();
        addResult(tr("Success"),tr("已显示前视图"));
    });
    toolBar_view->addAction(act);
    act = new QAction(QIcon(":/res/toolBar/Back.png"),"后视图",this);
    connect(act,&QAction::triggered,this,[=](){
        occWidget->getView()->SetProj(V3d_Ypos);
        occWidget->getView()->FitAll();
        addResult(tr("Success"),tr("已显示后视图"));
    });
    toolBar_view->addAction(act);
    act = new QAction(QIcon(":/res/toolBar/Top.png"),"顶视图",this);
    connect(act,&QAction::triggered,this,[=](){
        occWidget->getView()->SetProj(V3d_Zpos);
        occWidget->getView()->FitAll();
        addResult(tr("Success"),tr("已显示顶视图"));
    });
    toolBar_view->addAction(act);
    act = new QAction(QIcon(":/res/toolBar/Bottom.png"),"底视图",this);
    connect(act,&QAction::triggered,this,[=](){
        occWidget->getView()->SetProj(V3d_Zneg);
        occWidget->getView()->FitAll();
        addResult(tr("Success"),tr("已显示底视图"));
    });
    toolBar_view->addAction(act);

    toolBar_view->addSeparator();

    act = new QAction(QIcon(":/res/toolBar/view_shade.png"),"显示模式",this);
    connect(act,&QAction::triggered,this,[=](){
        if(shadedModel)
        {
            occWidget->getContext()->SetDisplayMode(AIS_WireFrame,Standard_True);
            act->setIcon(QIcon(":/res/toolBar/view_wire.png"));
            shadedModel = false;
        }
        else
        {
            occWidget->getContext()->SetDisplayMode(AIS_Shaded,Standard_True);
            act->setIcon(QIcon(":/res/toolBar/view_shade.png"));
            shadedModel = true;
        }
        addResult(tr("Success"),tr("切换显示模式成功"));
    });
    toolBar_view->addAction(act);
    act = new QAction(QIcon(":/res/toolBar/color_tool.png"),"设置颜色",this);
    connect(act,&QAction::triggered,this,&MainWindow::on_setItemColor);
    toolBar_view->addAction(act);
    act = new QAction(QIcon(":/res/toolBar/delete.png"),"删除选中模型",this);
    connect(act,&QAction::triggered,this,&MainWindow::on_deleteSelected);
    toolBar_view->addAction(act);

    toolBar_view->addSeparator();

    act = new QAction(QIcon(":/res/toolBar/selectPnt.png"),"选择点",this);
    connect(act,&QAction::triggered,this,[=](){
        occWidget->getContext()->Deactivate();
        occWidget->getContext()->Activate(TopAbs_VERTEX ,Standard_True);
        addResult(tr("Success"),tr("切换至点选择模式"));
    });
    toolBar_view->addAction(act);
    act = new QAction(QIcon(":/res/toolBar/selectEdge.png"),"选择线",this);
    connect(act,&QAction::triggered,this,[=](){
        occWidget->getContext()->Deactivate();
        occWidget->getContext()->Activate(TopAbs_SOLID ,Standard_True);
        addResult(tr("Success"),tr("切换至线选择模式"));
    });
    toolBar_view->addAction(act);
    act = new QAction(QIcon(":/res/toolBar/selectFace.png"),"选择面",this);
    connect(act,&QAction::triggered,this,[=](){
        occWidget->getContext()->Deactivate();
        occWidget->getContext()->Activate(TopAbs_FACE ,Standard_True);
        addResult(tr("Success"),tr("切换至面选择模式"));
    });
    toolBar_view->addAction(act);
    act = new QAction(QIcon(":/res/toolBar/selectShape.png"),"选择实体",this);
    connect(act,&QAction::triggered,this,[=](){
        occWidget->getContext()->Deactivate();
        occWidget->getContext()->Activate(TopAbs_SHAPE ,Standard_True);
        addResult(tr("Success"),tr("已切换至实体选择模式"));
    });
    toolBar_view->addAction(act);

    this->addToolBar(Qt::TopToolBarArea,toolBar_view);
}

void MainWindow::initDockWidget()
{
    QDockWidget *treeDock = new QDockWidget("工程窗口",this);
    itemTree = new QTreeWidget;
    itemTree->header()->setSectionResizeMode(QHeaderView::Fixed);
    itemTree->setHeaderLabel("工程信息");
    itemTree->headerItem()->setTextAlignment(0,Qt::AlignHCenter);
    itemTree->addTopLevelItem(new QTreeWidgetItem(itemTree,{"几何"},0));
    itemTree->addTopLevelItem(new QTreeWidgetItem(itemTree,{"测量"},0));
    treeDock->setWidget(itemTree);
    this->addDockWidget(Qt::LeftDockWidgetArea,treeDock);

    QDockWidget *dataDock = new QDockWidget("数据窗口",this);
    posDlg = new DialogPosition;
    posDlg->setMinimumWidth(320);
    posDlg->setMaximumHeight(340);
    connect(posDlg,&DialogPosition::addMeasurePoint,this,&MainWindow::on_addMeasurePnt);    
    dataDock->setWidget(posDlg);
    this->addDockWidget(Qt::LeftDockWidgetArea,dataDock);

    measWid = new MeasureWidget;
    measWid->setMinimumWidth(400);
    measDock = new QDockWidget("测量窗口",this);
    measDock->setWidget(measWid);
    this->addDockWidget(Qt::RightDockWidgetArea,measDock);
    measDock->hide();
    currentType = GeometryType::UNKNOWN;

    connect(measWid,&MeasureWidget::measureEnd,this,[=](const QList<QList<gp_Pnt>> &result, const QList<QList<gp_Pnt>> &virtuData) {
        if(result.isEmpty())
            return;

       occWidget->createLabel(result,virtuData,currentType);
    });
    connect(measWid,&MeasureWidget::readyClose,this,[=]() {
        measDock->hide();
        measWid->SetFeatureSize(1);
        currentType = GeometryType::UNKNOWN;
    });
}

void MainWindow::initRobot()
{
    robot = new Robot;
    RLAPI_Reader *aReaderAPI = new RLAPI_Reader;

    QList<gp_Pnt> MotionCenters;
    QList<gp_Ax1> MotionAxis;
    MotionCenters = {gp_Pnt(0,0,208.5),
                     gp_Pnt(41,0,300.5),
                     gp_Pnt(0,0,669.5),
                     gp_Pnt(29,0,750.5),
                     gp_Pnt(0,0,1119.5),
                     gp_Pnt(29,0,1200.5)};
    MotionAxis = {gp_Ax1(MotionCenters[0],{0,0,1}),
                  gp_Ax1(MotionCenters[1],{0,1,0}),
                  gp_Ax1(MotionCenters[2],{0,0,1}),
                  gp_Ax1(MotionCenters[3],{0,1,0}),
                  gp_Ax1(MotionCenters[4],{0,0,1}),
                  gp_Ax1(MotionCenters[5],{0,1,0})};
    gp_Trsf last;
    last.SetValues(1,0,0,0,
                   0,1,0,0,
                   0,0,1,1394);

    robot->SetMotionAxis(MotionAxis);
    robot->SetCorrectAng({-120,60,160,120,60,120});
    robot->SetLinkShapes(aReaderAPI->ReadJointModels(qApp->applicationDirPath() + "/robot"));
    robot->Assembel(aReaderAPI->ParseJointAssemble());
    robot->SetTCPTrsf(last);

    robot->PerformFK({210,-63.7,-348.4,-311.1,-52.6,-90.9});

    for (int i=0;i<robot->Links().size();++i)
    {
       QList<Handle(XCAFPrs_AISObject)> shapes = robot->Links()[i]->Shapes();
       for(int j=0;j<shapes.size();++j)
       {
            occWidget->getContext()->Display(shapes[j],Standard_False);
            occWidget->getContext()->Deactivate(shapes[j]);
       }
    }
    occWidget->getView()->SetProj(V3d_Xneg);
    occWidget->getView()->FitAll();

    delete aReaderAPI;
}

void MainWindow::createCategoryProject(SARibbonCategory *page)
{
    SARibbonPannel* pannel = page->addPannel("");

    QAction* act = new QAction(this);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/project/open.png"));
    act->setText("打开项目");

#ifdef MEASURE_TEST
    connect(act,&QAction::triggered,this,&MainWindow::test);
#endif

    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/project/create.png"));
    act->setText("新建项目");
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/project/NewWorkpiece.png"));
    act->setText("新建工件");
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/project/save.png"));
    act->setText("保存");
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/project/saveas.png"));
    act->setText("另存为");
    pannel->addLargeAction(act);

    pannel->addSeparator();

    act = new QAction(this);
    act->setIcon(QIcon(":/res/project/import.png"));
    act->setText("导入");
    connect(act,&QAction::triggered,this,&MainWindow::on_importModel);
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/project/export.png"));
    act->setText("导出");
    connect(act,&QAction::triggered,this,&MainWindow::on_exportModel);
    pannel->addLargeAction(act);

    pannel->addSeparator();

    act = new QAction(this);
    act->setIcon(QIcon(":/res/project/close.png"));
    act->setText("关闭");
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/project/exit.png"));
    act->setText("退出程序");
    connect(act,&QAction::triggered,this,&MainWindow::close);
    pannel->addLargeAction(act);
}

void MainWindow::createCategorySetting(SARibbonCategory *page)
{
    SARibbonPannel* pannel = page->addPannel(tr(""));

    QAction *act = new QAction(this);
    act->setIcon(QIcon(":/res/setting/connect.png"));
    act->setText("连接设备");
    connect(act,&QAction::triggered,this,&MainWindow::chooseConnectModel);
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/setting/structure.png"));
    act->setText("结构参数");
    connect(act,&QAction::triggered,this,&MainWindow::readStructSlots);
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/setting/coffe.png"));
    act->setText("偏心参数");
    connect(act,&QAction::triggered,this,&MainWindow::readCoeffSlots);
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/setting/calibrate.png"));
    act->setText("标定");
    connect(act,&QAction::triggered,this,[=]() {
        setMethod(mCalibration);
        mCalibration->assigAbstractObject(mCalibration);
    });
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/setting/tcp.png"));
    act->setText("通讯");
    connect(act,&QAction::triggered,this,[=]() {
        setMethod(mCoordinate);
        mCoordinate->assigAbstractObject(mCoordinate);
    });
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/setting/system.png"));
    act->setText("系统详情");
    connect(act,&QAction::triggered,mExplanation,&explanation::showUi);
    pannel->addLargeAction(act);
}

void MainWindow::createCategoryMeasure(SARibbonCategory *page)
{
    SARibbonPannel* pannel = page->addPannel(tr(""));

    QAction* act = new QAction(this);
    act->setIcon(QIcon(":/res/measure/VPoint.png"));
    act->setText("矢量点");    
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/measure/Line.png"));
    act->setText("直线");
    connect(act,&QAction::triggered,this,&MainWindow::on_measueAction);
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/measure/Plane.png"));
    act->setText("平面");
    connect(act,&QAction::triggered,this,&MainWindow::on_measueAction);
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/measure/Circle.png"));
    act->setText("圆");
    connect(act,&QAction::triggered,this,&MainWindow::on_measueAction);
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/measure/Arc.png"));
    act->setText("圆弧");
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/measure/Ellipse.png"));
    act->setText("椭圆");
//    connect(act,&QAction::triggered,this,&MainWindow::on_measueAction);
    pannel->addLargeAction(act);

    pannel->addSeparator();

    act = new QAction(this);
    act->setIcon(QIcon(":/res/measure/Grove.png"));
    act->setText("方槽");
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/measure/RGrove.png"));
    act->setText("圆槽");
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/measure/Cylinder.png"));
    act->setText("圆柱");
    connect(act,&QAction::triggered,this,&MainWindow::on_measueAction);
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/measure/Cone.png"));
    act->setText("圆锥");
    connect(act,&QAction::triggered,this,&MainWindow::on_measueAction);
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/measure/Sphere.png"));
    act->setText("球");
    connect(act,&QAction::triggered,this,&MainWindow::on_measueAction);
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/measure/LZ.png"));
    act->setText("棱柱");
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/measure/YH.png"));
    act->setText("圆环");
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/measure/SFace.png"));
    act->setText("曲面");
    pannel->addLargeAction(act);

    pannel->addSeparator();

    act = new QAction(this);
    act->setIcon(QIcon(":/res/measure/Distance.png"));
    act->setText("特征间距");
    connect(act,&QAction::triggered,this,&MainWindow::on_measueAction);
    pannel->addLargeAction(act);
}

void MainWindow::createCategoryTolerance(SARibbonCategory *page)
{
    SARibbonPannel* pannel = page->addPannel("形状公差");

    QAction* act = new QAction(this);
    act->setIcon(QIcon(":/res/tolerance/Linearity.png"));
    act->setText("直线度");
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/tolerance/Planarity.png"));
    act->setText("平面度");
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/tolerance/Circularity.png"));
    act->setText("圆度");
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/tolerance/Cylindricity.png"));
    act->setText("圆柱度");
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/tolerance/Conicity.png"));
    act->setText("圆锥度");
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/tolerance/Sphericity.png"));
    act->setText("球度");
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/tolerance/ARC.png"));
    act->setText("圆弧度");
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/tolerance/LineProfile.png"));
    act->setText("线轮廓度");
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/tolerance/PlaneProfile.png"));
    act->setText("面轮廓度");
    pannel->addLargeAction(act);

    pannel = page->addPannel("位置公差");

    act = new QAction(this);
    act->setIcon(QIcon(":/res/tolerance/Parallelism.png"));
    act->setText("平行度");
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/tolerance/Verticality.png"));
    act->setText("垂直度");
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/tolerance/Gradient.png"));
    act->setText("倾斜度");
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/tolerance/Position.png"));
    act->setText("点位置度");
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/tolerance/Position.png"));
    act->setText("线位置度");
    pannel->addSmallAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/tolerance/Position.png"));
    act->setText("面位置度");
    pannel->addSmallAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/tolerance/Position.png"));
    act->setText("复合位置度");
    pannel->addSmallAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/tolerance/Concentricity.png"));
    act->setText("同心度");
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/tolerance/Axiality.png"));
    act->setText("同轴度");
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/tolerance/Symmetry.png"));
    act->setText("对称度");
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/tolerance/Runout.png"));
    act->setText("圆跳动");
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/tolerance/TotalRunout.png"));
    act->setText("全跳动");
    pannel->addLargeAction(act);
}

void MainWindow::createCategoryConstruct(SARibbonCategory *page)
{
    SARibbonPannel* pannel = page->addPannel("");

    QAction* act = new QAction(this);
    act->setIcon(QIcon(":/res/construct/Point.png"));
    act->setText("点");
    connect(act,&QAction::triggered,this,[=] {
        constructWidget->setConstrutObject(0);
        constructWidget->show();
    });
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/construct/Line.png"));
    act->setText("直线");
    connect(act,&QAction::triggered,this,[=] {
        constructWidget->setConstrutObject(1);
        constructWidget->show();
    });
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/construct/Plane.png"));
    act->setText("平面");
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/construct/Circle.png"));
    act->setText("圆");
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/construct/Curve.png"));
    act->setText("曲线");
    pannel->addLargeAction(act);

    pannel->addSeparator();

    act = new QAction(this);
    act->setIcon(QIcon(":/res/construct/Ellipse.png"));
    act->setText("椭圆");
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/construct/Grove.png"));
    act->setText("方槽");
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/construct/RGrove.png"));
    act->setText("圆槽");
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/construct/Cylinder.png"));
    act->setText("圆柱");
    connect(act,&QAction::triggered,this,[=] {
        constructWidget->setConstrutObject(2);
        constructWidget->show();
    });
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/construct/Cone.png"));
    act->setText("圆锥");
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/construct/Sphere.png"));
    act->setText("球");
    connect(act,&QAction::triggered,this,[=] {
        constructWidget->setConstrutObject(3);
        constructWidget->show();
    });
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/construct/LZ.png"));
    act->setText("棱柱");
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/construct/YH.png"));
    act->setText("圆环");
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/construct/SFace.png"));
    act->setText("曲面");
    pannel->addLargeAction(act);
}

void MainWindow::createCategoryReport(SARibbonCategory *page)
{
    SARibbonPannel* pannel = page->addPannel("");

    QAction* act = new QAction(this);
    act->setIcon(QIcon(":/res/report/fresh.png"));
    act->setText("显示报告");
    connect(act,&QAction::triggered,this,&MainWindow::on_showReport);
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/report/save.png"));
    act->setText("保存报告");
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/report/print.png"));
    act->setText("打印报告");
    pannel->addLargeAction(act);

    pannel->addSeparator();

    act = new QAction(this);
    act->setIcon(QIcon(":/res/report/text.png"));
    act->setText("文本");
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/report/analize.png"));
    act->setText("形状分析");
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/report/over.png"));
    act->setText("超差报告");
    pannel->addLargeAction(act);
}

void MainWindow::createCategoryVision(SARibbonCategory *page)
{
    SARibbonPannel* pannel = page->addPannel("显示特征");

    QAction* act = new QAction(this);
    act->setIcon(QIcon(":/res/vision/CAD.png"));
    act->setText("显示CAD");
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/vision/gdt.png"));
    act->setText("显示GDT");
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/vision/rotate.png"));
    act->setText("显示旋转部件");
    pannel->addLargeAction(act);

    pannel = page->addPannel("显示标签");

    act = new QAction(this);
    act->setIcon(QIcon(":/res/vision/feat.png"));
    act->setText("显示特征标签ID");
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/vision/size.png"));
    act->setText("显示尺寸标签ID");
    pannel->addLargeAction(act);

    pannel = page->addPannel("其他");

    act = new QAction(this);
    act->setIcon(QIcon(":/res/vision/cloud.png"));
    act->setText("显示所有点云");
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/vision/interact.png"));
    act->setText("显示全部截面");
    pannel->addLargeAction(act);
}

void MainWindow::createCategoryHelp(SARibbonCategory *page)
{
    SARibbonPannel* pannel = page->addPannel("");

    QAction* act = new QAction(this);
    act->setIcon(QIcon(":/res/help/index.png"));
    act->setText("索引");
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/help/userguid.png"));
    act->setText("使用帮助");
    pannel->addLargeAction(act);

    pannel->addSeparator();

    act = new QAction(this);
    act->setIcon(QIcon(":/res/help/newfunc.png"));
    act->setText("新增功能");
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/help/form.png"));
    act->setText("用户论坛");
    pannel->addLargeAction(act);

    pannel->addSeparator();

    act = new QAction(this);
    act->setIcon(QIcon(":/res/help/update.png"));
    act->setText("检查更新");
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/res/help/about.png"));
    act->setText("关于");
    pannel->addLargeAction(act);
}

void MainWindow::on_doConstruct(int constructType)
{
    switch (constructType)
    {
    case 0:
    {
        TopoDS_Shape pntShape = ModelMaker::make_point(constructWidget->Point);
        Handle(AIS_Shape) pntAisModel = new AIS_Shape(pntShape);
        occWidget->getContext()->Display(pntAisModel,Standard_True);
        occWidget->getView()->FitAll();
        addItem(tr("Point"));
        m_displayedShapes.append(pntAisModel);
        addResult(tr("Success"),tr("成功构建点"));
        break;
    }
    case 1:
    {
        TopoDS_Shape lineShape = ModelMaker::make_line(constructWidget->line.startPnt,constructWidget->line.endPnt);
        Handle(AIS_Shape) lineAisModel = new AIS_Shape(lineShape);
        occWidget->getContext()->Display(lineAisModel,Standard_True);
        occWidget->getView()->FitAll();
        addItem(tr("Line"));
        m_displayedShapes.append(lineAisModel);
        addResult(tr("Success"),tr("成功构建直线"));
        break;
    }
    case 2:
    {
        gp_Ax2 cylinderAxis = gp_Ax2(constructWidget->cylinder.position,constructWidget->cylinder.direction);
        TopoDS_Shape cylinderShape = ModelMaker::make_cylinder(cylinderAxis,constructWidget->cylinder.cylinderRadius,constructWidget->cylinder.height);
        Handle(AIS_Shape) cylinderAisModel = new AIS_Shape(cylinderShape);
        occWidget->getContext()->Display(cylinderAisModel,Standard_True);
        occWidget->getView()->FitAll();
        addItem(tr("Cylinder"));
        m_displayedShapes.append(cylinderAisModel);
        addResult(tr("Success"),tr("成功构建圆柱"));
        break;
    }
    case 3:
    {
        TopoDS_Shape ballShape = ModelMaker::make_sphere(constructWidget->ball.position,constructWidget->ball.radius);
        Handle(AIS_Shape) ballAisModel = new AIS_Shape(ballShape);
        occWidget->getContext()->Display(ballAisModel,Standard_True);
        occWidget->getView()->FitAll();
        addItem(tr("Sphere"));
        m_displayedShapes.append(ballAisModel);
        addResult(tr("Success"),tr("成功构建球"));
    }
    }
}

void MainWindow::on_setItemColor()
{
    if(occWidget->getContext()->HasSelectedShape())
    {
        QColor aColor ;
        Handle(AIS_InteractiveObject) Current = occWidget->getContext()->SelectedInteractive() ;
        if ( Current->HasColor () )
        {
            Quantity_Color aShapeColor;
            occWidget->getContext()->Color( Current, aShapeColor );
            aColor.setRgb( (Standard_Integer)(aShapeColor.Red() * 255), (Standard_Integer)(aShapeColor.Green() * 255),
                           (Standard_Integer)(aShapeColor.Blue() * 255));
        }
        else
            aColor.setRgb( 255, 255, 255 );

        QColor aRetColor = QColorDialog::getColor( aColor );
        if ( aRetColor.isValid() )
        {
            Quantity_Color color( aRetColor.red() / 255., aRetColor.green() / 255.,
                                  aRetColor.blue() / 255., Quantity_TOC_RGB );

            Current->SetColor(color);
            Current->Redisplay(Standard_True);

            addResult(tr("Success"),tr("切换颜色成功"));
        }
    }
    else
    {
        QMessageBox::critical(this,tr("错误"),tr("选择对象后再操作!"));
        addResult(tr("Failed"),tr("切换颜色失败"));
    }
}

void MainWindow::on_deleteSelected()
{
    if(occWidget->getContext()->HasSelectedShape())
    {
        occWidget->getContext()->EraseSelected(Standard_True);
    }
    else
        return;
}

void MainWindow::on_importModel()
{
    QString modelFileName = QFileDialog::getOpenFileName(this,tr("选择模型文件"),"","*.step *.STEP *.stp *.STP\n"
                                                                              "*.iges *.IGES *.igs *.IGS\n"
                                                                              "*.stl *.STL\n"
                                                                              "*.brep *.brp");
    if(modelFileName.isEmpty())
        return;

    TCollection_AsciiString theAscii(modelFileName.toUtf8().data());

    QFileInfo info(modelFileName);

    std::shared_ptr<XSControl_Reader> aReader;
    if(info.suffix()=="step"||info.suffix()=="stp"||info.suffix()=="STEP"||info.suffix()=="STP")
    {
        aReader = std::make_shared<STEPControl_Reader>();
    }
    else if(info.suffix()=="iges"||info.suffix()=="igs"||info.suffix()=="IGES"||info.suffix()=="IGS")
    {
        aReader = std::make_shared<IGESControl_Reader>();
    }
    else if(info.suffix()=="stl"||info.suffix()=="STL")
    {
        TopoDS_Shape aShape;
        StlAPI::Read(aShape,theAscii.ToCString());
        Handle(AIS_Shape) anAIS = new AIS_Shape(aShape);
        addItem(typeName(aShape.ShapeType()));
        m_displayedShapes.append(anAIS);
        occWidget->getContext()->Display(anAIS,false);
        return;
    }
    else if(info.suffix()=="brep"||info.suffix()=="brp")
    {
        TopoDS_Shape aShape;
        BRep_Builder aBuilder;
        BRepTools::Read(aShape,theAscii.ToCString(),aBuilder);
        Handle(AIS_Shape) anAIS = new AIS_Shape(aShape);
        anAIS->Attributes()->SetFaceBoundaryDraw(true);
        anAIS->Attributes()->SetFaceBoundaryAspect(
                    new Prs3d_LineAspect(Quantity_NOC_BLACK, Aspect_TOL_SOLID, 1.));
        anAIS->Attributes()->SetIsoOnTriangulation(true);
        addItem(typeName(aShape.ShapeType()));
        m_displayedShapes.append(anAIS);
        occWidget->getContext()->Display(anAIS,false);
        return;
    }

    if(!aReader->ReadFile(theAscii.ToCString()))
    {
        QMessageBox::critical(this,tr("错误"),tr("模型导入失败!"));
        addResult(tr("Failed"),tr("读取文件出错!"));
        return;
    }

    for(int i=0;i<aReader->TransferRoots();++i)
    {
        Handle(AIS_Shape) anAIS = new AIS_Shape(aReader->Shape(i+1));
        anAIS->Attributes()->SetFaceBoundaryDraw(true);
        anAIS->Attributes()->SetFaceBoundaryAspect(
                    new Prs3d_LineAspect(Quantity_NOC_BLACK, Aspect_TOL_SOLID, 1.));
        anAIS->Attributes()->SetIsoOnTriangulation(true);
        addItem(typeName(aReader->Shape(i+1).ShapeType()));
        m_displayedShapes.append(anAIS);
        occWidget->getContext()->Display(anAIS,false);
    }

    occWidget->getView()->FitAll();

    itemTree->setCurrentItem(itemTree->topLevelItem(0)->child(0));
    addResult(tr("Success"),tr("成功导入模型"));
}

void MainWindow::on_exportModel()
{
    if(m_displayedShapes.isEmpty())
    {
        QMessageBox::information(this,tr("提示"),tr("场景内容为空!"));
        addResult(tr("Failed"),tr("导出内容为空!"));
        return;
    }

    QString modelFileName = QFileDialog::getSaveFileName(this,tr("选择要保存的位置"),"","*.step *.STEP *.stp *.STP\n"
                                                                                "*.iges *.IGES *.igs *.IGS\n"
                                                                                "*.brep");
    if(modelFileName.isEmpty())
        return;

    QFileInfo info(modelFileName);
    TCollection_AsciiString theAscii(modelFileName.toUtf8().data());
    if(info.suffix()=="step"||info.suffix()=="stp"||info.suffix()=="STEP"||info.suffix()=="STP")
    {
        STEPControl_Writer stepWriter;

        for(int i=0;i<m_displayedShapes.size();i++)
        {
            Handle(AIS_Shape) anIS = Handle(AIS_Shape)::DownCast (m_displayedShapes.at(i));
            if (anIS.IsNull())
            {
                return;
            }

            if (stepWriter.Transfer (anIS->Shape(), STEPControl_AsIs) != IFSelect_RetDone)
            {
                QMessageBox::critical(this,tr("错误"),tr("模型导出失败!"));
                addResult(tr("Failed"),tr("读取场景内容出错!"));
                return;
            }
            if(!stepWriter.Write (theAscii.ToCString()))
            {
                QMessageBox::critical(this,tr("错误"),tr("模型导出失败!"));
                addResult(tr("Failed"),tr("写入文件出错!"));
                return;
            }
        }
    }
    else if(info.suffix()=="iges"||info.suffix()=="igs"||info.suffix()=="IGES"||info.suffix()=="IGS")
    {
        IGESControl_Writer igesWriter(Interface_Static::CVal ("XSTEP.iges.unit"),
                                      Interface_Static::IVal ("XSTEP.iges.writebrep.mode"));
        IGESControl_Controller::Init();
        for (int j=0;j<m_displayedShapes.size();j++)
        {
            Handle(AIS_Shape) anIS = Handle(AIS_Shape)::DownCast (m_displayedShapes.at(j));
            if (anIS.IsNull())
            {
                return;
            }
            igesWriter.AddShape (anIS->Shape());
        }

        igesWriter.ComputeModel();
        if(!igesWriter.Write (theAscii.ToCString()))
        {
            QMessageBox::critical(this,tr("错误"),tr("模型导出失败!"));
            addResult(tr("Failed"),tr("写入文件出错!"));
            return;
        }
    }
    else if(info.suffix() == "brep")
    {
        for(int i=0;i<m_displayedShapes.size();i++)
        {
            Handle(AIS_Shape) anIS = Handle(AIS_Shape)::DownCast (m_displayedShapes.at(i));
            if (anIS.IsNull())
            {
                return;
            }

            BRepTools::Write(anIS->Shape(),theAscii.ToCString());
        }
    }
    addResult(tr("Success"),tr("导出模型成功!"));
}

void MainWindow::on_addMeasurePnt(const gp_Pnt &pnt)
{
    SetRobotJoint();
    if(mCommonData->getRecordFlag() == true){

        //实际测点与关节臂末端不一致，显示虚测点
        TopoDS_Shape aPt = ModelMaker::make_point(robot->GetTCP());
        Handle(AIS_Shape) ptAIS = new AIS_Shape(aPt);
        ptAIS->Attributes()->SetPointAspect(new Prs3d_PointAspect(Aspect_TOM_O_STAR,Quantity_NOC_GREEN,1));
        occWidget->getContext()->Display(ptAIS,Standard_True);

        if(!measDock->isHidden()) {
            measWid->AppendPnt(pnt);
            measWid->AppendVirtualPnt(robot->GetTCP());
        }
    }    
}

void MainWindow::on_measueAction()
{
    QAction* act = (QAction*)sender();
    if(act) {
        measWid->SetTitle(act->text());
        measDock->show();
        if(act->text() == "直线") {
            currentType = GeometryType::LINE;
        }
        else if(act->text() == "平面") {
            currentType = GeometryType::PLANE;
        }
        else if(act->text() == "圆") {
            currentType = GeometryType::CIRCLE;
        }
        else if(act->text() == "球") {
            currentType = GeometryType::SPHERE;
        }
        else if(act->text() == "圆柱") {
            currentType = GeometryType::CYLINDER;
        }
        else if(act->text() == "圆锥") {
            currentType = GeometryType::CONE;
        }
        else if(act->text() == "特征间距") {
            currentType = GeometryType::DISTANCE;
            measWid->SetFeatureSize(2);
        }
    }
}

void MainWindow::on_showReport()
{
    DMISReport *report = new DMISReport;
    report->AddPixmap(occWidget->DumpToMap());
    report->AddItems(mDMISItems);
    QPixmap result = report->GetReport();
    delete report;

    LB_ImageViewer *wid = new LB_ImageViewer;
    wid->SetPixmap(result);
    wid->showMaximized();
    wid->setAttribute(Qt::WA_DeleteOnClose);
}

void MainWindow::addItem(QString itemName)
{
    QTreeWidgetItem *tmpItem = new QTreeWidgetItem;
    tmpItem->setText(0,itemName+tr("%1").arg(itemTree->topLevelItem(0)->childCount()));
    itemTree->topLevelItem(0)->addChild(tmpItem);
}

void MainWindow::addItem(MeasureItem *aItm)
{
    QTreeWidgetItem *tmpItem = new QTreeWidgetItem;
    tmpItem->setText(0,aItm->getID());
    switch(aItm->Type()) {
    case GeometryType::LINE:
    {
        tmpItem->setIcon(0,QPixmap(":/res/tolerance/Linearity.png"));
        break;
    }
    case GeometryType::PLANE:
    {
        tmpItem->setIcon(0,QPixmap(":/res/tolerance/Planarity.png"));
        break;
    }
    case GeometryType::CIRCLE:
    {
        tmpItem->setIcon(0,QPixmap(":/res/tolerance/Circularity.png"));
        break;
    }
    case GeometryType::SPHERE:
    {
        tmpItem->setIcon(0,QPixmap(":/res/tolerance/Sphericity.png"));
        break;
    }
    case GeometryType::CYLINDER:
    {
        tmpItem->setIcon(0,QPixmap(":/res/tolerance/Cylindricity.png"));
        break;
    }
    case GeometryType::CONE:
    {
        tmpItem->setIcon(0,QPixmap(":/res/tolerance/Conicity.png"));
        break;
    }
    case GeometryType::DISTANCE:
    {
        tmpItem->setIcon(0,QPixmap(":/res/tolerance/Position.png"));
        break;
    }
    case GeometryType::UNKNOWN:
        break;
    }

    itemTree->topLevelItem(1)->addChild(tmpItem);
    mDMISItems.append(aItm);
}

void MainWindow::addResult(QString resultType, QString result)
{
    resultStatus->showMessage(resultType + " : " +result, 3000);
}

QString MainWindow::typeName(int TopoShape)
{
    switch (TopoShape) {
    case 0:return QString("COMPOUND");
    case 1:return QString("COMPSOLID");
    case 2:return QString("SOLID");
    case 3:return QString("SHELL");
    case 4:return QString("FACE");
    case 5:return QString("WIRE");
    case 6:return QString("EDGE");
    case 7:return QString("VERTEX");
    case 8:return QString("SHAPE");
    }
    return QString("SHAPE");
}

void MainWindow::SetRobotJoint()
{
    robot->PerformFK({mCommonData->getAxisAngle(0),
                     mCommonData->getAxisAngle(1),
                     mCommonData->getAxisAngle(2),
                     mCommonData->getAxisAngle(3),
                     mCommonData->getAxisAngle(4),
                     mCommonData->getAxisAngle(5)});
    occWidget->getView()->Update();
}

void MainWindow::readStructSlots(){
    if(mImportFile == nullptr){
        mImportFile = new importStructFile(this);
    }
    else{
        delete mImportFile;
        mImportFile = new importStructFile(this);
    }
    bool result = mImportFile->importFileMain(false,commonData::structureType);
    if(result){
        QString coffInfo = QString("%1%2%3").arg("结构参数导入成功(").arg(commonData::SizeOFStruct).arg("项）");
        QMessageBox::about(this,tr("参数导入提醒"),coffInfo);
    }
    else {
        QMessageBox::about(this,tr("参数导入提醒"),tr("结构参数导入失败！"));
    }
}

void MainWindow::readCoeffSlots(){
    if(mImportFile == nullptr){
        mImportFile = new importCoeffFile(this);
    }
    else{
        delete mImportFile;
        mImportFile = new importCoeffFile(this);
    }
    bool result = mImportFile->importFileMain(false,commonData::coeffType);
    if(result){
        QString coffInfo = QString("%1%2%3").arg("偏心误差导入成功(").arg(mCommonData->getCoeffParameterSize()).arg("项）");
        QMessageBox::about(this,tr("参数导入提醒"),coffInfo);
    }
    else {
        QMessageBox::about(this,tr("参数导入提醒"),tr("偏心误差导入失败！"));
    }
}

void MainWindow::test()
{
    gp_Pnt pnt(QRandomGenerator::global()->bounded(50,100),
               QRandomGenerator::global()->bounded(50,100),
               QRandomGenerator::global()->bounded(50,100));
    robot->PerformFK({static_cast<double>(QRandomGenerator::global()->bounded(80,90)),
                      static_cast<double>(QRandomGenerator::global()->bounded(80,90)),
                      static_cast<double>(QRandomGenerator::global()->bounded(80,90)),
                      static_cast<double>(QRandomGenerator::global()->bounded(80,90)),
                      static_cast<double>(QRandomGenerator::global()->bounded(80,90)),
                      static_cast<double>(QRandomGenerator::global()->bounded(80,90))});

    TopoDS_Shape aPt = ModelMaker::make_point(robot->GetTCP());
    Handle(AIS_Shape) ptAIS = new AIS_Shape(aPt);
    ptAIS->Attributes()->SetPointAspect(new Prs3d_PointAspect(Aspect_TOM_O_STAR,Quantity_NOC_GREEN,1));
    occWidget->getContext()->Display(ptAIS,Standard_True);
    if(!measDock->isHidden()) {
        measWid->AppendPnt(pnt);
        measWid->AppendVirtualPnt(robot->GetTCP());
    }

    occWidget->getView()->Update();
}

void MainWindow::chooseConnectModel(){
    QMessageBox box(QMessageBox::Warning, "连接方式选择","请连接方式选择：");
    box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    box.setButtonText(QMessageBox::Yes, QString("Normal"));
    box.setButtonText(QMessageBox::No, QString("Unique ID"));
    int ret = box.exec();

    if(ret == QMessageBox::Yes){
        decodeModelChoose();
        mUniqueID->manulConnect();
    }
    else if(ret == QMessageBox::No){
        mUniqueID->uniqueIDMain();
    }
}

void MainWindow::decodeModelChoose(){
    QMessageBox chooseBox(QMessageBox::Warning, "正常模式","请选择通讯位数：");
    chooseBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    chooseBox.setButtonText(QMessageBox::Yes, QString("26位"));
    chooseBox.setButtonText(QMessageBox::No, QString("50位"));
    int chooseRet = chooseBox.exec();
    if(chooseRet == QMessageBox::Yes){
        //26位
        mCommonData->setFlagFor50Bytes(false);
        mCommonData->setFlagFor26Bytes(true);
    }
    else if(chooseRet == QMessageBox::No){
        //50位
        mCommonData->setFlagFor50Bytes(true);
        mCommonData->setFlagFor26Bytes(false);
    }
}

void MainWindow::closeEvent(QCloseEvent *event){
    int ret = QMessageBox::question(this,"关闭提示","是否关闭窗口？",QMessageBox::Yes,QMessageBox::No);
    if(ret == QMessageBox::Yes){
        mCalibration->dropImageThread();
        event->accept();
    }
    else if(ret == QMessageBox::No){
        event->ignore();
    }
}

void MainWindow::initialMethod(){
    if(mCommonData->getCoeffParameterSize() == 0||mCommonData->getCoeffParameterItem(0) == 0){
        QMessageBox::about(this,"提示","偏心参数需要重新重新导入");
    }
    if(mCommonData->getStructParameterItem(0) == 0){
        QMessageBox::about(this,"提示","结构参数需要重新重新导入");
    }
}

void MainWindow::setMethod(Method * arg){
    Decode::removeAll();//清空队列
    Decode::addMethodObject(arg);//添加
    initialMethod();
    arg->pretreatment();//UI界面展示等等
}
