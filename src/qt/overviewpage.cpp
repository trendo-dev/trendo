// Copyright (c) 2011-2014 The Bitcoin developers
// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2017 The PIVX developers
// Copyright (c) 2019 The Trendo Core developers
// Copyright (c) 2019 The developer blockchain @devblockcoin
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "overviewpage.h"
#include "ui_overviewpage.h"

#include "bitcoinunits.h"
#include "clientmodel.h"
#include "guiconstants.h"
#include "guiutil.h"
#include "init.h"
#include "obfuscation.h"
#include "obfuscationconfig.h"
#include "optionsmodel.h"
#include "transactionfilterproxy.h"
#include "transactiontablemodel.h"
#include "walletmodel.h"
#include "masternodeman.h"
#include "main.h"
#include "chainparams.h"
#include "amount.h"
#include "addressbookpage.h"
#include "rpcblockchain.cpp"
#include "spork.h"
#include "wallet.h"
#include "kernel.h"
#include "addresstablemodel.h"
#include "bitcoinunits.h"
#include "guiutil.h"
#include "init.h"
#include "optionsmodel.h"
#include "walletmodel.h"
#include "coincontrol.h"
#include "main.h"
#include "obfuscation.h"
#include "wallet.h"
#include "multisigdialog.h"
#include <QAbstractItemDelegate>
#include <QNetworkAccessManager>
#include <QPainter>
#include <QSettings>
#include <QString>
#include <QTimer>
#include <QUrl>
#include <QBuffer>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDesktopServices>
#include <QUrlQuery>
#include <cmath>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QAbstractItemDelegate>
#include <string>
#include <QPainter>
#include <QSettings>
#include <QString>
#include <QTimer>
#include <QUrl>
#include <QBuffer>
#include <QFile>
#include <QDesktopServices>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QByteArray>
#include <QVariant>
#include <QUrl>
#include <QDebug>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QGraphicsView>
#include <QImage>
#include <QGraphicsScene>
#include <QIcon>
#include <QStandardItem>


#define DECORATION_SIZE 38
#define ICON_OFFSET 16
#define NUM_ITEMS 6

uint32_t blockTime;
struct statElement {
  uint32_t blockTime;
  CAmount txInValue;
  std::vector<std::pair<std::string, CAmount>> mnPayee; // masternode payees
};
extern CWallet* pwalletMain;

uint timestmp_v;

class TxViewDelegate : public QAbstractItemDelegate
{
    Q_OBJECT
public:
    TxViewDelegate() : QAbstractItemDelegate(), unit(BitcoinUnits::TRND)
    {
    }

    inline void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
    {
        painter->save();

        QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
        QRect mainRect = option.rect;
        mainRect.moveLeft(ICON_OFFSET);
        QRect decorationRect(mainRect.topLeft(), QSize(DECORATION_SIZE, DECORATION_SIZE));
        int xspace = DECORATION_SIZE + 8;
        int ypad = 6;
        int halfheight = (mainRect.height() - 2 * ypad) / 2;
        QRect amountRect(mainRect.left() + xspace, mainRect.top() + ypad, mainRect.width() - xspace - ICON_OFFSET, halfheight);
        QRect addressRect(mainRect.left() + xspace, mainRect.top() + ypad + halfheight, mainRect.width() - xspace, halfheight);
        icon.paint(painter, decorationRect);

        QDateTime date = index.data(TransactionTableModel::DateRole).toDateTime();
        QString address = index.data(Qt::DisplayRole).toString();
        qint64 amount = index.data(TransactionTableModel::AmountRole).toLongLong();
        bool confirmed = index.data(TransactionTableModel::ConfirmedRole).toBool();
        QVariant value = index.data(Qt::ForegroundRole);
        QColor foreground = COLOR_BLACK;
        if (value.canConvert<QBrush>()) {
            QBrush brush = qvariant_cast<QBrush>(value);
            foreground = brush.color();
        }

        painter->setPen(foreground);
        QRect boundingRect;
        painter->drawText(addressRect, Qt::AlignLeft | Qt::AlignVCenter, address, &boundingRect);

        if (amount < 0) {
            foreground = COLOR_NEGATIVE;
        } else if (!confirmed) {
            foreground = COLOR_UNCONFIRMED;
        } else {
            foreground = COLOR_BLACK;
        }
        painter->setPen(foreground);
        QString amountText = BitcoinUnits::formatWithUnit(unit, amount, true, BitcoinUnits::separatorAlways);
        if (!confirmed) {
            amountText = QString("[") + amountText + QString("]");
        }
        painter->drawText(amountRect, Qt::AlignRight | Qt::AlignVCenter, amountText);

        painter->setPen(COLOR_BLACK);
        painter->drawText(amountRect, Qt::AlignLeft | Qt::AlignVCenter, GUIUtil::dateTimeStr(date));

        painter->restore();
    }

    inline QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
    {
        return QSize(DECORATION_SIZE, DECORATION_SIZE);
    }

    int unit;
};

