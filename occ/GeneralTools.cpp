#include "GeneralTools.h"

#include <gp_Lin.hxx>
#include <gp_Pln.hxx>
#include <gp_Elips.hxx>
#include <gp_Sphere.hxx>
#include <gp_Cylinder.hxx>
#include <math_Matrix.hxx>
#include <GProp_PEquation.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopExp.hxx>
#include <BRep_Tool.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Compound.hxx>
#include <BRep_Builder.hxx>
#include <ShapeBuild_ReShape.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS_Edge.hxx>

#include "math.h"
#include "pca.h"

GeneralTools::GeneralTools(void)
{
}

bool GeneralTools::FitLine(list<gp_Pnt> pts, gp_Lin &line)
{
    if(!pts.empty())
    {
        TColgp_Array1OfPnt Pnts(1,pts.size());
        int i=1;
        for (list<gp_Pnt>::iterator it = pts.begin();it != pts.end();it++)
        {
            Pnts(i++) = *it;
        }
        GProp_PEquation gpe(Pnts, 1);
        if(gpe.IsLinear())
        {
            line = gpe.Line();
            return true;
        }
    }
    return false;
}

bool GeneralTools::FitPlane(list<gp_Pnt> pts, gp_Pln &plane)
{
    if(!pts.empty())
    {
        TColgp_Array1OfPnt Pnts(1,pts.size());
        int i=1;
        for (list<gp_Pnt>::iterator it = pts.begin();it != pts.end();it++)
        {
            Pnts(i++) = *it;
        }
        GProp_PEquation gpe(Pnts, 1);
        if(gpe.IsPlanar())
        {
            plane = gpe.Plane();
            return true;
        }
    }
    return false;
}

