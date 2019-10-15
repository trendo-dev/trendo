// Copyright (c) 2018 The GALI developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
//developer, telegram @devblockcoin

#ifndef BITCOIN_QT_GOVERNANCEPAGE_H
#define BITCOIN_QT_GOVERNANCEPAGE_H

#include "masternode.h"
#include "sync.h"
#include "util.h"

#include <QFrame>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenu>
#include <QTimer>
#include <QWidget>

class ClientModel;
class WalletModel;

namespace Ui
{
    class CoininfoPage;
}

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

class CoininfoPage : public QWidget
{
    Q_OBJECT

public:
    explicit CoininfoPage(QWidget* parent = 0);
    ~CoininfoPage();

    void setClientModel(ClientModel* clientModel);
    void setWalletModel(WalletModel* walletModel);
    void lockUpdating(bool lock);

private:
    QMenu* contextMenu;
    int64_t nTimeFilterUpdated;
    bool fFilterUpdated;
    bool fLockUpdating;


Q_SIGNALS:

private:
    QTimer* timer;
    Ui::CoininfoPage* ui;
    ClientModel* clientModel;
    WalletModel* walletModel;
    QString strCurrentFilter;

};

#endif // BITCOIN_QT_GOVERNANCEPAGE_H
