#include <QCoreApplication>

#include "headers/server.h"

int main(int argc, char *argv[]) {
  QCoreApplication a(argc, argv);

  // making server
  Server server;

  // starting server (if port is 0, port number is automatically chosen)
  if (!server.startServer(server.port)) {
    qDebug() << "Error:" << server.errorString();
    return 1;
  }
  // printing port number that server runs on
  qDebug() << server.serverPort();
  qDebug() << "Server up";

  return a.exec();
}
