#include "headers/server.h"

#include <QDebug>
#include <QSqlDatabase>
#include <QTextStream>
#include <QtSql>
#include <iostream>

#include "headers/client.h"
#include "headers/simplecrypt.h"

Server::Server(QObject *parent) : QTcpServer(parent) {}
Server::~Server() { db.close(); }

bool Server::startServer(quint16 port) {

  //  QFile databaseInfo("db.txt");
  //  if (databaseInfo.open(QFile::ReadOnly | QFile::Text)) {
  //    QTextStream in(&databaseInfo);
  //    QString line;

  //    line = in.readLine();

  //    QList<QString> info = line.split("$$$");
  //    dbHostname = info[0];
  //    dbUsername = info[1];
  //    dbPassword = info[2];
  //    dbDatabaseName = info[3];
  //    dbPort = info[4];
  //  } else {
  //    std::cout << "Error while opening file db.txt";
  //        return -1;
  //  }

  //  db = QSqlDatabase::addDatabase("QSQLITE");
  //  db.setHostName(dbHostname);
  //  db.setUserName(dbUsername);
  //  db.setPassword(dbPassword);
  //  db.setDatabaseName(dbDatabaseName);
  //  db.setPort(dbPort.toInt());

  db = QSqlDatabase::addDatabase("QMYSQL");
  db.setHostName("b94tnhh8w1kwl9kczzst-mysql.services.clever-cloud.com");
  db.setUserName("uiesn64ti0hf90xg");
  db.setPassword("lY957lzqrPJVapv0sME9");
  db.setDatabaseName("b94tnhh8w1kwl9kczzst");
  db.setPort(3306);

  if (db.open()) {

    // unique id (no need to check or use select last id from db or something)
    //    quint64 id = QDateTime::currentMSecsSinceEpoch();
    //    QString hashedPassword;
    //    SimpleCrypt crypto(Q_UINT64_C(0x0c2ad4a4acb9f023));
    //    hashedPassword = crypto.encryptToString(password);

    //    query.prepare("INSERT INTO Accounts "
    //                    "VALUES (:id,:username,:password)");
    //    query.bindValue(":id", id);
    //    query.bindValue(":username", username);
    //    query.bindValue(":password", hashedPassword);

    QSqlQuery query(db);
    query.exec("INSERT INTO Accounts "
               "VALUES ('12345','username','password')");

    query.exec("SELECT id FROM Accounts");
    qDebug() << "pre";
    while (query.next()) {
      qDebug() << query.value(0).toInt();
    }
    qDebug() << "posle";

    if (db.isOpen()) {
      qDebug("radi ovde");
    }

    qDebug() << "radi";

  } else {
    qDebug() << "ne radi";
  }

  // listening to all incoming connections
  return listen(QHostAddress::Any, port);
}

void Server::incomingConnection(qintptr socketDesc) {
  qDebug() << "Connected client with socket descriptor:" << socketDesc;
  auto socket = new Client(socketDesc, NULL, true, this);
  allClients << socket;

  clients[socket->socketDesc()] = socket;

  //  for (const auto &c : allClients) {
  //    // let every other client know new client has connected
  //    if (c != socket) {
  //      QTextStream stream(c);
  //      stream << "Connected client: " << socket->socketDescriptor();
  //      c->flush();
  //    }
  //  }
  // when any client writes message, server sends that message to everyone else
  // (with broadcastAll)
  connect(socket, &Client::readSignal, this, &Server::serveClient);
  connect(socket, &Client::stateChangedSignal, this,
          &Server::clientDisconnected);
}