bool GeneralTools::FitCylinder(QList<gp_Pnt> pts, gp_Cylinder &cylinder)
{
    #define  MAX_POINT_NUM 2000

    int i=0,j=0;              //循环用变量
    int int_temp=pts.size();
    int loop_times=0,pp=0;    //loop_time等价于t，表示迭代循环次数
    double a=1,b=1,c=1;
    double x0=0,y0=0,z0=0;
    double D=0,s=0,S=0,dx=0,dy=0,dz=0;
    double R=0;                               //半径
    double d_temp1=0,d_temp2=0,d_temp3=0;
    double B[MAX_POINT_NUM][7]={{0}};
    double L[MAX_POINT_NUM]   ={0};

    double mean_x=0,mean_y=0,mean_z=0;
    bool while_flag=1;

    math_Matrix C(1,2,1,7);
    math_Matrix W(1,2,1,1);
    math_Matrix N(1,9,1,9);
    math_Matrix N_inv(1,9,1,9);
    math_Matrix UU(1,9,1,1);
    math_Matrix para(1,9,1,1);//参数矩阵
    //变量初始化
    C.Init(0);W.Init(0);N.Init(0);N_inv.Init(0);UU.Init(0);
    para.Init(0);
    para(1,1)=1;

    //求解重心、初始化
    for(i=0;i<int_temp;i++)
    {
        d_temp1+=pts[i].X();
        d_temp2+=pts[i].Y();
        d_temp3+=pts[i].Z();
    }
    mean_x=d_temp1/int_temp; mean_y=d_temp2/int_temp;mean_z=d_temp3/int_temp;
    x0=mean_x;y0=mean_y;z0=mean_z;
    R =20;
    //迭代循环，最优化矩阵para
    while(while_flag==true)       //即(max(abs(para(1:7)))>0.00001)
    {
        //1. a、b、c的符号修正
        if(a<0)
        {
            a=-a;b=-b;c=-c;
        }
        if(a==0)
        {
            if(b<0)
            {
                b=-b;c=-c;
            }
            if(b==0)
            {
                if(c<0)
                    c=-c;
            }
        }
        s=sqrt(pow(a,2)+pow(b,2)+pow(c,2))+0.0000001;       //防止a b c 同时为0
        a=a/s+0.0000001;b=b/s+0.0000001;c=c/s+0.0000001;
        //2. 计算矩阵B和L
        for(i=0;i<int_temp;i++)                              //int_temp为点云总数
        {
            //数据计算
            D=a*(pts[i].X()-x0)+b*(pts[i].Y()-y0)+c*(pts[i].Z()-z0);       //D=a*(X(i)-x0)+b*(Y(i)-y0)+c*(Z(i)-z0);
            dx=x0+a*D-pts[i].X();dy=y0+b*D-pts[i].Y();dz=z0+c*D-pts[i].Z();//dx=x0+a*D-X(i);dy=y0+b*D-Y(i);dz=z0+c*D-Z(i);
            S=sqrt(pow(dx,2)+pow(dy,2)+pow(dz,2));                                           //S=sqrt(dx^2+dy^2+dz^2);
            B[i][0]=(dx*(a*(pts[i].X()-x0)+D)+dy*b*(pts[i].X()-x0)+dz*c*(pts[i].X()-x0))/S;   //b1
            B[i][1]=(dx*a*(pts[i].Y()-y0)+dy*(b*(pts[i].Y()-y0)+D)+dz*c*(pts[i].Y()-y0))/S;
            B[i][2]=(dx*a*(pts[i].Z()-z0)+dy*b*(pts[i].Z()-z0)+dz*(c*(pts[i].Z()-z0)+D))/S;
            B[i][3]=(dx*(1-pow(a,2))-dy*a*b-dz*a*c)/S;
            B[i][4]=(-dx*a*b+dy*(1-pow(b,2))-dz*b*c)/S;
            B[i][5]=(-dx*a*c-dy*b*c+dz*(1-pow(c,2)))/S;
            B[i][6]=-1;
            //数据存储

            //B=[B;b1 b2 b3 b4 b5 b6 b7]; 此步骤已整合至上10行的B[i][0-6]中
            L[i]=R-S;  //l=[R-S];
            //L=[L;l];
        }
        //3. 计算矩阵C(2*7)和W(2*1)
        d_temp1=1-pow(a,2)-pow(b,2)-pow(c,2);

        if(fabs(a)>=fabs(b) && fabs(a)>=fabs(c))
        {
            C.Init(0);W.Init(0);
            C(1,1) = 2*a; C(1,2) = 2*b;
            C(1,3) = 2*c; C(2,4) = 1;

            W(1,1) = d_temp1; W(2,1) = mean_x-x0;
        }
        if(fabs(b)>=fabs(a) && fabs(b)>=fabs(c))
        {
            C.Init(0);W.Init(0);
            C(1,1) = 2*a; C(1,2) = 2*b;
            C(1,3) = 2*c; C(2,5) = 1;

            W(1,1) = d_temp1; W(2,1) = mean_y-y0;
        }
        if(fabs(c)>=fabs(a) && fabs(c)>=fabs(b))
        {
            C.Init(0);W.Init(0);
            C(1,1) = 2*a; C(1,2) = 2*b;
            C(1,3) = 2*c; C(2,6) = 1;

            W(1,1) = d_temp1; W(2,1) = mean_z-z0;
        }
        //4. 计算para矩阵
        //Nbb=B'*B;U=B'*L;
        //N=[Nbb C';C zeros(2)];
        //UU=[U;W];
        //para=inv(N)*UU;
        //4.1 计算矩阵N
//        cvZero(N);        // N= |Nbb(7*7)  C'(7*2)|
        N.Init(0);
        //    |C  (2*7)  O(2*2) |
        for(i=0;i<7;i++)
        {
            for(j=0;j<7;j++)
            {
                d_temp1=0;
                for(pp=0;pp<int_temp;pp++)
                {
                    d_temp1+=B[pp][i]*B[pp][j];
                }
                N(i+1,j+1) = d_temp1;
            }
        }
        for(i=0;i<2;i++)
            for(j=0;j<7;j++)
                N(i+8,j+1) = C(i+1,j+1);
        for(i=0;i<2;i++)
            for(j=0;j<7;j++)
                N(j+1,i+8) = C(i+1,j+1);
        //4.2 计算矩阵UU
        for(i=0;i<7;i++)
        {
            d_temp1=0;
            for(pp=0;pp<int_temp;pp++)
            {
                d_temp1+=B[pp][i]*L[pp];
            }
            UU(i+1,1) = d_temp1;
        }
        for(i=0;i<2;i++)
            UU(i+8,1) = W(i+1,1);
        //4.2 计算矩阵para
        N_inv = N.Inverse();
        para = N_inv.Multiplied(UU);
        a = a+para(1,1);
        b = b+para(2,1);
        c = c+para(3,1);
        x0 = x0+para(4,1);
        y0 = y0+para(5,1);
        z0 = z0+para(6,1);
        R = R+para(7,1);
        loop_times=loop_times+1;     //t=t+1
        //5. 计算while标志while_flag为0或者1：若max(abs(para(1:7)))>0.00001，则while_flag=1；否则，为0.
        d_temp1 = para(1,1);
        for(i=1;i<7;i++)
        {
            if(fabs(d_temp1) < fabs(para(i+1,1)))
                d_temp1 = para(i+1,1);
        }
        if(fabs(d_temp1)>0.000001)
            while_flag=1;
        else
            while_flag=0;
    }
    cout<<"loop times:"<<loop_times<<endl;
    cylinder.SetRadius(R);
    cylinder.SetAxis(gp_Ax1(gp_Pnt(x0,y0,z0),gp_Dir(a,b,c)));
    return true;
}

