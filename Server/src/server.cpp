#include <QDebug>
#include <QSqlDatabase>
#include <QTextStream>
#include <QtSql>
#include <iostream>

#include "headers/client.h"
#include "headers/server.h"
#include "headers/simplecrypt.h"

Server::Server(QObject *parent) : QTcpServer(parent) {}
Server::~Server() { db.close(); }

bool Server::startServer(quint16 port) {

  qDebug() << "start server";
  QFile databaseInfo("../Server/db.txt");
  //  qDebug() << QFile::exists("../Server/db.txt");
  if (databaseInfo.open(QFile::ReadOnly | QFile::Text)) {
    //    qDebug() << "open";
    QTextStream in(&databaseInfo);
    QString line;

    line = in.readLine();

    QList<QString> info = line.split("$$$");
    dbHostname = info[0];
    dbUsername = info[1];
    dbPassword = info[2];
    dbDatabaseName = info[3];
    dbPort = info[4];
    qDebug() << dbHostname << dbUsername << dbPassword << dbDatabaseName
             << dbPort;
  } else {
    qDebug() << "Error while opening file db.txt";
    return false;
  }

  db = QSqlDatabase::addDatabase("QMYSQL");

  // doesnt work (known issue)
  db.setConnectOptions("MYSQL_OPT_RECONNECT=1");

  db.setHostName(dbHostname);
  db.setUserName(dbUsername);
  db.setPassword(dbPassword);
  db.setDatabaseName(dbDatabaseName);
  db.setPort(dbPort.toInt());

  //  db = QSqlDatabase::addDatabase("QMYSQL");

  //  //   doesnt work (known issue)
  //  db.setConnectOptions("MYSQL_OPT_RECONNECT=1");

  //  db.setHostName("b94tnhh8w1kwl9kczzst-mysql.services.clever-cloud.com");
  //  db.setUserName("uiesn64ti0hf90xg");
  //  db.setPassword("lY957lzqrPJVapv0sME9");
  //  db.setDatabaseName("b94tnhh8w1kwl9kczzst");
  //  db.setPort(3306);

  if (db.open()) {

    qDebug() << "db connected";

  } else {

    qDebug() << "db not connected";

    return false;
  }

  //  QString s = "prvi$$$drugi$$$treci$$$";
  //  QList<QString> lista = s.split("$$$");
  //  for (int i = 0; i < lista.length(); i++) {
  //    qDebug() << "broj " << i << " string " << lista[i];
  //  }
  //  qDebug() << "lista length " << lista.length();

  // listening to all incoming connections
  return listen(QHostAddress::Any, port);
}