void Server::clientDisconnected(Client *client, int state) {
  qDebug() << "Client with socket descriptor:" << client->socketDesc()
           << "changed state";
  if (state == QTcpSocket::UnconnectedState) {
    qDebug() << "Disconnected client with socket descriptor"
             << client->socketDesc();
    allClients.removeOne(client);

    // setting client in map that server has to null
    clients[client->socketDesc()] = NULL;

    // if name is null then client never logged in
    //    if (clients[client->socketDesc()]->getUsername().isNull()) {
    //      qDebug() << "username je null";

    // if its not null we notice everyone else that he disconnected
    //    } else {

    if (client->getShowOnlineStatus()) {
      QMapIterator<int, Client *> iterator(clients);
      while (iterator.hasNext()) {
        iterator.next();
        auto c = iterator.value();
        if (c != client && c != NULL) {
          QTextStream stream(c);
          stream << offlineString << separator << client->getUsername();
          qDebug() << client->getUsername();
          c->flush();
        }
      }
    }
    //      client->setUsername(NULL);
    //        QMap<int, Client *>::iterator i;
    //        for (i = map.begin(); i != map.end(); ++i)
    //            cout << i.key() << ": " << i.value() << Qt::endl;
    //      for (const auto &c : allClients) {
    //        QTextStream stream(c);
    //        stream << client->username() << " is offline.";
    //        c->flush();
    //      }
    //    }

    // only if client has logged in first
    //    if (!client->username().isNull()) {
    //      // notice everyone that client has disconnected
    //      for (const auto &c : allClients) {
    //        QTextStream stream(c);
    //        stream << client->username() << " is offline.";
    //        c->flush();
    //      }
    //    }
  }
}

