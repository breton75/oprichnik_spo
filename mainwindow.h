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
    
private:
//    QMap<int, SvDevicePull*> *thr_map;    
    QSqlQueryModel *model;
    SvDevicePull *dev_pull;
    QThread *thr_pull = nullptr;
    
    
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


class SvDevicePull: public QObject // QThread
{
  Q_OBJECT
  
public:
  SvDevicePull(QSqlDatabase *db, QObject *parent = 0);
  
  ~SvDevicePull();
  
  QTimer *timer; // = QTimer();
  QTimer *awaitResponse; // = QTimer();
  bool isOnline;
  
  void stop();
  
  QString ip;
  quint16 port;
  
  
//protected:
//  void run() Q_DECL_OVERRIDE;
    
private:
  QTcpSocket *_socket;
  int _id;
  QSqlDatabase* _db;
  
private slots:
  void getStatus();
  void getData();
  void disconnectFormHost();
  
signals:
  void status();
  
};

#endif // MAINWINDOW_H
