#include "headers/mainwindow.h"
#include "ui_mainwindow.h"
#include <QRegexpValidator>
#include <QTime>
#include <iostream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  ui->stackedWidget->setCurrentWidget(ui->loginPage);

  ui->hostname->setText("localhost");
  ui->port->setText("12345");
  ui->username->setText("user");
  ui->password->setText("123");

  // only numbers can be entered in port field
  ui->port->setValidator(new QRegExpValidator(QRegExp("[0-9]*"), this));

  ui->usernameSignupPage->setValidator(
      new QRegExpValidator(QRegExp("[a-zA-Z0-9]{3,20}"), this));

  ui->passwordSignupPage->setValidator(
      new QRegExpValidator(QRegExp("[^$]{3,20}"), this));

  ui->username->setValidator(
      new QRegExpValidator(QRegExp("[a-zA-Z0-9]{3,20}"), this));

  ui->password->setValidator(new QRegExpValidator(QRegExp("[^$]{3,20}"), this));

  ui->oldPassword->setValidator(
      new QRegExpValidator(QRegExp("[^$]{3,20}"), this));

  ui->newPassword->setValidator(
      new QRegExpValidator(QRegExp("[^$]{3,20}"), this));

  // setting tab order so we can use tab
  this->setTabOrder(ui->hostname, ui->port);
  this->setTabOrder(ui->port, ui->username);
  this->setTabOrder(ui->username, ui->password);

  // setting tab order so we can use tab
  this->setTabOrder(ui->usernameSignupPage, ui->passwordSignupPage);

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

  if (username.isEmpty() || username.length() < 3) {
    //    ui->username->setStyleSheet("background-color: red");
    ui->username->setStyleSheet("border: 1px solid red");

    // else needed because if username and password get red, if we fix one then
    // if we try to login one should stay red and other doesnt
  } else {
    ui->username->setStyleSheet("border: 1px solid gray");
  }

  if (password.isEmpty() || password.length() < 3) {
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
  connect(mSocket, &QTcpSocket::connected, this, &MainWindow::afterConnect);
}

void MainWindow::afterConnect() {
  QTextStream stream(mSocket);
  stream << loginCheckString << separator << username << separator << password
         << separator << ui->showOnlineStatus->isChecked();
  qDebug() << loginCheckString << separator << username << separator << password
           << separator << ui->showOnlineStatus->isChecked();

  mSocket->flush();
}

void MainWindow::chatHandler(QString message) { ui->chat->append(message); }

void MainWindow::chatPageHandler(QString message) {

  //  promeniti separator kod servera ?

  if (message.startsWith(successfulLoginString + separator + allOnlineString)) {
    qDebug() << "ovo sada ispisuje";
    qDebug() << "ovo je message " << message;
    QList<QString> messageData = message.split(separator);

    QList<QString>::iterator i;

    for (i = messageData.begin() + 2; i != messageData.end(); i++) {
      onlineUsersWithShownOnlineStatus.append(*i);
      qDebug() << "poruke " << *i;
    }

    ui->onlineUsers->clear();

    for (i = onlineUsersWithShownOnlineStatus.begin();
         i != onlineUsersWithShownOnlineStatus.end(); i++) {
      ui->onlineUsers->append(*i);
    }

    ui->numberOfOnlineUsers->display(onlineUsersWithShownOnlineStatus.length());

  } else if (message.startsWith(onlineString)) {
    QList<QString> messageData = message.split(separator);
    QString username = messageData[1];
    //    qDebug() << "font " << ui->chat->fontWeight();
    ui->chat->setFontWeight(75);
    ui->chat->append(username + " is online.");
    ui->chat->setFontWeight(50);

    onlineUsersWithShownOnlineStatus.append(username);
    ui->onlineUsers->append(username);
    ui->numberOfOnlineUsers->display(onlineUsersWithShownOnlineStatus.length());
    //  } else if (message.startsWith(allOnlineString)) {

    //    qDebug() << "ovo sada ispisuje";
    //    qDebug() << "ovo je message " << message;
    //    QList<QString> messageData = message.split(separator);

    //    QList<QString>::iterator i;

    //    for (i = messageData.begin() + 1; i != messageData.end(); i++) {
    //      onlineUsersWithShownOnlineStatus.append(*i);
    //    }

    //    ui->onlineUsers->clear();

    //    for (i = onlineUsersWithShownOnlineStatus.begin();
    //         i != onlineUsersWithShownOnlineStatus.end(); i++) {
    //      ui->onlineUsers->append(*i);
    //    }

    //    for (const auto &i : onlineUsersWithShownOnlineStatus) {
    //      ui->onlineUsers->append(i);
    //    }
  } else if (message.startsWith(offlineString)) {
    QList<QString> messageData = message.split(separator);
    QString username = messageData[1];

    if (!username.isEmpty()) {
      //    qDebug() << "font " << ui->chat->fontWeight();
      ui->chat->setFontWeight(75);
      ui->chat->append(username + " is offline.");
      ui->chat->setFontWeight(50);

      onlineUsersWithShownOnlineStatus.removeOne(username);
      ui->onlineUsers->clear();

      QList<QString>::iterator i;

      for (i = onlineUsersWithShownOnlineStatus.begin();
           i != onlineUsersWithShownOnlineStatus.end(); i++) {
        ui->onlineUsers->append(*i);
      }
      ui->numberOfOnlineUsers->display(
          onlineUsersWithShownOnlineStatus.length());
    }
  }
}

