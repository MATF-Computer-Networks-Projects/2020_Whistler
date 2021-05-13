#include "headers/client.h"

Client::Client(qintptr socketDesc, QString username, bool showOnlineStatus,
               QObject *parent)
    : QTcpSocket(parent), mSocketDesc(socketDesc), mUsername(username),
      mShowOnlineStatus(showOnlineStatus) {

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

QString Client::getUsername() { return mUsername; }

void Client::setUsername(QString username) { mUsername = username; }

void Client::setShowOnlineStatus(bool showOnlineStatus) {
  mShowOnlineStatus = showOnlineStatus;
}

bool Client::getShowOnlineStatus() { return mShowOnlineStatus; }
