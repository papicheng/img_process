#include "imgprocess.h"
#include "qdir.h"
#include "qstring.h"
#include <direct.h>
using namespace cv;
using namespace std;

void generateDirecorys_cbc(string path){//支持中文
    string tmpPath = "";
    for(auto ch : path){
        tmpPath += ch;
        if(ch == '/' || ch == '\\'){
            if(_access(tmpPath.c_str(), 0) != 0){
                int ret = _mkdir(tmpPath.c_str());
                if(ret != 0){
                    cout<<"mkdir error~"<<endl;
                    return;
                }
            }
        }
    }
}

void generateDirecorys(QString directory_){//不支持中文
    QString directory = directory_.toLocal8Bit();
    QDir sourceDir(directory);
    if(sourceDir.exists()) return;
    QString tempDir;
    QStringList directorys = directory.split("/");
    for(int i = 0;i < directorys.count() - 1;++i){
        QString path = directorys[i];
        tempDir += path + "/";
        QDir dir(tempDir);
        if(!dir.exists() &&  !dir.mkdir(tempDir)) return;
    }
}

void img_process(string path, string outputPath, string filename) {
    outputPath += "/";
    Mat img = imread(filename, 1);
    int halfRow = img.rows / 2, halfCol = img.cols / 2;
    Mat img_lur;
    int m = 300;
    img_lur = img(Range(halfRow - m, halfRow + m), Range(halfCol - 3 * m, halfCol + 3 * m));
    resize(img_lur, img_lur, Size(), 0.2, 0.2);
    //imwrite(path + "/cbc/img_lur0.jpg", img_lur);
    medianBlur(img_lur, img_lur, 31);
    resize(img_lur, img_lur, Size(img.cols, img.rows));
    resize(img_lur, img_lur, Size(), 0.01, 0.01);
    medianBlur(img_lur, img_lur, 31);
    resize(img_lur, img_lur, Size(img.cols, img.rows));
    resize(img_lur, img_lur, Size(), 0.01, 0.01);
    medianBlur(img_lur, img_lur, 31);
    resize(img_lur, img_lur, Size(img.cols, img.rows));
    //imwrite(path + "/cbc/img_lur.jpg", img_lur);
    Mat gray;
    cvtColor(img, gray, CV_BGR2GRAY);
    Mat binary;
    int blockSize = 85;
    int constValue = 10;
    adaptiveThreshold(gray, binary, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY_INV, blockSize, constValue);
    //imwrite(path + "cbc/binary0.jpg", binary);
    //threshold(gray, binary, 150, 255, THRESH_BINARY_INV |THRESH_OTSU );
    Mat erode_img;
    Mat structure_element = getStructuringElement(MORPH_RECT, Size(17, 17));
    erode(binary, erode_img, structure_element);
    //imwrite(path + "/cbc/erode0.jpg", erode_img);
    structure_element = getStructuringElement(MORPH_RECT, Size(23, 23));
    dilate(erode_img, erode_img, structure_element);
    Mat labels, centroids, stats;
    int n_labels = connectedComponentsWithStats(erode_img, labels, stats, centroids);
    uchar* p;
    int dis = 200;
    for (int i = 1; i < n_labels; ++i) {
        if (stats.at<int>(i, CC_STAT_AREA) < 5200 || (
                    stats.at<int>(i, CC_STAT_LEFT) > dis && (stats.at<int>(i, CC_STAT_WIDTH) + stats.at<int>(i, CC_STAT_LEFT)) < (img.cols - dis) && stats.at<int>(i, CC_STAT_TOP) > dis && (stats.at<int>(i, CC_STAT_TOP) + stats.at<int>(i, CC_STAT_HEIGHT)) < (img.rows - dis)
                    ) ) {

            for (int row = stats.at<int>(i, CC_STAT_TOP); row < stats.at<int>(i, CC_STAT_TOP) + stats.at<int>(i, CC_STAT_HEIGHT); ++row) {
                p = erode_img.ptr<uchar>(row);
                for (int col = stats.at<int>(i, CC_STAT_LEFT); col < stats.at<int>(i, CC_STAT_LEFT) + stats.at<int>(i, CC_STAT_WIDTH); col++) {
                    if (p[col] > 0) p[col] = 0;

                }
            }
        }
    }
    structure_element = getStructuringElement(MORPH_RECT, Size(7, 7));
    dilate(erode_img, erode_img, structure_element);
    //imwrite(path + "/cbc/erode1.jpg", erode_img);
    bitwise_not(erode_img, erode_img);
    bitwise_and(erode_img, binary, erode_img);
    //imwrite(path + "/cbc/erode.jpg", erode_img);

    for (int i = 0; i < erode_img.rows; ++i) {
        p = erode_img.ptr<uchar>(i);
        for (int j = 0; j < erode_img.cols; ++j) {
            if (p[j] > 0) {
                img_lur.at<Vec3b>(i, j)[0] = img.at<Vec3b>(i, j)[0];
                img_lur.at<Vec3b>(i, j)[1] = img.at<Vec3b>(i, j)[1];
                img_lur.at<Vec3b>(i, j)[2] = img.at<Vec3b>(i, j)[2];
            }
        }
    }
    int n = 15;
    img_lur = img_lur(Range(n, img_lur.rows - n), Range(n, img_lur.cols - n));
    erode_img = erode_img(Range(n, erode_img.rows - n), Range(n, erode_img.cols - n));
    img = img(Range(n, img.rows - n), Range(n, img.cols - n));
//    n_labels = connectedComponentsWithStats(erode_img, labels, stats, centroids);
//    for (int i = 1; i < n_labels; ++i) {
//        //cout << stats.at<int>(i, CC_STAT_AREA) << endl;
//        if (stats.at<int>(i, CC_STAT_AREA) > 200) {
//            for (int row = stats.at<int>(i, CC_STAT_TOP); row < stats.at<int>(i, CC_STAT_TOP) + stats.at<int>(i, CC_STAT_HEIGHT); ++row) {
//                p = erode_img.ptr<uchar>(row);
//                for (int col = stats.at<int>(i, CC_STAT_LEFT); col < stats.at<int>(i, CC_STAT_LEFT) + stats.at<int>(i, CC_STAT_WIDTH); col++) {
//                    img_lur.at<Vec3b>(row, col)[0] = img.at<Vec3b>(row, col)[0];
//                    img_lur.at<Vec3b>(row, col)[1] = img.at<Vec3b>(row, col)[1];
//                    img_lur.at<Vec3b>(row, col)[2] = img.at<Vec3b>(row, col)[2];
//                }
//            }
//        }
//    }

    string file_name = filename.substr(path.size() + 1);
    string newPath = outputPath + file_name;//QString::fromStdString().toLocal8Bit().toStdString();
    //cout<<newPath<<endl;
    generateDirecorys_cbc(newPath);
    //generateDirecorys(QString::fromStdString(newPath));
    imwrite(newPath, img_lur);
    SetResolution(newPath.c_str(), 300);
}

