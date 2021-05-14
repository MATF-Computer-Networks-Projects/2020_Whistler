#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

private slots:
  void on_signupButton_clicked();

  void on_loginButton_clicked();

  void on_back_clicked();

  void on_signupButtonSignupPage_clicked();

  void accountCheckSignup();
  void accountCheckLogin();

  void on_sendButton_clicked();

  void chatHandler(QString message);
  void chatPageHandler(QString message);
  void changePasswordHandler(QString message);

  void on_logout_clicked();

  void on_changePassword_clicked();

  void on_confirmChangePassword_clicked();

  void on_backChangePasswordPage_clicked();

  void clearInputFields();
  void clearInputFieldsLoginPage();
  void clearInputFieldsSignupPage();
  void clearInputFieldsChatPage();
  void clearInputFieldsChangePasswordPage();

  void afterConnect();

private:
  Ui::MainWindow *ui;
  QTcpSocket *mSocket;
  QString username;
  QString password;
  QString hostname;
  QString port;

  QList<QString> onlineUsersWithShownOnlineStatus;

  QString serverHostname = "localhost";

  QString signupCheckString = "0xa$124432";
  QString loginCheckString = "0xa$124jk2";
  QString logoutString = "0xa$1212k2";
  QString onlineString = "0xa$121422";
  QString offlineString = "0xa$121322";
  QString changePasswordString = "0xa$112322";
  QString allOnlineString = "0xa$111432";
  QString successfulLoginString = "0xa$234232";
  QString separator = "$$$";
  int serverPort = 12345;
};
#endif // MAINWINDOW_H
