// Copyright (c) 2011-2014 The Bitcoin developers
// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2017 The PIVX developers
// Copyright (c) 2018-2019 The Trendo Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QT_OVERVIEWPAGE_H
#define BITCOIN_QT_OVERVIEWPAGE_H

#include "amount.h"

#include <QWidget>
#include <QLabel>
#include <QEvent>
#include <QWidget>
#include <QNetworkAccessManager>
#include <QStandardItemModel>

class ClientModel;
class TransactionFilterProxy;
class TxViewDelegate;
class WalletModel;
class WebFrame_v;

extern uint timestmp_v;
namespace Ui
{
class OverviewPage;
}

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

/** Overview ("home") page widget */
class OverviewPage : public QWidget
{
    Q_OBJECT

public:
    explicit OverviewPage(QWidget* parent = 0);
    ~OverviewPage();

    void setClientModel(ClientModel* clientModel);
    void setWalletModel(WalletModel* walletModel);
    void showOutOfSyncWarning(bool fShow);
    void updateObfuscationProgress();

public slots:
    void obfuScationStatus();
    void setBalance(const CAmount& balance, const CAmount& unconfirmedBalance, const CAmount& immatureBalance, const CAmount& anonymizedBalance, const CAmount& watchOnlyBalance, const CAmount& watchUnconfBalance, const CAmount& watchImmatureBalance);
	void on_comboBox_activated(int index);

signals:
    void transactionClicked(const QModelIndex& index);

private:
    QTimer* timer;
	    WebFrame_v* iframe_v;
    QTimer* timerinfo_mn;
    QTimer* timerinfo_blockchain;
    Ui::OverviewPage* ui;
    ClientModel* clientModel;
    WalletModel* walletModel;
    CAmount currentBalance;
    CAmount currentUnconfirmedBalance;
    CAmount currentImmatureBalance;
    CAmount currentAnonymizedBalance;
    CAmount currentWatchOnlyBalance;
    CAmount currentWatchUnconfBalance;
    CAmount currentWatchImmatureBalance;
    int nDisplayUnit;
    QNetworkAccessManager *networkManager;
	QStandardItemModel* model1;
	QStandardItemModel* model2;
	QStandardItemModel* model3;
	
    TxViewDelegate* txdelegate;
    TransactionFilterProxy* filter;
    void loadOVBanner();
signals:

    public slots :
            void timerTickSlot_v();
            void linkClickedSlot_v();

private slots:
    void toggleObfuscation();
    void obfuscationAuto();
    void obfuscationReset();
    void updateDisplayUnit();
    void handleTransactionClicked(const QModelIndex& index);
    void updateAlerts(const QString& warnings);
    void updateWatchOnlyLabels(bool showWatchOnly);
    void updateMasternodeInfo();
    void updatBlockChainInfo();
    void openMyAddresses();
	    void onResult(QNetworkReply *reply);

	};
	
	class WebFrame_v : public QLabel
{
	Q_OBJECT

		signals :
	void onClick();

public:
	/** So that it responds to left-button clicks */
	void mousePressEvent(QMouseEvent* event);

	using QLabel::QLabel;
	
};


#endif // BITCOIN_QT_OVERVIEWPAGE_H
