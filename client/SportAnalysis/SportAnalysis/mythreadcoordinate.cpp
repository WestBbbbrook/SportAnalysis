#include "mythreadcoordinate.h"
WinsockMatTransmissionClient socketCoordinate;
int mouseX;
int mouseY;
MythreadCoordinate::MythreadCoordinate()
{

}
void MythreadCoordinate::run(){

    //emit Log(QString("AA"));
    //发送鼠标点击的坐标，格式： click:x:y
    string str = "click:"+std::to_string(mouseX)+":"+std::to_string(mouseY);
    socketCoordinate.sendString(str);
}
