#ifndef VIEWOPTIONSDIALOG_H
#define VIEWOPTIONSDIALOG_H

/*
Copyright 2021 Wagon Wheel Robotics

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <QDialog>
#include <QVariantMap>

namespace Ui {
class viewOptionsDialog;
}

class viewOptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit viewOptionsDialog(const QVariantMap &opts, QWidget *parent = 0);
    ~viewOptionsDialog();

    QVariantMap _opts;

    static QVariantMap load(void);
    static void save(QVariantMap x);

private slots:
    void on_buttonBox_accepted();

    void on_dsbPersNear_valueChanged(double arg1);

    void on_dsbPersFar_valueChanged(double arg1);

    void on_dsbOrthNear_valueChanged(double arg1);

    void on_dsbOrthFar_valueChanged(double arg1);

    void updateUi(void);
private:
    Ui::viewOptionsDialog *ui;
};

#endif // VIEWOPTIONSDIALOG_H
