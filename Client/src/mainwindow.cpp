#include "headers/mainwindow.h"
#include "ui_mainwindow.h"
#include <QRegexpValidator>
#include <iostream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  ui->stackedWidget->setCurrentWidget(ui->loginPage);

  // only numbers can be entered in port field
  ui->port->setValidator(new QRegExpValidator(QRegExp("[0-9]*"), this));

  ui->usernameSignupPage->setValidator(
      new QRegExpValidator(QRegExp("[a-zA-Z0-9]{3,20}"), this));

  ui->passwordSignupPage->setValidator(
      new QRegExpValidator(QRegExp(".{3,20}"), this));

  //  mSocket = new QTcpSocket(this);
  //  mSocket->connectToHost("localhost", serverPort);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::on_signupButton_clicked() {

  ui->stackedWidget->setCurrentWidget(ui->signupPage);
  clearInputFields();
}

void MainWindow::on_loginButton_clicked() {
  qDebug() << "login button";

  username = ui->username->text();
  password = ui->password->text();
  hostname = ui->hostname->text();
  port = ui->port->text();

  if (username.isEmpty()) {
    //    ui->username->setStyleSheet("background-color: red");
    ui->username->setStyleSheet("border: 1px solid red");

    // else needed because if username and password get red, if we fix one then
    // if we try to login one should stay red and other doesnt
  } else {
    ui->username->setStyleSheet("border: 1px solid gray");
  }

  if (password.isEmpty()) {
    //    ui->username->setStyleSheet("background-color: red");
    ui->password->setStyleSheet("border: 1px solid red");
  } else {
    ui->password->setStyleSheet("border: 1px solid gray");
  }

  if (hostname.isEmpty()) {
    //    ui->username->setStyleSheet("background-color: red");
    ui->hostname->setStyleSheet("border: 1px solid red");
  } else {
    ui->hostname->setStyleSheet("border: 1px solid gray");
  }

  if (port.isEmpty()) {
    //    ui->username->setStyleSheet("background-color: red");
    ui->port->setStyleSheet("border: 1px solid red");
  } else {
    ui->port->setStyleSheet("border: 1px solid gray");
  }

  mSocket = new QTcpSocket(this);
  mSocket->connectToHost(hostname, port.toInt());

  connect(mSocket, &QTcpSocket::readyRead, this,
          &MainWindow::accountCheckLogin);

  QTextStream stream(mSocket);
  stream << loginCheckString << separator << username << separator << password;
  mSocket->flush();
}

void MainWindow::chatHandler(QString message) {

  if (message.startsWith(onlineString) || message.startsWith(offlineString)) {
    QList<QString> messageData = message.split(separator);
    QString messageText = messageData[1];
    //    qDebug() << "font " << ui->chat->fontWeight();
    ui->chat->setFontWeight(75);
    ui->chat->append(messageText);
    ui->chat->setFontWeight(50);
  } else {
    ui->chat->append(message);
  }
}

void MainWindow::on_back_clicked() {
  ui->stackedWidget->setCurrentWidget(ui->loginPage);
  clearInputFields();
}

void MainWindow::clearInputFields() {

  //  // clear input fields of login page
  //  ui->username->clear();
  //  ui->password->clear();
  //  ui->hostname->clear();
  //  ui->port->clear();

  //  ui->username->setStyleSheet("border: 1px solid gray");
  //  ui->password->setStyleSheet("border: 1px solid gray");
  //  ui->hostname->setStyleSheet("border: 1px solid gray");
  //  ui->port->setStyleSheet("border: 1px solid gray");

  //  // clear input fields of signup page
  //  ui->usernameSignupPage->clear();
  //  ui->passwordSignupPage->clear();

  //  ui->usernameSignupPage->setStyleSheet("border: 1px solid gray");
  //  ui->passwordSignupPage->setStyleSheet("border: 1px solid gray");

  ui->message->clear();
  ui->chat->clear();
}

