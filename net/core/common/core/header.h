#ifndef CORE_HEADER_H
#define CORE_HEADER_H
#include "common/res_code.h"
#include "common/int_types.h"

enum Method{
	ROUTE,
	ANSWER,
	END,
	STOP
};

#define HEADER_SIZE 10

struct Header{
public:
	uint32_t length;
	URI_TYPE uri;
	uint16_t resCode;
public:
	Header():length(), resCode(RES_SUCCESS){}

	URI_TYPE getUri() const {
		return uri;
	}
	void setUri(URI_TYPE u) {
		uri = u;
	}

	uint16_t getResCode() const {
		return resCode;
	}

	void setResCode(uint16_t res){
		resCode = res;
	}

	bool isSuccess() const{
		return resCode == RES_SUCCESS;
	}

	uint32_t getLength(){
		return length;
	}
};
#endif

