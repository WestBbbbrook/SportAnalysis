#include "mainwindow.h"
#include "mythread.h"
#include "winsockmattransmissionclient.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    QPalette pal(w.palette());
    //设置背景黑色
    pal.setColor(QPalette::Background, Qt::white);
    w.setAutoFillBackground(true);
    w.setPalette(pal);
    w.show();
    return a.exec();
}
