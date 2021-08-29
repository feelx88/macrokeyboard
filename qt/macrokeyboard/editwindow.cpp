#include "editwindow.h"
#include "ui_editwindow.h"

EditWindow::EditWindow(const QString &input, const QString &command, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditWindow)
{
    ui->setupUi(this);

    setWindowTitle("Edit Command");
    setWindowModality(Qt::WindowModal);
    show();

    ui->lineEdit->setText(input);
    ui->plainTextEdit->setPlainText(command);

    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, [&]{
        emit EditWindow::save(ui->lineEdit->text(), ui->plainTextEdit->toPlainText());
        close();
    });
}

EditWindow::~EditWindow()
{
    delete ui;
}
