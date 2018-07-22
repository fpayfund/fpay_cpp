#include "common/core/request.h"
#include "common/core/ibase.h"
#include "common/res_code.h"
#include "core/sox/logger.h"
static std::string zero_place("");

using namespace core;
/*Request::Request(const Request &r) :
	up(NULL, 0){
	length = r.length;
	uri = r.uri;
	resCode = r.resCode;
	sid = r.sid;
	tag = r.tag;
	key = r.key;
	handler = r.handler;
	cmd = r.cmd;
}*/
Request::Request(const char *data, uint32_t sz) :
	up(data, sz), cpBuffer(NULL), od(data), os(sz),  handler(NULL), cmd(NULL){
}


Request::Request(const char *data, uint32_t sz, bool copy) :
up(data, sz), cpBuffer(NULL), od(data), os(sz), handler(NULL), cmd(NULL){
    if (copy)
    {
        cpBuffer = new char[sz];
        memcpy(cpBuffer, data, sz);
        up.reset(cpBuffer, sz);
        od = cpBuffer;
    }
}


Request::~Request() {
    if (handler && cmd) {
        handler->destroyForm(cmd);
    }
    if (cpBuffer != NULL)
        delete[] cpBuffer;
}

void Request::head(){
	length = up.pop_uint32();

	uri = up.pop_uint32();
	resCode = up.pop_uint16();

}

bool Request::setFormHandler(IFormHandle *h) {
	if (h) {
		if(getResCode() == RES_SUCCESS){
			handler = h;
			cmd = h->handlePacket(up);
		}
		return true;
	}else{
		return false;
	}
}

void Request::forceHandler(IFormHandle *h){
	if (h) {
		handler = h;
		cmd = h->handlePacket(up);
	}else{
		handler = NULL;
	}
}

void Request::leftPack(std::string &out){
	out.assign(up.data(), up.size());
}


/*
void Request::basePack() {
	pk.push_uint32(length);

	pk.push_uint32(uri);
	pk.push_uint16(sid);
	pk.push_uint16(resCode);

	pk.push_uint8(tag);
}

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
}

void Request::endPack(uint16_t sid) {
	if(!key.empty()){
		pk << key ;
	}
	
	pk.replace_uint32(0, pb.size());
	pk.replace_uint16(8, sid);
}

void Request::endPack(uint16_t sid, const std::string &uid) {
	pk << uid;
	pk.replace_uint32(0, pb.size());
	pk.replace_uint16(8, sid);
}

const char *Request::data() {
	return pk.data();
}

uint32_t Request::size() {
	return pk.size();
}
*/
uint32_t Request::peeklen(const void * d) {
	uint32_t i = *((uint32_t*)d);
	return XHTONL(i);
}

std::string Request::getKey(){
	return key;
}

void Request::setKey(const std::string &k){
	key = k;
}


bool Request::ifSuccess(uint32_t res)
{
	return res == RES_SUCCESS;
}


void Request::packOrgin(sox::Pack &pk) const{
	pk.push(od, os);
}

//wuji start
//想获取原始的包体（不包括头）
// void Request::GetBody(std::string &out)
// {
// 	char *pszTemp=od;
// 	pszTemp+=HEADER_SIZE;
// 	out.assign(pszTemp,os-HEADER_SIZE);
// }
////想获取原始的包体（包括头）
// void Request::GetHeadAndBody(std::string &out)
// {
// 	out.assign(od,os);
// }
//wuji end