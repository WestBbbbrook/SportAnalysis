#include "readfilethread.h"
float w_label_9;
int now;
double xpianyi[]={480,480,0,0};
double ypianyi[]={180,180,0,0};

readFileThread::readFileThread()
{

}
void readFileThread::SplitString(const string &s, vector<string> &v, const string &c)
{
    string::size_type pos1, pos2;
    pos2 = s.find(c);
    pos1 = 0;
    while(string::npos != pos2)
    {
        v.push_back(s.substr(pos1, pos2 - pos1));

        pos1 = pos2 + c.size();
        pos2 = s.find(c, pos1);
    }
    if(pos1 != s.length())
        v.push_back(s.substr(pos1));
}
vector<vector<Point> > readFileThread::readDBPoints(string txtfilename)
{
//	char filename[100];
//	strcpy(filename, txtfilename);
    //ifstream in("D:\\Projects\\opencv\\MODELPOINT\\fanshou-left.txt");
    ifstream in(txtfilename.c_str());
    string txtline;
    vector<vector<Point> > pointsframes;
    int step=0;
    while (getline(in, txtline))//逐行读取数据并存于s中，直至数据全部读取
    {
        if((step++)%3!=0)
            continue;
        //std::cout << txtline.c_str() << std::endl;
        vector<string> splitwords;
        SplitString(txtline, splitwords, ":");
        //std::cout << splitwords.size() << std::endl;
        vector<Point> points;
        int vindex = 1;
        float ratio=w_label_9/W_MD;
        for(int i = 0; i < 18; ++i)
        {
            double x = atof(splitwords[vindex++].c_str());
            double y = atof(splitwords[vindex++].c_str());
            //points.push_back(Point((x-X_CHANGE_MD)*ratio,(y-Y_CHANGE_MD)*ratio));
            //points.push_back(Point(x-X_CHANGE_MD,y-Y_CHANGE_MD));//可以用
            //points.push_back(Point(x-480,y-180));//可以用***
            points.push_back(Point(x-xpianyi[now],y-ypianyi[now]));//可以用***
            //std::cerr<<qPrintable("pppppp");

            //points.push_back(Point(x,y));
            //std::cout << x << " " << y << endl;
        }
        pointsframes.push_back(points);
        //std::cout << stod(splitwords[1])<<" "<<stod(splitwords[2]);
        //std::cout << std::endl;

    }
    in.close();
    return pointsframes;
}
void readFileThread::run(){
    vector<string> files;
    vector<vector<Point> >localMediaPoint;
    std::cerr<<qPrintable("zzz");
    QDir dir("E:\\Qt\\MODELPOINT");
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    QFileInfoList list = dir.entryInfoList();
    for (int i = 0; i < list.size(); ++i) {
             QFileInfo fileInfo = list.at(i);
             files.push_back("E:\\Qt\\MODELPOINT\\"+fileInfo.fileName().toStdString());
             cerr<<qPrintable(QString::fromStdString("E:\\Qt\\MODELPOINT\\"+fileInfo.fileName().toStdString()));
         }
//    //获取该路径下的所有文件
//    getFiles("E:\\Qt\\MODELPOINT", files );
//    std::cerr<<qPrintable("duwenjianming");
    sort(files.begin(), files.end());
    int size = files.size();
    std::cerr<<qPrintable("paixu ");

    for (int i = 0;i < size;i++)
    {
        std::cerr<<qPrintable("hahha");

        //cout<<files[i].c_str()<<endl;
        localMediaPoint=readDBPoints(string(files[i]));
        localAllPoint.push_back(localMediaPoint);
        localMediaPoint.clear();
        std::cerr<<qPrintable(QString::number(i));
        now=i;//!!
        std::cerr<<qPrintable("hehhe");

    }
}
