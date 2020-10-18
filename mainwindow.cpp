#pragma execution_character_set("utf-8")

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "imgprocess.h"
using namespace std;

bool FlagInputFile = false;
bool FlagOutputFile = false;
QString inputDir;//输入文件夹
QString outputDir; //输出文件夹
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("图像修复");
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
        string inputpath = inputDir.toStdString();
        string outputpath = outputDir.toStdString();
        vector<string> files;
        getFiles(inputpath, files);

        ThreadPool pool(50);
        int file_num = 0;
        while(file_num < files.size()){
            try {
                pool.enqueue(img_process, inputpath, outputpath, files[file_num]);
                file_num++;
            } catch (exception& e) {
                ui->label->setText(e.what());
            }
        }
        ui->label->setText("全部完成！！！");
    }
}
