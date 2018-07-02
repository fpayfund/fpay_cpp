#ifndef IREQUEST_H_
#define IREQUEST_H_
#include "common/packet.h"
#include "common/int_types.h"
#include "common/socketinc.h"
#include "header.h"

namespace core {
	struct IFormHandle;
	struct IContext;
	
	class Request: public Header{
 public:		
		sox::Unpack up;
		char* cpBuffer;
		const char *od; //origin data
		uint32_t os;	//origin size
	public:
		std::string key;	
				
		IFormHandle *handler;
		void *cmd;
		
		int connectType;

		std::string to; 
	public:
		static uint32_t peeklen(const void * d);

//		Request(const Request&);

		Request(const char *data, uint32_t sz);
		Request(const char *data, uint32_t sz, bool copy);
		
		void head();

		virtual ~Request();
		
		bool setFormHandler(IFormHandle *h);
		
		void forceHandler(IFormHandle *h);

		
		void setKey(const std::string &u);

		std::string getKey();
	
		void *getCmd() const{
			return cmd;
		}

		void setConnType(int tp){
			connectType = tp;
		}

		int getConnType() const{
			return connectType;
		}

    sox::Unpack& getPackData(){ return up; };

		static bool ifSuccess(uint32_t);

		void packOrgin(sox::Pack &pk) const;

		void leftPack(std::string &out);
		void reset() {up.reset(od, os);}
	};
}
#endif /*IREQUEST_H_*/
