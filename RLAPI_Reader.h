#ifndef RLAPI_READER_H
#define RLAPI_READER_H

#include <QObject>

#include <AIS_Shape.hxx>
#include <XCAFPrs_AISObject.hxx>
#include <XCAFDoc_ColorTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>

class RobotLink;

class RLAPI_Reader
{
public:
    //! constructor
    RLAPI_Reader();

    //! deconstructor
    ~RLAPI_Reader();

    //! read the ACMM links under the path
    QList<RobotLink*> ReadJointModels(const QString &aFilePath);
    QList<gp_Trsf> ParseJointAssemble();

private:
    Handle(XCAFDoc_ColorTool) aColorTool;
    Handle(XCAFDoc_ShapeTool) aShapeTool;

    //! use the OCC library to read a step/iges/brep format file
    QList<Handle(XCAFPrs_AISObject)> ReadAModelFile(const QString &aModelFileName);

    void visit(const TDF_Label &label, QList<Handle(XCAFPrs_AISObject)> &result);
};

#endif // RLAPI_READER_H