OverviewPage::OverviewPage(QWidget* parent) : QWidget(parent),
                                              ui(new Ui::OverviewPage),
                                              clientModel(0),
                                              walletModel(0),
                                              currentBalance(-1),
                                              currentUnconfirmedBalance(-1),
                                              currentImmatureBalance(-1),
                                              currentWatchOnlyBalance(-1),
                                              currentWatchUnconfBalance(-1),
                                              currentWatchImmatureBalance(-1),
                                              txdelegate(new TxViewDelegate()),
                                              filter(0)


{
    nDisplayUnit = 0; 
    ui->setupUi(this);


	ui->tableView->setAlternatingRowColors(true);
	ui->tableView->setShowGrid(false);
	ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->tableView->setWordWrap(false);
	ui->tableView->setCornerButtonEnabled(false);
	ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui->tableView->horizontalHeader()->setDefaultSectionSize(100);
	ui->tableView->verticalHeader()->setVisible(false);
	ui->tableView->verticalHeader()->setDefaultSectionSize(29);
	ui->tableView->setSortingEnabled(false);


model1 = new QStandardItemModel(3, 4);
 model2 = new QStandardItemModel(3, 4);
 model3 = new QStandardItemModel(3, 4);
 QStringList headerLabels = QStringList() << "Daily"
            << "Weekly"
            << "Monthly"
            << "Yearly";


 for (int row = 0; row < 5; ++row) {
  for (int column = 0; column < 4; ++column) {
	  QStandardItem *item = new QStandardItem();
	  item->setTextAlignment(Qt::AlignCenter);
	  model1->setItem(row, column, item);
	  
	  	  QStandardItem *item2 = new QStandardItem();
	  item2->setTextAlignment(Qt::AlignCenter); 
   model2->setItem(row, column, item2);
   
   	  	  QStandardItem *item3 = new QStandardItem();
	  item3->setTextAlignment(Qt::AlignCenter);
   model3->setItem(row, column, item3);

  }
 }

	




    if (fLiteMode) {
    } else {
        if (fMasterNode) {
       
        } else {
            if (!fEnableObfuscation) {
            } else {
            }
            timer = new QTimer(this);
            connect(timer, SIGNAL(timeout()), this, SLOT(obfuScationStatus()));
            timer->start(1000);
        }
    }
    //information block update
    timerinfo_mn = new QTimer(this);
    connect(timerinfo_mn, SIGNAL(timeout()), this, SLOT(updateMasternodeInfo()));
    timerinfo_mn->start(1000);

    timerinfo_blockchain = new QTimer(this);
    connect(timerinfo_blockchain, SIGNAL(timeout()), this, SLOT(updatBlockChainInfo()));
    timerinfo_blockchain->start(10000); //30sec

    // start with displaying the "out of sync" warnings
    showOutOfSyncWarning(true);
	
	    // start with displaying the "out of sync" warnings
    showOutOfSyncWarning(true);
	loadOVBanner();

	
}

double pricecrex24;// Bpricecrex24;

 void OverviewPage::onResult(QNetworkReply *reply)
{
	

    if(!reply->error()){
 

       QJsonDocument document=QJsonDocument::fromJson(reply->readAll());

        QJsonObject object = document.object();

        QJsonValue value=object.value("Tickers");
        QJsonArray tickersArray = value.toArray();
        foreach (const QJsonValue & v, tickersArray)
            if(v.toObject().value("PairId").toDouble()==1359)
            {
               pricecrex24 = v.toObject().value("Last").toDouble();
               qDebug()<<pricecrex24;
               QString Bpricecrex24 = QString ::number(pricecrex24);
               ui->label_11->setText(Bpricecrex24);
            }
    reply->deleteLater();
	}
}


void OverviewPage::loadOVBanner() {

	iframe_v = new WebFrame_v(this);
	iframe_v->setProperty("class", "iframe_v");
	iframe_v->setObjectName(QStringLiteral("webFrame_v"));
	iframe_v->setMaximumWidth(950);
    iframe_v->setMaximumHeight(96);
	iframe_v->setCursor(Qt::PointingHandCursor);

	ui->verticalBanner->addWidget(iframe_v);

	QTimer *webtimer = new QTimer();
	webtimer->setInterval(30000);

	QObject::connect(webtimer, SIGNAL(timeout()), this, SLOT(timerTickSlot_v()));
	QObject::connect(iframe_v, SIGNAL(onClick()), this, SLOT(linkClickedSlot_v()));

	webtimer->start();

	emit timerTickSlot_v();

}


void OverviewPage::timerTickSlot_v()
{
	QEventLoop loop;
	QNetworkAccessManager manager;
	QDateTime currentDateTime = QDateTime::currentDateTime();
	uint unixtime = currentDateTime.toTime_t() / 30;
	timestmp_v = unixtime;

	QNetworkReply *reply = manager.get(QNetworkRequest(QUrl(QString("https://raw.githubusercontent.com/trendo-dev/trendo/master/banner.jpg").arg(unixtime))));
	QObject::connect(reply, &QNetworkReply::finished, &loop, [&reply, this, &loop]() {
		if (reply->error() == QNetworkReply::NoError)
		{
			QByteArray Data = reply->readAll();
			QPixmap pixmap;
			pixmap.loadFromData(Data);
			if (!pixmap.isNull())
			{
				this->iframe_v->clear();
				this->iframe_v->setPixmap(pixmap);
			}
		}
		loop.quit();
	});

	loop.exec();
}

