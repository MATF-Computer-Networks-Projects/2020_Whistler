#include "headers/mainwindow.h"
#include "ui_mainwindow.h"
#include <QComboBox>
#include <QFontDatabase>
#include <QGraphicsScene>
#include <QMovie>
#include <QRegExpValidator>
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

  // send message with enter
  connect(ui->message, SIGNAL(returnPressed()), ui->sendButton,
          SIGNAL(clicked()));

  // login using enter
  connect(ui->hostname, SIGNAL(returnPressed()), ui->loginButton,
          SIGNAL(clicked()));
  connect(ui->port, SIGNAL(returnPressed()), ui->loginButton,
          SIGNAL(clicked()));
  connect(ui->username, SIGNAL(returnPressed()), ui->loginButton,
          SIGNAL(clicked()));
  connect(ui->password, SIGNAL(returnPressed()), ui->loginButton,
          SIGNAL(clicked()));

  // signup using enter
  connect(ui->usernameSignupPage, SIGNAL(returnPressed()),
          ui->signupButtonSignupPage, SIGNAL(clicked()));
  connect(ui->passwordSignupPage, SIGNAL(returnPressed()),
          ui->signupButtonSignupPage, SIGNAL(clicked()));

  // change password using enter
  connect(ui->oldPassword, SIGNAL(returnPressed()), ui->confirmChangePassword,
          SIGNAL(clicked()));
  connect(ui->newPassword, SIGNAL(returnPressed()), ui->confirmChangePassword,
          SIGNAL(clicked()));

  movieError = new QMovie("../Client/assets/pictureError.gif");

  themeModern();

  // setting tab order so we can use tab
  this->setTabOrder(ui->hostname, ui->port);
  this->setTabOrder(ui->port, ui->username);
  this->setTabOrder(ui->username, ui->password);

  // setting tab order so we can use tab
  this->setTabOrder(ui->usernameSignupPage, ui->passwordSignupPage);
}

MainWindow::~MainWindow() {
  delete ui;
  //  delete mSocket;
  //  delete movieError;
}

void MainWindow::on_signupButton_clicked() {

  ui->stackedWidget->setCurrentWidget(ui->signupPage);
  clearInputFieldsLoginPage();
}

