#ifndef EDITWINDOW_H
#define EDITWINDOW_H

#include <QDialog>

namespace Ui {
class EditWindow;
}

class EditWindow : public QDialog
{
    Q_OBJECT

public:
    explicit EditWindow(const QString &input, const QString &command, QWidget *parent = nullptr);
    ~EditWindow();

signals:
    void save(const QString & input, const QString &command);

private:
    Ui::EditWindow *ui;
};

#endif // EDITWINDOW_H
