#include "stdafx.h"
#include "common/core/ibase.h"
#include "core/sox/logger.h"
#include "AbstractConn.h"

using namespace core;

AbstractConn::AbstractConn() :serial(0), tmo(-1) {
	
}

AbstractConn::~AbstractConn() {

}

uint16_t AbstractConn::nextSerial() {
	++serial;
	if (serial == 0)
		serial = 1;
	return serial;
}

/*
void AbstractConn::packHelper(IResponse *resp, sox::PackBuffer &buffer,
		LinkPacker &packet) {
	LFrame lf;
	resp->toLFrame(lf);
	if (lf.sid == 0)
		lf.sid = nextSerial();
	if (resp->getCtx() != NULL) {
		mContext[lf.sid] = resp->getCtx();
	}

	if (lf.key.empty()) {
		lf.key = uid;
	}

	log(Debug, "packet sid is: ", lf.sid);
	packet << lf;
	//	size_t offset = buffer.size();

	if (resp->getObject())
		resp->getObject()->marshal(packet);

	packet.endpack();
}

void AbstractConn::packResponse(IResponse *resp, std::string &out) {
	sox::PackBuffer buffer;
	LinkPacker packet(buffer);
	packHelper(resp, buffer, packet);
	out.assign(packet.data(), packet.size());
}*/

void AbstractConn::send(Sender &resp) {
	resp.endPack();
//	log(Debug, "send package:", sid);
	sendBin(resp.header(), resp.headerSize() + resp.bodySize(), resp.getUri());
}

void AbstractConn::notifyErr(int err, const char *msg){
	log(Info, "Conn err: %d from %s", err, msg);
	if(eHandler){
		eHandler->onError(err, msg, this);
	}else{
		//delete this;
	}
}

void AbstractConn::setTimeout(int tm){
	tmo = tm;
}

void *AbstractConn::getData(){
	return NULL;
}
