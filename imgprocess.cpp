#include "imgprocess.h"
#include "qdir.h"
#include "qstring.h"
using namespace cv;
using namespace std;

void generateDirecorys(QString directory){
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
    Mat img_lur;
    resize(img, img_lur, Size(), 0.1, 0.1);
    int m = 45;
    img_lur = img_lur(Range(m, img_lur.rows - m), Range(m, img_lur.cols - m));
    //imwrite(path + "/cbc/img_lur0.jpg", img_lur);
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
    for (int i = 1; i < n_labels; ++i) {
        if (stats.at<int>(i, CC_STAT_AREA) < 5500) {
            for (int row = stats.at<int>(i, CC_STAT_TOP); row < stats.at<int>(i, CC_STAT_TOP) + stats.at<int>(i, CC_STAT_HEIGHT); ++row) {
                p = erode_img.ptr<uchar>(row);
                for (int col = stats.at<int>(i, CC_STAT_LEFT); col < stats.at<int>(i, CC_STAT_LEFT) + stats.at<int>(i, CC_STAT_WIDTH); col++) {
                    if (p[col] > 0) p[col] = 0;
                }
            }
        }
    }
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

    string file_name = filename.substr(path.size());
    string newPath = outputPath + file_name;
    generateDirecorys(QString::fromStdString(newPath));
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