void Server::serveClient(Client *client) {
  qDebug() << "Read from client:" << client;
  QTextStream streamFrom(client);
  auto message = streamFrom.readAll();

  // signup check
  if (message.startsWith(signupCheckString)) {
    qDebug() << "Checking account on signup";
    QList<QString> userInfo = message.split(separator);
    QString username = userInfo[1];
    QString password = userInfo[2];
    qDebug() << "username" << username;

    QSqlQuery query(db);

    query.prepare("SELECT * FROM Accounts where username=(:username)");

    query.bindValue(":username", username);

    query.exec();
    int numRows;

    // First way
    if (db.driver()->hasFeature(QSqlDriver::QuerySize)) {
      qDebug() << "prvo";
      numRows = query.size();
      //      if (query.isActive()) {
      //        qDebug() << "active";
      //      } else {
      //        qDebug() << "not active";
      //      }
      //      while (query.next()) {
      //        int id = query.value(0).toInt();
      //        QString username = query.value(1).toString();
      //        QString password = query.value(2).toString();
      //        qDebug() << id << username << password;
      //      }
    } else {
      qDebug() << "drugo";
      // this can be very slow
      query.at();
      numRows = query.at() + 1;
    }

    // if there is no rows with that username we add user to database
    if (numRows == 0) {
      streamFrom << "Successful";

      // unique id (no need to check or use select last id from db or something)
      quint64 id = QDateTime::currentMSecsSinceEpoch();
      QString hashedPassword;
      SimpleCrypt crypto(hashingValue);
      hashedPassword = crypto.encryptToString(password);

      // add new user to database
      query.prepare("INSERT INTO Accounts "
                    "VALUES (:id,:username,:password)");
      query.bindValue(":id", id);
      query.bindValue(":username", username);
      query.bindValue(":password", hashedPassword);
      query.exec();

    } else {
      streamFrom << "Unsuccessful";
    }

    // // Second way
    //    query.next();
    //    if (query.isValid()) {
    //      qDebug() << query.isValid();
    //      // there is instance in database with that username
    //      int id = query.value(0).toInt();
    //      QString username = query.value(1).toString();
    //      QString password = query.value(2).toString();
    //      qDebug() << id << username << password;
    //      streamFrom << "Unsuccessful";
    //      qDebug() << "Unsuccessful";
    //    } else {
    //      streamFrom << "Successful";
    //      qDebug() << "Successful";
    //    }

    client->flush();

    // login check
  } else if (message.startsWith(loginCheckString)) {
    qDebug() << "Checking account on login";
    QList<QString> userInfo = message.split(separator);
    QString username = userInfo[1];
    QString password = userInfo[2];
    bool showOnlineStatus = userInfo[3].toInt();
    qDebug() << "username" << username;
    qDebug() << "username" << showOnlineStatus;

    QSqlQuery query(db);

    query.prepare("SELECT * FROM Accounts where username=(:username)");

    query.bindValue(":username", username);

    query.exec();
    int numRows;

    // First way
    if (db.driver()->hasFeature(QSqlDriver::QuerySize)) {
      qDebug() << "prvo";
      numRows = query.size();

    } else {
      qDebug() << "drugo";
      // this can be very slow
      query.at();
      numRows = query.at() + 1;
    }
    qDebug() << numRows;

    // if there is no rows with that username then that is wrong username
    if (numRows == 0) {
      streamFrom << "Unsuccessful user doesnt exist";
      qDebug() << "Unsuccessful user doesnt exist";

    } else {
      query.next();
      if (query.isValid()) {
        qDebug() << query.isValid();
        // there is instance in database with that username
        int idDb = query.value(0).toInt();
        QString usernameDb = query.value(1).toString();
        QString passwordDb = query.value(2).toString();
        qDebug() << idDb << usernameDb << passwordDb;

        // checking if password is correct
        QString unhashedPassword;
        SimpleCrypt crypto(Q_UINT64_C(0x0c2ad4a4acb9f023));
        unhashedPassword = crypto.decryptToString(passwordDb);

        qDebug() << "unhashed password" << unhashedPassword;

        if (unhashedPassword == password) {
          streamFrom << "Successful";
          qDebug() << "Successful";

          //          if (clients[client->socketDesc()] != NULL) {
          //            clients[client->socketDesc()]->setUsername(username);
          //            qDebug() << "postavljam username na " << username << "
          //            sada je "
          //                     <<
          //                     clients[client->socketDesc()]->getUsername();
          //          } else {

          //          client->setUsername(username);
          //          clients[client->socketDesc()] = client;

          clients[client->socketDesc()]->setUsername(username);
          clients[client->socketDesc()]->setShowOnlineStatus(showOnlineStatus);

          qDebug() << "provera nakon logina ime clienta je "
                   << clients[client->socketDesc()]->getUsername();
          //          }

          // send list of online clients to new client
          QMapIterator<int, Client *> iterator(clients);
          QString onlineUsers = "";
          while (iterator.hasNext()) {
            iterator.next();
            auto c = iterator.value();
            if (c != client && c != NULL && c->getShowOnlineStatus()) {
              onlineUsers.append(c->getUsername());
              onlineUsers.append(separator);
            }
          }
          QTextStream stream(client);
          stream << allOnlineString << separator << onlineUsers;
          client->flush();

          // send to other clients message that client is online only if he set
          // showOnlineStatus
          if (showOnlineStatus) {
            QMapIterator<int, Client *> iterator(clients);
            while (iterator.hasNext()) {
              iterator.next();
              auto c = iterator.value();
              if (c != client && c != NULL) {
                QTextStream stream(c);
                stream << onlineString << separator << client->getUsername();
                c->flush();
                qDebug() << "login poruka od " << client->getUsername();
              }
            }
          }

          //          for (const auto &c : allClients) {
          //            if (c != client) {
          //              QTextStream stream(c);
          //              stream << username << " is online.";
          //              c->flush();
          //              qDebug() << "login poruka od " <<
          //              client->getUsername();
          //            }
          //          }

          //          if (clients[client->socketDesc()] == NULL) {
          //            clients[client->socketDesc()]->setUsername(username);
          //            qDebug() << "postavljam username na " << username << "
          //            sada je "
          //                     <<
          //                     clients[client->socketDesc()]->getUsername();
          //          } else {
          //            clients[client->socketDesc()] = client;
          //          }

        } else {

          streamFrom << "Unsuccessful wrong password";
          qDebug() << "Unsuccessful wrong password";
        }
      }

      client->flush();
    }
    // if client logouts
  } else if (message.startsWith(logoutString)) {
    QList<QString> messageData = message.split(separator);
    QString username = messageData[1];

    // setting client in map that server has to null
    clients[client->socketDesc()] = NULL;

    if (client->getShowOnlineStatus()) {
      QMapIterator<int, Client *> iterator(clients);
      while (iterator.hasNext()) {
        iterator.next();
        auto c = iterator.value();
        if (c != client && c != NULL) {
          QTextStream stream(c);
          stream << offlineString << separator << client->getUsername();
          c->flush();
          qDebug() << "logout poruka od " << client->getUsername();
        }
      }
    }

    //    clients[client->socketDesc()] = NULL;
    //    client->setUsername(nullptr);

    //    for (const auto &c : allClients) {
    //      if (c != client) {
    //        QTextStream stream(c);
    //        stream << username << " is offline.";
    //        c->flush();
    //      }
    //    }

    // if client changes password
  } else if (message.startsWith(changePasswordString)) {

    QList<QString> userInfo = message.split(separator);
    QString username = userInfo[1];
    QString oldPassword = userInfo[2];
    QString newPassword = userInfo[3];
    qDebug() << "password change username" << username;

    QSqlQuery query(db);

    query.prepare("SELECT * FROM Accounts where username=(:username)");

    query.bindValue(":username", username);

    query.exec();

    query.next();

    // there is instance in database with that username
    qint64 idDb = query.value(0).toULongLong();
    QString usernameDb = query.value(1).toString();
    QString passwordDb = query.value(2).toString();
    qDebug() << idDb << usernameDb << passwordDb;

    // checking if password is correct
    QString unhashedPassword;
    SimpleCrypt crypto(Q_UINT64_C(0x0c2ad4a4acb9f023));
    unhashedPassword = crypto.decryptToString(passwordDb);

    qDebug() << "unhashed password" << unhashedPassword;

    if (unhashedPassword == oldPassword) {

      QString newPasswordHashed = crypto.encryptToString(newPassword);

      qDebug() << "stari password " << oldPassword;
      qDebug() << "novi password " << newPassword;

      qDebug() << "stari password hash " << passwordDb;
      qDebug() << "novi password hash " << newPasswordHashed;

      query.prepare("UPDATE Accounts SET password=(:newPasswordHashed) where "
                    "username=(:username)");

      query.prepare(
          "UPDATE Accounts SET password=(:newPasswordHashed) where id=(:idDb)");

      qDebug() << idDb;

      query.bindValue(":newPasswordHashed", newPasswordHashed);
      query.bindValue(":idDb", idDb);
      //      query.bindValue(":username", username);

      query.exec();

      streamFrom << changePasswordString << separator
                 << "Successful change password";
      qDebug() << "Successful change password";

      qDebug() << "query " << query.lastQuery();
      //      auto a = query.exec();
      //      qDebug() << "exec update";
      //      qDebug() << "rezultat exec " << a;
      qDebug() << "error " << query.lastError();
      qDebug() << "promenjeno redova " << query.numRowsAffected();

      query.prepare("SELECT * FROM Accounts where username=(:username)");

      query.bindValue(":username", username);

      query.exec();

      query.next();

      // there is instance in database with that username
      quint64 idDb = query.value(0).toULongLong();
      QString usernameDb = query.value(1).toString();
      QString passwordDb = query.value(2).toString();
      qDebug() << idDb << usernameDb << passwordDb;

      QString unhashedPassword;
      SimpleCrypt crypto(Q_UINT64_C(0x0c2ad4a4acb9f023));
      unhashedPassword = crypto.decryptToString(passwordDb);

      qDebug() << "novi password " << unhashedPassword;

    } else {
      streamFrom << changePasswordString << separator
                 << "Unsuccessful change password";
      qDebug() << "Unsuccessful change password";
    }

    // basic message
  } else {
    QList<QString> messageData = message.split(separator);
    QString username = messageData[0];
    QString messageText = messageData[1];

    // sending message to all clients except the one who sent it
    QMapIterator<int, Client *> iterator(clients);
    while (iterator.hasNext()) {
      iterator.next();
      auto c = iterator.value();
      if (c != client && c != NULL) {
        QTextStream stream(c);
        stream << client->getUsername() << " sent: " << messageText;
        c->flush();
        qDebug() << "obicna poruka od " << client->getUsername() << " "
                 << messageText;
      }
    }

    // sending message to all clients except the one who sent it
    //    for (const auto &c : allClients) {
    //      if (c != client) {
    //        QTextStream stream(c);
    //        stream << username << " sent: " << messageText;
    //        c->flush();
    //      }
    //    }
  }
}
