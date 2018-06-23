
#ifndef MASTERNODE_UTIL_H
#define MASTERNODE_UTIL_H

#include "compat.h"
#include "tinyformat.h"
#include "utiltime.h"
#include "amount.h"

#include <exception>
#include <map>
#include <stdint.h>
#include <string>
#include <vector>

#include <boost/filesystem/path.hpp>
#include <boost/signals2/signal.hpp>
#include <boost/thread/exceptions.hpp>

#include "base58.h"
#include "init.h"
#include "netbase.h"
#include "validation.h"
#include "privatesend-server.h"
#include "util.h"
#include "utilmoneystr.h"

#include <fstream>
#include <iomanip>
#include <univalue.h>


std::string makeGenkey()
{
    //genkey
    CKey secret;
    secret.MakeNewKey(false);

    auto mnGenkey = CBitcoinSecret(secret).ToString();

    return mnGenkey;
}

void writeDigitalcoinConfFile(string _line)
{
    FILE *  fileout=NULL;
    
    boost::filesystem::path pathDebug = GetDataDir() / "digitalcoin.conf";

    fileout = fopen (pathDebug.string().c_str(),"aw");
    string s =string("\n")+_line;

    fprintf(fileout,s.c_str());

    fclose (fileout);
}

std::string getConfParam(std::string _arg)
{
    BOOST_FOREACH(auto ar, mapArgs) 
    {
        if(ar.first==_arg)
            return ar.second;    
    }
    return string("");
}

CBitcoinAddress GetAccountAddress( string strAccount, bool bForceNew=false);

void writeMasternodeConfFile(string _alias, string _ipport,string mnprivkey,string _output,string _index)
{
    FILE *  fileout=NULL;
    boost::filesystem::path pathDebug2 = GetDataDir() / "masternode.conf";

    fileout = fopen (pathDebug2.string().c_str(),"w");// use "a" for append, "w" to overwrite, previous content will be deleted

    string s =string("\n")+_alias+string(" ")+_ipport+string(" ")+mnprivkey+string(" ")+_output+string(" ")+_index;

    fprintf(fileout,s.c_str());
    fclose (fileout); // must close after opening
}

void writeDigitalcoinMasternodeConfInfo(string mnGenkey, string strIpPort)
{
    writeDigitalcoinConfFile("masternode=1");
    writeDigitalcoinConfFile("masternodeprivkey="+mnGenkey);
    writeDigitalcoinConfFile("externalip="+strIpPort);
    writeDigitalcoinConfFile("masternodeaddr="+strIpPort);
}

std::vector<std::pair<string,string>> checkMasternodeOutputs()
{
    std::vector<std::pair<string,string>> result;

    std::vector<COutput> vPossibleCoins;
    pwalletMain->AvailableCoins(vPossibleCoins, true, NULL, false, ONLY_1000);

    UniValue obj(UniValue::VOBJ);
    BOOST_FOREACH(COutput& out, vPossibleCoins) 
    {
        result.push_back(std::pair<string,string>{out.tx->GetHash().ToString(),std::to_string(out.i)});
    }

    return result;
}

void cleanDigitalcoinConf()
{
    boost::filesystem::path pathDebug = GetDataDir() / "digitalcoin.conf";

    std::vector<std::string> lines;

    std::string item_name;
    std::ifstream nameFileout;

    nameFileout.open(pathDebug.string().c_str());

    while (nameFileout >> item_name)
    {
        if(item_name.find("masternode") == std::string::npos
        && item_name.find("externalip") == std::string::npos)
        {
           lines.push_back(item_name);
        }
    }

    nameFileout.close();
    
    remove(pathDebug.string().c_str());
    
    for(std::string& key : lines)
        writeDigitalcoinConfFile(key);
}

void RemoveMasternodeConfigs()
{
    cleanDigitalcoinConf();
    boost::filesystem::path masternodeConfPath = GetDataDir() / "masternode.conf";

    if (boost::filesystem::exists(masternodeConfPath.string().c_str()))        
        remove(masternodeConfPath.string().c_str());
}


#endif // MASTERNODE_UTIL_H