void getFiles(const string& path, vector<string>& files)
{
    long long hFile = 0;
    struct _finddata_t fileinfo;
    string p;
    if ((hFile = _findfirst(p.assign(path).append("/*").c_str(), &fileinfo)) != -1)
    {
        do
        {
            if ((fileinfo.attrib & _A_SUBDIR))
            {
                if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
                    getFiles(p.assign(path).append("/").append(fileinfo.name), files);
            }
            else
            {
                files.push_back(p.assign(path).append("/").append(fileinfo.name));
            }
        } while (_findnext(hFile, &fileinfo) == 0);
        _findclose(hFile);
    }
}
bool SetResolution(const char* path, int iResolution)
{
    FILE * file = fopen(path, "rb+");// - 打开图片文件
    if (!file)return false;
    int len = _filelength(_fileno(file));// - 获取文件大小
    char* buf = new char[len];
    fread(buf, sizeof(char), len, file);// - 将图片数据读入缓存
    char * pResolution = (char*)&iResolution;// - iResolution为要设置的分辨率的数值，如72dpi
    // - 设置JPG图片的分辨率
    buf[0x0D] = 1;// - 设置使用图片密度单位
    // - 水平密度，水平分辨率
    buf[0x0E] = pResolution[1];
    buf[0x0F] = pResolution[0];
    // - 垂直密度，垂直分辨率
    buf[0x10] = pResolution[1];
    buf[0x11] = pResolution[0];

    // - 将文件指针移动回到文件开头
    fseek(file, 0, SEEK_SET);
    // - 将数据写入文件，覆盖原始的数据，让修改生效
    fwrite(buf, sizeof(char), len, file);
    fclose(file);
    return true;
}