void Server::incomingConnection(qintptr socketDesc) {

  qDebug() << "Connected client with socket descriptor:" << socketDesc;

  auto socket = new Client(socketDesc, nullptr, true, this);

  clients[socket->socketDesc()] = socket;

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

    // setting client in map that server has to null
    clients[client->socketDesc()] = nullptr;

    // if client that disconnected had showOnlineStatus=true then we send all
    // other clients he is disconnected
    if (client->getShowOnlineStatus()) {
      QMapIterator<int, Client *> iterator(clients);
      while (iterator.hasNext()) {
        iterator.next();
        auto c = iterator.value();
        if (c != client && c != nullptr) {
          QTextStream stream(c);
          stream << offlineString << separator << client->getUsername();
          c->flush();
          //          qDebug() << "in loop " << client->getUsername();
        }
      }
      qDebug() << "disconnected client " << client->getUsername();
    }
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
    qDebug() << "signup check username " << username;

    QSqlQuery query(db);

    query.prepare("SELECT * FROM Accounts where username=(:username)");

    query.bindValue(":username", username);

    query.exec();
    int numRows;

    // First way
    if (db.driver()->hasFeature(QSqlDriver::QuerySize)) {
      qDebug() << "db query size works";
      numRows = query.size();

    } else {
      qDebug() << "db query size doesnt work";
      // this can be very slow
      query.at();
      numRows = query.at() + 1;
    }

    // not really needed
    query.finish();

    // if there is no rows with that username we add user to database
    if (numRows == 0) {
      //      streamFrom << "Successful";

      // unique id (no need to check or use select last id from db or something)
      quint64 id = QDateTime::currentMSecsSinceEpoch();
      QString hashedPassword;
      SimpleCrypt crypto(hashingValue);
      // hashing password
      hashedPassword = crypto.encryptToString(password);

      // add new user to database
      query.prepare("INSERT INTO Accounts "
                    "VALUES (:id,:username,:password)");
      query.bindValue(":id", id);
      query.bindValue(":username", username);
      query.bindValue(":password", hashedPassword);
      query.exec();

      // if its actually added to database
      if (query.numRowsAffected() == 1) {
        streamFrom << "Successful";

        qDebug() << "successful query rows affected "
                 << query.numRowsAffected();
      } else {
        streamFrom << "Unsuccessful";

        qDebug() << "unsuccessful query rows affected "
                 << query.numRowsAffected();
      }

      // not really needed
      query.finish();

    } else {
      streamFrom << "Unsuccessful";

      qDebug() << "unsuccessful already in database";
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
    qDebug() << "show online status" << showOnlineStatus;

    bool alreadyLoggedIn = false;

    // if in map of all online clients there is client with username we get then
    // he cant login
    QMapIterator<int, Client *> iterator(clients);
    while (iterator.hasNext()) {
      iterator.next();
      auto c = iterator.value();
      if (c != client && c != nullptr) {
        if (c->getUsername() == username) {
          alreadyLoggedIn = true;
          break;
        }
      }
    }

    if (!alreadyLoggedIn) {
      // db.isOpen doesnt work as it should (known issue)
      qDebug() << "baza otvorena " << db.isOpen();

      QSqlQuery query(db);

      query.prepare("SELECT * FROM Accounts where username=:username");

      query.bindValue(":username", username);

      query.exec();
      int numRows;

      qDebug() << "query error " << query.lastError();
      qDebug() << "query executed " << query.executedQuery();

      if (db.driver()->hasFeature(QSqlDriver::QuerySize)) {
        qDebug() << "db query size works";
        numRows = query.size();

      } else {
        qDebug() << "db query size doesnt work";
        query.at();
        numRows = query.at() + 1;
      }

      qDebug() << "number of rows " << numRows;

      // error (usually db connection)
      if (numRows == -1) {

        qDebug() << "db error numRows==-1";

        // if there is no rows with that username then that is wrong username
      } else if (numRows == 0) {
        streamFrom << "Unsuccessful user doesnt exist";
        qDebug() << "Unsuccessful user doesnt exist";

        client->flush();

      } else {
        query.next();
        if (query.isValid()) {
          qDebug() << "query valid " << query.isValid();

          // there is instance in database with that username
          int idDb = query.value(0).toInt();
          QString usernameDb = query.value(1).toString();
          QString passwordDb = query.value(2).toString();

          qDebug() << idDb << usernameDb << passwordDb;

          // checking if password is correct
          QString unhashedPassword;
          SimpleCrypt crypto(hashingValue);
          unhashedPassword = crypto.decryptToString(passwordDb);

          qDebug() << "unhashed password" << unhashedPassword;

          if (unhashedPassword == password) {
            //            streamFrom << successfulLoginString;
            //            client->flush();
            qDebug() << "Successful passwords match";

            //          if (clients[client->socketDesc()] != nullptr) {
            //            clients[client->socketDesc()]->setUsername(username);
            //            qDebug() << "postavljam username na " << username <<
            //            " sada je "
            //                     <<
            //                     clients[client->socketDesc()]->getUsername();
            //          } else {

            //          client->setUsername(username);
            //          clients[client->socketDesc()] = client;

            // updating client with username and showOnlineStatus
            clients[client->socketDesc()]->setUsername(username);
            clients[client->socketDesc()]->setShowOnlineStatus(
                showOnlineStatus);

            // send list of online clients to new client
            QMapIterator<int, Client *> iterator(clients);
            QString onlineUsers = "";
            while (iterator.hasNext()) {
              iterator.next();
              auto c = iterator.value();
              if (c != client && c != nullptr && c->getShowOnlineStatus()) {
                onlineUsers.append(separator);
                onlineUsers.append(c->getUsername());
              }
            }

            // samo jednom reaguje na jedan signal (sa jednim slotom)
            qDebug() << "client is valid " << client->isValid();
            qDebug() << "all online users " << onlineUsers;

            streamFrom << successfulLoginString << separator << allOnlineString
                       << onlineUsers;

            qDebug() << "sending message to client " << successfulLoginString
                     << separator << allOnlineString << onlineUsers;

            client->flush();

            // send to other clients message that client is online only if he
            // set showOnlineStatus
            if (showOnlineStatus) {
              QMapIterator<int, Client *> iterator(clients);
              while (iterator.hasNext()) {
                iterator.next();
                auto c = iterator.value();
                if (c != client && c != nullptr) {
                  QTextStream stream(c);
                  stream << onlineString << separator << client->getUsername();
                  c->flush();
                }
              }
            }

          } else {

            streamFrom << "Unsuccessful wrong password";
            client->flush();
            qDebug() << "Unsuccessful wrong password";
          }
        }

        //        client->flush();
      }
      query.finish();
    } else {
      streamFrom << "Unsuccessful already logged in";
      client->flush();
      clients[client->socketDesc()] = nullptr;
    }
    // if client logouts
  } else if (message.startsWith(logoutString)) {
    QList<QString> messageData = message.split(separator);
    QString username = messageData[1];

    // if client who logouts had showOnlineStatus=true then we notice everyone
    // else that he logged out
    if (client->getShowOnlineStatus()) {
      QMapIterator<int, Client *> iterator(clients);
      while (iterator.hasNext()) {
        iterator.next();
        auto c = iterator.value();
        if (c != client && c != nullptr) {
          QTextStream stream(c);
          stream << offlineString << separator << username;
          c->flush();
        }
      }
    }

    // setting client in map that server has to null
    clients[client->socketDesc()] = nullptr;

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
    SimpleCrypt crypto(hashingValue);
    unhashedPassword = crypto.decryptToString(passwordDb);

    qDebug() << "unhashed password" << unhashedPassword;

    // not really needed
    query.finish();

    if (unhashedPassword == oldPassword) {

      QString newPasswordHashed = crypto.encryptToString(newPassword);

      qDebug() << "old password " << oldPassword;
      qDebug() << "new password " << newPassword;

      query.prepare(
          "UPDATE Accounts SET password=(:newPasswordHashed) where id=(:idDb)");

      qDebug() << idDb;

      query.bindValue(":newPasswordHashed", newPasswordHashed);
      query.bindValue(":idDb", idDb);

      query.exec();

      if (query.numRowsAffected() == 1) {

        streamFrom << changePasswordString << separator
                   << "Successful change password";
        qDebug() << "Successful change password";
      }

      // not really needed
      query.finish();

      qDebug() << "query " << query.lastQuery();
      qDebug() << "error " << query.lastError();
      qDebug() << "num rows affected " << query.numRowsAffected();

    } else {
      streamFrom << changePasswordString << separator
                 << "Unsuccessful change password";
      qDebug() << "Unsuccessful change password";
    }

    // basic message
  } else {
    QList<QString> messageData = message.split(separator);
    QString usernameAndTimestamp = messageData[0];
    QString messageText = messageData[1];

    // sending message to all clients except the one who sent it
    QMapIterator<int, Client *> iterator(clients);
    while (iterator.hasNext()) {
      iterator.next();
      auto c = iterator.value();
      if (c != client && c != nullptr) {
        QTextStream stream(c);
        stream << usernameAndTimestamp << " sent: " << messageText;
        c->flush();
        qDebug() << "basic message " << client->getUsername() << " "
                 << messageText;
      }
    }
  }
}
