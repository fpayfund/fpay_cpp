#include "AStatLinkHandler.h"

using namespace core;

AStatLinkHandler::AStatLinkHandler():totalProc(0), flowStat(0), intevelProc(0), limitSize(-1){
	
}

uint32_t AStatLinkHandler::getAndRstIntevel(){
	uint32_t ret = intevelProc;
	intevelProc = 0;
	return ret;
}

uint64_t AStatLinkHandler::getFlow()
{
	uint64_t data = flowStat;
	flowStat = 0;
	return data;
}

void AStatLinkHandler::incProc(){
	intevelProc++;
	totalProc++;
}

void AStatLinkHandler::incProc(uint32_t PktSize)
{
	intevelProc++;
	totalProc++;
	uint64_t utmp = PktSize;
	flowStat += utmp;
}

uint64_t AStatLinkHandler::getTotal(){
	return totalProc;
}

void AStatLinkHandler::setPackLimit(uint32_t sz){
	limitSize = sz;
}