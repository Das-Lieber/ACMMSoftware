#include "occview.h"

#include "LabelWidget.h"
#include "MeasureItem.h"
#include "modelmaker.h"

#include <QScreen>
#include <QMessageBox>

#include <ProjLib.hxx>
#include <ElSLib.hxx>

occView::occView(QWidget *parent) : QWidget(parent)
{   
    if (m_context.IsNull())
    {
        Handle(Aspect_DisplayConnection) m_display_donnection = new Aspect_DisplayConnection();
        if (m_graphicDriver.IsNull())
        {
            m_graphicDriver = new OpenGl_GraphicDriver(m_display_donnection);
        }
        WId window_handle =  winId();
        Handle(WNT_Window) wind = new WNT_Window((Aspect_Handle) window_handle);
        m_viewer = new V3d_Viewer(m_graphicDriver);
        m_view = m_viewer->CreateView();
        m_view->SetWindow(wind);

        if (!wind->IsMapped())
        {
            wind->Map();
        }
        m_context = new AIS_InteractiveContext(m_viewer);
        m_viewer->SetDefaultLights();
        m_viewer->SetLightOn();
        m_view->ChangeRenderingParams().Method = Graphic3d_RM_RASTERIZATION;
        m_view->ChangeRenderingParams().IsAntialiasingEnabled = Standard_False;
        m_view->SetBgGradientColors(Quantity_NOC_BLUE4,Quantity_NOC_LIGHTBLUE,Aspect_GFM_VER,Standard_False);
        m_view->TriedronDisplay(Aspect_TOTP_RIGHT_LOWER, Quantity_NOC_BLACK, 0.2, V3d_ZBUFFER);
        m_view->MustBeResized();
        m_context->SetDisplayMode(AIS_Shaded, Standard_True);
    }

    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NoSystemBackground);
    setBackgroundRole( QPalette::NoRole );
    setFocusPolicy( Qt::StrongFocus );
    setMouseTracking(true);

    m_context->HighlightStyle()->SetColor(Quantity_NOC_CYAN1);
    m_context->SelectionStyle()->SetColor(Quantity_NOC_CYAN1);
    m_context->HighlightStyle(Prs3d_TypeOfHighlight_LocalDynamic)->SetColor(Quantity_NOC_CYAN1);
    m_context->HighlightStyle(Prs3d_TypeOfHighlight_LocalSelected)->SetColor(Quantity_NOC_CYAN1);

    m_context->HighlightStyle()->SetDisplayMode(1);
    m_context->SelectionStyle()->SetDisplayMode(1);
    m_context->SelectionStyle()->SetTransparency(0.2f);
    m_context->HighlightStyle()->SetTransparency(0.2f);

    m_context->MainSelector()->SetPickClosest(Standard_False);

    m_rubberBand = new QRubberBand(QRubberBand::Rectangle,this);
    m_rubberBand->setStyle(QStyleFactory::create("windows"));

    m_manipulator = new AIS_Manipulator();
    m_manipulator->SetPart (0, AIS_MM_Scaling, Standard_True);
    m_manipulator->SetPart (1, AIS_MM_Rotation, Standard_True);
    m_manipulator->SetPart (2, AIS_MM_Translation, Standard_True);
    m_manipulator->EnableMode (AIS_MM_Translation);
    m_manipulator->EnableMode (AIS_MM_Rotation);
    m_manipulator->EnableMode (AIS_MM_Scaling);

//    Handle(AIS_ViewCube) aCube = new AIS_ViewCube();
//    aCube->SetAutoStartAnimation(Standard_True);
//    aCube->SetSize(60);
//    aCube->SetFontHeight(13);
//    m_context->Display(aCube,Standard_False);
}

void occView::getWindowSize(int &x,int &y)
{
    Handle(Aspect_Window) window = m_view->Window();
    window->Size(x,y);
    maxXWindow = x;
    maxYWindow = y;
}

QPixmap occView::DumpToMap()
{
    QScreen *screen = QGuiApplication::primaryScreen();
    QPixmap wid = screen->grabWindow(this->winId());
    return wid;
}

QPaintEngine *occView::paintEngine() const
{
    return nullptr;
}

void occView::paintEvent(QPaintEvent *)
{
    m_view->Redraw();
}

void occView::resizeEvent(QResizeEvent *)
{
    getWindowSize(maxXWindow,maxYWindow);
    if( !m_view.IsNull() )
    {
        m_view->MustBeResized();
    }
}

/*************************************************************************************
 *                                    鼠标、滚轮事件
 * **********************************************************************************/
void occView::mousePressEvent(QMouseEvent *event)
{
    if(event->button()==Qt::RightButton)
    {
        m_view->StartRotation(event->x(),event->y());
    }
    else if(event->button()==Qt::LeftButton)
    {
        m_context->MoveTo(event->pos().x(),event->pos().y(),m_view,Standard_True);
        startPnt = event->pos();

        if(!m_manipulator->IsAttached())
            m_rubberBand->setGeometry(QRect(startPnt,QSize(1,1)));
        else
            m_manipulator->StartTransform(event->pos().x(),event->pos().y(),m_view);
    }
    else if(event->button() == Qt::MidButton)
    {
        midBtn_x=event->x();
        midBtn_y=event->y();
    }
}

void occView::mouseReleaseEvent(QMouseEvent *event)
{
    unsetCursor();
    m_context->MoveTo(event->pos().x(),event->pos().y(),m_view,Standard_True);

    if(event->button()==Qt::LeftButton)
    {
        if(!m_rubberBand->isHidden()&&!m_manipulator->IsAttached())
            m_rubberBand->hide();

        if(m_manipulator->IsAttached())
            m_manipulator->StopTransform();

        if(event->pos()==startPnt)
        {
            AIS_StatusOfPick t_pick_status = AIS_SOP_NothingSelected;
            if(qApp->keyboardModifiers()==Qt::ControlModifier)
            {
                t_pick_status = m_context->ShiftSelect(true);
            }
            else
            {
                t_pick_status = m_context->Select(true);
            }

            if(t_pick_status == AIS_SOP_OneSelected || t_pick_status == AIS_SOP_SeveralSelected)
            {
                emit selectShapeChanged(m_context->SelectedShape());
                m_context-> InitSelected();
            }

            emit pickPixel(event->x(),event->y());
        }
        else
        {
            m_context->Select(startPnt.x(),startPnt.y(),event->x(),event->y(),m_view,Standard_True);
            m_context-> InitSelected();
        }
    }
}

void occView::mouseMoveEvent(QMouseEvent *event)
{
    if(event->buttons()&Qt::RightButton)
    {
        setCursor(Qt::ClosedHandCursor);
        m_view->Rotation(event->x(),event->y());

        AutoArrangeLabel();
    }
    else if(event->buttons()&Qt::MidButton)
    {
        setCursor(Qt::SizeAllCursor);
        m_view->Pan(event->pos().x()-midBtn_x,midBtn_y-event->pos().y());
        midBtn_x=event->x();
        midBtn_y=event->y();

        AutoArrangeLabel();
    }
    else if(event->buttons()&Qt::LeftButton)
    {
        if(!m_manipulator->IsAttached())
        {
            m_rubberBand->show();
            m_rubberBand->setGeometry(QRect(startPnt,event->pos()).normalized());
        }
        else
        {
            m_manipulator->Transform(event->pos().x(),event->pos().y(),m_view);
            m_view->Redraw();
        }
    }
    else
    {
        m_context->MoveTo(event->pos().x(),event->pos().y(),m_view,Standard_True);
    }
}