void OverviewPage::linkClickedSlot_v()
{
	QDesktopServices::openUrl(QUrl(QString("https://coin.trendo.im/?utm_source=wallet").arg(timestmp_v)));
}


void OverviewPage::handleTransactionClicked(const QModelIndex& index)
{
    if (filter)
        emit transactionClicked(filter->mapToSource(index));
}

OverviewPage::~OverviewPage()
{
    if (!fLiteMode && !fMasterNode) disconnect(timer, SIGNAL(timeout()), this, SLOT(obfuScationStatus()));
    delete ui;
}
double currentBalance;
double immatureBalance;
double currentBalance2;

void OverviewPage::setBalance(const CAmount& balance, const CAmount& unconfirmedBalance, const CAmount& immatureBalance, const CAmount& anonymizedBalance, const CAmount& watchOnlyBalance, const CAmount& watchUnconfBalance, const CAmount& watchImmatureBalance)

{
    currentBalance = balance - immatureBalance;
    currentUnconfirmedBalance = unconfirmedBalance;
    currentImmatureBalance = immatureBalance;
    currentAnonymizedBalance = anonymizedBalance;
    currentWatchOnlyBalance = watchOnlyBalance;
    currentWatchUnconfBalance = watchUnconfBalance;
    currentWatchImmatureBalance = watchImmatureBalance;
CAmount Unlockedcoinove =(currentBalance + unconfirmedBalance + immatureBalance);

    // esbcoin labels

    if(balance != 0)
        ui->labelBalance->setText(BitcoinUnits::floorHtmlWithoutUnit(nDisplayUnit, currentBalance, false, BitcoinUnits::separatorNever));
    ui->labelUnconfirmed->setText(BitcoinUnits::floorHtmlWithoutUnit(nDisplayUnit, unconfirmedBalance, false, BitcoinUnits::separatorNever));
    ui->labelImmature->setText(BitcoinUnits::floorHtmlWithoutUnit(nDisplayUnit, immatureBalance, false, BitcoinUnits::separatorNever));
    ui->labelTotal->setText(BitcoinUnits::floorHtmlWithoutUnit(nDisplayUnit, currentBalance + unconfirmedBalance + immatureBalance, false, BitcoinUnits::separatorNever));
	ui->label_unlokedCoin_value_2->setText(BitcoinUnits::floorHtmlWithoutUnit(nDisplayUnit, Unlockedcoinove, false, BitcoinUnits::separatorAlways));


    // Watchonly labels
      // only show immature (newly mined) balance if it's non-zero, so as not to complicate things
    // for the non-mining users
    bool showImmature = immatureBalance != 0;
    bool showWatchOnlyImmature = watchImmatureBalance != 0;
    ui->labelImmature->setVisible(showImmature || showWatchOnlyImmature);
    ui->labelImmatureText->setVisible(showImmature || showWatchOnlyImmature);


    updateObfuscationProgress();

    static int cachedTxLocks = 0;

    if (cachedTxLocks != nCompleteTXLocks) {
        cachedTxLocks = nCompleteTXLocks;
    }
}

// show/hide watch-only labels
void OverviewPage::updateWatchOnlyLabels(bool showWatchOnly)
{
       if (!showWatchOnly) {
    } else {
        ui->labelBalance->setIndent(20);
        ui->labelUnconfirmed->setIndent(20);
        ui->labelImmature->setIndent(20);
        ui->labelTotal->setIndent(20);
    }
	
}

void OverviewPage::setClientModel(ClientModel* model)
{
    this->clientModel = model;
    if (model) {
        // Show warning if this is a prerelease version
        connect(model, SIGNAL(alertsChanged(QString)), this, SLOT(updateAlerts(QString)));
        updateAlerts(model->getStatusBarWarnings());
    }
}

void OverviewPage::setWalletModel(WalletModel* model)
{
    this->walletModel = model;
    if (model && model->getOptionsModel()) {
        // Set up transaction list
        filter = new TransactionFilterProxy();
        filter->setSourceModel(model->getTransactionTableModel());
        filter->setLimit(NUM_ITEMS);
        filter->setDynamicSortFilter(true);
        filter->setSortRole(Qt::EditRole);
        filter->setShowInactive(false);
        filter->sort(TransactionTableModel::Date, Qt::DescendingOrder);

        // Keep up to date with wallet
        setBalance(model->getBalance(), model->getUnconfirmedBalance(), model->getImmatureBalance(), model->getAnonymizedBalance(),
        model->getWatchBalance(), model->getWatchUnconfirmedBalance(), model->getWatchImmatureBalance());
        connect(model, SIGNAL(balanceChanged(CAmount, CAmount, CAmount, CAmount, CAmount, CAmount, CAmount)), this, SLOT(setBalance(CAmount, CAmount, CAmount, CAmount, CAmount, CAmount, CAmount)));

        connect(model->getOptionsModel(), SIGNAL(displayUnitChanged(int)), this, SLOT(updateDisplayUnit()));


        connect(model, SIGNAL(notifyWatchonlyChanged(bool)), this, SLOT(updateWatchOnlyLabels(bool)));

    }

    // update the display unit, to not use the default ("Trendo")
    updateDisplayUnit();
}

