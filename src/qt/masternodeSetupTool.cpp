#include "masternodelist.h"
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
#include <QInputDialog>
#include "rpc/server.h"
#include <QString>
#include <QHostAddress> 
#include <QHostInfo>
#include <QtNetwork>

#include "optionsmodel.h"
#include "coincontroldialog.h"
#include "bitcoinunits.h"
#include "qobject.h"

CBitcoinAddress GetAccountAddress( string strAccount, bool bForceNew=false);

std::string MasternodeSetupTool::checkExternalIp()
{
    
    QString info;
    QEventLoop loop;

        QNetworkAccessManager networkManager;

        QUrl url("https://api.ipify.org");
        //the query used to add the parameter "format=json" to the request
        QUrlQuery query;
        query.addQueryItem("format", "json");
        //set the query on the url
        url.setQuery(query);

        //make a *get* request using the above url
        QNetworkReply* reply = networkManager.get(QNetworkRequest(url));

        QObject::connect(reply, &QNetworkReply::finished,
                        [&](){
            if(reply->error() != QNetworkReply::NoError) {
                //failure

                //Display message box
                //QMessageBox::information(this, tr("ERROR"),QString("error ..."),QMessageBox::Ok);
                m_qobj->showMessage(std::string("Error : can't get your external IP address.\nYou have to correct it manually in digitalcoin.conf and masternode.conf...%1"),info.toStdString());

                qDebug() << "error: " << reply->error();
                loop.quit();
            } else { //success

                //parse the json reply to extract the IP address
                QJsonObject jsonObject= QJsonDocument::fromJson(reply->readAll()).object();
                QHostAddress ip(jsonObject["ip"].toString());
                //do whatever you want with the ip
                info= jsonObject["ip"].toString();

    loop.quit();
            }
            //delete reply later to prevent memory leak
            reply->deleteLater();
            //a.quit();
        });
       

    //QObject::connect(&anObject, SIGNAL(signalToWait()), &loop, SLOT(quit()));
    // Timeout to avoid infite waiting
    QTimer timer;
    timer.setInterval(1*1000); //10s
    timer.setSingleShot(true);
    //QObject::connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
    
    // -- Do stuff that should trigger the signal
    // ...
    
    // -- Start to wait
    // What is done after "loop.exec()" is not executed until "loop.quit()" is called
    timer.start();
    loop.exec();
    timer.stop();

    // m_qobj->showMessage(std::string("So you want to setup your masternode on this IP : %1?"),info.toStdString());

    if(info=="")
    {
        bool ok;
        QString text = QInputDialog::getText(m_qobj, tr("What is your public IP address?"),
                                            tr("We need your external IP address (format : x.x.x.x),\nYou can go on showip.net to get it ;)"), QLineEdit::Normal,
                                            QString(), &ok);
        /*if (ok && !text.isEmpty())
            textLabel->setText(text);*/
            info = text;  
    }
    return info.toStdString();

}

void MasternodeSetupTool::makeTransaction(WalletModel * walletModel)
{

    CBitcoinAddress mnAddress = GetAccountAddress("Masternode",false);

    QString addr = QString::fromStdString(mnAddress.ToString());
    QString label = "Masternode Collateral Transaction";
    QString msg = "Masternode collateral transaction.";
    
    SendCoinsRecipient recipient(addr, label, 5*100000000, msg);
    recipient.inputType = ALL_COINS;
    recipient.fUseInstantSend = false;

    QList<SendCoinsRecipient> recipients;
    recipients.append(recipient);
    
    WalletModelTransaction currentTransaction(recipients);
    WalletModel::SendCoinsReturn prepareStatus;
    if (walletModel->getOptionsModel()->getCoinControlFeatures()) // coin control enabled
        prepareStatus = walletModel->prepareTransaction(currentTransaction, CoinControlDialog::coinControl);
    else
        prepareStatus = walletModel->prepareTransaction(currentTransaction);

    processSendCoinsReturn(walletModel, prepareStatus,
        BitcoinUnits::formatWithUnit(walletModel->getOptionsModel()->getDisplayUnit(), currentTransaction.getTransactionFee()));

    if(prepareStatus.status != WalletModel::OK)
    {
  
        return;
    }

    /*WalletModel::SendCoinsReturn sendStatus = */walletModel->sendCoins(currentTransaction);
 
    //m_qobj->showMessage("5dgc paid at : %1",rpc_result.write());
}

std::string MasternodeSetupTool::makeGenkey()
{

    //genkey
    CKey secret;
    secret.MakeNewKey(false);

    auto mnGenkey = CBitcoinSecret(secret).ToString();

    return mnGenkey;

}

