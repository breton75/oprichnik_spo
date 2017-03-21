#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QObject>
#include <QtSql>


#include <QTimer>
#include <QTcpSocket>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pbnLoadConfig_clicked();

    void on_pbnCRC16_clicked();

    void on_bnStart_clicked();
    
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

class SvDevicePull: public QRunnable
{
public:
  QTimer timer;
  QTimer awaitResponse;
  bool isOnline;
  
  QString ip;
  quint16 port;
  
  void run();
    
private:
  QTcpSocket socket;
  
private slots:
  void getStatus();
  void getData();
  void disconnectFormHost();
  
signals:
  void status();
  
};

#endif // MAINWINDOW_H
