#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSystemTrayIcon>
#include <QMenu>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

struct MainWindowPrivate
{
    QSystemTrayIcon *sysTrayIcon;
};

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    _impl(new MainWindowPrivate)
{
    ui->setupUi(this);

    _impl->sysTrayIcon = new QSystemTrayIcon(QIcon(":/input-keyboard.svg"), this);

    connect(_impl->sysTrayIcon, &QSystemTrayIcon::activated, this, [=](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::DoubleClick)
        {
            this->show();
        }
    });

    QMenu *menu = new QMenu(this);
    _impl->sysTrayIcon->setContextMenu(menu);

    QAction *settingsAction = menu->addAction("Settings");
    connect(settingsAction, &QAction::triggered, this, &QWidget::show);

    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    for (const auto &port : ports)
    {
        ui->comboBox->addItem(port.portName());
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
