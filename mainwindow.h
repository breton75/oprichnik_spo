#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QObject>
#include <QtSql>

#include <QTimer>
#include <QTcpSocket>
#include <QThread>
#include <QMap>
#include <QDebug>
#include <QSqlQueryModel>
#include <QTableView>

#include <QSplitter>
#include <QFileDialog>
#include <QMessageBox>

#include "u_log.h"
#include "pg_scripts.h"

#include "sg_comport.h"

#include "sv_pgdb.h"

namespace Ui {
class MainWindow;
}

struct SensorData
{
  int id;
  QByteArray data;
};

class SvDevicePull;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

  
private slots:
    void on_pbnLoadConfig_clicked();

    void on_pbnCRC16_clicked();

    
/** свиридов **/
    void on_bnStart_clicked();
    void slotTableDoubleClicked(QModelIndex);
    void on_bnStop_clicked();
    
public slots:
  void dataUpdated();
    
private:
//    QMap<int, SvDevicePull*> *thr_map;    
    QSqlQueryModel *model;
    SvDevicePull *dev_pull = nullptr;
//    QThread *thr_pull = nullptr;
    
    
    
/** **/
    
    
private:
    Ui::MainWindow *ui;

    // в статике - все работает, в динамике - ошибка "Driver not loaded"
    QSqlDatabase db; //* db;
    QSqlQuery query; //* query;
    int fTablesCount;

    void initDB(bool withCheckTables);
    void clearDB();
    void restoreProjDataFromFile();
    void fix(QString msg); // тестовое сообщение
};


class SvDevicePull: public QThread
{
  Q_OBJECT
  
public:
  SvDevicePull(QSqlDatabase db, QObject *parent = 0);
  
  ~SvDevicePull();
  
//  QTimer *timer; // = QTimer();
//  QTimer *awaitResponse; // = QTimer();
  
  bool isPulling() { return _isPulling; }
  bool isFinished() { return _isFinished; }
  
//  void stop();
   
  
protected:
  void run() Q_DECL_OVERRIDE;
    
private:
  QString _ip;
  quint16 _port;
  
//  quint16 _net_idx;
//  QDateTime _last_pull;
//  quint32 _pull_time;
//  quint32 _timeout;
  
//  SgComPort* _com;
  
  QTcpSocket *_socket;
  int _id;
  QSqlDatabase _db;
  
  bool _isPulling = false;
  bool _isFinished = true;
  
  QTimer _awaitigTimer;
  
//  SvPGDB* pgdb;
  
public slots:
  void startPulling();
  void stopPulling();
  
//  void getStatus();
//  void getData();
//  void disconnectFormHost();
  
signals:
  void data_updated();
  
};

#endif // MAINWINDOW_H
