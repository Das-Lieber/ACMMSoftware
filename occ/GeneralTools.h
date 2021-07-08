#pragma once
#include <Geom_Surface.hxx>
#include <gp_Pnt.hxx>
#include <gp_Circ.hxx>
#include <gp_Ax2.hxx>
#include <TopoDS_Shape.hxx>

#include <QList>
#include <list>
#include <map>

using namespace std;
 class GeneralTools
{
public:
    GeneralTools(void);

    static bool FitLine(list<gp_Pnt> pts,gp_Lin &line);
    static bool FitPlane(list<gp_Pnt> pts, gp_Pln &plane);
    static bool FitCylinder(QList<gp_Pnt> pts, gp_Cylinder &cylinder);
    static bool FitCone(list<gp_Pnt> pts, gp_Cone &cone);
    static bool FitEllips(list<gp_Pnt> pts,gp_Elips& aElips);
    static bool FitCone(Handle(Geom_Surface) aSurface,gp_Ax1& axR);
    static bool FitCicle(list<gp_Pnt2d> pts2d,gp_Pnt2d &Center, Standard_Real& Radius);
    static bool FitCicle(list<gp_Pnt> pts,gp_Circ& cir,double &error);
    static bool FitSphere(list<gp_Pnt> pts,gp_Sphere& sphere);

    static double EvaluateLine(const QList<gp_Pnt> &pts, const gp_Lin &line);
    static double EvaluatePlane(const QList<gp_Pnt> &pts, const gp_Pln &plane);
    static double EvaluateSphere(const QList<gp_Pnt> &pts, const gp_Sphere &sphere);
    static double EvaluateCylinder(const QList<gp_Pnt> &pts, const gp_Cylinder &cylinder);

    static list<TopoDS_Shape> HandleProbeStyliShape(TopoDS_Shape shape);
    static TopoDS_Shape MakeCompoundFromShapes(list<TopoDS_Shape> origalShapes);
    static TopoDS_Shape RemoveDegenerateEdge(TopoDS_Shape oldshape);

 private:
    static list<gp_Pnt2d> TranslatePntToPnt2d(list<gp_Pnt> points, gp_Ax2 coordAx);
    static gp_Pnt TranslatePnt2dToPnt(gp_Pnt2d point, gp_Ax2 coordAx);
    static bool GetCicle(Handle(Geom_Curve) aCurve,gp_Circ& cir);
 };
