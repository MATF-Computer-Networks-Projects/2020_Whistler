#include "headers/mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  mSocket = new QTcpSocket(this);
  mSocket->connectToHost("localhost", 12345);
}

MainWindow::~MainWindow() { delete ui; }
