#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cctype>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include "helper/ecc_helper.h"
#include "tinyxml/tinyxml.h"
#include "ecc_helper.h"
#include "FirstBlockConfig.h"

using namespace imtixml;

FirstBlockConfig* FirstBlockConfig::_instance = NULL;

FirstBlockConfig* FirstBlockConfig::getInstance()
{
    if (!_instance) {
        _instance = new FirstBlockConfig();
    }
    return _instance;
}

FirstBlockConfig::FirstBlockConfig()
{
}

FirstBlockConfig::~FirstBlockConfig()
{
}

bool FirstBlockConfig::Load(const char* fileName)
{
    FILE* fp = fopen(fileName, "r");
    if (!fp) {
        std::cout << "open file fail: " << fileName << std::endl;
        return false;
    }

    TiXmlDocument doc; 
    doc.LoadFile(fp);
    TiXmlHandle docH(&doc);
    TiXmlHandle root = docH.FirstChildElement("block");
    if (!root.Element()) {
        fclose(fp);
        return false;
    }

	node = root.FirstChildElement("id").Element();
    if (!node || !node->GetText()) {
        return false;
    }
    KeyFromBase58(node->GetText(),this->id);

	node = root.FirstChildElement("root_address").Element();
    if (!node || !node->GetText()) {
        return false;
    }
    Base58AddressToBin(node->GetText(),this->rootAddr);

	node = root.FirstChildElement("public_key").Element();
    if (!node || !node->GetText()) {
        return false;
    }
    KeyFromBase58(node->GetText(),this->publicKey);


	imtixml::TiXmlElement *node = root.FirstChildElement("timestamp").Element();
    if (node && node->GetText()) {
        this->timestamp = atoi(node->GetText()); 
    }

	node = root.FirstChildElement("sign").Element();
    if (!node || !node->GetText()) {
        return false;
    }
    SignFromBase58(node->GetText(),this->sign);

    
	node = root.FirstChildElement("pay_id").Element();
    if (!node || !node->GetText()) {
        return false;
    }
    KeyFromBase58(node->GetText(),this->payId);


	node = root.FirstChildElement("to_address").Element();
    if (!node || !node->GetText()) {
        return false;
    }
    Base58AddressToBin(node->GetText(),this->toAddr);
    
    imtixml::TiXmlElement *node = root.FirstChildElement("amount").Element();
    if (node && node->GetText()) {
        this->amount = atoi(node->GetText()); 
    }

	node = root.FirstChildElement("sign").Element();
    if (!node || !node->GetText()) {
        return false;
    }
    SignFromBase58(node->GetText(),this->sign);

	node = root.FirstChildElement("blockCache").Element();
	if (!node || !node->GetText()) {
		return false;
	}
	this->blockCache = node->GetText();

	return true;
}