void MainWindow::on_loginButton_clicked() {
  qDebug() << "login button";

  errorHandleLoginClear();

  username = ui->username->text();
  password = ui->password->text();
  hostname = ui->hostname->text();
  port = ui->port->text();

  bool loginUsernameCorrect = true;
  bool loginPasswordCorrect = true;
  bool loginHostnameCorrect = true;
  bool loginPortCorrect = true;

  if (username.isEmpty() || username.length() < 3) {
    ui->username->setStyleSheet("border: 1px solid red");

    loginUsernameCorrect = false;

    errorHandleLogin();

    // else needed because if username and password get red, if we fix one then
    // if we try to login one should stay red and other doesnt
  } else {
    ui->username->setStyleSheet("border: 1px solid gray");
  }

  if (password.isEmpty() || password.length() < 3) {
    ui->password->setStyleSheet("border: 1px solid red");

    loginPasswordCorrect = false;

    errorHandleLogin();
  } else {
    ui->password->setStyleSheet("border: 1px solid gray");
  }

  if (hostname.isEmpty()) {
    ui->hostname->setStyleSheet("border: 1px solid red");

    loginHostnameCorrect = false;

    errorHandleLogin();
  } else {
    ui->hostname->setStyleSheet("border: 1px solid gray");
  }

  if (port.isEmpty()) {
    ui->port->setStyleSheet("border: 1px solid red");

    loginPortCorrect = false;

    errorHandleLogin();
  } else {
    ui->port->setStyleSheet("border: 1px solid gray");
  }

  if (loginUsernameCorrect && loginPasswordCorrect && loginHostnameCorrect &&
      loginPortCorrect) {
    mSocket = new QTcpSocket(this);
    mSocket->connectToHost(hostname, port.toInt());

    connect(mSocket, &QTcpSocket::readyRead, this,
            &MainWindow::accountCheckLogin);
    connect(mSocket, &QTcpSocket::connected, this, &MainWindow::afterConnect);
  }
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

  // when we log in we get all online users
  if (message.startsWith(successfulLoginString + separator + allOnlineString)) {
    qDebug() << "logged in and getting all online users";
    qDebug() << "whole message received from server " << message;
    QList<QString> messageData = message.split(separator);

    ui->onlineUsers->clear();

    QList<QString>::iterator i;

    // adding all online users to list
    for (i = messageData.begin() + 2; i != messageData.end(); i++) {
      onlineUsersWithShownOnlineStatus.append(*i);
      qDebug() << "users online " << *i;

      ui->onlineUsers->append(*i);
    }

    //    for (i = onlineUsersWithShownOnlineStatus.begin();
    //         i != onlineUsersWithShownOnlineStatus.end(); i++) {
    //      ui->onlineUsers->append(*i);
    //    }

    ui->numberOfOnlineUsers->display(onlineUsersWithShownOnlineStatus.length());

    // we get noticed when someone logs in
  } else if (message.startsWith(onlineString)) {
    QList<QString> messageData = message.split(separator);
    QString username = messageData[1];

    //    qDebug() << "pre ispisa online" << ui->chat->fontWeight();
    ui->chat->setFontWeight(75);
    ui->chat->append(username + " is online.");
    //    qDebug() << "posle ispisa 1 online" << ui->chat->fontWeight();
    ui->chat->setFontWeight(50);

    //    qDebug() << "posle ispisa 2 online" << ui->chat->fontWeight();

    // adding new online user to list of online users
    onlineUsersWithShownOnlineStatus.append(username);
    ui->onlineUsers->append(username);
    ui->numberOfOnlineUsers->display(onlineUsersWithShownOnlineStatus.length());

    // we get noticed when logs out
  } else if (message.startsWith(offlineString)) {
    QList<QString> messageData = message.split(separator);
    QString username = messageData[1];

    if (!username.isEmpty()) {

      //      qDebug() << "pre ispisa offline" << ui->chat->fontWeight();
      ui->chat->setFontWeight(75);
      ui->chat->append(username + " is offline.");
      //      qDebug() << "posle ispisa 1 offline" << ui->chat->fontWeight();
      ui->chat->setFontWeight(50);

      //      qDebug() << "posle ispisa 2 offline" << ui->chat->fontWeight();

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
  clearInputFieldsSignupPage();
  clearInputFieldsLoginPage();
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

  //  movieError->stop();
  //  ui->labelLogin->setMovie(nullptr);

  errorHandleLoginClear();
}

void MainWindow::clearInputFieldsSignupPage() {

  // clear input fields of signup page
  ui->usernameSignupPage->clear();
  ui->passwordSignupPage->clear();

  ui->usernameSignupPage->setStyleSheet("border: 1px solid gray");
  ui->passwordSignupPage->setStyleSheet("border: 1px solid gray");

  ui->usernameSignupPage->setPlaceholderText("");

  //  ui->labelSignup->setMovie(nullptr);

  errorHandleSignupClear();
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

  //  ui->labelChangePassword->setMovie(nullptr);

  errorHandleChangePasswordClear();
}

void MainWindow::on_signupButtonSignupPage_clicked() {

  errorHandleSignupClear();

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

    errorHandleSignup();

    qDebug() << "error signup";
  } else {
    mSocket = new QTcpSocket(this);

    mSocket->connectToHost(serverHostname, serverPort);

    connect(mSocket, &QTcpSocket::readyRead, this,
            &MainWindow::accountCheckSignup);

    QTextStream stream(mSocket);
    stream << signupCheckString << separator << username << separator
           << password;

    mSocket->flush();
  }
}

void MainWindow::accountCheckSignup() {

  QTextStream stream(mSocket);
  auto accountCheckResponse = stream.readAll();

  if (accountCheckResponse.startsWith("Successful")) {
    ui->stackedWidget->setCurrentWidget(ui->loginPage);
    qDebug() << "Successful signup";
  } else {

    clearInputFieldsSignupPage();
    ui->usernameSignupPage->setStyleSheet("border: 1px solid red");
    ui->usernameSignupPage->setPlaceholderText("Username taken");

    errorHandleSignup();

    qDebug() << "Unsuccessful signup";
  }
  mSocket->disconnectFromHost();
}

void MainWindow::accountCheckLogin() {

  QTextStream stream(mSocket);
  auto message = stream.readAll();

  qDebug() << "Server sent message " << message;

  if (message.startsWith(successfulLoginString)) {
    ui->stackedWidget->setCurrentWidget(ui->chatPage);

    clearInputFieldsLoginPage();

    qDebug() << "after login logoutClicked " << logoutClicked;

    // setting logoutClicked to false because if client logged out and then
    // logged back in logoutClicked will be true (because we set it to true when
    // he logged out). Since we dont delete this if client logs back in
    // logoutClicked will be true
    logoutClicked = false;

    connect(mSocket, &QTcpSocket::disconnected, this,
            &MainWindow::afterDisconnect);

    // if its successful login see if there are users online to write in online
    // field
    chatPageHandler(message);
    qDebug() << "Successful login";

  } else if (message.startsWith("Unsuccessful user doesnt exist")) {
    ui->username->setStyleSheet("border: 1px solid red");
    ui->password->setStyleSheet("border: 1px solid red");

    mSocket->disconnectFromHost();

    qDebug() << "Unsuccessful user doesnt exist";

    errorHandleLogin();

  } else if (message.startsWith("Unsuccessful wrong password")) {
    ui->password->setStyleSheet("border: 1px solid red");
    qDebug() << "Unsuccessful wrong password";

    errorHandleLogin();

    mSocket->disconnectFromHost();

  } else if (message.startsWith("Unsuccessful already logged in")) {

    ui->username->setStyleSheet("background-color: red");
    ui->password->setStyleSheet("background-color: red");

    errorHandleLogin();
  } else if (message.startsWith(
                 "Unsuccessful db error (probably not connected")) {

    ui->username->setStyleSheet("background-color: yellow");
    ui->password->setStyleSheet("background-color: yellow");

    //    errorHandleLogin();

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

void MainWindow::closeEvent(QCloseEvent *event) {

  qDebug() << "logout " << logoutClicked;
  qDebug() << "ui login " << (ui->loginPage->isVisible());
  qDebug() << "ui signup " << (ui->signupPage->isVisible());
  qDebug() << "ui chat page " << (ui->chatPage->isVisible());
  qDebug() << "ui change password page "
           << (ui->changePasswordPage->isVisible());

  // if we exit the app while we are on login screen or signup screen we dont
  // need to logout/disconnect from host (because we already did that)
  if (ui->chatPage->isVisible() || ui->changePasswordPage->isVisible()) {
    on_logout_clicked();
  } else {
    QApplication::quit();
  }

  //    event->ignore();
}

// if server is down (also doing this function after logout)
void MainWindow::afterDisconnect() {

  qDebug() << "logout clicked " << logoutClicked;
  // only if really is server down (not if we logout)
  if (!logoutClicked) {
    QString timestamp = QDateTime::currentDateTime().toString().split(" ")[3];
    qDebug() << "server down" << timestamp;

    ui->logout->click();
    ui->username->setStyleSheet("background-color: orange");
    ui->password->setStyleSheet("background-color: orange");
  }

  mSocket->deleteLater();
}

void MainWindow::errorHandleLogin() {
  ui->labelLogin->setMovie(movieError);
  movieError->start();
}

void MainWindow::errorHandleSignup() {
  ui->labelSignup->setMovie(movieError);
  movieError->start();
}

void MainWindow::errorHandleChangePassword() {
  ui->labelChangePassword->setMovie(movieError);
  movieError->start();
}

void MainWindow::errorHandleLoginClear() { ui->labelLogin->setMovie(nullptr); }
void MainWindow::errorHandleSignupClear() {
  ui->labelSignup->setMovie(nullptr);
}
void MainWindow::errorHandleChangePasswordClear() {
  ui->labelChangePassword->setMovie(nullptr);
}

void MainWindow::on_sendButton_clicked() {

  QString message = ui->message->text();

  if (message.isEmpty()) {

  } else {
    qDebug() << "send button clicked " << message;

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

  qDebug() << "Server send on change password " << messageText;

  if (messageText.startsWith("Successful change password")) {

    clearInputFieldsChangePasswordPage();

    ui->oldPassword->setStyleSheet("border: 1px solid green");
    ui->newPassword->setStyleSheet("border: 1px solid green");

    qDebug() << "Successful change password";

  } else {

    errorHandleChangePassword();

    ui->oldPassword->setStyleSheet("border: 1px solid red");
  }
}

void MainWindow::on_logout_clicked() {

  QTextStream stream(mSocket);
  stream << logoutString << separator << username;

  mSocket->flush();

  // set logoutClicked to true because we use this to know whether disconnect
  // signal came from logout click or not
  logoutClicked = true;

  clearInputFieldsChatPage();

  ui->stackedWidget->setCurrentWidget(ui->loginPage);

  onlineUsersWithShownOnlineStatus.clear();

  mSocket->disconnectFromHost();
}

void MainWindow::on_changePassword_clicked() {
  qDebug() << "change password clicked";
  ui->stackedWidget->setCurrentWidget(ui->changePasswordPage);
}

void MainWindow::on_confirmChangePassword_clicked() {

  errorHandleChangePasswordClear();

  QString oldPassword = ui->oldPassword->text();
  QString newPassword = ui->newPassword->text();

  if (oldPassword.isEmpty() || oldPassword.length() < 3) {
    ui->oldPassword->setStyleSheet("border: 1px solid red");

    errorHandleChangePassword();

  } else if (newPassword.isEmpty() || newPassword.length() < 3) {
    ui->newPassword->setStyleSheet("border: 1px solid red");

    errorHandleChangePassword();

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
}

void MainWindow::on_backChangePasswordPage_clicked() {
  ui->stackedWidget->setCurrentWidget(ui->chatPage);
  clearInputFieldsChangePasswordPage();
}

void MainWindow::on_tabWidget_currentChanged(int index) {
  if (index == 0) {
    themeModern();

  } else if (index == 1) {
    themeLight();

  } else if (index == 2) {
    themeDark();

  } else {
    theme90s();
  }
}

void MainWindow::themeModern() {
  this->setStyleSheet(
      "background-color:rgb(134,134,134); font: 10pt \"Calibri\";");

  ui->centralwidget->setStyleSheet(
      "QLineEdit{background:white;}QPushButton{"
      "background:rgb(201,201,201);}QTextEdit{background-color:white;}"
      "QTabBar::tab{background-color:rgb(230,230,230);padding:1px 11px; "
      "font: 10pt "
      "\"Calibri\"; "
      "border:1px solid black "
      "}QTabBar::tab:selected{background-color:rgb(201,201,201);}QTabBar::"
      "tab:!"
      "selected {margin-top: 2px;}");
  //    ui->tabWidget->themeModern->setBackgroundColor;
  //    ui->
  ui->headerLabelWhistler->setStyleSheet("font: 48pt \"Calibri\";");
}

void MainWindow::themeLight() {
  this->setStyleSheet(
      "background-color:rgb(234,234,234); font: 10pt \"Calibri\";");

  ui->centralwidget->setStyleSheet(
      "QLineEdit{background:white;}QPushButton{"
      "background:rgb(230,230,230);}QTextEdit{background-color:white;}"
      "QTabBar::tab{background-color:rgb(201,201,201);padding:1px 11px; "
      "font: 10pt "
      "\"Calibri\"; "
      "border:1px solid black "
      "}QTabBar::tab:selected{background-color:rgb(230,230,230);}QTabBar::"
      "tab:!"
      "selected {margin-top: 2px;}");
  ui->headerLabelWhistler->setStyleSheet("font: 48pt \"Calibri\";");
}

void MainWindow::themeDark() {
  this->setStyleSheet(
      "background-color:rgb(50,50,50); font: 10pt \"Calibri\";");

  ui->centralwidget->setStyleSheet(
      "QLineEdit{background:black; color:white}QPushButton{"
      "background:rgb(80,80,80);}QTextEdit{background-color:black;color:"
      "white}"
      "QTabBar::tab{background-color:black; "
      "color:rgb(230,230,230);padding:1px 11px; "
      "font: 10pt "
      "\"Calibri\"; "
      "border:1px solid black "
      "}QTabBar::tab:selected{background-color:rgb(80,80,80);}QTabBar::"
      "tab:!"
      "selected {margin-top: 2px;}");
  ui->headerLabelWhistler->setStyleSheet("font: 48pt \"Calibri\";");
}

void MainWindow::theme90s() {
  this->setStyleSheet("background-color:rgb(188,157,64); font: 10pt "
                      "\"Calibri\"; color:rgb(0,0,203)");

  ui->centralwidget->setStyleSheet(
      "QLineEdit{background:rgb(0,0,203); color:rgb(188,157,64)}QPushButton{"
      "background:rgb(0,0,203);color:rgb(188,157,64)}QTextEdit{background-"
      "color:rgb(0,0,203);color:"
      "rgb(188,157,64)}"
      "QTabBar::tab{background-color:rgb(0,0,203); "
      "color:rgb(0,0,203);padding:1px 11px; "
      "font: 10pt "
      "\"Calibri\"; "
      "border:1px solid black "
      "}QTabBar::tab:selected{background-color:rgb(188,157,64);}QTabBar::"
      "tab:!"
      "selected {margin-top: 2px; color:rgb(188,157,64)}");
  ui->headerLabelWhistler->setStyleSheet("font: 48pt \"Calibri\";");
}