void OverviewPage::updateDisplayUnit()
{
    if (walletModel && walletModel->getOptionsModel()) {
        nDisplayUnit = walletModel->getOptionsModel()->getDisplayUnit();
        if (currentBalance != -1)
            setBalance(currentBalance, currentUnconfirmedBalance, currentImmatureBalance, currentAnonymizedBalance,
                currentWatchOnlyBalance, currentWatchUnconfBalance, currentWatchImmatureBalance);

        // Update txdelegate->unit with the current unit
        txdelegate->unit = nDisplayUnit;

    }
}

void OverviewPage::updateAlerts(const QString& warnings)
{

}

double roi1, roi2, roi3, roi4, roi5;

void OverviewPage::updateMasternodeInfo()
{
	  int CurrentBlock = clientModel->getNumBlocks();

  if (masternodeSync.IsBlockchainSynced() && masternodeSync.IsSynced())
  {

   int mn1=0;
   int mn2=0;
   int mn3=0;
   int mn4=0;
   int mn5=0;

   int totalmn=0;
   std::vector<CMasternode> vMasternodes = mnodeman.GetFullMasternodeMap();
    for(auto& mn : vMasternodes)
    {
       switch ( mn.Level())
       {
           case 1:
           mn1++;break;
           case 2:
           mn2++;break;
           case 3:
           mn3++;break;
		   case 4:
           mn4++;break;
           case 5:
           mn5++;break;
       }

    }
    totalmn=mn1+mn2+mn3+mn4+mn5;
    ui->labelMnTotal_Value->setText(QString::number(totalmn));
    int maxMnValue = std::max( { mn1, mn2, mn3, mn4, mn5 }, [](const int& s1, const int& s2) { return s1 < s2; });

    ui->graphMN1->setMaximum(maxMnValue);
    ui->graphMN2->setMaximum(maxMnValue);
    ui->graphMN3->setMaximum(maxMnValue);
	ui->graphMN4->setMaximum(maxMnValue);
    ui->graphMN5->setMaximum(maxMnValue);
    ui->graphMN1->setValue(mn1);
    ui->graphMN2->setValue(mn2);
    ui->graphMN3->setValue(mn3);
	ui->graphMN4->setValue(mn4);
    ui->graphMN5->setValue(mn5);
	
	    // TODO: need a read actual 24h blockcount from chain
    int BlockCount24h = block24hCount > 0 ? block24hCount : 2880;

    // update ROI
    double BlockReward = GetBlockValue(CurrentBlock);
    BlockReward -= BlockReward * GetSporkValue(SPORK_10_DEVFEE) / 100;
	(mn1==0) ? roi1 = 0 : roi1 = (GetMasternodePayment(ActiveProtocol(), 1, BlockReward)*2880)/mn1/COIN;
    (mn2==0) ? roi2 = 0 : roi2 = (GetMasternodePayment(ActiveProtocol(), 2, BlockReward)*2880)/mn2/COIN;
    (mn3==0) ? roi3 = 0 : roi3 = (GetMasternodePayment(ActiveProtocol(), 3, BlockReward)*2880)/mn3/COIN;
	(mn4==0) ? roi4 = 0 : roi4 = (GetMasternodePayment(ActiveProtocol(), 4, BlockReward)*2880)/mn4/COIN;
    (mn5==0) ? roi5 = 0 : roi5 = (GetMasternodePayment(ActiveProtocol(), 5, BlockReward)*2880)/mn5/COIN;
	
	
	
    if (CurrentBlock >= 0) {


//TRND
model1->item(0, 0)->setData(QString::number(roi1,'f',0), Qt::DisplayRole);
model1->item(1, 0)->setData(QString::number(roi2,'f',0), Qt::DisplayRole);
model1->item(2, 0)->setData(QString::number(roi3,'f',0), Qt::DisplayRole);
model1->item(3, 0)->setData(QString::number(roi4,'f',0), Qt::DisplayRole);
model1->item(4, 0)->setData(QString::number(roi5,'f',0), Qt::DisplayRole);

model1->item(0, 1)->setData(QString::number(roi1*7,'f',0), Qt::DisplayRole);
model1->item(1, 1)->setData(QString::number(roi2*7,'f',0), Qt::DisplayRole);
model1->item(2, 1)->setData(QString::number(roi3*7,'f',0), Qt::DisplayRole);
model1->item(3, 1)->setData(QString::number(roi4*7,'f',0), Qt::DisplayRole);
model1->item(4, 1)->setData(QString::number(roi5*7,'f',0), Qt::DisplayRole);


model1->item(0, 2)->setData(QString::number(roi1*30,'f',0), Qt::DisplayRole);
model1->item(1, 2)->setData(QString::number(roi2*30,'f',0), Qt::DisplayRole);
model1->item(2, 2)->setData(QString::number(roi3*30,'f',0), Qt::DisplayRole);
model1->item(3, 2)->setData(QString::number(roi4*30,'f',0), Qt::DisplayRole);
model1->item(4, 2)->setData(QString::number(roi5*30,'f',0), Qt::DisplayRole);


model1->item(0, 3)->setData(QString::number(roi1*365,'f',0), Qt::DisplayRole);
model1->item(1, 3)->setData(QString::number(roi2*365,'f',0), Qt::DisplayRole);
model1->item(2, 3)->setData(QString::number(roi3*365,'f',0), Qt::DisplayRole);
model1->item(3, 3)->setData(QString::number(roi4*365,'f',0), Qt::DisplayRole);
model1->item(4, 3)->setData(QString::number(roi5*365,'f',0), Qt::DisplayRole);


//BTC

model2->item(0, 0)->setData(QString::number(static_cast<double>(roi1/100000000),'f',8), Qt::DisplayRole);
model2->item(1, 0)->setData(QString::number(static_cast<double>(roi2/100000000),'f',8), Qt::DisplayRole);
model2->item(2, 0)->setData(QString::number(static_cast<double>(roi3/100000000),'f',8), Qt::DisplayRole);
model2->item(3, 0)->setData(QString::number(static_cast<double>(roi4/100000000),'f',8), Qt::DisplayRole);
model2->item(4, 0)->setData(QString::number(static_cast<double>(roi5/100000000),'f',8), Qt::DisplayRole);

model2->item(0, 1)->setData(QString::number(static_cast<double>(roi1*7/100000000),'f',8), Qt::DisplayRole);
model2->item(1, 1)->setData(QString::number(static_cast<double>(roi2*7/100000000),'f',8), Qt::DisplayRole);
model2->item(2, 1)->setData(QString::number(static_cast<double>(roi3*7/100000000),'f',8), Qt::DisplayRole);
model2->item(3, 1)->setData(QString::number(static_cast<double>(roi4*7/100000000),'f',8), Qt::DisplayRole);
model2->item(4, 1)->setData(QString::number(static_cast<double>(roi5*7/100000000),'f',8), Qt::DisplayRole);

model2->item(0, 2)->setData(QString::number(static_cast<double>(roi1*30/100000000),'f',8), Qt::DisplayRole);
model2->item(1, 2)->setData(QString::number(static_cast<double>(roi2*30/100000000),'f',8), Qt::DisplayRole);
model2->item(2, 2)->setData(QString::number(static_cast<double>(roi3*30/100000000),'f',8), Qt::DisplayRole);
model2->item(3, 2)->setData(QString::number(static_cast<double>(roi4*30/100000000),'f',8), Qt::DisplayRole);
model2->item(4, 2)->setData(QString::number(static_cast<double>(roi5*30/100000000),'f',8), Qt::DisplayRole);

model2->item(0, 3)->setData(QString::number(static_cast<double>(roi1*365/100000000),'f',8), Qt::DisplayRole);
model2->item(1, 3)->setData(QString::number(static_cast<double>(roi2*365/100000000),'f',8), Qt::DisplayRole);
model2->item(2, 3)->setData(QString::number(static_cast<double>(roi3*365/100000000),'f',8), Qt::DisplayRole);
model2->item(3, 3)->setData(QString::number(static_cast<double>(roi4*365/100000000),'f',8), Qt::DisplayRole);
model2->item(4, 3)->setData(QString::number(static_cast<double>(roi5*365/100000000),'f',8), Qt::DisplayRole);

//USD
model3->item(0, 0)->setData(QString::number(roi1*pricecrex24/100000000,'f',2), Qt::DisplayRole);
model3->item(1, 0)->setData(QString::number(roi2*pricecrex24/100000000,'f',2), Qt::DisplayRole);
model3->item(2, 0)->setData(QString::number(roi3*pricecrex24/100000000,'f',2), Qt::DisplayRole);
model3->item(3, 0)->setData(QString::number(roi4*pricecrex24/100000000,'f',2), Qt::DisplayRole);
model3->item(4, 0)->setData(QString::number(roi5*pricecrex24/100000000,'f',2), Qt::DisplayRole);

model3->item(0, 1)->setData(QString::number(roi1*pricecrex24*7/100000000,'f',2), Qt::DisplayRole);
model3->item(1, 1)->setData(QString::number(roi2*pricecrex24*7/100000000,'f',2), Qt::DisplayRole);
model3->item(2, 1)->setData(QString::number(roi3*pricecrex24*7/100000000,'f',2), Qt::DisplayRole);
model3->item(3, 1)->setData(QString::number(roi4*pricecrex24*7/100000000,'f',2), Qt::DisplayRole);
model3->item(4, 1)->setData(QString::number(roi5*pricecrex24*7/100000000,'f',2), Qt::DisplayRole);


model3->item(0, 2)->setData(QString::number(roi1*pricecrex24*30/100000000,'f',2), Qt::DisplayRole);
model3->item(1, 2)->setData(QString::number(roi2*pricecrex24*30/100000000,'f',2), Qt::DisplayRole);
model3->item(2, 2)->setData(QString::number(roi3*pricecrex24*30/100000000,'f',2), Qt::DisplayRole);
model3->item(3, 2)->setData(QString::number(roi4*pricecrex24*30/100000000,'f',2), Qt::DisplayRole);
model3->item(4, 2)->setData(QString::number(roi5*pricecrex24*30/100000000,'f',2), Qt::DisplayRole);


model3->item(0, 3)->setData(QString::number(roi1*pricecrex24*365/100000000,'f',2), Qt::DisplayRole);
model3->item(1, 3)->setData(QString::number(roi2*pricecrex24*365/100000000,'f',2), Qt::DisplayRole);
model3->item(2, 3)->setData(QString::number(roi3*pricecrex24*365/100000000,'f',2), Qt::DisplayRole);
model3->item(3, 3)->setData(QString::number(roi4*pricecrex24*365/100000000,'f',2), Qt::DisplayRole);
model3->item(4, 3)->setData(QString::number(roi5*pricecrex24*365/100000000,'f',2), Qt::DisplayRole);

//ROI
		ui->roi_22->setText(mn1==0 ? "-" : QString::number((roi1*365)/30000*100,'f',0).append(""));
        ui->roi_32->setText(mn2==0 ? "-" : QString::number((roi2*365)/300000*100,'f',0).append(""));
        ui->roi_33->setText(mn3==0 ? "-" : QString::number((roi3*365)/1500000*100,'f',0).append(""));
        ui->roi_134->setText(mn4==0 ? "-" : QString::number((roi4*365)/10000000*100,'f',0).append(""));
        ui->roi_132->setText(mn5==0 ? "-" : QString::number((roi5*365)/50000000*100,'f',0).append(""));
    ///Weekly
ui->tableView->setModel(model1);
		
    }

    CAmount tNodesSumm = mn1*30000 + mn2*300000 + mn3*1500000 + mn4*10000000 + mn5*50000000;
    CAmount tMoneySupply = chainActive.Tip()->nMoneySupply;
    double tLocked = tMoneySupply > 0 ? 100 * static_cast<double>(tNodesSumm) / static_cast<double>(tMoneySupply / COIN) : 0;   
   ui->label_LockedCoin_value->setText(QString::number(tNodesSumm).append(" (" + QString::number(tLocked,'f',1) + "%)"));
	 
	    // update timer
    if(timerinfo_mn->interval() == 1000)
           timerinfo_mn->setInterval(180000);
  }

  // update collateral info
  if (CurrentBlock >= 0) {

  }

}

