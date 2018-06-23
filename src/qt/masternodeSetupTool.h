#ifndef MASTERNODESETUPTOOL_H
#define MASTERNODESETUPTOOL_H


#include "primitives/transaction.h"
#include "platformstyle.h"
#include "sync.h"
#include "util.h"

#include <QMenu>
#include <QTimer>
#include <QWidget>



#include "ui_masternodelist.h"

#include "activemasternode.h"
#include "clientmodel.h"
#include "init.h"
#include "guiutil.h"
#include "masternode-sync.h"
#include "masternodeconfig.h"
#include "masternodeman.h"
#include "sync.h"
#include "wallet/wallet.h"
#include "walletmodel.h"

#include <QTimer>
#include <QMessageBox>
#include "rpc/server.h"
#include <QString>
#include <QHostAddress> 
#include <QHostInfo>
#include <QtNetwork>
//#include "masternodelist.h"
namespace Ui {
    class MasternodeList;
}

class ClientModel;
class WalletModel;

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE
class MasternodeList;
/** Masternode Manager page widget */
class MasternodeSetupTool : QObject
{

public:
    MasternodeSetupTool()=default;
    ~MasternodeSetupTool()=default;
    
    //Bloc masternode tool
    void processSendCoinsReturn(WalletModel * walletModel, const WalletModel::SendCoinsReturn &sendCoinsReturn, const QString &msgArg = QString());
    void makeTransaction(WalletModel * walletModel);
    std::string checkExternalIp();

    MasternodeList *  m_qobj=nullptr;

};
#endif // MASTERNODELIST_H
