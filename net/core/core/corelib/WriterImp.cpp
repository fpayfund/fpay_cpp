#include "WriterImp.h"
#include "core/sox/tcpsock.h"
#include "core/sox/selsel.h"
#include "core/sox/logger.h"
using namespace core;
using namespace sox;


/*
void AbstractWriter::reset(){
	buf->clear();
	buf->setMethod(END);
}*/
AbstractWriter::~AbstractWriter(){

}

void AbstractWriter::stop(){
	send(&stopPacket);
}


void AbstractWriter::send(IPacketBase *ss){
	sendBuf.push_back(ss);
}