bool GeneralTools::FitCone(list<gp_Pnt> pts, gp_Cone &cone)
{
        return false;
}

bool GeneralTools::FitEllips(list<gp_Pnt> pts,gp_Elips& aElips)
{
    double tot = 0.001;
    PCA pca(pts);
    pca.Run();
    vector<gp_Vec> aVectors = pca.GetEigenVector();
    gp_Pnt acenter = pca.GetCenterPoint();
    if(aVectors[0].Magnitude ()<=tot || aVectors[1].Magnitude ()<=tot)
        return false;
    gp_Vec normalV = aVectors[0].Crossed(aVectors[1]);
    normalV.Normalize();
    gp_Ax2 coordAx(acenter,normalV,aVectors[0]);
    list<gp_Pnt2d> pts2d = TranslatePntToPnt2d(pts,coordAx);

    long double A = 0.00,B = 0.00,C = 0.00,D = 0.00,E = 0.00;
    long double x2y2=0.0,x1y3=0.0,x2y1=0.0,x1y2=0.0,x1y1=0.0,yyy4=0.0,yyy3=0.0,yyy2=0.0,xxx2=0.0,xxx1=0.0,yyy1=0.0,x3y1=0.0,xxx3=0.0;
    int N = pts2d.size();
    for(list<gp_Pnt2d>::iterator it=pts2d.begin();it!=pts2d.end();it++)
    {
        long double xi = it->X(), yi = it->Y();

        x2y2 += xi*xi*yi*yi;
        x1y3 += xi*yi*yi*yi;
        x2y1 += xi*xi*yi;
        x1y2 += xi*yi*yi;
        x1y1 += xi*yi;
        yyy4 += yi*yi*yi*yi;
        yyy3 += yi*yi*yi;
        yyy2 += yi*yi;
        xxx2 += xi*xi;
        xxx1 += xi;
        yyy1 += yi;
        x3y1 += xi*xi*xi*yi;
        xxx3 += xi*xi*xi;
    }
    long double matrix[5][5]={{x2y2,x1y3,x2y1,x1y2,x1y1},
                              {x1y3,yyy4,x1y2,yyy3,yyy2},
                              {x2y1,x1y2,xxx2,x1y1,xxx1},
                              {x1y2,yyy3,x1y1,yyy2,yyy1},
                              {x1y1,yyy2,xxx1,yyy1,static_cast<long double>(N)}
                             };

    long double matrix2[5][1]={{x3y1},{x2y2},{xxx3},{x2y1},{xxx2}};
    long double matrix3[5][1] = {{A},{B},{C},{D},{E}};

    ////求矩阵matrix的逆，结果为InverseMatrix
    ///单位矩阵
    int n = 5;
    long double E_Matrix[5][5];
    long double mik;
    long double m = 2*n;
    for(int i = 0; i < n; i++)
    {
        for(int j = 0; j < n; j++)
        {
            if(i == j)
                E_Matrix[i][j] = 1.00;
            else
                E_Matrix[i][j] = 0.00;
        }
    }
    long double CalcuMatrix[5][2*5];
    for(int i = 0; i < n; i++)
    {
        for(int j = 0; j < n; j++)
        {
            CalcuMatrix[i][j] = matrix[i][j];
        }
        for(int k = n; k < m; k++)
        {
            CalcuMatrix[i][k] = E_Matrix[i][k-n];
        }
    }

    for(int i = 1; i <= n-1; i++)
    {
        for(int j = i+1; j <= n; j++)
        {
            mik = CalcuMatrix[j-1][i-1]/CalcuMatrix[i-1][i-1];
            for(int k = i+1;k <= m; k++)
            {
                CalcuMatrix[j-1][k-1] -= mik*CalcuMatrix[i-1][k-1];
            }
        }
    }
    for(int i=1;i<=n;i++)
    {
        long double temp = CalcuMatrix[i-1][i-1];
        for(int j=1;j<=m;j++)
        {
            CalcuMatrix[i-1][j-1] = CalcuMatrix[i-1][j-1]/temp;
        }
    }
    for(int k=n-1;k>=1;k--)
    {
        for(int i=k;i>=1;i--)
        {
            mik = CalcuMatrix[i-1][k];
            for(int j=k+1;j<=m;j++)
            {
                CalcuMatrix[i-1][j-1] -= mik*CalcuMatrix[k][j-1];
            }
        }
    }
    long double InverseMatrix[5][5];
    for(int i=0;i<n;i++)
    {
        for(int j=0;j<n;j++)
        {
            InverseMatrix[i][j] = CalcuMatrix[i][j+n];
        }
    }
    for(int i=0;i<5;i++)
    {
        for(int j=0;j<5;j++)
        {
            if(fabs(InverseMatrix[i][j]) < 0.0000001)
                InverseMatrix[i][j] = 0.00;
        }
    }
    ///求参数A,B,C,D,E
    for(int i=0;i<5;i++)
    {
        for(int j=0;j<5;j++)
        {
            matrix3[i][0] += InverseMatrix[i][j]*(-matrix2[j][0]);
        }
    }
    A = matrix3[0][0];
    B = matrix3[1][0];
    C = matrix3[2][0];
    D = matrix3[3][0];
    E = matrix3[4][0];
    ///求拟合结果重要参数
    double Xc = (2*B*C-A*D)/(A*A-4*B);
    double Yc = (2*D-A*D)/(A*A-4*B);
    double a = sqrt(fabs(2*(A*C*D-B*C*C-D*D+4*B*E-A*A*E)/((A*A-4*B)*(B-sqrt(A*A+(1-B)*(1-B))+1))));
    double b = sqrt(fabs(2*(A*C*D-B*C*C-D*D+4*B*E-A*A*E)/((A*A-4*B)*(B+sqrt(A*A+(1-B)*(1-B))+1))));
    double theta = atan2(a*a-b*b*B,a*a*B-b*b);
    gp_Pnt centerP = TranslatePnt2dToPnt(gp_Pnt2d(Xc,Yc),coordAx);
    coordAx.Translate(acenter,centerP);
    gp_Ax2 otherAx = coordAx.Rotated(coordAx.Axis(),theta);
    gp_Elips aElips1(otherAx,a>b?a:b,a>b?b:a);
    aElips = aElips1;
    return true;
}

