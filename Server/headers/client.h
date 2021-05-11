#ifndef CLIENT_H
#define CLIENT_H

#include <QTcpSocket>

class Client : public QTcpSocket {
  Q_OBJECT
public:
  Client(qintptr socketDesc, QObject *parent = nullptr);
  int socketDesc();

private:
  int mSocketDesc;

public slots:
  void emitReadSignal();
  void emitStateChangedSignal(int state);

signals:
  void readSignal(Client *);
  void stateChangedSignal(Client *, int);
};

#endif // CLIENT_H
