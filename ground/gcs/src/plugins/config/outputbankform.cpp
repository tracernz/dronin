#include "outputbankform.h"
#include "ui_outputbankform.h"
#include "outputchannelform.h"

OutputBankForm::OutputBankForm(int bank, QList<int> channels, QWidget *parent) :
    QGroupBox(parent),
    ui(new Ui::OutputBankForm)
{
    ui->setupUi(this);

    setObjectName("OutputBank" + QString::number(bank));

    QString chanText;
    int last = -1;
    bool spanning = false;

    foreach (int chan, channels) {
        Q_ASSERT(chan > 0);
        if (chan - 1 != last) {
            if (spanning)
                chanText += "-" + QString::number(last);
            if (chanText.length() > 0)
                chanText += ", ";
            chanText += QString::number(chan);
            spanning = false;
        } else {
            spanning = true;
        }
        last = chan;
    }
    if (spanning)
        chanText += "-" + QString::number(last);

    setTitle(QString("Bank %1: Channels %2").arg(bank).arg(chanText));
}

OutputBankForm::~OutputBankForm()
{
    delete ui;
}