bool GeneralTools::FitCone(Handle(Geom_Surface) aSurface,gp_Ax1& axR)
{
    Standard_Real LowerU=0;
    Standard_Real UpperU=0;
    Standard_Real LowerV=0;
    Standard_Real UpperV=0;
    aSurface->Bounds(LowerU, UpperU, LowerV, UpperV);
    Standard_Real nowU = 0;
    Standard_Real nowV = 0;
    if(LowerU != Precision::Infinite())
        nowU = LowerU;
    else if(UpperU != Precision::Infinite())
        nowU = UpperU;
    if(LowerV != Precision::Infinite())
        nowV = LowerV;
    else if(UpperV != Precision::Infinite())
        nowV = UpperV;
    Handle(Geom_Curve) Ucurve = aSurface->UIso(nowU);
    gp_Circ cir;
    if(!GetCicle(Ucurve, cir))
    {
        Handle(Geom_Curve) Vcurve = aSurface->UIso(nowV);
        if(!GetCicle(Vcurve, cir))
        {
            return false;
        }
    }
    axR.SetLocation(cir.Location());
    axR.SetDirection(cir.Axis().Direction());
    return true;
}

bool GeneralTools::GetCicle(Handle(Geom_Curve) aCurve,gp_Circ& cir)
{
    Standard_Real firstParameter = aCurve->FirstParameter();
    Standard_Real lastParameter = aCurve->LastParameter();
    int num=100;
    Standard_Real step = (lastParameter - firstParameter)/num;
    list<gp_Pnt> pts;
    for(int i=0;i<=num;i++)
    {
        gp_Pnt P(0,0,0);
        aCurve->D0(firstParameter+step*i,P);
        pts.push_back(P);
    }
    double temp;
    return FitCicle(pts,cir,temp);
}

