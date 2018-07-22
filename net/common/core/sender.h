#ifndef CORE_SENDER_H
#define CORE_SENDER_H
#include "common/packet.h"
#include "header.h"
namespace core{
class Request;

struct Sender: public Header{
protected:
	sox::PackBuffer pb;
	sox::Pack hpk;
	sox::Pack pk;
public:
	Sender();

	Sender(URI_TYPE, const sox::Marshallable &);
  Sender & operator = (const Sender& sender);
	Sender(const Sender &);

	const char *body();
	
	size_t bodySize();

	const char *header();

	size_t headerSize();

	void marshall(const sox::Marshallable &);

	void marshall(URI_TYPE, const sox::Marshallable &);

	void marshall(const char *, size_t sz);

	void endPack();

	void clear();
};
}
#endif

