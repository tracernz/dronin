#ifndef OUTPUTBANKFORM_H
#define OUTPUTBANKFORM_H

#include "configtaskwidget.h"

namespace Ui {
class OutputBankForm;
}

class OutputBankForm : public ConfigTaskWidget
{
    Q_OBJECT

public:
    explicit OutputBankForm(QWidget *parent = 0);
    ~OutputBankForm();

    void setChannels(QList<int> channels);
    void addChannelWidget(QWidget *widget);

private:
    Ui::OutputBankForm *ui;
};

#endif // OUTPUTBANKFORM_H