bool GeneralTools::FitCicle(list<gp_Pnt> pts, gp_Circ& cir, double &error)
{
    if (pts.size() < 3)
        return false;
    double tot = 0.001;
    PCA pca(pts);
    pca.Run();
    vector<gp_Vec> aVectors = pca.GetEigenVector();
    gp_Pnt acenter = pca.GetCenterPoint();
    if(aVectors[0].Magnitude ()<=tot || aVectors[1].Magnitude ()<=tot)
        return false;
    gp_Vec normalV = aVectors[0].Crossed(aVectors[1]);
    normalV.Normalize();
    gp_Ax2 coordAx(acenter,normalV,aVectors[0]);
    list<gp_Pnt2d> pts2d = TranslatePntToPnt2d(pts,coordAx);
    gp_Pnt2d Center2d(0,0);
    Standard_Real Radius = 0;
    if(!FitCicle(pts2d,Center2d,Radius))
    {
        return false;
    }
    gp_Pnt centerP = TranslatePnt2dToPnt(Center2d,coordAx);
    gp_Circ cir1(coordAx.Translated(coordAx.Location(),centerP),Radius);
    cir = cir1;


    double max = 0;
    double min = Center2d.Distance(pts2d.front());
    foreach(gp_Pnt2d pnt,pts2d) {
        max = (max>Center2d.Distance(pnt))? max : Center2d.Distance(pnt);
        min = (min<Center2d.Distance(pnt))? min : Center2d.Distance(pnt);
    }
    error = max-min;

    return true;
}

bool GeneralTools::FitCicle(list<gp_Pnt2d> pts2d,gp_Pnt2d &Center, Standard_Real& Radius)
{
    if (pts2d.size() < 3)
        return false;
    double X1,X2, X3,Y1,Y2,Y3,X1Y1,X1Y2,X2Y1;
    X1=X2=X3=Y1=Y2=Y3=X1Y1=X1Y2=X2Y1=0;
    int i = 0;
    bool Equalx = false, Equaly = false;
    for (list<gp_Pnt2d>::iterator it=pts2d.begin();it!=pts2d.end();it++)
    {
        X1 = X1 + it->X();
        Y1 = Y1 + it->Y();
        if (it->Y() != Y1 / (i + 1))
        {
            Equaly = true;
        }
        if (it->X() != X1 / (i + 1))
        {
            Equalx = true;
        }
        X2 = X2 + it->X() * it->X();
        Y2 = Y2 + it->Y() * it->Y();
        X3 = X3 + it->X() * it->X() * it->X();
        Y3 = Y3 + it->Y() * it->Y() *it->Y();
        X1Y1 = X1Y1 + it->X() * it->Y();
        X1Y2 = X1Y2 + it->X() * it->Y() * it->Y();
        X2Y1 = X2Y1 + it->X() * it->X() * it->Y();
        i++;
    }
    if(!Equalx||!Equaly)
    {
        return false;
    }
    double C, D, E, G, H, N;
    double a, b, c;
    N = pts2d.size();
    C = N*X2 - X1*X1;
    D = N*X1Y1 - X1*Y1;
    E = N*X3 + N*X1Y2 - (X2 + Y2)*X1;
    G = N*Y2 - Y1*Y1;
    H = N*X2Y1 + N*Y3 - (X2 + Y2)*Y1;
    a = (H*D - E*G) / (C*G - D*D + 1e-10);
    b = (H*C - E*D) / (D*D - G*C + 1e-10);
    c = -(a*X1 + b*Y1 + X2 + Y2) / (N + 1e-10);

    Center.SetCoord(a / (-2),b / (-2));
    double dTemp = a*a + b*b - 4 * c;
    if (dTemp < 0)
    {
        return false;
    }
    Radius = sqrt(dTemp) / 2;
    return true;
}