void occView::wheelEvent(QWheelEvent *event)
{
    m_view->StartZoomAtPoint(event->pos().x(),event->pos().y());
    m_view->ZoomAtPoint(0, 0, event->angleDelta().y(), 0);

    AutoArrangeLabel();
}

/*************************************************************************************
 *                               生成标签、引线
 *可以移植到主程序
 * **********************************************************************************/

/*************************************************
 * 功能：二维转三维
 * **********************************************/
gp_Pnt occView::point2dTo3d(Standard_Real x, Standard_Real y)
{
    Standard_Real  XEye,YEye,ZEye,XAt,YAt,ZAt;
    m_view->Eye(XEye,YEye,ZEye);
    m_view->At(XAt,YAt,ZAt);
    gp_Pnt EyePoint(XEye,YEye,ZEye);
    gp_Pnt AtPoint(XAt,YAt,ZAt);
    gp_Vec EyeVector(EyePoint,AtPoint);
    gp_Dir EyeDir(EyeVector);
    gp_Pln PlaneOfTheView = gp_Pln(AtPoint,EyeDir);
    Standard_Real X,Y,Z;
    m_view->Convert((int)x,(int)y,X,Y,Z);
    gp_Pnt ConvertedPoint(X,Y,Z);
    gp_Pnt2d ConvertedPointOnPlane = ProjLib::Project(PlaneOfTheView,ConvertedPoint);
    gp_Pnt shapePoint  = ElSLib::Value(ConvertedPointOnPlane.X(),ConvertedPointOnPlane.Y(),PlaneOfTheView);
    //Initialize with a very far point from the camera
    return shapePoint;
}

/*************************************************
 * 功能：生成标签及引线
 * 说明：以全局变量来记录各个类型测量点的数量
 * **********************************************/
void occView::createLabel(QList<QList<gp_Pnt>> plist, QList<QList<gp_Pnt>> virtData, QList<QList<Handle(AIS_InteractiveObject) >> shapes, const GeometryType &typ)
{
    gp_Pnt P1;//所有点的中心位置
    int size = 0;
    double px=0,py=0,pz=0;
    QList<Handle(AIS_Shape)> polys;
    foreach(QList<gp_Pnt> pl, virtData) {
        TopoDS_Shape polygon = ModelMaker::make_polygon(pl);
        Handle(AIS_Shape) polylines = new AIS_Shape(polygon);
        polylines->SetColor(Quantity_NOC_YELLOW3);
        polys.append(polylines);

        foreach(gp_Pnt pnt,pl) {
            px+=pnt.X();
            py+=pnt.Y();
            pz+=pnt.Z();
        }
        size += pl.size();
    }
    P1.SetX(px/size);
    P1.SetY(py/size);
    P1.SetZ(pz/size);

    //1.新建测量对象
    MeasureItem *aItm = new MeasureItem();
    aItm->setType(typ);

    //2.新建标签窗口
    LabelWidget *measureLabel = new LabelWidget(this);
    aItm->setLabel(measureLabel);

    //3.传入数据并处理
    aItm->setData(plist);
    if(!aItm->Evaluate())
    {
        QMessageBox::critical(this,"错误","测点过少或存在较大偏差，请重新测量！");

        foreach(QList<Handle(AIS_InteractiveObject)> list, shapes) {
            foreach(Handle(AIS_InteractiveObject) obj, list) {
                m_context->Remove(obj,Standard_False);
            }
        }
        m_view->Update();

        delete aItm;
        delete measureLabel;
        return;
    }

    foreach(Handle(AIS_Shape) polylines,polys) {
        m_context->Display(polylines, Standard_True);
        m_context->Deactivate(polylines);
    }

    //4.绘制
    gp_Pnt P2 = gp_Pnt(P1.X()+10,P1.Y()+10,P1.Z()+10);//设置标签起点初始坐标
    labelList.push_back(make_pair(measureLabel,P1));
    Standard_Integer x2d,y2d;
    m_view->Convert(P2.X(),P2.Y(),P2.Z(),x2d,y2d);
    measureLabel->move(x2d,y2d);
    measureLabel->show();
    gp_Pnt pnt = point2dTo3d(x2d,y2d);//设置标签引线终点初始坐标
    drawLabelLeadLine(P1,pnt);//画线

    emit newMeasureItem(aItm);
}

/*************************************************
 * 功能：画标签的引线
 * **********************************************/
void occView::drawLabelLeadLine(gp_Pnt P1, gp_Pnt P2)
{
    TopoDS_Edge anEdge = BRepBuilderAPI_MakeEdge(P1,P2);
    Handle(AIS_Shape) anAisPrismEdge = new AIS_Shape(anEdge);
    lineVector.push_back(anAisPrismEdge);
    anAisPrismEdge->SetZLayer(Graphic3d_ZLayerId_Topmost);
    anAisPrismEdge->SetColor(Quantity_NOC_GREEN1);
    getContext()->Display(anAisPrismEdge, Standard_True);
    getContext()->Deactivate(anAisPrismEdge);
}

/*************************************************
 * 功能：调用标签排布算法
 * **********************************************/
void occView::AutoArrangeLabel()
{
    if(labelList.size()<7)
    {
        arrangeLabelFew(labelList);
    }
    else
    {
        arrangeLabelMore(labelList);
    }
}

/*************************************************
 * 功能：调用标签排布算法后进行标签的显示
 * **********************************************/
void occView::showLabelandLine(vector<pair<double, double> > &line, vector<pair<double, double> > &labelXY, vector<QString> &labelName)
{
    int numLine = lineVector.size();
    for(int i=0;i<numLine;i++)
    {
        getContext()->Remove(lineVector[i],Standard_True);
    }
    lineVector.clear();
    int numLabel = labelName.size();
    for(int i=0;i<numLabel;i++)
    {
        QString goalName = labelName[i];
        for(list<pair<LabelWidget*,gp_Pnt>>::iterator it=labelList.begin();it!=labelList.end();it++)
        {
            LabelWidget* label = it->first;
            QString name = label->Title();
            if(name == goalName)
            {
                gp_Pnt P1 = it->second;
                Standard_Real x = line[i].first;
                Standard_Real y = line[i].second;
                gp_Pnt P2 = point2dTo3d(x,y);
                label->move(labelXY[i].first,labelXY[i].second);
                drawLabelLeadLine(P1,P2);
                break;
            }
        }
    }
}

/*************************************************
 * 函数功能：清除所有标签及引线
 * 注意事项：触发命令由主函数传递
 * **********************************************/
void occView::clearLabelAndLine()
{
    //清除标签
    for(list<pair<LabelWidget*,gp_Pnt>>::iterator it=labelList.begin();it!=labelList.end();it++)
    {
        it->first->hide();
    }
    labelList.clear();
    //清除引线
    lineVector.clear();
}

/*************************************************************************************
 *                                 标签排布
 * 注意：因为窗口大小获取原因所以和OCC配置写在同一个cpp内
 * **********************************************************************************/
