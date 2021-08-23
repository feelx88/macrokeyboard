#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <tuple>

#include <QSettings>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QDialogButtonBox>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

#define SETTINGS_KEY_PORT "port"
#define SETTINGS_KEY_COMMANDS "commands"

struct MainWindowPrivate
{
    MainWindow *_this;
    QSettings settings;
    QSystemTrayIcon *sysTrayIcon;
    QSerialPort *serialPort;
    QString serialPortName;
    QVariantMap commands;

    MainWindowPrivate(MainWindow *_this)
      : _this(_this),
        serialPort(new QSerialPort(_this))
    {}

    void createSysTrayIcon()
    {
      sysTrayIcon = new QSystemTrayIcon(QIcon(":/input-keyboard.png"), _this);
      sysTrayIcon->show();

      QObject::connect(sysTrayIcon, &QSystemTrayIcon::activated, _this, [=](QSystemTrayIcon::ActivationReason reason) {
          if (reason == QSystemTrayIcon::DoubleClick)
          {
              _this->show();
          }
      });

      QMenu *menu = new QMenu(_this);
      sysTrayIcon->setContextMenu(menu);
      menu->addAction("Settings", _this, &QWidget::show);
      menu->addAction("Quit", _this, &QApplication::exit);
    }

    void fillSerialPortsCombo()
    {
      _this->ui->comboBox->clear();

      QList<QSerialPortInfo> serialPorts = QSerialPortInfo::availablePorts();
      for (const auto &port : qAsConst(serialPorts))
      {
          _this->ui->comboBox->addItem(port.portName());
      }
    }

    void connectSerialPort()
    {
      if (serialPortName.isNull())
      {
        return;
      }

      serialPort->close();
      serialPort->clearError();
      serialPort->setPort(QSerialPortInfo(serialPortName));
      serialPort->open(QSerialPort::ReadOnly);
    }

    void pressKey(const QString &key)
    {
      if (key.isEmpty())
      {
        return;
      }

      auto command = commands.find(key);
      if (command != commands.end())
      {
        system(command.value().toString().toStdString().c_str());
      }
    }
};

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    _impl(new MainWindowPrivate(this))
{
    _impl->serialPortName = _impl->settings.value(SETTINGS_KEY_PORT, QVariant()).toString();
    _impl->commands = _impl->settings.value(SETTINGS_KEY_COMMANDS, QVariant()).toMap();
    _impl->connectSerialPort();

    ui->setupUi(this);
    setWindowIcon(QIcon(":/input-keyboard.png"));
    ui->tableWidget->setHorizontalHeaderLabels({"Input", "Command"});

    _impl->createSysTrayIcon();
    _impl->fillSerialPortsCombo();

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, [&]{
      QString port = ui->comboBox->currentText();
      _impl->serialPortName = port;

      _impl->commands.clear();
      for (int row = 0; row < ui->tableWidget->rowCount(); ++row)
      {
        _impl->commands.insert(ui->tableWidget->item(row, 0)->text(), ui->tableWidget->item(row, 1)->text());
      }

      _impl->settings.setValue(SETTINGS_KEY_PORT, _impl->serialPortName);
      _impl->settings.setValue(SETTINGS_KEY_COMMANDS, _impl->commands);
      _impl->connectSerialPort();
    });
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QWidget::close);

    for (auto [row, command] = std::tuple{0, _impl->commands.begin()}; command != _impl->commands.end(); ++row, ++command)
    {
      ui->tableWidget->insertRow(row);
      ui->tableWidget->setItem(row, 0, new QTableWidgetItem(command.key()));
      ui->tableWidget->setItem(row, 1, new QTableWidgetItem(command.value().toString()));
    }
    ui->tableWidget->resizeColumnsToContents();

    connect(ui->pushButton, &QPushButton::pressed, this, [=]{
      ui->tableWidget->insertRow(ui->tableWidget->rowCount());
    });

    connect(_impl->serialPort, &QSerialPort::readyRead, this, [=]{
      for(auto &part : _impl->serialPort->readAll().split('\n'))
      {
        _impl->pressKey(part.trimmed());
      }
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}