bool GeneralTools::FitSphere(list<gp_Pnt> pts,gp_Sphere& sphere)
{
    if (pts.size() < 3)
    {
        return false;
    }
    double sum_x = 0.0f, sum_y = 0.0f, sum_z = 0.0f;
    double sum_x2 = 0.0f, sum_y2 = 0.0f, sum_z2 = 0.0f;
    double sum_x3 = 0.0f, sum_y3 = 0.0f, sum_z3 = 0.0f;
    double sum_xy = 0.0f, sum_x1y2 = 0.0f, sum_x2y1 = 0.0f;
    double sum_xz = 0.0f, sum_x1z2 = 0.0f, sum_x2z1 = 0.0f;
    double sum_yz = 0.0f, sum_y1z2 = 0.0f, sum_y2z1 = 0.0f;
    int N = pts.size();
    for (list<gp_Pnt>::iterator it = pts.begin();it != pts.end();it++)
    {
        double x = it->X();
        double y = it->Y();
        double z = it->Z();
        double x2 = x * x;
        double y2 = y * y;
        double z2 = z * z;
        sum_x += x;
        sum_y += y;
        sum_z += z;

        sum_x2 += x2;
        sum_y2 += y2;
        sum_z2 += z2;

        sum_x3 += x2 * x;
        sum_y3 += y2 * y;
        sum_z3 += z2 * z;

        sum_xy += x * y;
        sum_x1y2 += x * y2;
        sum_x2y1 += x2 * y;

        sum_xz += x * z;
        sum_x1z2 += x * z2;
        sum_x2z1 += x2 * z;

        sum_yz += y * z;
        sum_y1z2 += y * z2;
        sum_y2z1 += y2 * z;
    }
    double C, D, E, P, G, F, H, J, K;
    double a, b, c, d;
    C = N * sum_x2 - sum_x * sum_x;
    D = N * sum_xy - sum_x * sum_y;
    P = N * sum_xz - sum_x * sum_z;
    E = N * sum_x3 + N * sum_x1y2 + N * sum_x1z2 - (sum_x2 + sum_y2 + sum_z2) * sum_x;

    G = N * sum_y2 - sum_y * sum_y;
    F = N * sum_yz - sum_y * sum_z;
    H = N * sum_x2y1 + N * sum_y3 + N * sum_y1z2 - (sum_x2 + sum_y2 + sum_z2) * sum_y;

    J = N * sum_z2 - sum_z * sum_z;
    K = N * sum_x2z1 + N * sum_y2z1 + N * sum_z3 - (sum_x2 + sum_y2 + sum_z2) * sum_z;

    double CC, DD, EE, GG, HH;
    CC = C*J - P*P;
    DD = D*J - F*P;
    EE = E*J - P*K;
    GG = G*J - F*F;
    HH = H*J - F*K;

    a = (HH * DD - EE * GG) / (CC * GG - DD * DD);
    b = (HH * CC - EE * DD) / (DD * DD - GG * CC);
    c = 0.0f;
    if (P != 0)
    {
        c = (-C*a - D*b - E) / P;
    }
    else if (F != 0)
    {
        c = (-D*a - G*b - H) / F;
    }
    else if (J != 0)
    {
        c = (-P*a - F*b - K) / J;
    }
    d = -(a * sum_x + b * sum_y + c * sum_z + sum_x2 + sum_y2 + sum_z2) / N;

    gp_Pnt centerP(a / (-2),b / (-2),c / (-2));
    gp_Ax2 ax2(centerP,gp::OZ().Direction());
    Standard_Real R = sqrt(a * a + b * b + c * c - 4 * d) / 2;
    gp_Sphere sphere1(ax2,R);
    sphere = sphere1;
    return true;
}

