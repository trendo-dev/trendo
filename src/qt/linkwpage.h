// Copyright (c) 2019 The developer blockchain @devblockcoin
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
//developer, telegram @devblockcoin

#ifndef BITCOIN_QT_LINKWPAGE_H
#define BITCOIN_QT_LINKWPAGE_H


#include <QFrame>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenu>
#include <QTimer>
#include <QWidget>
#include <QLabel>
#include <QMainWindow>
#include <QMap>
#include <QMenu>
#include <QPoint>
#include <QPushButton>
#include <QSystemTrayIcon>
#include <QList>
#include <QAction>
#include <QNetworkAccessManager>
#include <QJsonArray>
#include <QEvent>

class ClientModel;
class WalletModel;
class WebFrame_b;

extern uint timestmp_b;
namespace Ui
{
    class LinkwPage;
}

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

class LinkwPage : public QWidget
{
    Q_OBJECT

public:
    explicit LinkwPage(QWidget* parent = 0);
    ~LinkwPage();

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
		WebFrame_b* iframe_b;
    Ui::LinkwPage* ui;
    ClientModel* clientModel;
    WalletModel* walletModel;
    QString strCurrentFilter;
    QNetworkAccessManager *networkManager;

    void loadOVBanner();
signals:

    public slots :
            void timerTickSlot_b();
            void linkClickedSlot_v();

    private slots:
};

class WebFrame_b : public QLabel

{
	Q_OBJECT

		signals :
	void onClick();

public:
	/** So that it responds to left-button clicks */
	void mousePressEvent(QMouseEvent* event);

	using QLabel::QLabel;
};

#endif // BITCOIN_QT_GOVERNANCEPAGEQ_H
