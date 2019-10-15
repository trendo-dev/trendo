// Copyright (c) 2018 The GALI developers
// Copyright (c) 2019 The Developer blockchain - @devblockcoin
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
//developer, telegram @devblockcoin

#include "linkwpage.h"
#include "ui_linkwpage.h"
#include "walletmodel.h"
#include <QMessageBox>
#include <QString>
#include <QTimer>
#include <QToolButton>
#include <QLabel>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkReply>
#include <QUrl>
#include <QEventLoop>
#include <QBuffer>
#include <QDesktopServices>

extern CWallet* pwalletMain;

uint timestmp_b;

LinkwPage::LinkwPage(QWidget* parent) : QWidget(parent),
                                                  ui(new Ui::LinkwPage),
                                                  clientModel(0),
                                                  walletModel(0)
{
    ui->setupUi(this);
	loadOVBanner();

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateProposalList()));
    timer->start(100000);
    fLockUpdating = false;

//developer, telegram @devblockcoin
    QFrame *frames[9] = {ui->frame_5, ui->frame_7, ui->frame_6, ui->frame_8, ui->frame_9, ui->frame_10, ui->frame_13, ui->frame_14, ui->frame_16};
    int frame_count = sizeof(frames)/sizeof(frames[0]);

    networkManager = new QNetworkAccessManager();

    QEventLoop loop;

    connect(networkManager, SIGNAL(finished(QNetworkReply*)), &loop, SLOT(quit()));
    QNetworkReply *nnn = networkManager->get(QNetworkRequest(QUrl("https://raw.githubusercontent.com/trendo-dev/trendo/master/links.json")));
    loop.exec(); 

    QJsonArray ja; 
    if(!nnn->error())
    {
        QJsonDocument doc = QJsonDocument::fromJson(nnn->readAll());
        QJsonObject root = doc.object();
        QJsonValue jv_sections = root.value("sections");
        QJsonArray ja_sections; 
        if(jv_sections.isArray())
        {
            ja_sections = jv_sections.toArray();
        }

        for(int i=0; i<ja_sections.count(), i<frame_count; i++)
        {
            QJsonObject section = ja_sections.at(i).toObject();
            QString section_name = section.value("name_section").toString();
            QJsonValue  jv_sectors = section.value("sectors");
            QJsonArray ja_sectors;
            QVBoxLayout *vbl = new QVBoxLayout;
            if(jv_sectors.isArray())
            {
                ja_sectors = jv_sectors.toArray();
            }
            for(int j=0; j<ja_sectors.count(); j++)
            {
                QJsonObject sector = ja_sectors.at(j).toObject();
                QString sector_name = sector.value("name_sector").toString();

                QLabel *lbl_sector = new QLabel(sector_name);
                QFont font = lbl_sector->font();
                font.setBold(true);
                font.setUnderline(true);
                lbl_sector->setFont(font);
                lbl_sector->setStyleSheet("color: rgb(39, 54, 69);text-decoration: none;font-size: 18px;font-weight:bold;");
                vbl->addWidget(lbl_sector);

                QJsonValue jv_links = sector.value("links");
                QJsonArray ja_links;
                if(jv_links.isArray())
                {
                    ja_links = jv_links.toArray();
                }
                for(int k=0; k<ja_links.count(); k++)
                {
                    QJsonObject link = ja_links.at(k).toObject();
                    QString link_name = link.value("name").toString();
                    QString url = link.value("link").toString();

                    QString text = "<a href='"+url+"' style='color: #0c3ff5;text-decoration: none;   width: 63px; height: 16px; font-family: Lato; font-size: 14px; font-weight: normal; font-style: normal; font-stretch: normal; line-height: normal; letter-spacing: normal; color: #0c3ff5;'>"+link_name+"</a>";
                    QLabel *lbl_link = new QLabel;
                    lbl_link->setText(text);
                    lbl_link->setOpenExternalLinks(true);
					
                    vbl->addSpacing(2);
                    vbl->addWidget(lbl_link);

                    continue;
                }
				vbl->addStretch();
            }
            frames[i]->setLayout(vbl);
        }
    }
}



void LinkwPage::loadOVBanner() {

	iframe_b = new WebFrame_b(this);
	iframe_b->setProperty("class", "iframe_b");
	iframe_b->setObjectName(QStringLiteral("WebFrame_b"));
	iframe_b->setMaximumWidth(950);
    iframe_b->setMaximumHeight(96);
	iframe_b->setCursor(Qt::PointingHandCursor);

	ui->verticalBanner_2->addWidget(iframe_b);

	QTimer *webtimer = new QTimer();
	webtimer->setInterval(30000);

	QObject::connect(webtimer, SIGNAL(timeout()), this, SLOT(timerTickSlot_b()));
	QObject::connect(iframe_b, SIGNAL(onClick()), this, SLOT(linkClickedSlot_v()));

	webtimer->start();

	emit timerTickSlot_b();

}


void LinkwPage::timerTickSlot_b()
{
	QEventLoop loop;
	QNetworkAccessManager manager;
	QDateTime currentDateTime = QDateTime::currentDateTime();
	uint unixtime = currentDateTime.toTime_t() / 30;
	timestmp_b = unixtime;

	QNetworkReply *reply = manager.get(QNetworkRequest(QUrl(QString("https://raw.githubusercontent.com/trendo-dev/trendo/master/banner.jpg").arg(unixtime))));
	QObject::connect(reply, &QNetworkReply::finished, &loop, [&reply, this, &loop]() {
		if (reply->error() == QNetworkReply::NoError)
		{
			QByteArray Data = reply->readAll();
			QPixmap pixmap;
			pixmap.loadFromData(Data);
			if (!pixmap.isNull())
			{
				this->iframe_b->clear();
				this->iframe_b->setPixmap(pixmap);
			}
		}
		loop.quit();
	});

	loop.exec();
}

void LinkwPage::linkClickedSlot_v()
{
	QDesktopServices::openUrl(QUrl(QString("https://coin.trendo.im/?utm_source=wallet").arg(timestmp_b)));
}




LinkwPage::~LinkwPage()
{
    delete ui;
}

void LinkwPage::setClientModel(ClientModel* model)
{
    this->clientModel = model;
}

void LinkwPage::setWalletModel(WalletModel* model)
{
    this->walletModel = model;
}



void WebFrame_b::mousePressEvent(QMouseEvent* event)
{
	emit onClick();
}