double GeneralTools::EvaluateLine(const QList<gp_Pnt> &pts, const gp_Lin &line)
{
    double max = 0;
    foreach(gp_Pnt pnt, pts) {
        max = (max>line.Distance(pnt))? max : line.Distance(pnt);
    }
    return max;
}

double GeneralTools::EvaluatePlane(const QList<gp_Pnt> &pts, const gp_Pln &plane)
{
    double max = 0;
    foreach(gp_Pnt pnt, pts) {
        max = (max>plane.Distance(pnt))? max : plane.Distance(pnt);
    }
    return max;
}

double GeneralTools::EvaluateSphere(const QList<gp_Pnt> &pts, const gp_Sphere &sphere)
{
    gp_Pnt center = sphere.Location();
    double max = 0;
    double min = center.Distance(pts[0]);
    foreach(gp_Pnt pnt, pts) {
        max = (max>center.Distance(pnt))? max : center.Distance(pnt);
        min = (min<center.Distance(pnt))? min : center.Distance(pnt);
    }
    return max-min;
}

double GeneralTools::EvaluateCylinder(const QList<gp_Pnt> &pts, const gp_Cylinder &cylinder)
{
    gp_Ax1 axis = cylinder.Axis();
    gp_Lin center(axis);
    double max = 0;
    double min = center.Distance(pts[0]);
    foreach(gp_Pnt pnt, pts) {
        max = (max>center.Distance(pnt))? max : center.Distance(pnt);
        min = (min<center.Distance(pnt))? min : center.Distance(pnt);
    }
    return max-min;
}

list<gp_Pnt2d> GeneralTools::TranslatePntToPnt2d(list<gp_Pnt> points,gp_Ax2 coordAx)
{
    list<gp_Pnt2d> points2d;
    //int count = points.size();
    //points2d.reserve(count);
    math_Matrix matrix(0,3,0,3);  //变换矩阵
    matrix(0,0) = coordAx.XDirection().X();
    matrix(0,1) = coordAx.YDirection().X();
    matrix(0,2) = coordAx.Direction().X();
    matrix(0,3) = coordAx.Location().X();
    matrix(1,0) = coordAx.XDirection().Y();
    matrix(1,1) = coordAx.YDirection().Y();
    matrix(1,2) = coordAx.Direction().Y();
    matrix(1,3) = coordAx.Location().Y();
    matrix(2,0) = coordAx.XDirection().Z();
    matrix(2,1) = coordAx.YDirection().Z();
    matrix(2,2) = coordAx.Direction().Z();
    matrix(2,3) = coordAx.Location().Z();
    matrix(3,0) = 0;
    matrix(3,1) = 0;
    matrix(3,2) = 0;
    matrix(3,3) = 1;

    math_Matrix matrixInv(0,3,0,3);
    matrixInv = matrix.Inverse();
    for (list<gp_Pnt>::iterator it=points.begin();it != points.end();it++)
    {
        math_Matrix matrixP(0,3,0,0);
        matrixP(0,0) = it->X();
        matrixP(1,0) = it->Y();
        matrixP(2,0) = it->Z();
        matrixP(3,0) = 1;
        math_Matrix matrixP2(0,3,0,0);
        matrixP2 = matrixInv * matrixP;
        gp_Pnt2d P2d(matrixP2(0,0),matrixP2(1,0));
        points2d.push_back(P2d);
    }
    return points2d;
}

