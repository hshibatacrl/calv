#ifndef SERIALPORTDIALOG_H
#define SERIALPORTDIALOG_H

#include <QDialog>

namespace Ui {
class serialPortDialog;
}

class QSerialPort;

class serialPortDialog : public QDialog
{
    Q_OBJECT

public:
    explicit serialPortDialog(QWidget *parent = nullptr);
    ~serialPortDialog();
    virtual QSerialPort *get(QObject *parent);

private slots:
    void on_buttonBox_accepted();

private:
    Ui::serialPortDialog *ui;
};

#endif // SERIALPORTDIALOG_H
