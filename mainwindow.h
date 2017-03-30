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
#include <QMutex>

#include "u_log.h"
#include "pg_scripts.h"

#include "sg_comport.h"

//#include "sv_pgdb.h"

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
    QSqlQueryModel *_model;
    SvDevicePull *_dev_pull = nullptr;

    
    
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
  
  bool isPulling() { return _isPulling; }
  bool isFinished() { return _isFinished; }
  
  
protected:
  void run() Q_DECL_OVERRIDE;
    
private:
  QSqlDatabase _db;
  
  bool _isPulling = false;
  bool _isFinished = true;
  
public slots:
  void stopPulling();
  
signals:
  void data_updated();
  
};

#endif // MAINWINDOW_H
