#include "modelmaker.h"

ModelMaker::ModelMaker(QObject *parent) : QObject(parent)
{
}

TopoDS_Shape ModelMaker::make_cylinder(gp_Ax2 Axis, Standard_Real R, Standard_Real H)
{
    TopoDS_Shape t_topo_cylinder = BRepPrimAPI_MakeCylinder(Axis,R , H).Shape();
    return t_topo_cylinder;
}

TopoDS_Shape ModelMaker::make_sphere(gp_Pnt center, Standard_Real R)
{
    TopoDS_Shape t_topo_sphere = BRepPrimAPI_MakeSphere(center,R).Shape();
    return t_topo_sphere;
}

TopoDS_Shape ModelMaker::make_line(gp_Pnt Pnt1, gp_Pnt Pnt2)
{
     Handle(Geom_TrimmedCurve) geom_line =  GC_MakeSegment(Pnt1,Pnt2).Value();
     TopoDS_Shape t_topo_line = BRepBuilderAPI_MakeEdge(geom_line);
     return t_topo_line;
}

TopoDS_Shape ModelMaker::make_point(gp_Pnt point)
{
    BRep_Builder builder;
    TopoDS_Vertex vertex;
    builder.MakeVertex(vertex,point,0.001);//误差设为0.001
    return std::move(vertex);
}

TopoDS_Shape ModelMaker::make_BSpline(TColgp_Array1OfPnt aPoints)
{
    const Handle(TColgp_HArray1OfPnt) aPnts = new TColgp_HArray1OfPnt(aPoints);
    GeomAPI_Interpolate ptb(aPnts,Standard_False,1e-6);
    ptb.Perform();
    Handle(Geom_BSplineCurve) aBSplineCurve = ptb.Curve();
    TopoDS_Edge ed1 = BRepBuilderAPI_MakeEdge(aBSplineCurve);
    return std::move(ed1);
}

TopoDS_Shape ModelMaker::make_polygon(const QList<gp_Pnt> &pnts)
{
    TopoDS_Compound result;
    BRep_Builder aBuilder;
    aBuilder.MakeCompound(result);
    for(int i=0;i<pnts.size();++i) {
        int next = (i+1)%pnts.size();
        TopoDS_Shape seg = make_line(pnts[i],pnts[next]);
        aBuilder.Add(result,seg);
    }

    return result;
}
