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

		fprintf(stderr,"root element failed\n");
		return false;
    }

	imtixml::TiXmlElement *node = root.FirstChildElement("id").Element();
    if (!node || !node->GetText()) {
		fprintf(stderr,"id element failed\n");
        return false;
    }
    KeyFromBase58(node->GetText(),this->id.u8);

	node = root.FirstChildElement("root_address").Element();
    if (!node || !node->GetText()) {
		fprintf(stderr,"root_address element failed\n");
        return false;
    }
    Base58AddressToBin(node->GetText(),this->rootAddr.u8);

	node = root.FirstChildElement("public_key").Element();
    if (!node || !node->GetText()) {

		fprintf(stderr,"public_key element failed\n");
		return false;
    }
    KeyFromBase58(node->GetText(),this->publicKey.u8);


    node = root.FirstChildElement("timestamp").Element();
    if (!node || !node->GetText()) {
		fprintf(stderr,"timestamp element failed\n");
		return false;
	}
    this->timestamp = atoi(node->GetText()); 
    

	node = root.FirstChildElement("sign").Element();
    if (!node || !node->GetText()) {
		fprintf(stderr,"sign element faield\n");
		//return false;
    } else {
        SignFromBase58(node->GetText(),this->sign.u8);
    }
    
	node = root.FirstChildElement("pay_id").Element();
    if (!node || !node->GetText()) {
		fprintf(stderr,"pay_id element failed\n");
        return false;
    }
    KeyFromBase58(node->GetText(),this->payId.u8);


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
    

	node = root.FirstChildElement("pay_sign").Element();
    if (!node || !node->GetText()) {
		fprintf(stderr,"pay_sign element failed\n");
        //return false;
		DumpHex(this->sign.u8,64);
    } else {
        SignFromBase58(node->GetText(),this->sign.u8);
    }
	node = root.FirstChildElement("block_cache").Element();
	if (!node || !node->GetText()) {
		fprintf(stderr,"block_cache element failed\n");
		return false;
	}
	this->blockCache = node->GetText();

	return true;
}
