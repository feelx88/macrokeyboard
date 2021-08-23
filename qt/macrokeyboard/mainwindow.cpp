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
    QSerialPort *serialPort;
    QSystemTrayIcon *sysTrayIcon;
    QString serialPortName;
    QVariantMap commands;

    MainWindowPrivate(MainWindow *_this)
      : _this(_this),
        serialPort(new QSerialPort(_this)),
        sysTrayIcon(new QSystemTrayIcon(QIcon(":/input-keyboard.png"), _this))
    {}

    void initConfig()
    {
      serialPortName = settings.value(SETTINGS_KEY_PORT, QVariant()).toString();
      commands = settings.value(SETTINGS_KEY_COMMANDS, QVariant()).toMap();
    }

    void initSerialPort()
    {
      QObject::connect(serialPort, &QSerialPort::readyRead, _this, [=]{
        for(auto &part : serialPort->readAll().split('\n'))
        {
          handleCommand(part.trimmed());
        }
      });
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

    void initSysTrayIcon()
    {
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

    void initSerialPortsCombo()
    {
      _this->ui->comboBox->clear();

      QList<QSerialPortInfo> serialPorts = QSerialPortInfo::availablePorts();
      for (const auto &port : qAsConst(serialPorts))
      {
          _this->ui->comboBox->addItem(port.portName());
      }

      _this->ui->comboBox->setCurrentText(serialPortName);
    }

    void initDialogButtons()
    {
      QObject::connect(_this->ui->buttonBox, &QDialogButtonBox::accepted, _this, [&]{
        QString port = _this->ui->comboBox->currentText();
        serialPortName = port;

        commands.clear();
        for (int row = 0; row < _this->ui->tableWidget->rowCount(); ++row)
        {
          commands.insert(_this->ui->tableWidget->item(row, 0)->text(), _this->ui->tableWidget->item(row, 1)->text());
        }

        settings.setValue(SETTINGS_KEY_PORT, serialPortName);
        settings.setValue(SETTINGS_KEY_COMMANDS, commands);
        connectSerialPort();
      });
      QObject::connect(_this->ui->buttonBox, &QDialogButtonBox::rejected, _this, &QWidget::close);
    }

    void initCommandsTable()
    {
      _this->ui->tableWidget->setHorizontalHeaderLabels({"Input", "Command"});

      for (auto [row, command] = std::tuple{0, commands.begin()}; command != commands.end(); ++row, ++command)
      {
        _this->ui->tableWidget->insertRow(row);
        _this->ui->tableWidget->setItem(row, 0, new QTableWidgetItem(command.key()));
        _this->ui->tableWidget->setItem(row, 1, new QTableWidgetItem(command.value().toString()));
      }

      _this->ui->tableWidget->resizeColumnsToContents();

      QObject::connect(_this->ui->pushButton, &QPushButton::pressed, _this, [=]{
        _this->ui->tableWidget->insertRow(_this->ui->tableWidget->rowCount());
      });
    }

    void handleCommand(const QString &key)
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
    _impl->initConfig();
    _impl->initSerialPort();
    _impl->connectSerialPort();

    ui->setupUi(this);
    setWindowIcon(QIcon(":/input-keyboard.png"));

    _impl->initSysTrayIcon();
    _impl->initSerialPortsCombo();
    _impl->initDialogButtons();
    _impl->initCommandsTable();
}

MainWindow::~MainWindow()
{
    delete ui;
}
