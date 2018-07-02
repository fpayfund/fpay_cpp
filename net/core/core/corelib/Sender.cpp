#include "common/core/sender.h"
#include "common/core/ibase.h"
#include "common/res_code.h"
#include "core/sox/logger.h"
#include "common/core/request.h"

using namespace core;

Sender::Sender():pb(), hpk(pb), pk(pb, HEADER_SIZE){

}

Sender::Sender(const Sender &s): pb(),  hpk(pb), pk(pb, HEADER_SIZE){
	length = s.length;
	uri = s.uri;
	resCode = s.resCode;
	pk.push(s.pk.data(), s.pk.size());
}

Sender::Sender(URI_TYPE u, const sox::Marshallable &m) :
pb(), hpk(pb), pk(pb, HEADER_SIZE){
	marshall(u, m);
}

/*
Request::ResponseType Request::forward(URI_TYPE uri, 	sox::Marshallable *obj, core::IContext *ctx) {
	this->uri = uri;
	this->ctx = ctx;
	resCode = RES_SUCCESS;
	
	//log(Debug, "forward request: ", uri);

	basePack();
	
	obj->marshal(pk);

	return FORWARD;
}

Request::ResponseType Request::directForward(core::IContext *ctx) {
	this->ctx = ctx;
	//log(Debug, "direct forward request: ", uri);
	basePack();
	pk.push(up.data(), up.size());
	return FORWARD;
}

Request::ResponseType Request::directAnswer(){
	//log(Debug, "direct answer request: ", uri);
	basePack();
	pk.push(up.data(), up.size());
	return ANSWER;
}

Request::ResponseType Request::answer(URI_TYPE uri, uint16_t res,
		sox::Marshallable *obj) {
	this->uri = uri;
	resCode = res;
	basePack();
	if(obj)
		obj->marshal(pk);

	return ANSWER;
}*/

void Sender::endPack() {
	hpk.replace_uint32(0, length);

	hpk.replace_uint32(4, uri);
	hpk.replace_uint16(8, resCode);

	hpk.replace_uint32(0, (uint32_t)(headerSize() + bodySize()));
}

const char *Sender::body() {
	return pk.data();
}

size_t Sender::bodySize() {
	return pk.size();
}

const char *Sender::header(){
	return hpk.data();
}

size_t Sender::headerSize(){
	return HEADER_SIZE;
}

void Sender::clear(){
	// be care paket offset
	pb.resize(0 + HEADER_SIZE);
	resCode = RES_SUCCESS;
}

void Sender::marshall(const sox::Marshallable &m){
	pk << m;
}

void Sender::marshall(URI_TYPE u, const sox::Marshallable &m){
	setUri(u);
	m.marshal(pk);
}

void Sender::marshall(const char *data, size_t sz){
	pk.push(data, sz);
}

Sender & Sender::operator= (const Sender& sender){
	this->length = sender.length;
	this->uri = sender.uri;
	this->resCode = sender.resCode;
	pb.resize(0);
	pk.push(sender.pk.data(), sender.pk.size());
	return *this;
}