std::vector<std::pair<string,string>> MasternodeSetupTool::checkMasternodeOutputs()
{
    std::vector<std::pair<string,string>> result;

    std::vector<COutput> vPossibleCoins;
    pwalletMain->AvailableCoins(vPossibleCoins, true, NULL, false, ONLY_1000);

    UniValue obj(UniValue::VOBJ);
    BOOST_FOREACH(COutput& out, vPossibleCoins) 
    {
        //m_qobj->showMessageTwoArgs("outputs : %1 = %2",out.tx->GetHash().ToString(),strprintf("%d", out.i));        
        result.push_back(std::pair<string,string>{out.tx->GetHash().ToString(),std::to_string(out.i)});
    }

    return result;
}


void MasternodeSetupTool::writeDigitalcoinConfFile(string _line)
{
    //
    //  add a line to a file
    //
    // reopen the log file, if requested
    
    FILE *  fileout=NULL;
    
    //string strTimestamped=string("ploup=1");

    boost::filesystem::path pathDebug = GetDataDir() / "digitalcoin.conf";

    fileout = fopen (pathDebug.string().c_str(),"aw");// use "a" for append, "w" to overwrite, previous content will be deleted
    string s =string("\n")+_line;

    fprintf(fileout,s.c_str());

    fclose (fileout); // must close after opening
}

void MasternodeSetupTool::writeMasternodeConfFile(string _alias, string _ipport,string mnprivkey,string _output,string _index)
{
    FILE *  fileout=NULL;

    boost::filesystem::path pathDebug2 = GetDataDir() / "masternode.conf";

    fileout = fopen (pathDebug2.string().c_str(),"w");// use "a" for append, "w" to overwrite, previous content will be deleted

    // # Format: alias IP:port masternodeprivkey collateral_output_txid collateral_output_index

    string s =string("\n")+_alias+string(" ")+_ipport+string(" ")+mnprivkey+string(" ")+_output+string(" ")+_index;
    fprintf(fileout,s.c_str());

    fclose (fileout); // must close after opening

}
std::string MasternodeSetupTool::getConfParam(std::string _arg)
{
    BOOST_FOREACH(auto ar, mapArgs) 
    {
    
        if(ar.first==_arg)
        {
            //m_qobj->showMessageTwoArgs("arg : %1 = %2",ar.first,ar.second);
            return ar.second;    
        }

    }
    return string("");
}

void MasternodeSetupTool::processSendCoinsReturn(WalletModel * walletModel, const WalletModel::SendCoinsReturn &sendCoinsReturn, const QString &msgArg)
{
    QPair<QString, CClientUIInterface::MessageBoxFlags> msgParams;
    // Default to a warning message, override if error message is needed
    msgParams.second = CClientUIInterface::MSG_WARNING;

    // This comment is specific to SendCoinsDialog usage of WalletModel::SendCoinsReturn.
    // WalletModel::TransactionCommitFailed is used only in WalletModel::sendCoins()
    // all others are used only in WalletModel::prepareTransaction()
    switch(sendCoinsReturn.status)
    {
    case WalletModel::InvalidAddress:
        msgParams.first = tr("The recipient address is not valid. Please recheck.");
        break;
    case WalletModel::InvalidAmount:
        msgParams.first = tr("The amount to pay must be larger than 0.");
        break;
    case WalletModel::AmountExceedsBalance:
        msgParams.first = tr("The amount exceeds your balance.");
        break;
    case WalletModel::AmountWithFeeExceedsBalance:
        msgParams.first = tr("The total exceeds your balance when the %1 transaction fee is included.").arg(msgArg);
        break;
    case WalletModel::DuplicateAddress:
        msgParams.first = tr("Duplicate address found: addresses should only be used once each.");
        break;
    case WalletModel::TransactionCreationFailed:
        msgParams.first = tr("Transaction creation failed!");
        msgParams.second = CClientUIInterface::MSG_ERROR;
        break;
    case WalletModel::TransactionCommitFailed:
        msgParams.first = tr("The transaction was rejected! This might happen if some of the coins in your wallet were already spent, such as if you used a copy of wallet.dat and coins were spent in the copy but not marked as spent here.");
        msgParams.second = CClientUIInterface::MSG_ERROR;
        break;
    case WalletModel::AbsurdFee:
        msgParams.first = tr("A fee higher than %1 is considered an absurdly high fee.").arg(BitcoinUnits::formatWithUnit(walletModel->getOptionsModel()->getDisplayUnit(), maxTxFee));
        break;
    case WalletModel::PaymentRequestExpired:
        msgParams.first = tr("Payment request expired.");
        msgParams.second = CClientUIInterface::MSG_ERROR;
        break;
    // included to prevent a compiler warning.
    case WalletModel::OK:
    default:
        return;
    }
        m_qobj->showMessage("Error with transaction : %1" , msgParams.first.toStdString());   
//    Q_EMIT message(tr("Send Coins"), msgParams.first, msgParams.second);
}