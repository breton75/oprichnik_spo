/* класс только для работы с базами данных PostgreSQL */

#ifndef SV_PGDB_H
#define SV_PGDB_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlDriver>
#include <QSqlQueryModel>
#include <QThread>

//#include "sql_create_defs.h"
//#include "sql_select_defs.h"

#define CHECK_TABLE_EXISTS "select "\
"  table_name "\
"from information_schema.tables "\
"where table_name = %1"

struct struct_user_info
{
  int id;
  int employee_id;
  
  bool allowAdd;
  bool allowEdit;
  bool allowDelete;
  bool allowReports;
  
  bool viewDevices;
  bool viewEmployees;
  bool viewUsers;
};

class SvPGDB : public QObject // QThread //
{
    Q_OBJECT
  private:
    QString p_db_name = "";
    QString p_host_name = "";
    quint16 p_port = 0;
    QString p_user_name = "";
    QString p_password = "";
    
//    void run() Q_DECL_OVERRIDE;
    
  public:
    SvPGDB(QObject *parent = 0);
    SvPGDB(SvPGDB* pg, QObject *parent = 0);
    ~SvPGDB();
    
    QSqlDatabase db;
    bool connected;
    
    QSqlError connectToDB(QString dbName,
                          QString host,
                          quint16 port,
                          QString userName,
                          QString pass);
    
    QSqlError connectToDB();
    
    QSqlError disconnectFromDb();
    
    
    void setConnectionParams(QString dbName = "",
                             QString host = "",
                             quint16 port = 0,
                             QString userName = "",
                             QString pass = "");
    
    QString currentDBName()   { return p_db_name; }
    QString currentHostName() { return p_host_name; }
    quint16 currentPort()     { return p_port; }
    QString currentUserName() { return p_user_name; }
    QString currentPassword() { return p_password; }
    
  signals:
    
  public slots:
    QSqlError execSQL(QString queryText, bool doLog = true, bool commit = false);
    QSqlError execSQL(QString queryText, QByteArray *data1, QByteArray *data2 = 0, QByteArray *data3 = 0, QSqlQuery* query = 0, bool doLog = true);
//    QSqlError execSQL(QString queryText, QByteArray *data1, QByteArray *data2 = 0, QByteArray *data3 = 0, QSqlQuery* query, bool doLog = true);
    QSqlError execSQL(QString queryText, QSqlQuery *query, bool doLog = true);
    QSqlError execSQL(QString queryText, QSqlQueryModel *model, bool doLog = true);
    
    int checkTableExists(QString tableName);
};

#endif // SV_PGDB_H
