#include "sv_pgdb.h"
//#include "log.h"

#include <QDebug>

SvPGDB *PGDB;

int CURRENT_USER_ID = -1;
struct_user_info USER_INFO;

//using namespace log_ns;
//using namespace sql_create_defs;
//using namespace sql_insert_defs;

SvPGDB::SvPGDB(QObject *parent) :
  QObject(parent)
{
  this->setParent(parent);
}

SvPGDB::SvPGDB(SvPGDB* pg, QObject *parent) :
  QObject(parent)
{
  this->setParent(parent);
  
  if(pg)
  {
    this->p_db_name = pg->currentDBName();
    this->p_host_name = pg->currentHostName();
    this->p_password = pg->currentPassword();
    this->p_port = pg->currentPort();
    this->p_user_name = pg->currentUserName();
  }
}

//void SvPGDB::run()
//{
//  exec();
//}

QSqlError SvPGDB::connectToDB(QString dbName,
                              QString host,
                              quint16 port,
                              QString userName,
                              QString pass)
{
  try
  {
    this->db = QSqlDatabase::addDatabase("QPSQL", "PGConnection");
    
    /* ------- ловим ошибки драйвера ---------*/
    if(!this->db.isValid())
    {
      QSqlError err;
      err = this->db.lastError();
      QSqlDatabase::removeDatabase("PGConnection");
      return err;
    }

    /* ------- назначаем параметры подключения ---------*/
    if(dbName != "") p_db_name = dbName; 
    if(host != "") p_host_name = host;
    if(port) p_port = port;
    if(userName != "") p_user_name = userName;
    if(pass != "") p_password = pass;
    
    this->db.setDatabaseName(this->p_db_name);
    this->db.setHostName(this->p_host_name);
    this->db.setPort(this->p_port);
    
    this->db.setUserName(this->p_user_name);
    this->db.setPassword(this->p_password);
    
    /* ---------- подключаемся -------------*/
    this->db.open();
    
    return this->db.lastError();
    
  }
  catch(...)
  {
//    log(m_Err, "Error in SvPGDB");
  }
}

QSqlError SvPGDB::connectToDB()
{
  try
  {
    this->db = QSqlDatabase::addDatabase("QPSQL", "PGConnection");
    
    /* ------- ловим ошибки драйвера ---------*/
    if(!this->db.isValid())
    {
      QSqlError err = this->db.lastError();
      QSqlDatabase::removeDatabase("PGConnection");
      return err;
    }
    
    this->db.setDatabaseName(this->p_db_name);
    this->db.setHostName(this->p_host_name);
    this->db.setPort(this->p_port);
    
    this->db.setUserName(this->p_user_name);
    this->db.setPassword(this->p_password);
    
    /* ---------- подключаемся -------------*/
    this->db.open();
    
    return this->db.lastError();
    
  }
  catch(...)
  {
//    log(m_Err, "Error in SvPGDB");
  }
}

SvPGDB::~SvPGDB()
{
  QSqlError err = this->disconnectFromDb();
//  if(err.type() != QSqlError::NoError)
//    log_ns::log_SQL_error(err);
  
//  if(this->db.isOpen()) this->db.close();
  this->deleteLater();
  //  delete &(this->db);
//  delete this;
}

void SvPGDB::setConnectionParams(QString dbName,
                                 QString host,
                                 quint16 port,
                                 QString userName,
                                 QString pass)
{ 
  if(dbName != "") p_db_name = dbName; 
  if(host != "") p_host_name = host;
  if(port) p_port = port;
  if(userName != "") p_user_name = userName;
  if(pass != "") p_password = pass;

}

QSqlError SvPGDB::execSQL(QString queryText, bool doLog, bool commit)
{
  QSqlError err;
  if(queryText == "")
  {  
    err.setType(QSqlError::UnknownError);
    err.setDatabaseText("Query is empty");
    return err;
  }
  
//  if(doLog) log_ns::log(m_Data, queryText);
  
  QSqlQuery query = this->db.exec(queryText);
  err = query.lastError();
  query.finish();
  
//  if(doLog) log_ns::log_SQL_error(err);
  
  return err;  
  
//  if(!commit)
//  {
//    query = this->db.exec(queryText);
//  }
//  else
//  {
//    if(this->db.transaction())
//    {
//      query = this->db.exec(queryText);
//      if(query.lastError().type() == QSqlError::NoError) this->db.commit();
//      else this->db.rollback();  
//    }
//  }
  

  
}

QSqlError SvPGDB::execSQL(QString queryText, QByteArray *data1, QByteArray *data2, QByteArray *data3, QSqlQuery* query, bool doLog)
{
  QSqlError err;
  if((queryText == "") || (data1 == 0))
  {  
    err.setType(QSqlError::UnknownError);
    err.setDatabaseText("Query is empty");
    return err;
  }
  
//  if(doLog) log_ns::log(m_Data, queryText);
  
  QSqlQuery *q;
  if(!query) q = new QSqlQuery(this->db);
  else q = query;
  
  q->prepare(queryText);

  q->addBindValue(QVariant(*data1), QSql::In | QSql::Binary);
  if(data2) q->addBindValue(QVariant(*data2), QSql::In | QSql::Binary); 
  if(data3) q->addBindValue(QVariant(*data3), QSql::In | QSql::Binary);

  q->exec();
  
  err = q->lastError();
  
  if(!query)
  {
    q->finish();
    delete q;
  }
  
//  if(doLog) log_ns::log_SQL_error(err);
  
  return err;
  
}

QSqlError SvPGDB::execSQL(QString queryText, QSqlQuery* query, bool doLog)
{
  QSqlError err;
  if(queryText == "")
  {  
    err.setType(QSqlError::UnknownError);
    err.setDatabaseText("Query is empty");
    return err;
  }
  
//  if(doLog) log_ns::log(m_Data, queryText);

  *query = this->db.exec(queryText);
  
//  if(doLog) log_ns::log_SQL_error(query->lastError());
  
  return query->lastError();
  
}

QSqlError SvPGDB::execSQL(QString queryText, QSqlQueryModel *model, bool doLog)
{
  if(queryText == "")
  {  
    QSqlError err;
    err.setType(QSqlError::UnknownError);
    err.setDatabaseText("Query is empty");
    return err;
  }
  
//  if(doLog) log_ns::log(m_Data, queryText);
  
  model->setQuery(queryText, this->db);
  model->query().execBatch();
  
//  if(doLog) log_ns::log_SQL_error(model->query().lastError());

  return model->query().lastError();
  
}

QSqlError SvPGDB::disconnectFromDb()
{
  this->db.close();
  return this->db.lastError();

  //  if(!this->db.isOpen()) return QSqlError::NoError;
//  else
//  {
//    QSqlError err;
//    err.setType(QSqlError::UnknownError);
////    err.setDatabaseText();
//    err.text(this->db.lastError().text());
//    return this->db.lastError();
//  }
  
}

int SvPGDB::checkTableExists(QString tableName)
{
  QSqlQuery* query = new QSqlQuery(this->db);
  QSqlRecord rec;   
  QSqlError err;
  
  err = execSQL(QString(CHECK_TABLE_EXISTS).arg(tableName), query);
  if(err.type() != QSqlError::NoError) return err.type();
  
  bool found = false;
  
//  query->first();
//  rec = query->record();
//  while (query->next())
//    if(found = (tableName == query->value(rec.indexOf("table_name")))) break;
  found = query->size() > 0;
//  if(found) log_ns::log(m_Success, tableName + " found");
  return (found ? -1 : 0);
}