void MainWindow::on_back_clicked() {
  ui->stackedWidget->setCurrentWidget(ui->loginPage);
  clearInputFields();
}

void MainWindow::clearInputFields() {

  clearInputFieldsLoginPage();
  clearInputFieldsSignupPage();
  clearInputFieldsChatPage();
  clearInputFieldsChangePasswordPage();
}

void MainWindow::clearInputFieldsLoginPage() {
  // clear input fields of login page
  ui->username->clear();
  ui->password->clear();
  ui->hostname->clear();
  ui->port->clear();

  ui->username->setStyleSheet("border: 1px solid gray");
  ui->password->setStyleSheet("border: 1px solid gray");
  ui->hostname->setStyleSheet("border: 1px solid gray");
  ui->port->setStyleSheet("border: 1px solid gray");
}

void MainWindow::clearInputFieldsSignupPage() {

  // clear input fields of signup page
  ui->usernameSignupPage->clear();
  ui->passwordSignupPage->clear();

  ui->usernameSignupPage->setStyleSheet("border: 1px solid gray");
  ui->passwordSignupPage->setStyleSheet("border: 1px solid gray");
}

void MainWindow::clearInputFieldsChatPage() {

  ui->message->clear();
  ui->chat->clear();
  ui->onlineUsers->clear();
  ui->numberOfOnlineUsers->display(0);
}

void MainWindow::clearInputFieldsChangePasswordPage() {
  ui->oldPassword->clear();
  ui->newPassword->clear();

  ui->oldPassword->setStyleSheet("border: 1px solid gray");
  ui->newPassword->setStyleSheet("border: 1px solid gray");
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
    mSocket->connectToHost(serverHostname, serverPort);
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
  if (message.startsWith(successfulLoginString)) {
    ui->stackedWidget->setCurrentWidget(ui->chatPage);

    // if its successful login see if there are users online to write in online
    // field
    chatPageHandler(message);
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
  } else if (message.startsWith("Unsuccessful already logged in")) {
    ui->username->setStyleSheet("background-color: red");
    //    ui->username->setText("Something wrong");
    ui->password->setStyleSheet("background-color: red");
    //    ui->password->setText("Something wrong");

  } else if (message.startsWith(changePasswordString)) {
    changePasswordHandler(message);
  } else if (message.startsWith(onlineString) ||
             message.startsWith(offlineString) ||
             message.startsWith(allOnlineString)) {
    chatPageHandler(message);
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
    QString timestamp = QDateTime::currentDateTime().toString().split(" ")[3];
    stream << "|" << timestamp << "| " << username << separator << message;
    mSocket->flush();
    ui->chat->setFontItalic(true);
    ui->chat->append("|" + timestamp + "| Me: " + message);
    ui->chat->setFontItalic(false);
  }
  ui->message->clear();
  ui->message->setFocus();
}

void MainWindow::changePasswordHandler(QString message) {
  QTextStream stream(mSocket);
  auto messageFromServer = stream.readAll();
  QList<QString> messageData = message.split(separator);
  QString messageText = messageData[1];
  qDebug() << messageText << messageText << messageText;
  if (messageText.startsWith("Successful change password")) {
    //      ui->stackedWidget->setCurrentWidget(ui->chatPage);

    clearInputFieldsChangePasswordPage();

    ui->oldPassword->setStyleSheet("border: 1px solid green");
    ui->newPassword->setStyleSheet("border: 1px solid green");
    qDebug() << "Successful change password";
  } else {
    ui->oldPassword->setStyleSheet("border: 1px solid red");
  }
}

void MainWindow::on_logout_clicked() {
  QTextStream stream(mSocket);
  stream << logoutString << separator << username;
  mSocket->flush();
  //  clearInputFields();
  clearInputFieldsChatPage();
  ui->stackedWidget->setCurrentWidget(ui->loginPage);

  onlineUsersWithShownOnlineStatus.clear();
  mSocket->disconnectFromHost();
}

void MainWindow::on_changePassword_clicked() {
  qDebug() << "change password klik";
  ui->stackedWidget->setCurrentWidget(ui->changePasswordPage);
}

void MainWindow::on_confirmChangePassword_clicked() {
  QString oldPassword = ui->oldPassword->text();
  QString newPassword = ui->newPassword->text();

  if (oldPassword.isEmpty() || oldPassword.length() < 3) {
    ui->oldPassword->setStyleSheet("border: 1px solid red");
  } else if (newPassword.isEmpty() || newPassword.length() < 3) {
    ui->newPassword->setStyleSheet("border: 1px solid red");

    // if new password is same as old we just set color like its changed
  } else if (oldPassword == newPassword) {
    clearInputFieldsChangePasswordPage();
    ui->oldPassword->setStyleSheet("border: 1px solid green");
    ui->newPassword->setStyleSheet("border: 1px solid green");
  } else {
    // needed so when repeatedly client changes password other colors dont stay
    ui->oldPassword->setStyleSheet("border: 1px solid gray");
    ui->newPassword->setStyleSheet("border: 1px solid gray");
    QTextStream stream(mSocket);
    stream << changePasswordString << separator << username << separator
           << oldPassword << separator << newPassword;
    mSocket->flush();
  }
  //  clearInputFieldsChangePasswordPage();
}

void MainWindow::on_backChangePasswordPage_clicked() {
  ui->stackedWidget->setCurrentWidget(ui->chatPage);
  clearInputFieldsChangePasswordPage();
}
