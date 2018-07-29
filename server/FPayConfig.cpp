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

#include "tinyxml/tinyxml.h"
#include "FPayConfig.h"

using namespace imtixml;

FPayConfig* FPayConfig::_instance = NULL;

FPayConfig* FPayConfig::getInstance()
{
    if (!_instance) {
        _instance = new FPayConfig();
    }
    return _instance;
}

FPayConfig::FPayConfig()
     : blockInterval(500)
{
}

FPayConfig::~FPayConfig()
{
}

bool FPayConfig::Load(const char* fileName)
{
    FILE* fp = fopen(fileName, "r");
    if (!fp) {
        std::cout << "open file fail: " << fileName << std::endl;
        return false;
    }

    TiXmlDocument doc; 
    doc.LoadFile(fp);
    TiXmlHandle docH(&doc);
    TiXmlHandle root = docH.FirstChildElement("server");
    if (!root.Element()) {
        fclose(fp);
        return false;
    }

    bool ret = false;

    imtixml::TiXmlElement *node = root.FirstChildElement("blockInterval").Element();
    if (node && node->GetText()) {
        this->blockInterval = atoi(node->GetText()); 
    }

    node = root.FirstChildElement("blockCache").Element();
    if (!node || !node->GetText()) {
        return false;
    }
    this->blockCache = node->GetText();

    node = root.FirstChildElement("balanceCache").Element();
    if (!node || !node->GetText()) {
        return false;
    }
    this->balanceCache = node->GetText();

    node = root.FirstChildElement("paymentCache").Element();
    if (!node || !node->GetText()) {
        return false;
    }
    this->paymentCache = node->GetText();

    node = root.FirstChildElement("initBlockId").Element();
    if (!node || !node->GetText()) {
        return false;
    }
    std::string initBlockIdStr = node->GetText();
    for (uint32_t i = 0; i < initBlockIdStr.size() && i < sizeof(initBlockId.u8); i++) {
        initBlockId.u8[i] = initBlockIdStr[i];
    }

    node = root.FirstChildElement("lastBlockCacheId").Element();
    if (!node || !node->GetText()) {
        return false;
    }
    std::string lastBlockIdStr = node->GetText();
    for (uint32_t i = 0; i < lastBlockIdStr.size() && i < sizeof(lastBlockCacheId.u8); i++) {
        lastBlockCacheId.u8[i] = lastBlockIdStr[i];
    }

    node = root.FirstChildElement("txPoolCacheKey").Element();
    if (!node || !node->GetText()) {
        return false;
    }
    this->txPoolCacheKey = node->GetText();

    return true;
}
