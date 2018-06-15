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
#include "rpc/server.h"
#include <QString>
#include <QHostAddress> 
#include <QHostInfo>
#include <QtNetwork>


#include "qobject.h"

CBitcoinAddress GetAccountAddress( string strAccount, bool bForceNew=false);



QString MasternodeSetupTool::checkExternalIp()
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

    // Display message box
    //QMessageBox::information(this, tr("ERROR"),QString("error ..."),QMessageBox::Ok);

                qDebug() << "error: " << reply->error();
                loop.quit();
            } else { //success

    //QMessageBox::information(this, tr("Looks good!"),tr("waiting..."),QMessageBox::Ok);

                //parse the json reply to extract the IP address
                QJsonObject jsonObject= QJsonDocument::fromJson(reply->readAll()).object();
                QHostAddress ip(jsonObject["ip"].toString());
                //do whatever you want with the ip
                info= jsonObject["ip"].toString();

    loop.quit();
            }
            //delete reply later to prevent memory leak
            reply->deleteLater();
    //        a.quit();
        });
    //QMessageBox::information(this, tr("..."),tr("reequesting"),QMessageBox::Ok);
        

    //QObject::connect(&anObject, SIGNAL(signalToWait()), &loop, SLOT(quit()));
    // Timeout to avoid infite waiting
    QTimer timer;
    timer.setInterval(20*1000); //10s
    timer.setSingleShot(true);
    //QObject::connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
    
    // -- Do stuff that should trigger the signal
    // ...
    
    // -- Start to wait
    // What is done after "loop.exec()" is not executed until "loop.quit()" is called
    timer.start();
    loop.exec();
    timer.stop();

    m_qobj->showMessage(std::string("So you want to setup your masternode on this IP : %1?"),info.toStdString());

    return info;

}


void MasternodeSetupTool::checkMasternodeConf()
{
    
}


void MasternodeSetupTool::makeTransaction()
{
    UniValue addressInfo(UniValue::VARR);

    CBitcoinAddress mnAddress = GetAccountAddress("0",false);



    //if (pwalletMain->mapAddressBook.find(CBitcoinAddress(address).Get()) != pwalletMain->mapAddressBook.end())
    //    addressInfo.push_back(pwalletMain->mapAddressBook.find(CBitcoinAddress(address).Get())->second.name);


    /*//                    sendtoaddress
        "\nArguments:\n"
            "1. \"digitalcoinaddress\" (string, required) The digitalcoin address to send to.\n"
            "2. \"amount\"      (numeric or string, required) The amount in " + CURRENCY_UNIT + " to send. eg 0.1\n"
            "3. \"comment\"     (string, optional) A comment used to store what the transaction is for. \n"
            "                             This is not part of the transaction, just kept in your wallet.\n"
            "4. \"comment-to\"  (string, optional) A comment to store the name of the person or organization \n"
            "                             to which you're sending the transaction. This is not part of the \n"
            "                             transaction, just kept in your wallet.\n"
            "5. subtractfeefromamount  (boolean, optional, default=false) The fee will be deducted from the amount being sent.\n"
            "                             The recipient will receive less amount of Digitalcoin than you enter in the amount field.\n"
            "6. \"use_is\"      (bool, optional) Send this transaction as InstantSend (default: false)\n"
            "7. \"use_ps\"      (bool, optional) Use anonymized funds only (default: false)\n"
    */        

    addressInfo.push_back(string("\"\""));
    addressInfo.push_back("\""+mnAddress.ToString()+"\"");
    addressInfo.push_back(string("5"));



    m_qobj->showMessage("destination address : %1",addressInfo[1].get_str());
    m_qobj->showMessage("amount : %1",addressInfo[1].get_str());
    m_qobj->showMessage("amount : %1",addressInfo[2].get_str());

    UniValue res = sendfrom(addressInfo, true);

    m_qobj->showMessage("5dgc paid at : %1",res[0].get_str());
}

std::string MasternodeSetupTool::makeGenkey()
{

    //genkey
    CKey secret;
    secret.MakeNewKey(false);

    auto mnGenkey = CBitcoinSecret(secret).ToString();

    return mnGenkey;

}

void MasternodeSetupTool::checkMasternodePrivKey()
{


}


void MasternodeSetupTool::checkIsMasternode()
{


}
void MasternodeSetupTool::checkMasternodeAddr()
{

}
void MasternodeSetupTool::checkMasternodeOutputs()
{
    std::vector<COutput> vPossibleCoins;
    pwalletMain->AvailableCoins(vPossibleCoins, true, NULL, false, ONLY_1000);

    UniValue obj(UniValue::VOBJ);
    BOOST_FOREACH(COutput& out, vPossibleCoins) 
    {

       // QString gggoom = QString::fromStdString();
     //   QString gggoom2 = QString::fromStdString();

            m_qobj->showMessageTwoArgs("outputs : %1 = %2",out.tx->GetHash().ToString(),strprintf("%d", out.i));

       /* QString textpkkm = QString("outputs : %1, %2").arg(gggoom).arg(gggoom2);
        QMessageBox::information(this, QString::fromStdString("paid!"),textpkkm,QMessageBox::Ok);

        obj.push_back(Pair(out.tx->GetHash().ToString(), strprintf("%d", out.i)));*/
    }


}


void MasternodeSetupTool::writeConfFiles()
{
    //Arg
    UniValue addressInfo(UniValue::VARR);


    //
    //  add a line to a file
    //
    // reopen the log file, if requested
    
    FILE *  fileout=NULL;
    int ret;
    string strTimestamped=string("ploup=1");

    boost::filesystem::path pathDebug = GetDataDir() / "digitalcoin2.conf";

    fileout = fopen (pathDebug.string().c_str(),"aw");// use "a" for append, "w" to overwrite, previous content will be deleted

    fprintf(fileout,"\nmasternode=1");
    fprintf(fileout,"\nmasternodeprivkey=");
    fprintf(fileout,"\nmasternodeaddr=90.113.132.14:7999");
    fprintf(fileout,"\nexternalip=90.113.132.14:7999");

    fclose (fileout); // must close after opening

    boost::filesystem::path pathDebug2 = GetDataDir() / "masternode2.conf";

    fileout = fopen (pathDebug2.string().c_str(),"aw");// use "a" for append, "w" to overwrite, previous content will be deleted

    // # Format: alias IP:port masternodeprivkey collateral_output_txid collateral_output_index

    /* QString gggf3 = QString::fromStdString(addressInfo[1].get_str());

    QString textpk6 = QString("\nmasternodeprivkey=%1").arg(gggf3);
    QString textpk6 = QString("\nmasternodeaddr=%1").arg(info);
    QString textpk6 = QString("\nexternalip=%1").arg(info);
    QString textpk6 = QString("\nmasternode=1");

    QString textpk6 = QString("\nmyMasternode %1:%2 %3 %4").arg(info).arg(info).arg(gggf3).arg(obj[0].first).arg(obj[1].second);
    */

    fprintf(fileout,"\nmasternode=1");

    fclose (fileout); // must close after opening

}

std::string MasternodeSetupTool::getConfParam(std::string _arg)
{
    BOOST_FOREACH(auto ar, mapArgs) 
    {
    
        if(ar.first==_arg)
        {
            m_qobj->showMessageTwoArgs("arg : %1 = %2",ar.first,ar.second);
            return ar.second;    
        }

    }
}
