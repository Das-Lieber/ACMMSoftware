#ifndef MODELMAKER_H
#define MODELMAKER_H

#include <QObject>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <GC_MakeSegment.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRep_Builder.hxx>
#include <GeomAPI_Interpolate.hxx>
#include <Geom_BSplineCurve.hxx>

class ModelMaker : public QObject
{
    Q_OBJECT
public:
    explicit ModelMaker(QObject *parent = nullptr);

    //生成圆柱
    static TopoDS_Shape make_cylinder(gp_Ax2 Axis,Standard_Real R,  Standard_Real H);
    //球体
    static TopoDS_Shape make_sphere(gp_Pnt center,Standard_Real R);
    //线
    static TopoDS_Shape make_line(gp_Pnt Pnt1, gp_Pnt Pnt2);
    //点
    static TopoDS_Shape make_point(gp_Pnt point);
    //曲线，用于生成路径线
    static TopoDS_Shape make_BSpline(TColgp_Array1OfPnt aPoints);
    //折线，一个测量特征的所有测点
    static TopoDS_Shape make_polygon(const QList<gp_Pnt> &pnts);
};

#endif // MODELMAKER_H