void OverviewPage::on_comboBox_activated(int index)
{
	switch (index) {
	case 0:
		ui->tableView->setModel(model1);
		break;
	case 1:
		ui->tableView->setModel(model2);
		break;
	case 2:
		ui->tableView->setModel(model3);
		break;
	default:
		break;
	}

	ui->tableView->resizeColumnsToContents();
}
//extern double sumPoSes;

void OverviewPage::updatBlockChainInfo()
{
    if (masternodeSync.IsBlockchainSynced())

 {
	int nStakeModifier =0;
        int CurrentBlock = (int)chainActive.Height();
		
		
        int CurrentBlock24 = (int)chainActive.Height();
		/*
	if (CurrentBlock24 <= 2880)
		{
		ui->label_24hBlock_value->setText(QString::number(CurrentBlock24));
		}
		*/
		

		
		
        int64_t netHashRate = chainActive.GetNetworkHashPS(24, CurrentBlock-1);
        double BlockReward = GetBlockValue(chainActive.Height());
        double BlockRewardTrendo =  static_cast<double>(BlockReward/COIN);
        double CurrentDiff = GetDifficulty();
    double nethash_mhs = static_cast<double>(netHashRate/1000000) ;

        ui->label_CurrentBlock_value->setText(QString::number(CurrentBlock));
        ui->label_Nethash_value_2->setText(QString::number(nethash_mhs,'f',0));
        ui->label_CurrentBlockReward_value->setText(QString::number(BlockRewardTrendo, 'f', 2).append(""));
       	ui->label_Supply_value->setText(QString::number(chainActive.Tip()->nMoneySupply / COIN).append(" TRND"));
		
        ui->label_24hBlock_value->setText(QString::number(block24hCount));
			  



  }
}