void MainWindow::on_signupButtonSignupPage_clicked() {

  bool flagUsernameSignupPage;
  bool flagPasswordSignupPage;

  username = ui->usernameSignupPage->text();
  password = ui->passwordSignupPage->text();

  // regex checks if its > 20
  if (username.isEmpty() || username.length() < 3) {
    ui->usernameSignupPage->setStyleSheet("border: 1px solid red");
    flagUsernameSignupPage = true;
  } else {
    ui->usernameSignupPage->setStyleSheet("border: 1px solid gray");
    flagUsernameSignupPage = false;
  }

  if (password.isEmpty() || password.length() < 3) {
    ui->passwordSignupPage->setStyleSheet("border: 1px solid red");
    flagPasswordSignupPage = true;
  } else {
    ui->passwordSignupPage->setStyleSheet("border: 1px solid gray");
    flagPasswordSignupPage = false;
  }

  if (flagUsernameSignupPage || flagPasswordSignupPage) {
    qDebug() << "greska";
  } else {
    mSocket = new QTcpSocket(this);
    mSocket->connectToHost("localhost", serverPort);
    connect(mSocket, &QTcpSocket::readyRead, this,
            &MainWindow::accountCheckSignup);
    QTextStream stream(mSocket);
    stream << signupCheckString << separator << username << separator
           << password;
    mSocket->flush();
    // switch to login page (if there is error handle function will return to
    // signup page)
    //    ui->stackedWidget->setCurrentWidget(ui->loginPage);
  }
}

void MainWindow::accountCheckSignup() {
  QTextStream stream(mSocket);
  auto accountCheckResponse = stream.readAll();
  if (accountCheckResponse.startsWith("Successful")) {
    ui->stackedWidget->setCurrentWidget(ui->loginPage);
    qDebug() << "Successful signup";
  } else {
    //    ui->stackedWidget->setCurrentWidget(ui->signupPage);
    ui->usernameSignupPage->setStyleSheet("border: 1px solid red");
    ui->usernameSignupPage->setText("Username taken");
    qDebug() << "Unsuccessful signup";
    //  mSocket->abort();
  }
  mSocket->disconnectFromHost();
}

void MainWindow::accountCheckLogin() {
  QTextStream stream(mSocket);
  auto message = stream.readAll();
  qDebug() << message << message << message;
  if (message.startsWith("Successful")) {
    ui->stackedWidget->setCurrentWidget(ui->chatPage);
    qDebug() << "Successful login";
  } else if (message.startsWith("Unsuccessful user doesnt exist")) {
    ui->username->setStyleSheet("border: 1px solid red");
    //    ui->username->setText("Something wrong");
    ui->password->setStyleSheet("border: 1px solid red");
    //    ui->password->setText("Something wrong");

    mSocket->disconnectFromHost();

    qDebug() << "Unsuccessful user doesnt exist";
  } else if (message.startsWith("Unsuccessful wrong password")) {
    ui->password->setStyleSheet("border: 1px solid red");
    qDebug() << "Unsuccessful wrong password";

    mSocket->disconnectFromHost();

    // its message from other clients (chat message)
  } else {
    chatHandler(message);
  }

  //  mSocket->abort();
}

void MainWindow::on_sendButton_clicked() {

  QString message = ui->message->text();
  if (message.isEmpty()) {

  } else {
    qDebug() << "send button klik " << message;
    QTextStream stream(mSocket);
    stream << username << separator << message;
    mSocket->flush();
    //    QString tmp = "[";
    //    tmp.append(username);
    //    tmp.append("]:");
    //    tmp.append(message);
    ui->chat->setFontItalic(true);
    ui->chat->append(message);
    ui->chat->setFontItalic(false);
  }
  ui->message->clear();
  ui->message->setFocus();
}

void MainWindow::on_logout_clicked() {
  QTextStream stream(mSocket);
  stream << logoutString << separator << username;
  mSocket->flush();
  clearInputFields();
  ui->stackedWidget->setCurrentWidget(ui->loginPage);
}
