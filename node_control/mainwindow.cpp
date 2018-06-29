﻿/****************************************************************************
**
** Copyright (C) 2012 Denis Shienkov <denis.shienkov@gmail.com>
** Copyright (C) 2012 Laszlo Papp <lpapp@kde.org>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtSerialPort module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "console.h"
#include "settingsdialog.h"
#include "circle.h"
#include "define.h"
#include "linkscene.h"

#include <QMessageBox>
#include <QLabel>
#include <QGraphicsScene>
#include <QtSerialPort/QSerialPort>
#include <QTimer>
#include <QDebug>

//! [0]
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
//! [0]
    ui->setupUi(this);
    console = ui->console;
    console->setEnabled(false);
//! [1]
    serial = new QSerialPort(this);
//! [1]
    settings = new SettingsDialog;

    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setEnabled(false);
    ui->actionQuit->setEnabled(true);
    ui->actionConfigure->setEnabled(true);
    ui->currentComboBox->setCurrentIndex(NSZ+1);
    gScene = new LinkScene(this);
    ui->graphicsView->setScene(gScene);

    status = new QLabel;
    ui->statusBar->addWidget(status);

    initActionsConnections();
    setCentralWidget(ui->gridLayoutWidget);

    setWindowState(Qt::WindowMaximized);

    connect(serial, static_cast<void (QSerialPort::*)(QSerialPort::SerialPortError)>(&QSerialPort::error),
            this, &MainWindow::handleError);

//! [2]
    connect(serial, &QSerialPort::readyRead, this, &MainWindow::readData);
//! [2]
    connect(console, &Console::getData, this, &MainWindow::writeData);
//! [3]
}
//! [3]

MainWindow::~MainWindow()
{
    delete settings;
    delete gScene;
    delete ui;
}

//! [4]
void MainWindow::openSerialPort()
{
    SettingsDialog::Settings p = settings->settings();
    serial->setPortName(p.name);
    serial->setBaudRate(p.baudRate);
    serial->setDataBits(p.dataBits);
    serial->setParity(p.parity);
    serial->setStopBits(p.stopBits);
    serial->setFlowControl(p.flowControl);
    if (serial->open(QIODevice::ReadWrite)) {
        console->setEnabled(true);
        console->setLocalEchoEnabled(p.localEchoEnabled);
        ui->actionConnect->setEnabled(false);
        ui->actionDisconnect->setEnabled(true);
        ui->actionConfigure->setEnabled(false);
        showStatusMessage(tr("Connected to %1 : %2, %3, %4, %5, %6")
                          .arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits)
                          .arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl));
    } else {
        QMessageBox::critical(this, tr("Error"), serial->errorString());

        showStatusMessage(tr("Open error"));
    }
}
//! [4]

//! [5]
void MainWindow::closeSerialPort()
{
    if (serial->isOpen())
        serial->close();
    console->setEnabled(false);
    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setEnabled(false);
    ui->actionConfigure->setEnabled(true);
    showStatusMessage(tr("Disconnected"));
}
//! [5]

void MainWindow::about()
{
    QMessageBox::about(this, tr("About Simple Terminal"),
                       tr("The <b>Simple Terminal</b> example demonstrates how to "
                          "use the Qt Serial Port module in modern GUI applications "
                          "using Qt, with a menu bar, toolbars, and a status bar."));
}

//! [6]
void MainWindow::writeData(const QByteArray &data)
{
    serial->write(data);
    console->putData(data);
    ui->treeWidget->addMsg(data);
    gScene->changedLinkState(data);
}
//! [6]

//! [7]
void MainWindow::readData()
{
    serialData += serial->readAll();
    parseData();
}

void MainWindow::parseData()
{
    int begin = 0;
    while(begin < serialData.size() && serialData[begin] != quint8(0xFE)){
        ++begin;
    }
    if(serialData.size() - begin >= serialData[begin+5]+7){
        QByteArray frame = serialData.mid(begin, serialData[begin+5]+7);
        serialData.remove(0, begin+serialData[begin+5]+7);
        parseFrame(frame);
    }
}
//! [7]

//! [8]
void MainWindow::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        QMessageBox::critical(this, tr("Critical Error"), serial->errorString());
        closeSerialPort();
    }
}

//! [8]

void MainWindow::initActionsConnections()
{
    connect(ui->actionConnect, &QAction::triggered, this, &MainWindow::openSerialPort);
    connect(ui->actionDisconnect, &QAction::triggered, this, &MainWindow::closeSerialPort);
    connect(ui->actionQuit, &QAction::triggered, this, &MainWindow::close);
    connect(ui->actionConfigure, &QAction::triggered, settings, &MainWindow::show);
    connect(ui->actionClear, &QAction::triggered, console, &Console::clear);
    connect(ui->actionClear, &QAction::triggered, ui->treeWidget, &MsgTree::clear);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::about);
    connect(ui->actionAboutQt, &QAction::triggered, qApp, &QApplication::aboutQt);
    connect(gScene, &LinkScene::sendLinkData, this, &MainWindow::writeData);
}

void MainWindow::showStatusMessage(const QString &message)
{
    status->setText(message);
}

quint8 MainWindow::getCheck(const QByteArray &frame)
{
    quint16 checksum = 0;
    for(int i = 0; i < frame.size()-1; i++){
        checksum += frame[i];
    }
    return quint8(~checksum);
}

void MainWindow::parseFrame(QByteArray frame)
{
    if(frame[frame.size()-1] != getCheck(frame)){
        if(!(status->text().endsWith(QString(" || check error!")))){
            showStatusMessage(status->text()+QString(" || check error!"));
        }
        return;
    }else if(status->text().endsWith(QString(" || check error!"))){
        showStatusMessage(status->text().left(status->text().size()-16));
    }

    if(ui->currentComboBox->currentIndex() > NSZ ||
       ui->currentComboBox->currentIndex() == quint8(frame[1]) ||
       ui->currentComboBox->currentIndex() == quint8(frame[2])){
        ui->console->putData(frame);
    }
    ui->treeWidget->addMsg(frame);
    gScene->changedLinkState(frame);
}

void MainWindow::on_sendButton_clicked()
{
    QByteArray data(11, char(0xFE));
    data[1] = 0x00;
    data[2] = char(ui->fromNodeComboBox->currentIndex());     //02 dst mac
    data[3] = char(0x01);                                     //03 frame No.
    data[4] = char(0x00);                                //04 MAC Ctrl
    data[5] = char(0x04);                                     //05 length
    data[6] = data[1];                                      //06 srcIP
    data[7] = data[2];                                      //07 dstIP
    data[8] = char(0x00);                                 //08 IP Ctrl
    data[9] = char(CMD::SendCtrl);                            //09 CMD
    data[10]= char(ui->toNodeComboBox->currentIndex());       //10 SendCtrl dstIP
    data.append(ui->sendDataLineEdit->text());           //datas
    data.append(char(0x01));                                //check pos

    data[5] = char(ui->sendDataLineEdit->text().size() + 5);      //length
    data[data.size()-1] = char(getCheck(data));                   //calclute check pos

    writeData(data);
}

void MainWindow::on_currentComboBox_currentIndexChanged(int index)
{
    ui->treeWidget->showOnly(index);
}
