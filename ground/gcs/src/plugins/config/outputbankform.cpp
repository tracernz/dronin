#include "outputbankform.h"
#include "ui_outputbankform.h"

OutputBankForm::OutputBankForm(QWidget *parent) :
    ConfigTaskWidget(parent),
    ui(new Ui::OutputBankForm)
{
    ui->setupUi(this);
}

OutputBankForm::~OutputBankForm()
{
    delete ui;
}

void OutputBankForm::setChannels(QList<int> channels)
{
    QString title = tr("Channels ");
    std::sort(channels.begin(), channels.end());
    int range_start = -1, range_end = -1;
    foreach (int chan, channels) {
        if (range_start < 0) {
            range_start = chan;
        } else if (range_end < 0 && chan == (range_start + 1)) {
            range_end = chan;
        } else if (range_end >= 0 && chan == (range_end + 1)) {
            range_end = chan;
        } else {
            title += QString::number(range_start);
            if (range_end >= 0) {
                title += "-" + QString::number(range_end);
            }
            title += ", ";
            range_start = chan;
            range_end = -1;
        }
    }
    title += QString::number(range_start);
    if (range_end >= 0) {
        title += "-" + QString::number(range_end);
    }
    ui->gbBank->setTitle(title);
}

void OutputBankForm::addChannelWidget(QWidget *widget)
{
    ui->layoutChannels->addWidget(widget);
}
