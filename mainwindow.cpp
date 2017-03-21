#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSplitter>
#include <QFileDialog>
#include <QMessageBox>

#include "../common/u_log.h"
#include "pg_scripts.h"
#include "u_crc16.h"

// тестовое сообщение
void MainWindow::fix(QString msg)
{
    return;
    QMessageBox::information(0, "Контроль запуска СПО", msg);
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->mainToolBar->setVisible(false);
    ui->menuBar->setVisible(false);
    //ui->centralWidget->setLayout(ui->gridLayout);


    ui->frame_2->setLayout(ui->horizontalLayout);
    this->setWindowIcon(QIcon("./Database.png"));

    QSplitter* sp = new QSplitter(Qt::Vertical, this);
    sp->setGeometry(this->rect());
    sp->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    sp->addWidget(ui->frame_1);
    sp->addWidget(ui->frame_2);

    this->setCentralWidget(sp);

    assignLog(ui->textEdit);

    fTablesCount = sizeof(tablesList) / sizeof(tablesList[0]);
    //qDebug() << "TablesCount: " << fTablesCount;
    //return;

    // здесь / по кнопке ?
    initDB(true);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initDB(bool withCheckTables)
{
    //db = new QSqlDatabase();//"QPSQL");
    db = QSqlDatabase::addDatabase("QPSQL");//, "postgres");

    //db.addDatabase("QPSQL");
    db.setHostName("localhost");
    //db.setHostName("172.16.2.117");
    db.setPort(5432);
    db.setDatabaseName(DBName);//DATABASE_NAME); // postgres
    db.setUserName("postgres");
    db.setPassword("postgres");
    //db.setConnectOptions("Timeout=10;"); // error

    qDebug() << "connectOptions: " << db.connectOptions();

    //if(db.isOpen()) return;
    int err_count = 0;
    //QString db_error;
    QString msg;
    try {
        // пробуем открыть БД
        if (!db.open()) {
            // проверяем наличие БД
            msg = db.lastError().text();
            if(msg.contains(" not exist")) {
                msg = "БД проекта на сервере отсутствует - создаем";
                qDebug() << msg;
                logMsg(msgInfo, msg);
                // подключаемся к системной БД
                db.setDatabaseName("postgres");
                if(!db.open()) {
                    // что-то серьезное - здесь не разберемся
                    msg = "Ошибка подключения к БД postgres на сервере:<br>" + db.lastError().text();
                    logMsg(msgError, msg);
                    qDebug() << "Ошибка подключения к БД postgres на сервере:";
                    qDebug() << db.lastError().text();
                    qDebug() << "Выходим.";
                    return;
                }
                // создаем БД проекта
                //query = new QSqlQuery();
                //QSqlQuery qryCreateDB = QSqlQuery(db);
                //if (!db.exec(qryCreateDB)) {
                //QString str;
                //if (db.exec(qryCreateDB).lastError().text().trimmed() != "") {
                //if (db.exec(qryCreateDB).lastError().number() != QSqlError::NoError) {
                query = QSqlQuery(db);
                if (!query.exec(qryCreateDB)) {
                    msg = "Ошибка создания БД проекта на сервере:<br>" + query.lastError().text();
                    logMsg(msgError, msg);
                    qDebug() << "Ошибка создания БД проекта на сервере:";
                    qDebug() << query.lastError().text();
                    qDebug() << "Выходим.";
                    return;
                }
                query.finish();
                query.exec(qryCreateDB_SetComment);
                query.finish();

                msg = "БД проекта создана, подключаемся";
                logMsg(msgInfo, msg);
                qDebug() << msg;
                //query.finish();
                db.close();
                db.setDatabaseName(DBName);//DATABASE_NAME);
                // переподключаемся
                if (!db.open()) {
                    // неустранимо, выходим
                    msg = "Ошибка подключения к БД проекта:<br>" + db.lastError().text() + "<br>Попробуйте перезапустить программу.";
                    logMsg(msgError, msg);
                    qDebug() << "Ошибка подключения к БД " << DBName << ":";// DATABASE_NAME
                    qDebug() << db.lastError().text();
                    qDebug() << "Попробуйте перезапустить программу.";
                    qDebug() << "Выходим.";
                    return;
                }
                // подключились - создаем таблицы все подряд
                msg = "Подключение к БД проекта - успешно. Создаем таблицы.";
                logMsg(msgInfo, msg);
                query = QSqlQuery(db);
                for (int i = 0; i < fTablesCount; ++i) {
                    if (query.exec(tablesList[i][1])) {
                        msg = "Таблица " + tablesList[i][0] + " создана";
                        logMsg(msgInfo, msg);
                        qDebug() << "Таблица " << tablesList[i][0] << " создана";
                    } else {
                        err_count++;
                        msg = "Ошибка создания таблицы " + tablesList[i][0] + ":<br>" + query.lastError().text();
                        logMsg(msgFail, msg);
                        qDebug() << "Ошибка создания таблицы " << tablesList[i][0] << ":";
                        qDebug() << query.lastError().text();
                    }
                }
                // здесь нужно добавить заполнение справочников
                if(err_count > 0) {
                    qDebug() << "Есть ошибки (" << err_count << " штук)";
                }
                return;
            } else {
                // не подключились, но причина в другом; разбираться здесь не будем, просто выходим
                msg = "Ошибка подключения к БД проекта:<br>" + db.lastError().text() + "<br>Попробуйте перезапустить программу.";
                logMsg(msgError, msg);
                qDebug() << "Ошибка подключения к БД:";
                qDebug() << db.lastError().text();
                qDebug() << "Попробуйте перезапустить программу.";
                qDebug() << "Выходим.";
                return;
            }
        }

        query = QSqlQuery(db);
        if(!withCheckTables) return;

        // подключились штатно, проверяем наличие таблиц
        //query = new QSqlQuery();

//        qDebug() << "DB tables:";
//        foreach (QString tbl, db.tables()) {
//            qDebug() << tbl;
//        }

        for (int i = 0; i < fTablesCount; ++i) {
            if(db.tables().indexOf(tablesList[i][0]) >= 0) continue; // есть такая - идем дальше
            if (query.exec(tablesList[i][1])) {
                msg = "Таблица " + tablesList[i][0] + " создана";
                logMsg(msgInfo, msg);
                 qDebug() << "Таблица " << tablesList[i][0] << " создана";
                 //qDebug() << "Количество таблиц в БД: " << db.tables().size();
            } else {
                err_count++;
                qDebug() << "Ошибка создания таблицы " << tablesList[i][0] << ":";
                msg = "Ошибка создания таблицы " + tablesList[i][0] + ":<br>" + query.lastError().text();
                logMsg(msgFail, msg);
                qDebug() << query.lastError().text();
            }
        }
        if(err_count > 0) {
            msg = tr("Инициализация БД прошла с ошибками (количество: %1)").arg(err_count);
            logMsg(msgFail, msg);
            qDebug() << "Есть ошибки (" << err_count << " штук)";
        }
        query.finish();
    } catch (QException e) {
        msg = tr("Инициализация БД проекта - исключение:<br>").arg(e.what());
        logMsg(msgError, msg);
        qDebug() << "Инициализация БД проекта - исключение:";
        qDebug() << e.what();
        qDebug() << "DB error:";
        qDebug() << db.lastError().text();
    }
}

void MainWindow::clearDB()
{
    int n;
    // очистка БД
    if(query.exec("delete from ship_def;")) {
        n = query.numRowsAffected();
        qDebug() << "Таблица ship_def очищена, удалено записей: " << n;
        logMsg(msgInfo, tr("Таблица ship_def очищена, удалено записей: %1").arg(n));
    } else {
        qDebug() << "Ошибка очистки таблицы ship_def: " << query.lastError().text();
        logMsg(msgError, tr("Ошибка очистки таблицы ship_def:<br>%1").arg(query.lastError().text()));
    }

    if(query.exec("delete from consumers_spend;")) {
        n = query.numRowsAffected();
        qDebug() << "Таблица consumers_spend очищена, удалено записей: " << n;
        logMsg(msgInfo, tr("Таблица consumers_spend очищена, удалено записей: %1").arg(n));
    } else {
        qDebug() << "Ошибка очистки таблицы consumers_spend: " << query.lastError().text();
        logMsg(msgError, tr("Ошибка очистки таблицы consumers_spend:<br>%1").arg(query.lastError().text()));
    }

    QList<QString> tblListCPP;
    tblListCPP << "consumers" << "consumer_types" << "tanks_fuel_rest" << "tank_measures" << "fuel_measures"
                << "check_points_rest" << "check_points" << "fuel_density" << "tanks" << "tank_types"
                << "meter_types" << "run_mode_types" << "fuel_types" << "sensor_types";
    foreach (QString tblName, tblListCPP) {
        if(query.exec(tr("delete from %1;").arg(tblName))) {
            n = query.numRowsAffected();
            qDebug() << "Таблица " << tblName <<  " очищена, удалено записей: " << n;
            logMsg(msgInfo, tr("Таблица %1 очищена, удалено записей: %2").arg(tblName).arg(n));
            query.finish();
        }
        else qDebug() << "Ошибка очистки таблицы " << tblName << ": " << query.lastError().text();
    }
}

void MainWindow::restoreProjDataFromFile()
{
    if(!query.driver()->isOpen()) return;
    QString strFilter;
    QString loadFileName = QFileDialog::getOpenFileName(0,
                                                        tr("Загрузить конфигурацию из файла"), // заголовок окна
                                                        "oprichik.cfg", // каталог/файл по умолчанию
                                                        "*.cfg", // маска расширений
                                                        &strFilter // ???
                                                        );
    if(loadFileName.isEmpty()) return;
    //qDebug() << "Файл для загрузки конфигурации: " << loadFileName;
//    QStringList fConf = QStringList().op
    QFile fConfig(loadFileName);
    if(!fConfig.open(QIODevice::ReadOnly)) {
            logMsg(msgError, tr("Ошибка открытия файла '%1':<br>%2").arg(loadFileName).arg(fConfig.errorString()));
            return;
    }

    clearDB();

    QByteArray ba = fConfig.readAll();
    fConfig.close();
    qDebug() << "Config file size (bytes): " << ba.size();


    //QString st_ba(ba); // без перекодировки

    // перекодировка
    QTextCodec* codec = QTextCodec::codecForName("KOI8-R");
    QString st_ba(codec->toUnicode(ba));

    QStringList confData = st_ba.split("\r\n");//(ba);
    int linesCount = confData.size();
    qDebug() << "Config file - lines: " << linesCount;// << ", lines: " << confData.count();

    // тест результатов перекодировки (KOI8-R, CP866 (DOS), cp1251 = Windows-1251 (Win))
    // (здесь можно сделать авто-определение кодировки: 1-я строка - [Конфигурация])
//    for (int i = 0; i < 3; ++i) {
//        ui->teLog->append(confData.at(i));
//    }

    int currLine = 0;
    int errCount;
    int n;
    QString st, st1;
    QString qry;
    QStringList partHeaders = QStringList() << "Конфигурация" << "Типы" << "Цистерны" << "Потребители" << "Пересчет";
    QStringList confMainData = QStringList() << "Тип" << "Номер проекта" << "Заводской номер" << "Бортовой номер" << "Водоизмещение" << "Тип топлива" << "Количество цистерн" << "Количество потребителей";
    QStringList referenceTypes = QStringList() << "цистерн" << "измерителей" << "потребителей" << "режимов" << "топлива" << "событий";
    QStringList stParts;

    QString projType = "";
    QString projNum = "";
    QString projWorkNum = "";
    QString projBoardNum = "";
    QString projDisplacement = "";
    QString projFuelType = "";

    QString valName = "";

    int projDisp = 0;
    int projBoardNumber = 0;
    int projFuelTypeIdx = 0;
    int projTanksCount = 0;
    int projConsumersCount = 0;


    // основная конфигурация
    while (currLine < linesCount) {
        st = QString(confData.at(currLine)).trimmed();
        if(st.isEmpty()) {
            currLine++;
            continue;
        }
        if(st.startsWith('[')){
            st = st.mid(1, st.length() - 2); // убрали скобки
            //qDebug() << "Header: " << st;
            stParts = st.split(" "); // разбили на слова по пробелам
            st1 = stParts.at(0); // первое слово заголовка раздела
            //qDebug() << "\tWord 1: " << st1;
            int partIdx = partHeaders.indexOf(st1);

            switch (partIdx) {

            case 0: //"Конфигурация":
            {
                currLine++;
                st = QString(confData.at(currLine)).trimmed();
                while(!st.isEmpty()) {
                    stParts = st.split(":");
                    st1 = stParts.at(0);
                    partIdx = confMainData.indexOf(st1);
                    switch (partIdx) {
                    case 0: // "Тип"
                        projType = QString(stParts.at(1)).trimmed();
                        break;
                    case 1: // "Номер проекта"
                        projNum = QString(stParts.at(1)).trimmed();
                        break;
                    case 2: // "Заводской номер"
                        projWorkNum = QString(stParts.at(1)).trimmed();
                        break;
                    case 3: // "Бортовой номер"
                        projBoardNum = QString(stParts.at(1)).trimmed();
                        projBoardNumber = projBoardNum.toInt();
                        break;
                    case 4: // "Водоизмещение"
                        projDisplacement = QString(stParts.at(1)).trimmed();
                        projDisp = projDisplacement.toInt();
                        break;
                    case 5: // "Тип топлива"
                        projFuelType = QString(stParts.at(1)).trimmed();
                        projFuelTypeIdx = projFuelType.toInt();
                        break;
                    case 6: // "Количество цистерн"
                        projTanksCount = QString(stParts.at(1)).toInt();
                        break;
                    case 7: // "Количество потребителей"
                        projConsumersCount = QString(stParts.at(1)).toInt();
                        break;
                    default:
                        break;
                    }
                    currLine++;
                    st = QString(confData.at(currLine)).trimmed();
                }
                // с конфигурацией - все, записываем в БД после заполнения справочников - есть ссылка на тип топлива
            }
                break;
            case 1: //"Типы" - справочники
                {
                //referenceTypes = QStringList() << "цистерн" << "измерителей" << "потребителей" << "режимов" << "топлива" << "событий";
                QString refTableName;
                st1 = stParts.at(1); // второе слово заголовка раздела
                partIdx = referenceTypes.indexOf(st1);
                switch (partIdx) {
                case 0:
                    refTableName = "tank_types";
                    break;
                case 1:
                    if(stParts.size() > 2) refTableName = "sensor_types";
                    else refTableName = "meter_types";
                    //qDebug() << "Parts size: " << stParts.size() << ", ref.table: " << refTableName;
                    break;
                case 2:
                    refTableName = "consumer_types";
                    break;
                case 3:
                    refTableName = "run_mode_types";
                    break;
                case 4:
                    refTableName = "fuel_types";
                    break;
                case 5:
                    refTableName = "event_types";
                    break;
                default:
                    refTableName = "";
                    break;
                }

                if(refTableName == "") {
                    //currLine++;
                    break;
                }
                qry = tr("insert into %1 (id, name, description) values (:id, :name, :description);").arg(refTableName);
                //qDebug() << "Fill ref.table " << refTableName << " - query text: " << qry;
                //if(skipWriteToDB) break;

                currLine++;
                n = 0;
                errCount = 0;
                st = QString(confData.at(currLine)).trimmed();
                while (st != "") {
                    int c = st.indexOf("=");
                    int idx = st.left(c).toInt();//st.split("=");
                    int d = st.indexOf("(");
                    int l = st.length();
                    valName = st.mid(c + 1, d - c - 2);
                    QString valDesc = st.mid(d + 1, l - d - 2);
                    //qDebug() << "insert into " << refTableName << " values - id: " << idx << ", name: " << valName << ", description: " << valDesc;
                    query.prepare(qry);
                    query.bindValue(":id", idx);
                    query.bindValue(":name", valName);
                    query.bindValue(":description", valDesc);
                    if(!query.exec()) {
                        errCount++;
                        logMsg(msgError, tr("Ошибка вставки значений '%1', %2, %3 в справочник %4:<br>%5")
                                         .arg(idx)
                                         .arg(valName)
                                         .arg(valDesc)
                                         .arg(refTableName)
                                         .arg(query.lastError().text()));
                        logMsg(msgError, tr("Текст запроса: %1").arg(query.executedQuery()));
                        qDebug() << "Ошибка заполнения справочника " << refTableName << ":";
                        qDebug() << query.lastError().text();
                        qDebug() << "Текст запроса: " << query.executedQuery();
                    } else n++;
                    currLine++;
                    st = QString(confData.at(currLine)).trimmed();
                }
                // заполнили - протоколируем
                qDebug() << "Заполнен справочник " << refTableName << ", добавлено записей: " << n << ", ошибок: " << errCount;
                logMsg(msgInfo, tr("Заполнен справочник %1, добавлено записей: %2").arg(refTableName).arg(n));
                if(errCount > 0) logMsg(msgFail, tr("При этом возникло ошибок: %1").arg(errCount));
                }
                break;
            case 2: //"Цистерны":
                {
                    for (int i = 0; i < projTanksCount; ++i) {
                        currLine++;
                        st = QString(confData.at(currLine++)).trimmed();
                        // [Цистерна 1: главная топливная цистерна]
                        st = st.mid(1, st.length() - 2); // убрали квабратные скобки
                        //qDebug() << "убрали квабратные скобки: " << st;
                        stParts = st.split(":");
                        valName = QString(stParts.at(1)).trimmed();
                        stParts = QString(stParts.at(0)).split(" ");
                        int idx = QString(stParts.at(1)).toInt();
                        //qDebug() << "Tank Id:name " << idx << valName;


                        st = QString(confData.at(currLine++)).trimmed();//Тип цистерны: 1
                        int valTankType = QString(st.right(2)).toInt();
                        st = QString(confData.at(currLine++)).trimmed();//Тип измерителя: 2
                        int valSensorType = QString(st.right(2)).toInt();
                        st = QString(confData.at(currLine++)).trimmed();//Количество замеров: 7
                        int valMeasureCount = QString(st.right(2)).toInt();
                        //qDebug() << "tank type - sensor type - measure count: " << valTankType << valSensorType << valMeasureCount;
                        query.prepare("insert into tanks(id, tank_type, sensor_type, name) values (:id, :tank_type, :sensor_type, :name);");
                        query.bindValue(":id", idx);
                        query.bindValue(":tank_type", valTankType);
                        query.bindValue(":sensor_type", valSensorType);
                        query.bindValue(":name", valName);
                        if(!query.exec()) {
                            logMsg(msgError, tr("Ошибка вставки значений %1, %2, %3, '%4' в список цистерн:<br>%5")
                                             .arg(idx)
                                             .arg(valTankType)
                                             .arg(valSensorType)
                                             .arg(valName)
                                             .arg(query.lastError().text()));
                            logMsg(msgError, tr("Текст запроса: %1").arg(query.executedQuery()));
                            qDebug() << "Ошибка добавления цистерны '" << valName << "' в список:";
                            qDebug() << query.lastError().text();
                            qDebug() << "Текст запроса: \n" << query.executedQuery();
                            while (confData.at(currLine++).trimmed() != "") ;
                            continue;
                        }

                        errCount = 0;
                        query.prepare("insert into tank_measures(tank_id, measure_id, v_high, v_volume) values (:tank_id, :measure_id, :v_high, :v_volume);");
                        query.bindValue(":tank_id", idx);
                        for (int j = 0; j < valMeasureCount; ++j) {
                            st = QString(confData.at(currLine++)).trimmed(); // 0.500=0.500
                            stParts = st.split("=");
                            double valMeasureHigh = QString(stParts.at(0)).toFloat();
                            double valMeasureVolume = QString(stParts.at(1)).toFloat();
                            //qDebug() << "tank id - measure id - high - volume: " << idx << j << valMeasureHigh << valMeasureVolume;
                            query.bindValue(":measure_id", j);
                            query.bindValue(":v_high", valMeasureHigh);
                            query.bindValue(":v_volume", valMeasureVolume);
                            if(!query.exec()) {
                                errCount++;
                                logMsg(msgError, tr("Ошибка вставки значений '%1', %2, %3, %4 в список замеров цистерны '%5':<br>%6")
                                                 .arg(idx)
                                                 .arg(j)
                                                 .arg(valMeasureHigh)
                                                 .arg(valMeasureVolume)
                                                 .arg(valName)
                                                 .arg(query.lastError().text()));
                                logMsg(msgError, tr("Текст запроса: %1").arg(query.executedQuery()));
                                qDebug() << "Ошибка заполнения списка замеров цистерны'"  << valName << "':";
                                qDebug() << query.lastError().text();
                                qDebug() << "Текст запроса: \n" << query.executedQuery();
                            }
                        }
                        if(errCount > 0) {
                            qDebug() << "Добавление замеров цистерны " << valName << " - возникло ошибок: " << errCount;
                            logMsg(msgFail, tr("Добавление замеров цистерны %1 - возникло ошибок: %2").arg(valName).arg(errCount));
                        }
                        //while(QString(confData.at(currLine)).trimmed() != ""currLine++) ;
                    }
                    qDebug() << "Заполнен список цистерн, всего: " << projTanksCount;
                    logMsg(msgInfo, tr("Заполнен список цистерн, всего: %1").arg(projTanksCount));
                }
                break;
            case 3: //"Потребители":
                {
                for (int i = 0; i < projConsumersCount; ++i) {
                    currLine++;
                    st = QString(confData.at(currLine++)).trimmed();
                    // [Потребитель 1: основная ГДУ]
                    st = st.mid(1, st.length() - 2); // убрали квабратные скобки
                    //qDebug() << "убрали квабратные скобки: " << st;
                    //int c = st.indexOf(":");
                    //QString valIdx = st.mid(12, c - 12);
                    //int idx = QString(st.mid(12, c - 12)).toInt();
                    //QString valName = st.mid(c + 2, 100);
                    stParts = st.split(":");
                    valName = QString(stParts.at(1)).trimmed();
                    stParts = QString(stParts.at(0)).split(" ");
                    int idx = QString(stParts.at(1)).toInt();
                    //qDebug() << "Consumer Id:name " << idx << valName;
                    st = QString(confData.at(currLine++)).trimmed();//Тип потребителя: 1
                    int valConsType = QString(st.right(2)).toInt();
                    st = QString(confData.at(currLine++)).trimmed();//Тип измерителя: 2
                    int valMeterType = QString(st.right(2)).toInt();
                    st = QString(confData.at(currLine++)).trimmed();//Количество режимов работы: 3
                    int valRunModesCount = QString(st.right(2)).toInt();
                    //qDebug() << "consumer type - meter type - run mode count: " << valConsType << valMeterType << valRunModesCount;
                    query.prepare("insert into consumers(id, consumer_type, meter_type, name) values (:id, :consumer_type, :meter_type, :name);");
                    query.bindValue(":id", idx);
                    query.bindValue(":consumer_type", valConsType);
                    query.bindValue(":meter_type", valMeterType);
                    query.bindValue(":name", valName);
                    if(!query.exec()) {
                        logMsg(msgError, tr("Ошибка вставки значений '%1', %2, %3, %4 в список потребителей:<br>%5")
                                         .arg(idx)
                                         .arg(valConsType)
                                         .arg(valMeterType)
                                         .arg(valName)
                                         .arg(query.lastError().text()));
                        logMsg(msgError, tr("Текст запроса: %1").arg(query.executedQuery()));
                        qDebug() << "Ошибка добавления потребителя в список:";
                        qDebug() << query.lastError().text();
                        qDebug() << "Текст запроса: " << query.executedQuery();
                        while (confData.at(currLine++).trimmed() != "") ;
                        continue;
                    }

                    // подготовка к следующей части
                    query.prepare("insert into consumers_spend(consumer_id, run_mode_id, spend) values (:consumer_id, :run_mode_id, :spend);");
                    query.bindValue(":consumer_id", idx);

                    errCount = 0;
                    n = 0;
                    for (int j = 0; j < valRunModesCount; ++j) {
                        st = QString(confData.at(currLine++)).trimmed(); // 1=1.000
                        stParts = st.split("=");
                        int valRunModeId = QString(stParts.at(0)).toInt();
                        double valRunModeSpend = QString(stParts.at(1)).toFloat();
                        //qDebug() << "tank id - record id - mode id - spend: " << idx << j << valRunModeId << valRunModeSpend;
                        query.bindValue(":run_mode_id", valRunModeId);
                        query.bindValue(":spend", valRunModeSpend);
                        if(!query.exec()) {
                            errCount++;
                            logMsg(msgError, tr("Текст запроса: %1").arg(query.executedQuery()));
                            logMsg(msgError, tr("Ошибка вставки значений расхода потребителя '%1': %2, %3, %4 в список :<br>%5")
                                             .arg(valName)
                                             .arg(idx)
                                             .arg(valRunModeId)
                                             .arg(valRunModeSpend)
                                             .arg(query.lastError().text()));
                            qDebug() << "Ошибка заполнения списка расходов потребителя"  << valName << ":";
                            qDebug() << query.lastError().text();
                            qDebug() << "Текст запроса: " << query.executedQuery();
                        } else n++;
                    }
                    // !!!
                    qDebug() << "Заполнена таблица расхода по режимам работы потребителя " << valName << ", добавлено записей: " << n << ", ошибок: " << errCount;
                    logMsg(msgInfo, tr("Заполнена таблица расхода по режимам работы потребителя '%1', добавлено записей: %2").arg(valName).arg(n - errCount));
                    if(errCount > 0) logMsg(msgFail, tr("При этом возникло ошибок: %1").arg(errCount));
                    //while(QString(confData.at(currLine)).trimmed() != "") currLine++;
                }
                qDebug() << "Заполнен список потребителей, всего: " << projConsumersCount;
                logMsg(msgInfo, tr("Заполнен список ,потребителей всего: %1").arg(projConsumersCount));
                }
                break;
            case 4: //"Пересчет": // "Пересчет значений плотности"
                {
                //qDebug() << "Пересчет значений плотности";
                currLine++;

                st = QString(confData.at(currLine++)).trimmed();
                int fuelTypesCount = st.right(2).toInt();
                for (int i = 0; i < fuelTypesCount; ++i) {
                    n = 0;
                    errCount = 0;
                    st = QString(confData.at(currLine++)).trimmed();// [Тип топлива 1: летнее диз.топливо]
                    st = st.mid(1, st.length() - 2); // убрали квабратные скобки: 'Тип топлива 1: летнее диз.топливо'

                    stParts = st.split(":");
                    valName = QString(stParts.at(1)).trimmed(); //летнее диз.топливо
                    //qDebug() << "stParts.at(0): " << stParts.at(0);
                    stParts = QString(stParts.at(0)).split(" ");
                    int fuelTypeIdx = QString(stParts.at(2)).toInt();
                    //qDebug() << "fuelTypeIdx: " << fuelTypeIdx;
                    //int fuelTypeIdx = st.mid(st.indexOf(":") - 2, 2).toInt();

                    qry = "insert into fuel_density(id, fuel_type_id, fuel_temp, fuel_density) values (:id, :fuel_type_id, :fuel_temp, :fuel_density);";
                    query.prepare(qry);
                    query.bindValue(":fuel_type_id", fuelTypeIdx);
                    //currLine++;
                    st = QString(confData.at(currLine++)).trimmed();
                    while (st != "") { //
                        //qDebug() << "fuel_density string: " << st;
                        n++;
                        stParts = st.split("=");
                        double fuelTemp = QString(stParts.at(0)).toFloat();
                        double fuelDens = QString(stParts.at(1)).toFloat();
                        //qDebug() << "ID - fuel temp-density: "<< n << fuelTemp << fuelDens;
                        query.bindValue(":id", n);
                        query.bindValue(":fuel_temp", fuelTemp);
                        query.bindValue(":fuel_density", fuelDens);
                        if(!query.exec()) {
                            errCount++;
                            logMsg(msgError, tr("Текст запроса: %1").arg(query.executedQuery()));
                            logMsg(msgError, tr("Ошибка вставки значений '%1', %2, %3, %4 в список пересчетов плотности топлива:<br>%5")
                                             .arg(n)
                                             .arg(projFuelTypeIdx)
                                             .arg(fuelTemp)
                                             .arg(fuelDens)
                                             .arg(query.lastError().text()));
                            qDebug() << "Ошибка добавления записи в список пересчетов плотности топлива:";
                            qDebug() << "Текст запроса: " << query.executedQuery();
                            qDebug() << query.lastError().text();
                        }
                        st = QString(confData.at(currLine++)).trimmed();
                    }
                    qDebug() << "Заполнена таблица пересчета для топлива " << valName << ", добавлено записей: " << n << ", ошибок: " << errCount;
                    logMsg(msgInfo, tr("Заполнена таблица пересчета для топлива '', добавлено записей: ").arg(valName).arg(n - errCount));
                    if(errCount > 0) logMsg(msgFail, tr("При этом возникло ошибок: %1").arg(errCount));
                }
                qDebug() << "Заполнены таблицы пересчета плотности всех видов топлива, всего: " << fuelTypesCount;
                logMsg(msgInfo, tr("Заполнены таблицы пересчета плотности топлива, всего видов: %1").arg(fuelTypesCount));
                }
                break;
            default:
                //? - не поняли, идем дальше
                qDebug() << "Неопознанная ошибка, строка: " << currLine;
                currLine++;
                break;
            }

        }
        currLine++;
    }
    // восстановление основной конфигурации - после всего, когда заполнены справочники
    query.finish();
    if(query.exec("select count(*) as rec_count from ship_def;")) {
        query.next();
        int rec_count = query.value("rec_count").toInt();
        if(rec_count == 0) {
            qry = "INSERT INTO ship_def(ship_board_num, displacement, fuel_type, tanks_count, consumers_count,   ship_project, ship_man_number, ship_type) "
                    "VALUES (:ship_board_num, :displacement, :fuel_type, :tanks_count, :consumers_count,   :ship_project, :ship_man_number, :ship_type);";
        } else {
            // "update ... where ..."
            qry = "UPDATE ship_def "
                  "SET ship_board_num = :ship_board_num, displacement = :displacement, fuel_type = :fuel_type, tanks_count = :tanks_count, "
                  "consumers_count = :consumers_count, ship_project = :ship_project, ship_man_number = :ship_man_number, ship_type = :ship_type; "
                  ;
        }
        query.prepare(qry);
        query.bindValue(":ship_board_num", projBoardNumber);
        query.bindValue(":displacement", projDisp);
        //
        query.bindValue(":fuel_type", projFuelTypeIdx);
        query.bindValue(":tanks_count", projTanksCount);
        query.bindValue(":consumers_count", projConsumersCount);
        query.bindValue(":ship_project", projNum);
        query.bindValue(":ship_man_number", projWorkNum);
        query.bindValue(":ship_type", projType);
        if(query.exec()) {
            logMsg(msgInfo, "Оcновная конфигурация сохранена в БД");
        } else {
            logMsg(msgError, tr("Текст запроса: %1").arg(query.executedQuery()));
            logMsg(msgError, tr("Ошибка сохранения конфигурации:<br>%1").arg(query.lastError().text()));
            qDebug() << "Ошибка сохранения конфигурации:";
            qDebug() << "Текст запроса: " << query.executedQuery();
            qDebug() << query.lastError().text();
        }
        // здесь нет счетчиков, поправка не нужна
        //setSequenceValues();
    } else {
        logMsg(msgError, tr("Текст запроса: %1").arg(query.executedQuery()));
        logMsg(msgError, tr("Ошибка сохранения конфигурации в БД:<br>%1").arg(query.lastError().text()));
        qDebug() << "Ошибка сохранения конфигурации в БД:";
        qDebug() << "Текст запроса: " << query.executedQuery();
        qDebug() << query.lastError().text();
    }

}

void MainWindow::on_pbnLoadConfig_clicked()
{
    restoreProjDataFromFile();
}

void MainWindow::on_pbnCRC16_clicked()
{
    QByteArray ba("123456789");
    unsigned char* pc = reinterpret_cast<unsigned char*>(ba.data());//&test;
    unsigned short pc_len = 9;
    unsigned short crc = Crc16(pc, pc_len);// Crc16_quick(pc, pc_len);
    quint16 w_crc = crc;
    char lo_crc = w_crc % 16;
    char hi_crc = w_crc / 16;
    qDebug() << "word crc: " << w_crc //<< " (" << tr("%1").arg(w_crc) << tr("%1").arg(w_crc, 16)
             << ", lo: " << tr("%1").arg(lo_crc) << " (" << QByteArray().append(lo_crc)
             << "), hi: " << tr("%1").arg(hi_crc) << " (" << QByteArray().append(hi_crc) << ")";

    qDebug() << "crc quick: " << Crc16_quick(pc, pc_len);
    //ba.append(w_crc);// << crc;
    ba.append(lo_crc).append(hi_crc);
    qDebug() << "123456789 + crc = " << ba;
    qDebug() << "crc: " << crc << ", as QByteArray: " << QByteArray().append(lo_crc).append(hi_crc);
    logMsg(msgInfo, tr("123456789 + crc = %1").arg(QString(ba)));
    logMsg(msgInfo, tr("crc: %1, as QByteArray: %2").arg(crc).arg(QString(QByteArray().append(lo_crc).append(hi_crc))));

}
