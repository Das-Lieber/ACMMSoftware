#include "RLAPI_Reader.h"

#include <QDir>
#include <QFileInfoList>
#include <QTextStream>

#include <STEPCAFControl_Reader.hxx>
#include <TDF_Label.hxx>
#include <TDF_LabelSequence.hxx>
#include <TDocStd_Document.hxx>
#include <TDataStd_Name.hxx>
#include <TDF_ChildIterator.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFApp_Application.hxx>
#include <Graphic3d_MaterialAspect.hxx>

#include "occ/RobotLink.h"

RLAPI_Reader::RLAPI_Reader()     
{
}

RLAPI_Reader::~RLAPI_Reader()
{
}

QList<RobotLink *> RLAPI_Reader::ReadJointModels(const QString &aFilePath)
{
    QList<RobotLink *> Links;

    QDir jointFileDir(aFilePath);
    QStringList theDirFilter;
    theDirFilter<<"*.step"<<"*.stp";
    QFileInfoList aInfoList = jointFileDir.entryInfoList(theDirFilter,QDir::Files);
    for(int i=0;i<aInfoList.size();i++)
    {
        QString aFileName = jointFileDir.absolutePath()+"/"+aInfoList.at(i).fileName();
        RobotLink* alink = new RobotLink;
        alink->RShapes = ReadAModelFile(aFileName);
        Links.append(alink);
    }

    return Links;
}

QList<gp_Trsf> RLAPI_Reader::ParseJointAssemble()
{
    QList<gp_Trsf> result;

    QFile aFile(":/res/ACMM.wrl");
    aFile.open(QIODevice::ReadOnly);
    QTextStream aStream(&aFile);
    std::size_t translationIndex = 0;
    QList<double> transArgs;
    QRegExp aExp("(-?\\d\\d*\\.\\d+)|(-?\\d+)");
    while(!aStream.atEnd())
    {
        QString aLine = aStream.readLine();
        if(!aLine.contains("translation"))
            continue;

        int pos = 0;

        while ((pos = aExp.indexIn(aLine, pos)) != -1) {
            transArgs << aExp.cap(0).toDouble();
            pos += aExp.matchedLength();
        }
        translationIndex++;
    }
    aFile.close();

    gp_Trsf aJointTrsf;
    gp_Pnt aCenterPnt;
    for(std::size_t i=0;i<translationIndex;++i)
    {
        aJointTrsf.SetValues(1,0,0,transArgs.at(3*i),
                             0,1,0,transArgs.at(3*i+1),
                             0,0,1,transArgs.at(3*i+2));
        result.append(aJointTrsf);
    }

    return result;
}

QList<Handle(XCAFPrs_AISObject)> RLAPI_Reader::ReadAModelFile(const QString &aModelFileName)
{
    TCollection_AsciiString theAscii(aModelFileName.toUtf8().data());

    STEPCAFControl_Reader aStepReader;
    aStepReader.SetColorMode(true);
    aStepReader.SetNameMode(true);
    aStepReader.SetMatMode(true);
    aStepReader.SetLayerMode(true);
    aStepReader.SetPropsMode(true);

    if(!aStepReader.ReadFile(theAscii.ToCString()))
        return {};

    Handle(TDocStd_Document) aDoc;
    Handle(XCAFApp_Application) anApp = XCAFApp_Application::GetApplication();
    anApp->NewDocument("MDTV-XCAF", aDoc);
    aStepReader.Transfer(aDoc);

    TDF_Label aRootLabel = aDoc->Main();

    aShapeTool = XCAFDoc_DocumentTool::ShapeTool(aRootLabel);
    aColorTool = XCAFDoc_DocumentTool::ColorTool(aRootLabel);

    QList<Handle(XCAFPrs_AISObject)> aLink;

    visit(aRootLabel,aLink);

    return aLink;
}

void RLAPI_Reader::visit(const TDF_Label &label, QList<opencascade::handle<XCAFPrs_AISObject> > &result)
{
    TDF_LabelSequence subShapes;

    if(!aShapeTool->GetSubShapes(label,subShapes)) {
        Handle(XCAFPrs_AISObject) ais_Shape = new XCAFPrs_AISObject(label);
        Graphic3d_MaterialAspect aspect;
        aspect.SetMaterialName(Graphic3d_NOM_METALIZED);
        ais_Shape->SetMaterial(aspect);
        result.append(ais_Shape);
    }

    for(TDF_ChildIterator c(label); c.More(); c.Next())
    {
        visit(c.Value(),result);
    }
}
