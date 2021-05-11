#include <QCoreApplication>

#include "headers/server.h"

int main(int argc, char *argv[]) {
  QCoreApplication a(argc, argv);

  // making server
  Server Server;

  // starting server (if port is 0, port number is automatically chosen)
  if (!Server.startServer(0)) {
    qDebug() << "Error:" << Server.errorString();
    return 1;
  }
  // printing port number that server runs on
  qDebug() << Server.serverPort();
  qDebug() << "Server up";

  return a.exec();
}