void occView::arrangeLabelFew(list<pair<LabelWidget *, gp_Pnt> > labelList)
{
    int LBWIDTH = 85;
    int maxX = maxXWindow;
    int nums = labelList.size();
    vector<pair<double,double>> locationXy,labelXy;
    vector<QString> labelName;

    if(nums == 1)//只有一个标签，排列在上端最中央
    {
        LabelWidget *label = labelList.front().first;
        QString name = label->Title();
        labelName.push_back(name);
        labelXy.push_back(make_pair(maxX/2-20,mininterval));
        locationXy.push_back(make_pair(maxX/2,mininterval+30));
    }
    else if(nums == 2)//只有两个标签，排列在上端的最左和最右端
    {
        LabelWidget *label1 = labelList.front().first;
        QString name1 = label1->Title();
        gp_Pnt P1 = labelList.front().second;
        Standard_Integer x1,y1;
        m_view->Convert(P1.X(),P1.Y(),P1.Z(),x1,y1);

        LabelWidget *label2 = labelList.back().first;
        QString name2 = label2->Title();
        gp_Pnt P2 = labelList.back().second;
        Standard_Integer x2,y2;
        m_view->Convert(P2.X(),P2.Y(),P2.Z(),x2,y2);

        if(x1<x2)
        {
            labelName.push_back(name1);
            labelName.push_back(name2);
        }
        else
        {
            labelName.push_back(name2);
            labelName.push_back(name1);
        }
        labelXy.push_back(make_pair(mininterval,mininterval));
        labelXy.push_back(make_pair(maxX-LBWIDTH-mininterval,mininterval));
        locationXy.push_back(make_pair(mininterval+20,mininterval+15));
        locationXy.push_back(make_pair(maxX-20-mininterval,mininterval+15));
    }
    else//当有3-6个标签时，全部排列在上端一排
    {
        //无论有无相邻交叉，标签左上角顶点与引线终点坐标不变
        int nums = labelList.size();
        double interval = (maxX-nums*LBWIDTH-mininterval*2)/(nums-1);
        for(int i=0;i<nums;i++)
        {
            if(i == 0)
            {
                labelXy.push_back(make_pair(mininterval,mininterval));
                locationXy.push_back(make_pair(mininterval+20,mininterval+15));
            }
            else if(i == nums-1)
            {
                labelXy.push_back(make_pair(maxX-LBWIDTH-mininterval,mininterval));
                locationXy.push_back(make_pair(maxX-20-mininterval,mininterval+15));
            }
            else
            {
                labelXy.push_back(make_pair(mininterval+LBWIDTH*i+interval*i,mininterval));
                locationXy.push_back(make_pair(mininterval+LBWIDTH*i+interval*i+20,mininterval+15));
            }
        }
        //计算相邻交叉
        map<LabelWidget*,double> labelX,labelY;
        for(list<pair<LabelWidget*,gp_Pnt>>::iterator it=labelList.begin();it!=labelList.end();it++)
        {
            gp_Pnt P = it->second;
            LabelWidget *label = it->first;
            Standard_Integer x,y;
            m_view->Convert(P.X(),P.Y(),P.Z(),x,y);
            labelX.insert(make_pair(label,x));
            labelY.insert(make_pair(label,y));
        }
        vector<pair<LabelWidget*,double>> upXSort(labelX.begin(),labelX.end());
        std::sort(upXSort.begin(),upXSort.end(),CmpByValue());
        vector<pair<LabelWidget*,double>> upYSort(labelY.begin(),labelY.end());
        vector<double> labelK,labelB,labelYtoX;
        for(int i=0;i<nums;i++)//相邻交叉
        {
            LabelWidget *labelX = upXSort[i].first;
            QString xName = labelX->Title();
            for(vector<pair<LabelWidget*,double>>::iterator it=upYSort.begin();it!=upYSort.end();it++)
            {
                LabelWidget *labelY = it->first;
                QString yName = labelY->Title();
                if(xName == yName)
                {
                    labelYtoX.push_back(it->second);
                    double k = (locationXy[i].second-it->second)/(locationXy[i].first-upXSort[i].second);
                    labelK.push_back(k);
                    double b = locationXy[i].second-k*locationXy[i].first;
                    labelB.push_back(b);
                    break;
                }
            }
        }
        int flag = 0;
        double x = 0;
        for(int i=0;i<nums-1;i++)//相邻交叉
        {
            x = (labelB[i+1]-labelB[i])/(labelK[i]-labelK[i+1]);
            if((x<=locationXy[i].first && x>=upXSort[i].second)
                    || (x>=locationXy[i].first && x<=upXSort[i].second))
            {
                swap(upXSort[i],upXSort[i+1]);
                swap(labelYtoX[i],labelYtoX[i+1]);
                double k = (locationXy[i].second-labelYtoX[i])/(locationXy[i].first-upXSort[i].second);
                labelK[i] = k;
                double b = locationXy[i].second-k*locationXy[i].first;
                labelB[i] =  b;
                k = (locationXy[i+1].second-labelYtoX[i+1])/(locationXy[i+1].first-upXSort[i+1].second);
                labelK[i+1] = k;
                b = locationXy[i+1].second-k*locationXy[i+1].first;
                labelB[i+1] =  b;
                flag = 1;
            }
            if(flag)
            {
                if(i>0)
                {
                    i=i-2;
                }
                flag = 0;
            }
        }
        for(int i=0;i<nums;i++)
        {
            LabelWidget *label = upXSort[i].first;
            QString name = label->Title();
            labelName.push_back(name);
        }
    }
    showLabelandLine(locationXy,labelXy,labelName);
}

