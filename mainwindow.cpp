#pragma execution_character_set("utf-8")

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "imgprocess.h"
#include <QTextCodec>
#include <windows.h>
using namespace std;

bool FlagInputFile = false;
bool FlagOutputFile = false;

bool startFlag = false;
bool pauseFlag = false;

QString inputDir;//输入文件夹
QString outputDir; //输出文件夹

ThreadPool pool(40);

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("图像修复");
    ui->pb_stop->hide();
    ui->pb_start->hide();
    ui->pb_pause->hide();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pb_input_clicked()
{
    ui->label->clear();
    ui->lb_input->clear();
    QString curPath = QCoreApplication::applicationDirPath();
    QString dlgTitle = QString(QStringLiteral("选择输入目录").toLocal8Bit());
    inputDir = QFileDialog::getExistingDirectory(this, dlgTitle, curPath, QFileDialog::ShowDirsOnly);
    qDebug()<<inputDir<<endl;
    if(!inputDir.isEmpty()){
        FlagInputFile = true;
        FlagOutputFile = false;
        ui->lb_input->setText(inputDir);
    }
}

void MainWindow::on_pb_output_clicked()
{
    ui->lb_output->clear();
    if(FlagInputFile == false){
        ui->label->setText("请先选择输入目录！");
        return;
    }
    QString curPath = QCoreApplication::applicationDirPath();
    QString dlgTitle = QString(QStringLiteral("选择输出目录").toLocal8Bit());
    outputDir = QFileDialog::getExistingDirectory(this, dlgTitle, curPath, QFileDialog::ShowDirsOnly);
    qDebug()<<outputDir<<endl;
    if(!outputDir.isEmpty()){
        FlagInputFile = false;
        FlagOutputFile = true;
    }
    if(FlagOutputFile == true){
        FlagOutputFile = false;
        ui->lb_output->setText(outputDir);
        ui->label->setText("正在处理中...");
        QTextCodec * code = QTextCodec::codecForName("gb2312");
        string inputpath = code->fromUnicode(inputDir).data();
        string outputpath = code->fromUnicode(outputDir).data();

        vector<string> files;
        getFiles(inputpath, files);

        int file_num = 0;
        while(file_num < files.size()){
            try {
                for(int i = 0; i < files[file_num].size() - 1; ++i){
                    if(files[file_num][i] == '/' && files[file_num][i + 1] == '/'){
                        int j = i + 1;
                        for(; j < files[file_num].size() - 1; ++j){
                            files[file_num][j] = files[file_num][j + 1];
                        }
                        files[file_num][j] = '\0';
                        break;
                    }
                }
                //cout<<files[file_num]<<endl;
                pool.enqueue(img_process, inputpath, outputpath, files[file_num]);
                file_num++;
            } catch (exception& e) {
                ui->label->setText(e.what());
            }
        }
        ui->label->setText("全部完成！！！");
    }
}

void MainWindow::on_pb_start_clicked()
{
    startFlag = true;
    static int times = 0;
    if(times != 0 && pauseFlag == true){
        pauseFlag = false;
        HANDLE hEvent = CreateEvent(NULL,
                    TRUE,
                    TRUE,
                    NULL);
        WaitForSingleObject(hEvent, -1);
        SetEvent(hEvent);

        return;
    }
    ui->label->setText("已选择:开始");

}

void MainWindow::on_pb_stop_clicked()
{
    ui->label->clear();
    ui->label->setText("已选择:结束");
    pool.Stop();
    startFlag = false;
    pauseFlag = false;
}

void MainWindow::on_pb_pause_clicked()
{
    if(startFlag == false){
        return;
    }
    startFlag = false;
    pauseFlag = true;
    ui->label->setText("已选择:暂停");
    HANDLE hEvent = CreateEvent(NULL,
                TRUE,
                TRUE,
                NULL);
    WaitForSingleObject(hEvent, -1);
    ResetEvent(hEvent);
}
