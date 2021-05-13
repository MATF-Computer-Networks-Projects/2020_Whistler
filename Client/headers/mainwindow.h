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

  void on_logout_clicked();

private:
  Ui::MainWindow *ui;
  QTcpSocket *mSocket;
  QString username;
  QString password;
  QString hostname;
  QString port;

  QString signupCheckString = "0xa$124432";
  QString loginCheckString = "0xa$124jk2";
  QString logoutString = "0xa$1212k2";
  QString onlineString = "0xa$121422";
  QString offlineString = "0xa$121322";
  QString separator = "$$$";
  int serverPort = 12345;

  void clearInputFields();
};
#endif // MAINWINDOW_H