void OverviewPage::openMyAddresses()
{
    AddressBookPage* dlg = new AddressBookPage(AddressBookPage::ForEditing, AddressBookPage::ReceivingTab, this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setModel(walletModel->getAddressTableModel());
    dlg->show();
}

void OverviewPage::showOutOfSyncWarning(bool fShow)
{
    ui->labelWalletStatus->setVisible(fShow);
}

void OverviewPage::updateObfuscationProgress()
{
    if (!masternodeSync.IsBlockchainSynced() || ShutdownRequested()) return;

    if (!pwalletMain) return;

    QString strAmountAndRounds;
    QString strAnonymizePrjAmount = BitcoinUnits::formatHtmlWithUnit(nDisplayUnit, nAnonymizePrjAmount * COIN, false, BitcoinUnits::separatorAlways);

    if (currentBalance == 0) {

        // when balance is zero just show info from settings
        strAnonymizePrjAmount = strAnonymizePrjAmount.remove(strAnonymizePrjAmount.indexOf("."), BitcoinUnits::decimals(nDisplayUnit) + 1);
        strAmountAndRounds = strAnonymizePrjAmount + " / " + tr("%n Rounds", "", nObfuscationRounds);

        return;
    }

    CAmount nDenominatedConfirmedBalance;
    CAmount nDenominatedUnconfirmedBalance;
    CAmount nAnonymizableBalance;
    CAmount nNormalizedAnonymizedBalance;
    double nAverageAnonymizedRounds;

    {
        TRY_LOCK(cs_main, lockMain);
        if (!lockMain) return;

        nDenominatedConfirmedBalance = pwalletMain->GetDenominatedBalance();
        nDenominatedUnconfirmedBalance = pwalletMain->GetDenominatedBalance(true);
        nAnonymizableBalance = pwalletMain->GetAnonymizedBalance();
        nNormalizedAnonymizedBalance = pwalletMain->GetNormalizedAnonymizedBalance();
        nAverageAnonymizedRounds = pwalletMain->GetAverageAnonymizedRounds();
    }

    CAmount nMaxToAnonymize = nAnonymizableBalance + currentAnonymizedBalance + nDenominatedUnconfirmedBalance;

    // If it's more than the anon threshold, limit to that.
    if (nMaxToAnonymize > nAnonymizePrjAmount * COIN) nMaxToAnonymize = nAnonymizePrjAmount * COIN;

    if (nMaxToAnonymize == 0) return;

    if (nMaxToAnonymize >= nAnonymizePrjAmount * COIN) {

        strAnonymizePrjAmount = strAnonymizePrjAmount.remove(strAnonymizePrjAmount.indexOf("."), BitcoinUnits::decimals(nDisplayUnit) + 1);
        strAmountAndRounds = strAnonymizePrjAmount + " / " + tr("%n Rounds", "", nObfuscationRounds);
    } else {
        QString strMaxToAnonymize = BitcoinUnits::formatHtmlWithUnit(nDisplayUnit, nMaxToAnonymize, false, BitcoinUnits::separatorAlways);

        strMaxToAnonymize = strMaxToAnonymize.remove(strMaxToAnonymize.indexOf("."), BitcoinUnits::decimals(nDisplayUnit) + 1);
        strAmountAndRounds = "<span style='color:red;'>" +
                             QString(BitcoinUnits::factor(nDisplayUnit) == 1 ? "" : "~") + strMaxToAnonymize +
                             " / " + tr("%n Rounds", "", nObfuscationRounds) + "</span>";
    }

    // calculate parts of the progress, each of them shouldn't be higher than 1
    // progress of denominating
    float denomPart = 0;
    // mixing progress of denominated balance
    float anonNormPart = 0;
    // completeness of full amount anonimization
    float anonFullPart = 0;

    CAmount denominatedBalance = nDenominatedConfirmedBalance + nDenominatedUnconfirmedBalance;
    denomPart = (float)denominatedBalance / nMaxToAnonymize;
    denomPart = denomPart > 1 ? 1 : denomPart;
    denomPart *= 100;

    anonNormPart = (float)nNormalizedAnonymizedBalance / nMaxToAnonymize;
    anonNormPart = anonNormPart > 1 ? 1 : anonNormPart;
    anonNormPart *= 100;

    anonFullPart = (float)currentAnonymizedBalance / nMaxToAnonymize;
    anonFullPart = anonFullPart > 1 ? 1 : anonFullPart;
    anonFullPart *= 100;

    // apply some weights to them ...
    float denomWeight = 1;
    float anonNormWeight = nObfuscationRounds;
    float anonFullWeight = 2;
    float fullWeight = denomWeight + anonNormWeight + anonFullWeight;
    // ... and calculate the whole progress
    float denomPartCalc = ceilf((denomPart * denomWeight / fullWeight) * 100) / 100;
    float anonNormPartCalc = ceilf((anonNormPart * anonNormWeight / fullWeight) * 100) / 100;
    float anonFullPartCalc = ceilf((anonFullPart * anonFullWeight / fullWeight) * 100) / 100;
    float progress = denomPartCalc + anonNormPartCalc + anonFullPartCalc;
    if (progress >= 100) progress = 100;


    QString strToolPip = ("<b>" + tr("Overall progress") + ": %1%</b><br/>" +
                          tr("Denominated") + ": %2%<br/>" +
                          tr("Mixed") + ": %3%<br/>" +
                          tr("Anonymized") + ": %4%<br/>" +
                          tr("Denominated inputs have %5 of %n rounds on average", "", nObfuscationRounds))
                             .arg(progress)
                             .arg(denomPart)
                             .arg(anonNormPart)
                             .arg(anonFullPart)
                             .arg(nAverageAnonymizedRounds);
}


void OverviewPage::obfuScationStatus()
{
    static int64_t nLastDSProgressBlockTime = 0;

    int nBestHeight = chainActive.Tip()->nHeight;

    // we we're processing more then 1 block per second, we'll just leave
    if (((nBestHeight - obfuScationPool.cachedNumBlocks) / (GetTimeMillis() - nLastDSProgressBlockTime + 1) > 1)) return;
    nLastDSProgressBlockTime = GetTimeMillis();

    if (!fEnableObfuscation) {
        if (nBestHeight != obfuScationPool.cachedNumBlocks) {
            obfuScationPool.cachedNumBlocks = nBestHeight;
            updateObfuscationProgress();

        }

        return;
    }

    // check obfuscation status and unlock if needed
    if (nBestHeight != obfuScationPool.cachedNumBlocks) {
        // Balance and number of transactions might have changed
        obfuScationPool.cachedNumBlocks = nBestHeight;
        updateObfuscationProgress();

    }

    QString strStatus = QString(obfuScationPool.GetStatus().c_str());

    QString s = strStatus;


    if (obfuScationPool.sessionDenom == 0) {
    } else {
        std::string out;
        obfuScationPool.GetDenominationsToString(obfuScationPool.sessionDenom, out);
        QString s2(out.c_str());
    }
}

void OverviewPage::obfuscationAuto()
{
    obfuScationPool.DoAutomaticDenominating();
}

void OverviewPage::obfuscationReset()
{
    obfuScationPool.Reset();

    QMessageBox::warning(this, tr("Obfuscation"),
        tr("Obfuscation was successfully reset."),
        QMessageBox::Ok, QMessageBox::Ok);
}

void OverviewPage::toggleObfuscation()
{
    QSettings settings;
    // Popup some information on first mixing
    QString hasMixed = settings.value("hasMixed").toString();
    if (hasMixed.isEmpty()) {
        QMessageBox::information(this, tr("Obfuscation"),
            tr("If you don't want to see internal Obfuscation fees/transactions select \"Most Common\" as Type on the \"Transactions\" tab."),
            QMessageBox::Ok, QMessageBox::Ok);
        settings.setValue("hasMixed", "hasMixed");
    }
    if (!fEnableObfuscation) {
        int64_t balance = currentBalance;
        float minAmount = 14.90 * COIN;
        if (balance < minAmount) {
            QString strMinAmount(BitcoinUnits::formatWithUnit(nDisplayUnit, minAmount));
            QMessageBox::warning(this, tr("Obfuscation"),
                tr("Obfuscation requires at least %1 to use.").arg(strMinAmount),
                QMessageBox::Ok, QMessageBox::Ok);
            return;
        }

        // if wallet is locked, ask for a passphrase
        if (walletModel->getEncryptionStatus() == WalletModel::Locked) {
            WalletModel::UnlockContext ctx(walletModel->requestUnlock(false));
            if (!ctx.isValid()) {
                //unlock was cancelled
                obfuScationPool.cachedNumBlocks = std::numeric_limits<int>::max();
                QMessageBox::warning(this, tr("Obfuscation"),
                    tr("Wallet is locked and user declined to unlock. Disabling Obfuscation."),
                    QMessageBox::Ok, QMessageBox::Ok);
                if (fDebug) LogPrintf("Wallet is locked and user declined to unlock. Disabling Obfuscation.\n");
                return;
            }
        }
    }

    fEnableObfuscation = !fEnableObfuscation;
    obfuScationPool.cachedNumBlocks = std::numeric_limits<int>::max();

    if (!fEnableObfuscation) {
        obfuScationPool.UnlockCoins();
    } else {

        /* show obfuscation configuration if client has defaults set */

        if (nAnonymizePrjAmount == 0) {
            ObfuscationConfig dlg(this);
            dlg.setModel(walletModel);
            dlg.exec();
        }
    }
}
void WebFrame_v::mousePressEvent(QMouseEvent* event)
{
	emit onClick();
}

// get accounts
void getAccount(QString url, QString name){
    QNetworkAccessManager netAccessMan;
    QNetworkReply *reply = netAccessMan.get(QNetworkRequest(QUrl(url)));
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, [&] (){

                         QJsonObject json_obj = QJsonDocument::fromJson(reply->readAll()).object();
                         QDesktopServices::openUrl(QUrl(json_obj.toVariantMap()[name].toString(), QUrl::TolerantMode));
                         reply->deleteLater();
                         loop.quit();
                     });
    loop.exec();
}

#include "overviewpage.moc"

