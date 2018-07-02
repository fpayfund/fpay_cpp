#ifndef YY_FORM_DEF
#define YY_FORM_DEF
#include "common/core/ibase.h"
#include "common/helper/formhandlehelper.h"
#include <vector>

#define ON_VOID(hClass, fp)  {hClass::uri, WrapFormHandle(new misc::VoidFormHandle), fpt_vv, \
	(TargetFunc)(static_cast<void (PHClass::*)()>(fp)), NULL},

#define ON_RESULT(hClass, fp)  {hClass::uri, WrapFormHandle(new misc::VoidFormHandle), fpt_vr, \
	(TargetFunc)(static_cast<void (PHClass::*)(uint32_t)>(fp)), NULL},


#define ON_REQUEST(hClass, fp)  {hClass::uri, WrapFormHandle(new misc::FormHandleT<hClass>()), fpt_vc, \
	(TargetFunc)(static_cast<void (PHClass::*)(hClass *)>(fp)), NULL},

#define ON_RESPONSE(hClass, fp) {hClass::uri, WrapFormHandle(new misc::FormHandleT<hClass>()), fpt_vcr, \
	(TargetFunc)(static_cast<void (PHClass::*)(hClass *, uint32_t)>(fp)), NULL},

#define ON_LINK(hClass, fp) {hClass::uri, WrapFormHandle(new misc::FormHandleT<hClass>()), fpt_vcc, \
	(TargetFunc)(static_cast<void (PHClass::*)(hClass *, IConn *)>(fp)), NULL},

#define ON_LINKRESPONSE(hClass, fp) {hClass::uri, WrapFormHandle(new misc::FormHandleT<hClass>()), fpt_vccr, \
	(TargetFunc)(static_cast<void (PHClass::*)(hClass *, IConn *, uint32_t)>(fp)), NULL},

#define ON_LINKRESPONSEALL(hClass, fp) {hClass::uri, WrapFormHandle(new misc::FormHandleT<hClass>()), fpt_vccra, \
	(TargetFunc)(static_cast<void (PHClass::*)(hClass *, IConn *, uint32_t, Request &)>(fp)), NULL},

#define ON_KEYREQUEST(hClass, fp) {hClass::uri, WrapFormHandle(new misc::FormHandleT<hClass>()), fpt_vkc, \
	(TargetFunc)(static_cast<void (PHClass::*)(const std::string &key, hClass *)>(fp)), NULL},

#define ON_KEYRESPONSE(hClass, fp) {hClass::uri, WrapFormHandle(new misc::FormHandleT<hClass>()), fpt_vkcr, \
	(TargetFunc)(static_cast<void (PHClass::*)(const std::string &key, hClass *, uint32_t resCode)>(fp)), NULL},

#define ON_KEY(hClass, fp) {hClass::uri, WrapFormHandle(new misc::FormHandleT<hClass>()), fpt_vk, \
	(TargetFunc)(static_cast<void (PHClass::*)(const std::string &)>(fp)), NULL},

#define ON_UREQUEST(hClass, fp) {hClass::uri, WrapFormHandle(new misc::FormHandleT<hClass>()), fpt_vuc, \
	(TargetFunc)(static_cast<void (PHClass::*)(uint32_t, hClass *)>(fp)), NULL},

#define ON_SREQUEST(hClass, fp) {hClass::uri, WrapFormHandle(new misc::FormHandleT<hClass>()), fpt_vsc, \
	(TargetFunc)(static_cast<void (PHClass::*)(uint32_t, hClass *)>(fp)), NULL},

#define ON_SSREQUEST(hClass, fp) {hClass::uri, WrapFormHandle(new misc::FormHandleT<hClass>()), fpt_vssc, \
	(TargetFunc)(static_cast<void (PHClass::*)(uint32_t, uint32_t, hClass *)>(fp)), NULL},

#define ON_URESPONSE(hClass, fp) {hClass::uri, WrapFormHandle(new misc::FormHandleT<hClass>()), fpt_vucr, \
	(TargetFunc)(static_cast<void (PHClass::*)(uint32_t, hClass *, uint32_t resCode)>(fp)), NULL},

#define ON_UID(hClass, fp) {hClass::uri, WrapFormHandle(new misc::FormHandleT<hClass>()), fpt_vu, \
	(TargetFunc)(static_cast<void (PHClass::*)(uint32_t)>(fp)), NULL},

#define ON_KEYPROXY(hClass, fp) {hClass::uri, WrapFormHandle(new misc::FormHandleT<hClass>()), fpt_vpc, \
    (TargetFunc)(static_cast<void (PHClass::*)(const std::vector<uint32_t> &vecKey, hClass *)>(fp)), NULL},

#define ON_UKEYPROXY(hClass, fp) {hClass::uri, WrapFormHandle(new misc::FormHandleT<hClass>()), fpt_vupc, \
    (TargetFunc)(static_cast<void (PHClass::*)(uint32_t, const std::vector<uint32_t> &vecKey, hClass *)>(fp)), NULL},

#define ON_CONNECT_ID(hClass, fp) {hClass::uri, WrapFormHandle(new misc::FormHandleT<hClass>()), fpt_vci, \
    (TargetFunc)(static_cast<void (PHClass::*)(hClass *, uint32_t)>(fp)), NULL},

#define ON_WRITERRET(hClass, fp) {hClass::uri, WrapFormHandle(new misc::FormHandleT<hClass>()), fpt_wcc, \
	(TargetFunc)(static_cast<core::IWriter *(PHClass::*)(hClass *, IConn *)>(fp)), NULL},


namespace core{
union TargetProc{
	TargetFunc mf_oo;
	void (core::PHClass::*mf_vv)();
	void (core::PHClass::*mf_vr)(uint32_t);
	void (PHClass::*mf_vc)(void *);
	void (PHClass::*mf_vcr)(void *, uint32_t);
	void (PHClass::*mf_vcc)(void *, core::IConn *);
	void (PHClass::*mf_vccr)(void *, core::IConn *, uint32_t);
	void (PHClass::*mf_vccra)(void *, core::IConn *, uint32_t, Request &);
	void (PHClass::*mf_vkc)(const std::string &from, void *);
	void (PHClass::*mf_vkcr)(const std::string &form, void *, uint32_t);
	void (PHClass::*mf_vk)(const std::string &from);
	void (PHClass::*mf_vuc)(uint32_t, void *);
	void (PHClass::*mf_vucr)(uint32_t, void *, uint32_t);
	void (PHClass::*mf_vu)(uint32_t);
	void (PHClass::*mf_vsc)(uint32_t, void *);
	void (PHClass::*mf_vssc)(uint32_t, uint32_t, void *);
    void (PHClass::*mf_vpc)(const std::vector<uint32_t> &vecKey, void *);
    void (PHClass::*mf_vupc)(uint32_t, const std::vector<uint32_t> &vecKey, void *);
    void (PHClass::*mf_vci)(void *, uint32_t);
	IWriter *(PHClass::*mf_wcc)(void *, core::IConn *);
	//Request::ResponseType (IFormTarget::*)(void *) Request::ResponseType (IFormTarget::*)(void *) Request::ResponseType (IFormTarget::*)(void *) Request::ResponseType (IFormTarget::*)(void *) 
};
}
#endif

