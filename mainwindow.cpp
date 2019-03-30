#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    results = NULL;
    statusMessage = new QLabel();

    serial = new QSerialPort(this);

    ui->statusBar->addPermanentWidget(statusMessage, 1);

    on_reloadButton_released();
}

MainWindow::~MainWindow()
{
    delete statusMessage;
    delete ui;
    if(results != NULL)
        delete results;
}

void MainWindow::on_reloadButton_released()
{
    ui->comboBox->setDisabled(true);
    ui->comboBox->clear();
    SetStatus("Getting Serial Ports...");
    // Example use QSerialPortInfo
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        qDebug() << "Name: " << info.portName();
        ui->comboBox->addItem(info.portName() + " - " + info.description(), QString(info.portName()));
        qDebug() << "Description: " << info.description();
        qDebug() << "Manufacturer: " << info.manufacturer();
    }
    ui->comboBox->setEnabled(true);
    ReadyStatus();
}

void MainWindow::on_readButton_released()
{
    if(results != NULL) {
        delete results;
        results = NULL;
    }

    SetStatus("Reading...");
    ReadSerial(QVariant(ui->comboBox->currentData()).toString());

    if(results != NULL && results->length() >= EEPROM_SIZE) {
        if(results->length() > EEPROM_SIZE) {
            results->remove(0, results->length() - EEPROM_SIZE);
        }

        qDebug() << results->toHex();

        if(results->toInt() != 0)
        {
            //Load EEPROM data into fields
            ui->serialLineEdit->setText(QString(results->mid(0x34, 12)));
            XboxVersion(ui->serialLineEdit->text());
            ui->onlineKeyLineEdit->setText(QString(results->mid(0x48, 16).toHex()).toUpper());

            ui->dvdZoneLineEdit->setText(QString(results->mid(0xBC, 4).toHex()));

            ui->macLineEdit->setText(QString(results->mid(0x40, 6).toHex().toUpper()));

            ReadyStatus();
        }
        else {
            SetStatus("Arduino's EEPROM is empty.");
        }



    }
    else if(results != NULL && results->length() < EEPROM_SIZE) {
        SetStatus(QString(results->data()));
    }
    else {
        SetStatus("Invalid number of bytes read, try again.");
    }
}

void MainWindow::on_actionExit_triggered()
{
    QApplication::quit();
}

void MainWindow::on_actionSave_EEPROM_bin_triggered()
{
    QString filter = "Binary File (*.bin)";
    QFileDialog *fileDialog = new QFileDialog;
    fileDialog->setDefaultSuffix("bin");

    QString fileName = fileDialog->getSaveFileName(this, tr("Save File"), "EEPROM.bin",
                                                        filter, &filter);
    if (fileName != "") {
        QFile file(fileName);

        if (file.open(QIODevice::ReadWrite)) {
            QDataStream stream(&file);
            //stream << ui->textBrowser->toPlainText();
            stream.writeRawData(results->data(), EEPROM_SIZE);

            file.flush();
            file.close();
        }
        else {
            QMessageBox::critical(this, tr("Error"), tr("Could not save file"));
            return;
        }
    }
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, "About", "<p align='center'>DuinoBoX " VERSION
                                      "<br>An Xbox EEPROM Reader"
                                      "<br>(<a href=" GITHUB_URL ">GitHub Page</a>)<br>"
                                      "Copyright (C) 2017, Ben DeCamp</p>");
}

void MainWindow::ReadyStatus()
{
    statusMessage->setText("Ready");
    statusMessage->repaint();
}

void MainWindow::SetStatus(QString msg)
{
    statusMessage->setText(msg);
    statusMessage->repaint();
}

void MainWindow::ReadSerial(QString portName)
{
    serial->setPortName(portName);
    serial->setParity(QSerialPort::NoParity);
    serial->setBaudRate(QSerialPort::Baud9600, QSerialPort::AllDirections);
    serial->setDataBits(QSerialPort::Data8);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);

    if(serial->open(QIODevice::ReadWrite)) {
        QByteArray dataArray;

        QString bootStr;
        while (serial->waitForReadyRead(1000))
            bootStr.append(serial->readAll());

        if(bootStr != NULL)
            qDebug() << bootStr;

        writeData();

        while (serial->waitForReadyRead(1000)) {
            QByteArray temp = serial->readAll();
            dataArray.append(temp);
        }

        //qDebug() << QString(dataArray.toHex());
        results = new QByteArray(dataArray);

        serial->close();
    }
}

void MainWindow::writeData()
{
    QString test = "1";
    serial->write(test.toLocal8Bit());
}

void MainWindow::XboxVersion(QString serialNumber)
{
    QString xboxVersion = "";

    int yearWeekBuilt = serialNumber.mid(7, 2).toInt();
    int factoryCode = serialNumber.mid(10, 2).toInt();

    if(yearWeekBuilt < 23)
        xboxVersion = "1.0";
    else if(yearWeekBuilt == 23) {
        if(factoryCode == 3)
            xboxVersion = "1.0";
        else
            xboxVersion = "1.0-1.1";
    }
    else if(yearWeekBuilt <= 25)
        xboxVersion = "1.1";
    else if(yearWeekBuilt <= 30)
        xboxVersion = "1.2";
    else if(yearWeekBuilt <= 32)
        xboxVersion = "1.3";
    else if(yearWeekBuilt <= 33)
        xboxVersion = "1.4-1.5";
    else if(yearWeekBuilt <= 41)
        xboxVersion = "1.6";
    else
        xboxVersion = "1.6b";

    ui->xboxVersionLabel->setText(xboxVersion);
}
