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

#include "PayConfig.h"

using namespace imtixml;

PayConfig* PayConfig::_instance = NULL;

PayConfig* PayConfig::getInstance()
{
    if (!_instance) {
        _instance = new PayConfig();
    }
    return _instance;
}

PayConfig::PayConfig()
{
}

PayConfig::~PayConfig()
{
}

bool PayConfig::Load(const char* fileName)
{
    FILE* fp = fopen(fileName, "r");
    if (!fp) {
        std::cout << "open file fail: " << fileName << std::endl;
        return false;
    }

    TiXmlDocument doc; 
    doc.LoadFile(fp);
    TiXmlHandle docH(&doc);
    TiXmlHandle root = docH.FirstChildElement("pay");
    if (!root.Element()) {
        fclose(fp);

		fprintf(stderr,"root element failed\n");
		return false;
    }

	node = root.FirstChildElement("from_address").Element();
    if (!node || !node->GetText()) {
		fprintf(stderr,"from_address element failed\n");
        return false;
    }
    Base58AddressToBin(node->GetText(),this->fromAddr.u8);

	node = root.FirstChildElement("public_key").Element();
    if (!node || !node->GetText()) {

		fprintf(stderr,"public_key element failed\n");
		return false;
    }
    KeyFromBase58(node->GetText(),this->publicKey.u8);

	node = root.FirstChildElement("private_key").Element();
    if (!node || !node->GetText()) {

		fprintf(stderr,"private_key element failed\n");
		return false;
    }
    KeyFromBase58(node->GetText(),this->privateKey.u8);

	node = root.FirstChildElement("to_address").Element();
    if (!node || !node->GetText()) {
		fprintf(stderr,"to_address element failed\n");
        return false;
    }
    Base58AddressToBin(node->GetText(),this->toAddr.u8);

	node = root.FirstChildElement("amount").Element();
	if (!node && !node->GetText()) {
		fprintf(stderr,"amount element failed\n");
		return false;
	}
	this->amount = atoi(node->GetText()); 


	return true;
}
