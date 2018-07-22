#ifndef IBASE_COMMON_H_
#define IBASE_COMMON_H_
#include "common/packet.h"
#include "request.h"
#include "sender.h"
#include <string>

namespace sox{
	struct Marshallable;
}

namespace core {
	struct IConn;


struct IFormHandle {
	virtual ~IFormHandle(){}
	virtual void *handlePacket(sox::Unpack &up) = 0;
	virtual void destroyForm(void *form) = 0;
};

struct FormEntry;

struct IWriter{
	virtual ~IWriter(){}
	virtual void answer(uint32_t uri, const sox::Marshallable &obj) = 0;
	//virtual void answer(uint32_t uri, uint16_t res, const sox::Marshallable &obj) = 0;
	virtual void answerErr(uint32_t uri, uint16_t ResCode) = 0;
	virtual void stop() = 0;
	virtual int flush(IConn *ansConn = NULL) = 0;
	//virtual void writeObj(const sox::Marshallable &obj) = 0;
	//virtual void setMethod(Method m) = 0;
	//virtual void setUri(uint32_t uri) = 0;
	//virtual void setResCode(uint16_t res) = 0;
	//virtual Sender *getSender() = 0;
	//virtual void reset() = 0;
};

struct IWriterAware{
protected:
	IWriter *writer;
public:
	void setWriter(IWriter *w){
		writer = w;
	}
	inline IWriter *getWriter(){
		return writer;
	}
};

struct PHClass{

};
struct IFormTarget: public IWriterAware{
	void answer(uint32_t u, uint16_t res){
		writer->answerErr(u, res);
	}

	void answer(uint32_t u, uint16_t res, const sox::Marshallable &obj){
		writer->answer(u, obj);
	}
};

typedef void (PHClass::*TargetFunc)();
enum FormProcType{
	fpt_vv,				//void ::(void)
	fpt_vr,				//void ::(resCode)
	fpt_vc,           // void ::(Cmd*);
	fpt_vcr,           // void ::(Cmd*, rescode);
	fpt_vcc,			// void ::(Cmd *, Conn* );
	fpt_vccr,			// void ::(Cmd *, Conn*, rescode);
	fpt_vccra,			// void ::(Cmd *, Conn*, rescode, Request &);
	fpt_vtc,			// void ::(Cmd *, thread local db connection *);
	fpt_vkc,			// void ::(const std::string &, Cmd *)
	fpt_vkcr,			// void ::(const std::string &, Cmd *, uint32_t)
	fpt_vk,				// void ::(const std::string &)
	fpt_vuc,			// void ::(uint32_t , Cmd *)
	fpt_vucr,			// void ::(uint32_t , Cmd *, uint32_t)
	fpt_vu,				// void ::(uint32_t)
	fpt_vsc,			// void ::(sid, Cmd *)
	fpt_vssc,			// void ::(sid, toSrvId, Cmd *)
    fpt_vpc,
    fpt_vupc,
	fpt_wcc,
	fpt_null,			//do nothing
	ftp_last,
};
struct WrapFormHandle{
	IFormHandle *_f;
	WrapFormHandle(IFormHandle *f): _f(f){}
	WrapFormHandle(){_f = NULL;}
	~WrapFormHandle(){delete _f;}
	inline IFormHandle * get(){ return _f;}
	inline void reset(){_f = NULL;}
};
struct FormEntry{
	uint32_t uri;
	WrapFormHandle requestForm;
	int type;
	TargetFunc proc;
	PHClass *target;
	void reset(){requestForm.reset();}
};


#define DECLARE_FORM_MAP static core::FormEntry *getFormEntries(); \
						static core::FormEntry formEntries[];

#define DECLARE_LOCAL_FORM_MAP core::FormEntry *getLocalFormEntries(); \
							   core::FormEntry *localFormEntries; \
							   void copyLocalFormEntries(); \
							   void deleteLocalFormEntires();


#define BEGIN_FORM_MAP(theClass) \
	FormEntry* theClass::getFormEntries()\
{ return theClass::formEntries; } \
	FormEntry theClass::formEntries[] = \
{ \


#define END_FORM_MAP() \
{0, WrapFormHandle(NULL), fpt_null, NULL, NULL} \
}; \


struct IAppContextEx{
	virtual ~IAppContextEx(){}
	virtual IWriter *requestDispatch(Request &request, IConn *conn) = 0;
	virtual void setParent(IAppContextEx *p) = 0;
	//xxx todo be true virtual 
	virtual void addEntry(FormEntry *entries, void *target, IWriterAware *inf){}
	//virtual void setDispatcher(protocol::link::IResponseDispatcher * disp) = 0;
};


}
#endif /*IBASE_H_*/
