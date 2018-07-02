#include "MfcAppContext.h"
#include "common/core/form.h"
#include "core/sox/logger.h"
#include "core/sox/measure.h"
//#include "common/protocol/const.h"
#include "common/core/ilink.h"

using namespace sox;
using namespace core;
#define INVALID_UID -1

IWriter *MfcAppcontext::requestDispatch(Request &request, IConn *conn)
{
	entry_map_t::iterator it = entries.find(request.getUri());//找合适的消息映射入口


#ifndef WIN32
	uint32_t uri = request.getUri();
	uint32_t ip = 0;
	uint32_t serverId = 0;
	uint32_t npos = request.key.find_first_of("/");
	if(npos != std::string::npos){
		serverId = atoi(request.key.substr(npos+1).c_str());
	}
	if(serverId == 0 && conn != NULL){
		ip = conn->getPeerIp();
	}
	scoped_measure measure(uri, ip, serverId);
#endif

	if(it != entries.end())//找到对应的消息映射入口
	{
		FormEntry *entry = it->second;
		//注意次序
		if(request.isSuccess()){
			try{
				//log(Info, "uri: %u | %u", (request.getUri() & 0xff00) >> 8, request.getUri() & 0xff);
				request.setFormHandler(entry->requestForm.get());
			}catch (UnpackError&) {
				log(Warn, "Unpack error, uri: 0x%02x", request.getUri());
				return getWriter();
			}
		}

		//target->setWriter(getWriter());
		try
		{
			dispatchMessage(entry, request, conn);
		}
		catch(UnpackError&)
		{
			log(Info, "Unpack error, uri: %u - %u", request.getUri() >> 8,
						request.getUri() & 0xff);
			return getWriter();
		}

	}else
	{
		DefaultDispatch(request, conn);
	}

	return getWriter();
}


void MfcAppcontext::dispatchMessage(FormEntry* entry, Request &request, IConn *conn)
{
	PHClass *target  = (PHClass *)fetchPtr(entry, request, conn);
	if(target == NULL)
	{
		log(Info, "fetch null ptr uri: %u - %u", request.getUri() >> 8,
					request.getUri() & 0xff);
		return;
	}
	TargetProc prc;
	prc.mf_oo = entry->proc;
	void *cmd = request.getCmd();
	switch(entry->type)
	{
		case fpt_vv:
			(target->*prc.mf_vv)();
			break;
		case fpt_vr:
			(target->*prc.mf_vr)(request.getResCode());
			break;
		case fpt_vc:
			if(cmd){
				(target->*prc.mf_vc)(cmd);
			}else{
				log(Warn, "fpt_vc uri:%u is resCode:%u", request.getUri(), request.getResCode());
			}
			break;
		case fpt_vcr:           // void ::(Cmd*, rescode);
			(target->*prc.mf_vcr)(cmd, request.getResCode());
			break;
		case fpt_vcc:            // void ::(Cmd *, Conn* );
			if(cmd){
				(target->*prc.mf_vcc)(cmd, conn);
			}else{
				log(Warn, "fpt_vcc uri:%u is resCode:%u", request.getUri(), request.getResCode());
			}
			break;
		case fpt_vccr:
			(target->*prc.mf_vccr)(cmd, conn, request.getResCode());
			break;
		case fpt_vccra:
			(target->*prc.mf_vccra)(cmd, conn, request.getResCode(), request);
			break;
		case fpt_vk:
			(target->*prc.mf_vk)(request.getKey());
			break;
		case fpt_vkc:
			if(cmd){
				(target->*prc.mf_vkc)(request.getKey(), cmd);
			}else{
				log(Warn, "fpt_vkc uri:%u is resCode:%u", request.getUri(), request.getResCode());
			}
			break;
		case fpt_vkcr:
			(target->*prc.mf_vkcr)(request.getKey(), cmd, request.getResCode());
			break;
		default:
			otherSwitch(entry->type, target, prc, request, conn);
			break;
	}
}

void MfcAppcontext::otherSwitch(int, PHClass *, TargetProc, Request &req, IConn *){

}

void MfcAppcontext::clear(){
	entries.clear();
}

void * MfcAppcontext::fetchPtr(FormEntry *entry, Request &request, IConn *conn){

	return entry->target;
}

void MfcAppcontext::addEntry(FormEntry *es, void *target, IWriterAware *inf){
	if(inf){
		inf->setWriter(getWriter());
	}
	int i = 0;
	//target maybe null
	while(es[i].uri != 0){
		if(entries.find(es[i].uri) != entries.end()){
			log(Error, "dupicate form entry");
			assert(false);
		}
		entries[es[i].uri] = &es[i];
		es[i].target = (PHClass *)target;
		++i;
	}

}

void MfcAppcontext::DefaultDispatch(Request &request, IConn *conn){
	int i = request.getUri();

	log(Error, "not find request entry: %s", uri2str(i));
}

