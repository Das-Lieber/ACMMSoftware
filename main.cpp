#include "mainwindow.h"
#include <QApplication>
#include <QSplashScreen>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QPixmap splashMap(":/res/splash.jpg");
    QSplashScreen *splash = new QSplashScreen(splashMap,Qt::WindowStaysOnTopHint);
    splash->show();
    splash->setFont(QFont("Microsoft Yahei UI",12));
    splash->showMessage("软件初始化中，请稍后......",Qt::AlignBottom|Qt::AlignLeft);

    MainWindow window;
    window.initFunction();
    window.initRibbon();
    window.initToolBar();
    window.initDockWidget();
    window.initRobot();
    window.showMaximized();

    splash->finish(&window);
    delete splash;

    return a.exec();
}
