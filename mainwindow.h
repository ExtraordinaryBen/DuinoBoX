#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QLabel>
#include <QString>
#include <QTextStream>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>

#include <QtSerialPort/QSerialPortInfo>
#include <QtSerialPort/QSerialPort>

#define EEPROM_SIZE 256
#define VERSION "v0.5.5"
#define GITHUB_URL "https://github.com/ExtraordinaryBen/DuinoBoX"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    Ui::MainWindow *ui;
    ~MainWindow();

private slots:
   void on_reloadButton_released();
   void on_readButton_released();
   void on_actionExit_triggered();
   void on_actionSave_EEPROM_bin_triggered();
   void on_actionAbout_triggered();

private:
    QLabel *statusMessage;
    QByteArray *results;
    QSerialPort *serial;

    void ReadyStatus();
    void SetStatus(QString);
    void ReadSerial(QString);
    void writeData();
    void XboxVersion(QString);
};

#endif // MAINWINDOW_H
