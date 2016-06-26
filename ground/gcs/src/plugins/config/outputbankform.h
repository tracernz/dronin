#ifndef OUTPUTBANKFORM_H
#define OUTPUTBANKFORM_H

#include <QGroupBox>

namespace Ui {
class OutputBankForm;
}

class OutputBankForm : public QGroupBox
{
    Q_OBJECT

public:
    explicit OutputBankForm(int bank, QList<int> channels, QWidget *parent = 0);
    ~OutputBankForm();

private:
    Ui::OutputBankForm *ui;
};

#endif // OUTPUTBANKFORM_H
