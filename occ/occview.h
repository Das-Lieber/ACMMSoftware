#ifndef OCCVIEW_H
#define OCCVIEW_H

#include <QWidget>
#include <QApplication>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QStyleFactory>
#include <QRubberBand>

#include <AIS_Shape.hxx>
#include <AIS_InteractiveContext.hxx>
#include <Aspect_Handle.hxx>
#include <Aspect_DisplayConnection.hxx>
#include <Graphic3d_GraphicDriver.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <V3d_View.hxx>
#include <WNT_Window.hxx>
#include <AIS_Manipulator.hxx>
#include <AIS_ViewCube.hxx>

#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TColgp_Array1OfPnt2d.hxx>
#include <BRep_Tool.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <AIS_Shape.hxx>
#include <IntCurvesFace_ShapeIntersector.hxx>
#include <GeomAPI_ExtremaCurveCurve.hxx>

class LabelWidget;
class MeasureItem;
enum class GeometryType;

using namespace std;
class occView : public QWidget
{
    Q_OBJECT

public:
    explicit occView(QWidget *parent = nullptr);

    Handle(V3d_View) getView()
    {
        return m_view;
    }

    Handle(AIS_InteractiveContext) getContext()
    {
        return m_context;
    }

    Handle(AIS_Manipulator) getManipulator()
    {
        return m_manipulator;
    }

    void getWindowSize(int &x, int &y);
    QPixmap DumpToMap();
private:
    Handle(AIS_InteractiveContext) m_context;
    Handle(V3d_Viewer) m_viewer;
    Handle(V3d_View) m_view;
    Handle(Graphic3d_GraphicDriver) m_graphicDriver;
    Handle(AIS_Manipulator) m_manipulator;
    Standard_Integer midBtn_x;
    Standard_Integer midBtn_y;

    QPoint startPnt;
    QPoint endPnt;
    QRubberBand *m_rubberBand;

protected:
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);

    QPaintEngine *paintEngine() const;

/*************************************************************************************
*                                  生成标签、引线
* ***********************************************************************************/
public:
    gp_Pnt point2dTo3d(Standard_Real x, Standard_Real y);

    void createLabel(QList<QList<gp_Pnt> > plist, QList<QList<gp_Pnt> > virtData, QList<QList<Handle(AIS_InteractiveObject) > > shapes, const GeometryType &typ);
    void drawLabelLeadLine(gp_Pnt P1, gp_Pnt P2);
    void AutoArrangeLabel();
    void showLabelandLine(vector<pair<double,double>> &line,
                          vector<pair<double,double>> &labelXY, vector<QString> &labelName);
    void clearLabelAndLine();

private:    
    list<gp_Pnt> leadLineStart;//存标签引线
    list<pair<LabelWidget*,gp_Pnt>> labelList;//存产生的测量标签
    vector<Handle(AIS_Shape)> lineVector;//


    /**************************************************************************************
 *                                     标签排布
 * ***********************************************************************************/
public:
    void arrangeLabelFew(list<pair<LabelWidget*,gp_Pnt>> labelList);
    void arrangeLabelMore(list<pair<LabelWidget*,gp_Pnt>> labelList);

    struct CmpByValue
    {
        bool operator()(const pair<LabelWidget *,int> l1, const pair<LabelWidget *,int> l2)
        {
            return l1.second < l2.second;
        }
    };
    struct CmpByValueLarge
    {
        bool operator()(const pair<LabelWidget*,int> &l1, const pair<LabelWidget*,int> &l2)
        {
            return l1.second > l2.second;
        }
    };

private:
    int maxXWindow,maxYWindow;
    int mininterval = 15;

    /***************************************************************************************
 *
 * ************************************************************************************/

signals:
    void newMeasureItem(MeasureItem *item);
    void pickPixel(int x, int y);
    void selectShapeChanged(const TopoDS_Shape selected);
};

#endif // OCCVIEW_H