gp_Pnt GeneralTools::TranslatePnt2dToPnt(gp_Pnt2d point,gp_Ax2 coordAx)
{
    /*if(!hasCoordAx)
        return false;*/
    gp_Pnt Tpoint;
    math_Matrix matrix(0,3,0,3);  //变换矩阵
    matrix(0,0) = coordAx.XDirection().X();
    matrix(0,1) = coordAx.YDirection().X();
    matrix(0,2) = coordAx.Direction().X();
    matrix(0,3) = coordAx.Location().X();
    matrix(1,0) = coordAx.XDirection().Y();
    matrix(1,1) = coordAx.YDirection().Y();
    matrix(1,2) = coordAx.Direction().Y();
    matrix(1,3) = coordAx.Location().Y();
    matrix(2,0) = coordAx.XDirection().Z();
    matrix(2,1) = coordAx.YDirection().Z();
    matrix(2,2) = coordAx.Direction().Z();
    matrix(2,3) = coordAx.Location().Z();
    matrix(3,0) = 0;
    matrix(3,1) = 0;
    matrix(3,2) = 0;
    matrix(3,3) = 1;
    math_Matrix matrixP(0,3,0,0);
    matrixP(0,0) = point.X();
    matrixP(1,0) = point.Y();
    matrixP(2,0) = 0;
    matrixP(3,0) = 1;
    math_Matrix matrixP2(0,3,0,0);
    matrixP2 = matrix * matrixP;
    Tpoint.SetCoord(matrixP2(0,0),matrixP2(1,0),matrixP2(2,0));
    return Tpoint;
}
list<TopoDS_Shape> GeneralTools::HandleProbeStyliShape(TopoDS_Shape shape)
{
    TopTools_IndexedMapOfShape aFaces;
    TopExp::MapShapes(shape, TopAbs_FACE, aFaces);
    list<int> spheres;
    map<int,Standard_Real> nodeDistance;
    for(int i=1;i<aFaces.Extent();i++)
    {
        Standard_Real maxD = 0;
        TopTools_IndexedMapOfShape aVertex;
        TopExp::MapShapes(aFaces(i), TopAbs_VERTEX, aVertex);
        for(int j=1;j<aVertex.Extent();j++)
        {
            gp_Pnt Pt = BRep_Tool::Pnt(TopoDS::Vertex(aVertex(j)));
            Standard_Real nowZ = Pt.Distance(gp_Pnt(0,0,0));
            if(nowZ>maxD)
            {
                maxD = nowZ;
            }
        }
        nodeDistance[i] = maxD;
    }
    int styliNum = 3;
    int ssize = spheres.size();
    while(ssize<styliNum)
    {
        int maxD = 0;
        int maxFace = 0;
        for (map<int,Standard_Real>::iterator it=nodeDistance.begin();it != nodeDistance.end();it++)
        {
            if (it->second > maxD)
            {
                maxD = it->second;
                maxFace = it->first;
            }
        }
        spheres.push_back(maxFace);
        nodeDistance.erase(maxFace);
        ssize = spheres.size();
    }

    list<TopoDS_Shape> otherShapes;
    list<TopoDS_Shape> sphereShapes;
    for(int i=1;i<aFaces.Extent();i++)
    {
        if(find(spheres.begin(),spheres.end(),i)==spheres.end())
            otherShapes.push_back(aFaces(i));
        else
            sphereShapes.push_back(aFaces(i));
    }
    list<TopoDS_Shape> reShapes;
    reShapes.push_back(MakeCompoundFromShapes(sphereShapes));
    reShapes.push_back(MakeCompoundFromShapes(otherShapes));
    return reShapes;
}
TopoDS_Shape GeneralTools::MakeCompoundFromShapes(list<TopoDS_Shape> origalShapes)
{
     TopoDS_Compound aRes;
     BRep_Builder aBuilder;
     aBuilder.MakeCompound (aRes);
     for (list<TopoDS_Shape>::iterator it = origalShapes.begin();it != origalShapes.end();it++)
     {
         aBuilder.Add (aRes, *it);
     }
     return aRes;
}
TopoDS_Shape GeneralTools::RemoveDegenerateEdge(TopoDS_Shape oldshape)
{
    Handle_ShapeBuild_ReShape rebuild = new ShapeBuild_ReShape;
    rebuild->Apply(oldshape);
    TopExp_Explorer exp1;
    for (exp1.Init (oldshape, TopAbs_EDGE); exp1.More(); exp1.Next())
    {
        TopoDS_Edge edge = TopoDS::Edge(exp1.Current());
        if ( BRep_Tool::Degenerated(edge) )
            rebuild->Remove(edge);
    }
    TopoDS_Shape newshape = rebuild->Apply(oldshape);
    return newshape;
}