void occView::arrangeLabelMore(list<pair<LabelWidget *, gp_Pnt> > labelList)
{
    //四周显示标签——初：上下y值对半分、x值排序取中去边，两边y值排序。
    //            优化：计算交叉，移动交叉的两根引线反复计算反复移动
    int LBWIDTH = 85;
    int LBHEIGHT = 60;

    int i = 0;
    int maxX = maxXWindow;
    int maxY = maxYWindow;
    double nowX = mininterval;
    double nowY = mininterval;
    int labelNums = labelList.size();
    double initHeight = LBHEIGHT;
    map<LabelWidget*,int> upTest,downTest,upPy,downPy,upPx,downPx,sidePy,sidePx;
    for(list<pair<LabelWidget*,gp_Pnt>>::iterator it=labelList.begin();it!=labelList.end();it++)
    {
        gp_Pnt P = it->second;
        LabelWidget *label = it->first;
        Standard_Integer x,y;
        m_view->Convert(P.X(),P.Y(),P.Z(),x,y);
        upTest.insert(make_pair(label,y));
        downTest.insert(make_pair(label,y));
        upPy.insert(make_pair(label,y));
        downPy.insert(make_pair(label,y));
        upPx.insert(make_pair(label,x));
        downPx.insert(make_pair(label,x));
        sidePx.insert(make_pair(label,x));
        sidePy.insert(make_pair(label,y));
    }
    //计算各个面应显示的标签数量
    int firstIn = 0;
    int flag = 1;
    int upNums,downNums,rightNums,leftNums,initNums;
    double interval = 0;
    double toWidth = 0;
    initNums = labelNums/4;
    vector<pair<LabelWidget*,int>> upSort(upTest.begin(),upTest.end());
    std::sort(upSort.begin(),upSort.end(),CmpByValue());
    vector<pair<LabelWidget*,int>> downSort(downTest.begin(),downTest.end());
    std::sort(downSort.begin(), downSort.end(),CmpByValueLarge());
    if(labelNums%4 == 1)
    {
        if(initNums == 1)
        {
            upNums = initNums+2;
        }
        else
        {
            upNums = initNums+2;
            downNums = initNums+1;
            while(flag)
            {
                toWidth = 0;
                i = 0;
                for(vector<pair<LabelWidget*,int>>::iterator it = upSort.begin();it != upSort.end();it++)
                {
                    if(i<upNums)
                    {
                        toWidth += LBWIDTH;
                    }
                    else
                    {
                        interval = (maxXWindow - mininterval*2 - toWidth) / (upNums-1);
                    }
                    i++;
                }
                if(interval<mininterval)
                {
                    if(firstIn == 0)
                    {
                        upNums--;
                        firstIn = 1;
                    }
                    else
                    {
                        upNums--;
                        downNums--;
                    }
                }
                else
                {
                    flag = 0;
                }
            }
        }
    }
    if(labelNums%4 == 2)
    {
        if(initNums == 1)
        {
            upNums = initNums+2;
            downNums = initNums+2;
        }
        else
        {
            upNums = initNums+2;
            downNums = initNums+2;
            while(flag)
            {
                toWidth = 0;
                i = 0;
                for(vector<pair<LabelWidget*,int>>::iterator it = downSort.begin();it != downSort.end();it++)
                {
                    if(i<downNums)
                    {
                        toWidth += LBWIDTH;
                    }
                    else
                    {
                        interval = (maxXWindow - mininterval*2 - toWidth) / (downNums-1);
                    }
                    i++;
                }
                if(interval<mininterval)
                {
                    upNums--;
                    downNums--;
                }
                else
                {
                    flag = 0;
                }
            }
        }
    }
    if(labelNums%4 == 3)
    {
        if(initNums == 1)
        {
            upNums = initNums+2;
            downNums = initNums+2;
        }
        else
        {
            upNums = initNums+2;
            downNums = initNums+2;
            while(flag)
            {
                toWidth = 0;
                i = 0;
                for(vector<pair<LabelWidget*,int>>::iterator it = downSort.begin();it != downSort.end();it++)
                {
                    if(i<downNums)
                    {
                        toWidth += LBWIDTH;
                    }
                    else
                    {
                        interval = (maxXWindow - mininterval*2 - toWidth) / (downNums-1);
                    }
                    i++;
                }
                if(interval<mininterval)
                {
                    upNums--;
                    downNums--;
                }
                else
                {
                    flag = 0;
                }
            }
        }
    }
    if(labelNums%4 == 0)
    {
        upNums = initNums+1;
        downNums = initNums+1;
        while(flag)
        {
            toWidth = 0;
            i = 0;
            for(vector<pair<LabelWidget*,int>>::iterator it = downSort.begin();it != downSort.end();it++)
            {
                if(i<downNums)
                {
                    toWidth += LBWIDTH;
                }
                else
                {
                    interval = (maxXWindow - mininterval*2 - toWidth) / (downNums-1);
                }
                i++;
            }
            if(interval<mininterval)
            {
                upNums--;
                downNums--;
            }
            else
            {
                flag = 0;
            }
        }
    }
    //分配标签
    //上下对半分
    int upHalfNums,downHalfNums;
    vector<pair<LabelWidget*,int>> upPySort(upPy.begin(),upPy.end());
    std::sort(upPySort.begin(), upPySort.end(),CmpByValue());
    if(labelNums % 2 == 0)
    {
        upHalfNums = labelNums / 2;
    }
    else
    {
        upHalfNums =  labelNums / 2;
        upHalfNums++;
    }
    i = 0;
    for(vector<pair<LabelWidget*,int>>::iterator it = upPySort.begin();it != upPySort.end();it++)
    {
        if(i<upHalfNums)
        {
            downPx.erase(it->first);
            downPy.erase(it->first);
        }
        else
        {
            upPx.erase(it->first);
            upPy.erase(it->first);
        }
        i++;
    }
    //计算保留上下中间应剔除两边的数量及位置
    int upLefttoSide,upRighttoSide,downLefttoSide,downRighttoSide;
    int flagArrange = 0;
    upHalfNums = upPx.size();
    downHalfNums = downPx.size();
    if((upHalfNums - upNums) % 2 == 0)
    {
        upLefttoSide = (upHalfNums - upNums) / 2;
        upRighttoSide = (upHalfNums - upNums) / 2;
    }
    else
    {
        upLefttoSide = (upHalfNums - upNums) / 2;
        upRighttoSide = (upHalfNums - upNums) / 2;
        upRighttoSide++;
        flagArrange = 1;
    }
    if((downHalfNums - downNums) % 2 == 0)
    {
        downLefttoSide = (downHalfNums - downNums) / 2;
        downRighttoSide = (downHalfNums - downNums) / 2;
    }
    else
    {
        if(flagArrange == 0)
        {
            downLefttoSide = (downHalfNums - downNums) / 2;
            downRighttoSide = (downHalfNums - downNums) / 2;
            downRighttoSide++;
        }
        else
        {
            downLefttoSide = (downHalfNums - downNums) / 2;
            downRighttoSide = (downHalfNums - downNums) / 2;
            downLefttoSide++;
            flagArrange = 0;
        }
    }
    //剔除上半的两边
    double toWidthUp = 0;
    double toWidthDown = 0;
    vector<pair<LabelWidget*,int>> upPxSort(upPx.begin(),upPx.end());
    std::sort(upPxSort.begin(), upPxSort.end(),CmpByValue());
    vector<pair<LabelWidget*,int>> downPxSort(downPx.begin(),downPx.end());
    std::sort(downPxSort.begin(), downPxSort.end(),CmpByValue());
    i = 0;
    for(vector<pair<LabelWidget*,int>>::iterator it = upPxSort.begin();it != upPxSort.end();it++)
    {
        if(i<upLefttoSide)
        {
            upPx.erase(it->first);
            upPy.erase(it->first);
        }
        else if(i>=(upHalfNums-upRighttoSide))
        {
            upPx.erase(it->first);
            upPy.erase(it->first);
        }
        else
        {
            toWidthUp += LBWIDTH;
            sidePx.erase(it->first);
            sidePy.erase(it->first);
        }
        i++;
    }
    //剔除下半的两边
    i = 0;
    for(vector<pair<LabelWidget*,int>>::iterator it = downPxSort.begin();it != downPxSort.end();it++)
    {
        if(i<downLefttoSide)
        {
            downPx.erase(it->first);
            downPy.erase(it->first);
        }
        else if(i>=(downHalfNums-downRighttoSide))
        {
            downPx.erase(it->first);
            downPy.erase(it->first);
        }
        else
        {
            toWidthDown += LBWIDTH;
            sidePx.erase(it->first);
            sidePy.erase(it->first);
        }
        i++;
    }
    //分配左右
    vector<pair<LabelWidget*,int>> sidePxSort(sidePx.begin(),sidePx.end());
    std::sort(sidePxSort.begin(), sidePxSort.end(),CmpByValue());
    vector<pair<LabelWidget*,int>> sidePySort(sidePy.begin(),sidePy.end());
    std::sort(sidePySort.begin(), sidePySort.end(),CmpByValue());
    map<LabelWidget*, int> leftPx,rightPx,leftPy,rightPy;
    for(vector<pair<LabelWidget*,int>>::iterator it = sidePxSort.begin();it != sidePxSort.end();it++)
    {
        rightPx.insert(make_pair(it->first,it->second));
        leftPx.insert(make_pair(it->first,it->second));
    }
    for(vector<pair<LabelWidget*,int>>::iterator it = sidePySort.begin();it != sidePySort.end();it++)
    {
        rightPy.insert(make_pair(it->first,it->second));
        leftPy.insert(make_pair(it->first,it->second));
    }
    int sideNums = sidePxSort.size();
    if(sideNums % 2 == 0)
    {
        leftNums = rightNums = sideNums / 2;
    }
    else
    {
        leftNums = rightNums = sideNums / 2;
        rightNums++;
    }
    i = 0;
    for(vector<pair<LabelWidget*,int>>::iterator it = sidePxSort.begin();it != sidePxSort.end();it++)
    {
        if(i<leftNums)
        {
            rightPy.erase(it->first);
            rightPx.erase(it->first);
        }
        else
        {
            leftPy.erase(it->first);
            leftPx.erase(it->first);
        }
        i++;
    }
    double intervalUp = (maxXWindow - mininterval*2 - toWidthUp) / (upNums-1);
    double intervalDown = (maxXWindow - mininterval*2 - toWidthDown) / (downNums-1);
    double intervalRight = (maxYWindow - mininterval*2 - initHeight*(rightNums+2)) / (rightNums+1);
    double intervalLeft = (maxYWindow - mininterval*2 - initHeight*(leftNums+2)) / (leftNums+1);
    vector<pair<LabelWidget*,int>> rightPxSort2(rightPx.begin(),rightPx.end());
    std::sort(rightPxSort2.begin(), rightPxSort2.end(),CmpByValueLarge());
    vector<pair<LabelWidget*,int>> leftPxSort2(leftPx.begin(),leftPx.end());
    std::sort(leftPxSort2.begin(), leftPxSort2.end(),CmpByValue());
    for(vector<pair<LabelWidget*,int>>::iterator it = rightPxSort2.begin();it != rightPxSort2.end();it++)
    {
        if(intervalRight<mininterval)
        {
//            m_pDmisModel->hideLabel((it->first),false);
            rightPx.erase(it->first);
            rightPy.erase(it->first);
            rightNums--;
            intervalRight = (maxYWindow - mininterval*2 - initHeight*(rightNums+2)) / (rightNums+1);
        }
        else
        {
            break;
        }
    }
    for(vector<pair<LabelWidget*,int>>::iterator it = leftPxSort2.begin();it != leftPxSort2.end();it++)
    {
        if(intervalLeft<mininterval)
        {
//            m_pDmisModel->hideLabel((it->first),false);
            leftPx.erase(it->first);
            leftPy.erase(it->first);
            leftNums--;
            intervalLeft = (maxYWindow - mininterval*2 - initHeight*(leftNums+2)) / (leftNums+1);
        }
        else
        {
            break;
        }
    }
    //保存labelx,labely,locationx,locationy,nowX,nowY,whdth,name;
    vector<double> labelWidth;
    vector<pair<LabelWidget*,int>> upPxSort2(upPx.begin(),upPx.end());
    std::sort(upPxSort2.begin(), upPxSort2.end(),CmpByValue());
    vector<pair<LabelWidget*,int>> upPySort2(upPy.begin(),upPy.end());
    vector<pair<LabelWidget*,int>> rightPySort(rightPy.begin(),rightPy.end());
    std::sort(rightPySort.begin(), rightPySort.end(),CmpByValue());
    vector<pair<LabelWidget*,int>> rightPxSort(rightPx.begin(),rightPx.end());
    vector<pair<LabelWidget*,int>> downPxSort2(downPx.begin(),downPx.end());
    std::sort(downPxSort2.begin(), downPxSort2.end(),CmpByValueLarge());
    vector<pair<LabelWidget*,int>> downPySort2(downPy.begin(),downPy.end());
    vector<pair<LabelWidget*,int>> leftPySort(leftPy.begin(),leftPy.end());
    std::sort(leftPySort.begin(), leftPySort.end(),CmpByValueLarge());
    vector<pair<LabelWidget*,int>> leftPxSort(leftPx.begin(),leftPx.end());
    vector<pair<int,int>> labelXy;//标签左上角坐标
    vector<pair<double,double>> locationXy;//引线终点坐标
    vector<QString> labelName;//标签名字
    double Lx,Ly,labelx;
    QString strX,strY;
    //up(小到大)
    i = 0;
    for(vector<pair<LabelWidget*,int>>::iterator it = upPxSort2.begin();it != upPxSort2.end();it++)
    {
//        labelWidth.push_back(LBWIDTH);
        LabelWidget *labelX = it->first;
        strX = labelX->Title();
        labelName.push_back(strX);
        labelx = LBWIDTH;
        for(vector<pair<LabelWidget*,int>>::iterator it2 = upPySort2.begin();it2 != upPySort2.end();it2++)
        {
            LabelWidget *labelY = it2->first;
            strY = labelY->Title();
            if(strX == strY)
            {
                labelXy.push_back(make_pair(it->second,it2->second));
                if(i == 0)
                {
                    Lx = nowX + labelx;
                    Ly = nowY + initHeight/2;
                }
                else if(i == (upPxSort2.size()-1))
                {
                    Lx = nowX;
                    Ly = nowY + initHeight/2;
                }
                else
                {
                    Lx = nowX + labelx/2;
                    Ly = nowY + initHeight;
                }
                locationXy.push_back(make_pair(Lx,Ly));
                break;
            }
        }
        nowX += LBWIDTH+intervalUp;
        i++;
    }
    //right(小到大)
    nowY = mininterval + initHeight + intervalRight;
    for(vector<pair<LabelWidget*,int>>::iterator it = rightPySort.begin();it != rightPySort.end();it++)
    {
//        labelWidth.push_back(LBWIDTH);
        LabelWidget *labelY = it->first;
        strY = labelY->Title();
//        strY = it->first;
        labelName.push_back(strY);
        nowX = maxX-mininterval-LBWIDTH;
        for(vector<pair<LabelWidget*,int>>::iterator it2 = rightPxSort.begin();it2 != rightPxSort.end();it2++)
        {
            LabelWidget *labelX = it2->first;
            strX = labelX->Title();
//            strX = it2->first;
            if(strX == strY)
            {
                labelXy.push_back(make_pair(it2->second,it->second));
                Lx = nowX;
                Ly = nowY + initHeight/2;
                locationXy.push_back(make_pair(Lx,Ly));
                break;
            }
        }
        nowY += initHeight + intervalRight;
    }
    //down(大到小)
    i = 0;
    nowY = maxY-mininterval-initHeight;
    for(vector<pair<LabelWidget*,int>>::iterator it = downPxSort2.begin();it != downPxSort2.end();it++)
    {
//        labelWidth.push_back(LBWIDTH);
//        strX = it->first;
        LabelWidget *labelX = it->first;
        strX = labelX->Title();
        labelName.push_back(strX);
        if(i == 0)
        {
            nowX = maxX - mininterval - LBWIDTH;
        }
        else
        {
            nowX = nowX - intervalDown - LBWIDTH;
        }
        labelx = LBWIDTH;
        for(vector<pair<LabelWidget*,int>>::iterator it2 = downPySort2.begin();it2 != downPySort2.end();it2++)
        {
//            strY = it2->first;
            LabelWidget *labelY = it2->first;
            strY = labelY->Title();
            if(strX == strY)
            {
                labelXy.push_back(make_pair(it->second,it2->second));
                if(i == 0)
                {
                    Lx = nowX ;
                    Ly = nowY + initHeight/2;
                }
                else if(i == (downPxSort2.size()-1))
                {
                    Lx = nowX+ labelx;
                    Ly = nowY + initHeight/2;
                }
                else
                {
                    Lx = nowX + labelx/2;
                    Ly = nowY;
                }
                locationXy.push_back(make_pair(Lx,Ly));
                break;
            }
        }
        i++;
    }
    //left(大到小)
    nowX = mininterval;
    nowY = maxY-mininterval-initHeight-intervalLeft;
    for(vector<pair<LabelWidget*,int>>::iterator it = leftPySort.begin();it != leftPySort.end();it++)
    {
        LabelWidget *labelY = it->first;
        strY = labelY->Title();
        labelName.push_back(strY);
        labelx = LBWIDTH;
        for(vector<pair<LabelWidget*,int>>::iterator it2 = leftPxSort.begin();it2 != leftPxSort.end();it2++)
        {
//            strX = it2->first;
            LabelWidget *labelX = it2->first;
            strX = labelX->Title();
            if(strX == strY)
            {
                labelXy.push_back(make_pair(it2->second,it->second));
                Lx = nowX + labelx;
                Ly = nowY - initHeight/2;
                locationXy.push_back(make_pair(Lx,Ly));
                break;
            }
        }
        nowY = nowY - initHeight - intervalLeft;
    }
    //检测相邻位置标签的交叉情况
    i = 0;
    int j = 0;
    int repetitionFlag = 1;
    int victoryFlag = 1;
    int secondExamine = 0;
    int intoNums = 0;
    double k,b,k1,b1,k2,b2,crossX1,crossX2,distanceI,distanceJ,front,latter;
    double partWidth = 0;
    double partWidthFront = 0;
    double partWidthLatter = 0;
    double crossX;
    vector<pair<double,double>> labelKB;
    while(victoryFlag == 1)
    {
        for(i=0;i<labelNums;i++)
        {
            if(i == (labelNums-1))//最后一个
            {
                k = (locationXy[i].second - double(labelXy[i].second)) / (locationXy[i].first - double(labelXy[i].first));
                b = double(labelXy[i].second) - k*(double(labelXy[i].first));
                k2 = (locationXy[0].second - double(labelXy[0].second)) / (locationXy[0].first - double(labelXy[0].first));
                b2 = double(labelXy[0].second) - k2*(double(labelXy[0].first));
                if(k != k2)
                {
                    crossX2 = (b2-b) / (k-k2);
                    if((crossX2<=labelXy[i].first && crossX2>=locationXy[i].first) ||
                            (crossX2>=labelXy[i].first && crossX2<=locationXy[i].first))
                    {
                        if((crossX2<=labelXy[0].first && crossX2>=locationXy[0].first) ||
                               (crossX2>=labelXy[0].first && crossX2<=locationXy[0].first))
                        {
                            secondExamine = 1;
                            swap(labelName[i],labelName[0]);
                            swap(labelXy[i],labelXy[0]);
//                            swap(labelWidth[i],labelWidth[0]);
                            swap(locationXy[i].first,locationXy[0].first);
                            k = (locationXy[i].second - double(labelXy[i].second)) / (locationXy[i].first - double(labelXy[i].first));
                            b = double(labelXy[i].second) - k*(double(labelXy[i].first));
                            k1= (locationXy[i-1].second - double(labelXy[i-1].second)) / (locationXy[i-1].first - double(labelXy[i-1].first));
                            b1 = double(labelXy[i-1].second) - k1*(double(labelXy[i-1].first));
                            if(k != k1)
                            {
                                crossX1 = (b1-b) / (k-k1);
                                if((crossX1<=labelXy[i].first && crossX1>=locationXy[i].first) ||
                                        (crossX1>=labelXy[i].first && crossX1<=locationXy[i].first))
                                {
                                    if((crossX1<=labelXy[i-1].first && crossX1>=locationXy[i-1].first) ||
                                           (crossX1>=labelXy[i-1].first && crossX1<=locationXy[i-1].first))
                                    {
                                        swap(labelName[i],labelName[i-1]);
                                        swap(labelXy[i],labelXy[i-1]);
//                                        swap(labelWidth[i],labelWidth[i-1]);
                                        swap(locationXy[i].first,locationXy[i-1].first);
                                        i = i-3;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                k = (locationXy[i].second - double(labelXy[i].second)) / (locationXy[i].first - double(labelXy[i].first));
                b = double(labelXy[i].second) - k*(double(labelXy[i].first));
                k2 = (locationXy[i+1].second - double(labelXy[i+1].second)) / (locationXy[i+1].first - double(labelXy[i+1].first));
                b2 = double(labelXy[i+1].second) - k2*(double(labelXy[i+1].first));
                if(k != k2)
                {
                    crossX2 = (b2-b) / (k-k2);
                    if((crossX2<=labelXy[i].first && crossX2>=locationXy[i].first) ||
                            (crossX2>=labelXy[i].first && crossX2<=locationXy[i].first))
                    {
                        if((crossX2<=labelXy[i+1].first && crossX2>=locationXy[i+1].first) ||
                               (crossX2>=labelXy[i+1].first && crossX2<=locationXy[i+1].first))
                        {
                            swap(labelName[i],labelName[i+1]);
                            swap(labelXy[i],labelXy[i+1]);
//                            swap(labelWidth[i],labelWidth[i+1]);
                            if(i == 0)
                            {
//                                locationXy[0].first =  mininterval + labelWidth[0];
                                locationXy[0].first =  mininterval + LBWIDTH;
                            }
                            else if(i< (upNums - 1))//up中
                            {
                                for(j=0;j<i;j++)
                                {
//                                    partWidth += labelWidth[j];
                                    partWidth += LBWIDTH;
                                }
                                for(j=0;j<i+1;j++)
                                {
//                                    partWidthLatter += labelWidth[j];
                                    partWidthLatter += LBWIDTH;
                                }
//                                locationXy[i].first = mininterval + partWidth + intervalUp*i + labelWidth[i]/2;
                                locationXy[i].first = mininterval + partWidth + intervalUp*i + 20;
                                if(i == (upNums -2))
                                {
//                                    locationXy[i+1].first = maxX - mininterval - labelWidth[i+1];
                                    locationXy[i+1].first = maxX - mininterval - LBWIDTH;
                                }
                                else
                                {
//                                    locationXy[i+1].first = mininterval + partWidthLatter + intervalUp*(i+1) + labelWidth[i+1]/2;
                                    locationXy[i+1].first = mininterval + partWidthLatter + intervalUp*(i+1) + 20;
                                }
                                partWidth = 0;
                                partWidthLatter = 0;
                            }
                            else if(i == (upNums -1))//右上顶角
                            {
//                                locationXy[i].first = maxX - mininterval - labelWidth[i];
//                                locationXy[i+1].first = maxX - mininterval - labelWidth[i+1];
                                locationXy[i].first = maxX - mininterval - LBWIDTH;
                                locationXy[i+1].first = maxX - mininterval - LBWIDTH;
                            }
                            else if(i<=(upNums+rightNums))//right+右下顶角
                            {
                                if(i<(upNums+rightNums))
                                {
                                    swap(locationXy[i].first,locationXy[i+1].first);
                                }
                                else
                                {
//                                    locationXy[i].first = maxX - mininterval - labelWidth[i];
//                                    locationXy[i+1].first = maxX - mininterval - labelWidth[i] - intervalDown - labelWidth[i+1]/2;

                                    locationXy[i].first = maxX - mininterval - LBWIDTH;
                                    locationXy[i+1].first = maxX - mininterval - LBWIDTH - intervalDown - 20;
                                }
                            }
                            else if(i<(upNums+rightNums+downNums-1))//down中
                            {
                                for(j=upNums+rightNums;j<i;j++)
                                {
//                                    partWidth += labelWidth[j];
                                    partWidth += LBWIDTH;
                                }
                                for(j=upNums+rightNums;j<i+1;j++)
                                {
//                                    partWidthLatter += labelWidth[j];
                                    partWidthLatter += LBWIDTH;
                                }
//                                locationXy[i].first = maxX - mininterval - partWidth - intervalDown*(i-upNums-rightNums) - labelWidth[i]/2;
                                locationXy[i].first = maxX - mininterval - partWidth - intervalDown*(i-upNums-rightNums) - 20;
                                if(i == (upNums+rightNums+downNums-2))
                                {
//                                    locationXy[i+1].first = mininterval + labelWidth[i+1];
                                    locationXy[i+1].first = mininterval + LBWIDTH;
                                }
                                else
                                {
//                                    locationXy[i+1].first = maxX - mininterval - partWidthLatter - intervalDown*(i+1-upNums-rightNums) - labelWidth[i+1]/2;
                                    locationXy[i+1].first = maxX - mininterval - partWidthLatter - intervalDown*(i+1-upNums-rightNums) - 20;
                                }
                                partWidth = 0;
                                partWidthLatter = 0;
                            }
                            else if(i == (upNums+rightNums+downNums-1))//左下顶角
                            {
//                                locationXy[i].first = mininterval + labelWidth[i];
//                                locationXy[i+1].first = mininterval + labelWidth[i+1];
                                locationXy[i].first = mininterval + LBWIDTH;
                                locationXy[i+1].first = mininterval + LBWIDTH;
                            }
                            else//left
                            {
                                swap(locationXy[i].first,locationXy[i+1].first);
                            }

                            if(i != 0)
                            {
                                k = (locationXy[i].second - double(labelXy[i].second)) / (locationXy[i].first - double(labelXy[i].first));
                                b = double(labelXy[i].second) - k*(double(labelXy[i].first));
                                k1 = (locationXy[i-1].second - double(labelXy[i-1].second)) / (locationXy[i-1].first - double(labelXy[i-1].first));
                                b1 = double(labelXy[i-1].second) - k1*(double(labelXy[i-1].first));
                                if(k != k1)
                                {
                                    crossX1 = (b1-b) / (k-k1);
                                    if((crossX1<=labelXy[i].first && crossX1>=locationXy[i].first) ||
                                            (crossX1>=labelXy[i].first && crossX1<=locationXy[i].first))
                                    {
                                        if((crossX1<=labelXy[i-1].first && crossX1>=locationXy[i-1].first) ||
                                                (crossX1>=labelXy[i-1].first && crossX1<=locationXy[i-1].first))
                                        {
                                            swap(labelName[i],labelName[i-1]);
                                            swap(labelXy[i],labelXy[i-1]);
//                                            swap(labelWidth[i],labelWidth[i-1]);
                                            if(i<(upNums -1))//up中
                                            {
                                                for(int j=0;j<i;j++)
                                                {
//                                                    partWidth += labelWidth[j];
                                                    partWidth += LBWIDTH;
                                                }
                                                for(int j=0;j<i-1;j++)
                                                {
//                                                    partWidthFront += labelWidth[j];
                                                    partWidthFront += LBWIDTH;
                                                }
//                                                locationXy[i].first = mininterval + partWidth + intervalUp*i + labelWidth[i]/2;
                                                locationXy[i].first = mininterval + partWidth + intervalUp*i + 20;
                                                if(i != 1)
                                                {
//                                                    locationXy[i-1].first = mininterval + partWidthFront + intervalUp*(i-1) + labelWidth[i-1]/2;
                                                    locationXy[i-1].first = mininterval + partWidthFront + intervalUp*(i-1) + 20;
                                                }
                                                else
                                                {
//                                                    locationXy[i-1].first = mininterval + labelWidth[0];
                                                    locationXy[i-1].first = mininterval + LBWIDTH;
                                                }
                                                partWidth = 0;
                                                partWidthFront = 0;
                                            }
                                            else if(i == (upNums -1))//右上顶角
                                            {
//                                                locationXy[i].first = maxX - mininterval - labelWidth[i];
//                                                locationXy[i-1].first = maxX - mininterval - labelWidth[i] - intervalUp - labelWidth[i-1]/2;
                                                locationXy[i].first = maxX - mininterval - LBWIDTH;
                                                locationXy[i-1].first = maxX - mininterval - LBWIDTH - intervalUp - 20;
                                            }
                                            else if(i<=(upNums+rightNums))//right+右下顶角
                                            {
                                                swap(locationXy[i].first,locationXy[i-1].first);
                                            }
                                            else if(i<(upNums+rightNums+downNums-1))//down中
                                            {
                                                for(j=upNums+rightNums;j<i;j++)
                                                {
//                                                    partWidth += labelWidth[j];
                                                    partWidth += LBWIDTH;
                                                }
                                                for(j=upNums+rightNums;j<i-1;j++)
                                                {
//                                                    partWidthFront += labelWidth[j];
                                                    partWidthFront += LBWIDTH;
                                                }
//                                                locationXy[i].first = maxX - mininterval - partWidth - intervalDown*(i-upNums-rightNums)-labelWidth[i]/2;
                                                locationXy[i].first = maxX - mininterval - partWidth - intervalDown*(i-upNums-rightNums)-20;
                                                if(i == (upNums+rightNums+1))
                                                {
//                                                    locationXy[i-1].first = maxX - mininterval - labelWidth[i-1];
                                                    locationXy[i-1].first = maxX - mininterval - LBWIDTH;
                                                }
                                                else
                                                {
//                                                    locationXy[i-1].first = maxX - mininterval - partWidthFront - intervalDown*(i-1-upNums-rightNums) - labelWidth[i-1]/2;
                                                    locationXy[i-1].first = maxX - mininterval - partWidthFront - intervalDown*(i-1-upNums-rightNums) - 20;

                                                }
                                                partWidth = 0;
                                                partWidthFront = 0;
                                            }
                                            else if(i == (upNums+rightNums+downNums-1))//左下顶角
                                            {
//                                                locationXy[i].first = mininterval + labelWidth[i];
//                                                locationXy[i-1].first = mininterval + labelWidth[i] + intervalDown + labelWidth[i-1]/2;
                                                locationXy[i].first = mininterval + LBWIDTH;
                                                locationXy[i-1].first = mininterval + LBWIDTH + intervalDown + 20;
                                            }
                                            else//left
                                            {
                                                swap(locationXy[i].first,locationXy[i-1].first);
                                            }
                                            if(i == 1)
                                            {
                                                i = -1;
                                            }
                                            else
                                            {
                                                i = i-3;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            if(secondExamine == 1)
            {
                if(i == (labelNums - 1))
                {
                    secondExamine = 0;
                    i = -1;
                }
            }
        }
        if(repetitionFlag == 1)
        {
            if(intoNums < 30)
            {
                repetitionFlag = 0;
                victoryFlag = 1;
            }
            else
            {
                victoryFlag = 0;
            }
        }
        else
        {
            victoryFlag = 0;
        }
        //二次检测
        if(victoryFlag == 1)
        {
            labelKB.clear();
            for(i=0;i<labelNums;i++)
            {
                k = (locationXy[i].second - double(labelXy[i].second)) / (locationXy[i].first - double(labelXy[i].first));
                b = double(labelXy[i].second) - k*(double(labelXy[i].first));
                labelKB.push_back(make_pair(k,b));
            }
            for(i=0;i<labelNums-2;i++)
            {
                for(j=i+2;j<labelNums;j++)
                {
                    if(labelKB[j].first != labelKB[i].first)
                    {
                        crossX = (labelKB[i].second - labelKB[j].second) / (labelKB[j].first - labelKB[i].first);
                        if((crossX<=labelXy[i].first && crossX>=locationXy[i].first) ||
                                (crossX>=labelXy[i].first && crossX<=locationXy[i].first))
                        {
                            if((crossX<=labelXy[j].first && crossX>=locationXy[j].first) ||
                                    (crossX>=labelXy[j].first && crossX<=locationXy[j].first))
                            {
                                repetitionFlag = 1;
                                intoNums++;
                                distanceI = sqrt(pow((locationXy[j].second - labelXy[i].second),2) + pow((locationXy[j].first - labelXy[i].first),2));
                                distanceJ = sqrt(pow((locationXy[i].second - labelXy[j].second),2) + pow((locationXy[i].first - labelXy[j].first),2));
                                if(distanceI<distanceJ)//J 向后移动
                                {
                                    for(front = i;front<j;front++)
                                    {
                                        swap(labelName[front],labelName[front+1]);
//                                        swap(labelWidth[front],labelWidth[front+1]);
                                        swap(labelXy[front],labelXy[front+1]);
                                        k = (locationXy[front].second - double(labelXy[front].second)) / (locationXy[front].first - double(labelXy[front].first));
                                        b = double(labelXy[front].second) - k*(double(labelXy[front].first));
                                        labelKB[front].first = k;
                                        labelKB[front].second = b;
                                    }
                                    front = 0;
                                    break;
                                }
                                else//I 向前移动
                                {
                                    for(latter = j;latter>i;latter--)
                                    {
                                        swap(labelName[latter],labelName[latter-1]);
//                                        swap(labelWidth[latter],labelWidth[latter-1]);
                                        swap(labelXy[latter],labelXy[latter-1]);
                                        k = (locationXy[latter].second - double(labelXy[latter].second)) / (locationXy[latter].first - double(labelXy[latter].first));
                                        b = double(labelXy[latter].second) - k*(double(labelXy[latter].first));
                                        labelKB[latter].first = k;
                                        labelKB[latter].second = b;
                                    }
                                    latter = 0;
                                    break;
                                }
                            }
                        }
                    }
                }
                if(repetitionFlag == 1)
                {
                    break;
                }
            }
        }
    }
    intoNums = 0;
    nowX = mininterval;
    nowY = mininterval;
    vector<pair<double,double>> labelLeftUpXy;
    for(i=0;i<upNums;i++)
    {
        if(i == (upNums -1))
        {
//            nowX = maxX - mininterval - labelWidth[i];
            nowX = maxX - mininterval - LBWIDTH;
            labelLeftUpXy.push_back(make_pair(nowX,nowY));
//            arrangeArroundLabels(locationXy[i].first,locationXy[i].second,nowX,nowY,m_pDmisModel->findLabel(labelName[i]),false);
        }
        else
        {
//            arrangeArroundLabels(locationXy[i].first,locationXy[i].second,nowX,nowY,m_pDmisModel->findLabel(labelName[i]),false);
            labelLeftUpXy.push_back(make_pair(nowX,nowY));
//            nowX += labelWidth[i] + intervalUp;
            nowX += LBWIDTH + intervalUp;
        }
    }
    for(i=upNums;i<(upNums+rightNums);i++)
    {
//        nowX = maxX - mininterval - labelWidth[i];
        nowX = maxX - mininterval - LBWIDTH;
        nowY += initHeight + intervalRight;
        labelLeftUpXy.push_back(make_pair(nowX,nowY));
//        arrangeArroundLabels(locationXy[i].first,locationXy[i].second,nowX,nowY,m_pDmisModel->findLabel(labelName[i]),false);
    }
//    nowX = maxX - mininterval - labelWidth[upNums+rightNums];
    nowX = maxX - mininterval - LBWIDTH;
    nowY = maxY-mininterval-initHeight;
    for(i=upNums+rightNums;i<(upNums+rightNums+downNums);i++)
    {
        labelLeftUpXy.push_back(make_pair(nowX,nowY));
//        arrangeArroundLabels(locationXy[i].first,locationXy[i].second,nowX,nowY,m_pDmisModel->findLabel(labelName[i]),false);
        if(i == 6)
        {
            if(labelNums == 7)
            {
                nowX = 0;
            }
            else
            {
//                nowX = nowX - labelWidth[i+1] - intervalDown;
                nowX = nowX - LBWIDTH - intervalDown;
            }
        }
        else
        {
            if(i == (upNums+rightNums+downNums -2))
            {
                nowX = mininterval;
            }
            else
            {
//                nowX = nowX - intervalDown - labelWidth[i+1];
                nowX = nowX - intervalDown - LBWIDTH;
            }
        }
    }
    nowX = mininterval;
    nowY = maxY - mininterval - initHeight - intervalLeft - initHeight;
    for(i=upNums+rightNums+downNums;i<labelNums;i++)
    {
        labelLeftUpXy.push_back(make_pair(nowX,nowY));
//        arrangeArroundLabels(locationXy[i].first,locationXy[i].second,nowX,nowY,m_pDmisModel->findLabel(labelName[i]),false);
        nowY = nowY - initHeight - intervalLeft;
    }
    showLabelandLine(locationXy,labelLeftUpXy,labelName);
}
