#ifndef CLIENT_H
#define CLIENT_H

#include <QTcpSocket>

class Client : public QTcpSocket {
  Q_OBJECT
public:
  Client(qintptr socketDesc, QString username, bool showOnlineStatus,
         QObject *parent = nullptr);
  int socketDesc();
  QString getUsername();
  void setUsername(QString username);
  void setShowOnlineStatus(bool showOnlineStatus);
  bool getShowOnlineStatus();

private:
  int mSocketDesc;
  QString mUsername;
  bool mShowOnlineStatus;

public slots:
  void emitReadSignal();
  void emitStateChangedSignal(int state);

signals:
  void readSignal(Client *);
  void stateChangedSignal(Client *, int);
};

#endif // CLIENT_H
