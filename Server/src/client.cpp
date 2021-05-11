#include "headers/client.h"

Client::Client(qintptr socketDesc, QObject *parent)
    : QTcpSocket(parent), mSocketDesc(socketDesc) {

  setSocketDescriptor(socketDesc);

  connect(this, &Client::readyRead, this, &Client::emitReadSignal);
  connect(this, &Client::stateChanged, this, &Client::emitStateChangedSignal);
}

// get for socket descriptor
int Client::socketDesc() { return mSocketDesc; }

void Client::emitReadSignal() { emit readSignal(this); }

void Client::emitStateChangedSignal(int state) {
  emit stateChangedSignal(this, state);
}
