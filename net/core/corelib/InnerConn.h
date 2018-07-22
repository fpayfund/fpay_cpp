#ifndef __POPO3_INNERCONN_HPP__
#define __POPO3_INNERCONN_HPP__

#include <openssl/rc4.h>
#include <openssl/sha.h>

//#include "common/protocol/const.h"
#include "common/packet.h"
#include "core/sox/tcpsock.h"
#include "core/sox/sockbuffer.h"
#include "common/core/ibase.h"
#include "AbstractConn.h"

namespace core {

	struct RC4Alloc{
		RC4Alloc();
		~RC4Alloc();

		unsigned char *writeBuffer;
		size_t curSize;

		unsigned char *getWriteBuffer(){return writeBuffer;}
		size_t getCurSize(){return curSize;}
		void realloc(size_t sz);
	};

	class RC4Filter{
		bool encrypted;
		RC4_KEY rc4;
		static RC4Alloc writeBuffer;
		public:
		RC4Filter();
		~RC4Filter();
		void filterRead(char *, size_t);
		char* filterWrite(char *, size_t);
		bool isEncrypto() const;
		void setRC4Key(const unsigned char *, size_t);
	};
	class InnerConn : public AbstractConn, public sox::Tcpsock{
		public:
			typedef sox::BlockBuffer<sox::def_block_alloc_8k, 128*8> Buffer8x9k; // 1M WOW!
			typedef sox::BlockBuffer<sox::def_block_alloc_8k, 128*8> Buffer8x128k; // 1M WOW!

			typedef sox::SockBuffer<Buffer8x9k, RC4Filter> InputBuffer;
			typedef sox::SockBuffer<Buffer8x128k, RC4Filter> OutputBuffer;

			enum ConnType{
				ACCEPT,
				CONNECT
			};

			ConnType cType;
			InnerConn() {}
			InnerConn(SOCKET so, u_long ip, int port, ILinkHandler *h, ILinkEvent *eh) :
				Tcpsock(so) {
					//log(Debug, "tcp socket block is:", socket().isBlocking());
					setHandler(h);
					setLinkEvent(eh);
					cType = ACCEPT;
					init("Accept from", ip, port);
				}

			InnerConn(const std::string & ip, int port, int timeout, ILinkHandler *h, ILinkEvent *eh) :
				Tcpsock(ip, port, timeout){
					setHandler(h);
					setLinkEvent(eh);
					cType = CONNECT;
					init("Connect to", sox::aton_addr(ip), port);
				}

			InnerConn(u_long ip, int port, int timeout,  ILinkHandler *h, ILinkEvent *eh) :
				Tcpsock(ip, port, timeout) {
					setHandler(h);
					setLinkEvent(eh);
					cType = CONNECT;
					init("Connect to", ip, port);
				}


			virtual ~InnerConn();

			virtual	void sendBin(const char * data, size_t size, uint32_t uri);

			virtual void setEncKey(const unsigned char *key, size_t len);

			virtual void setTimeout(int);

			virtual void onTimeout();
			/*virtual void init_(){ }
			  virtual void connect_(const std::string& ip, unsigned port){
			  doConnect(ip, port, SOCKET_TIMEOUT);
			  }
			  virtual void close_(){
			  remove();
			  m_hdl->onClose(this);
			  }*/

			static void timval_reset()
			{
				/*uint32_t tick = OutputBuffer::getBufferTick();
				  uint32_t tick1 = OutputBuffer::getBufferTick1();
				  OutputBuffer::resetBufferTick();
				  log(Info, "OutputBuffer user tick:%u tick:%u", tick, tick1);*/
			}
		public:
			size_t get_input_buffer_size();
			size_t get_output_buffer_size();
			uint64_t connection_id;
		protected:
			void init(const char * info, u_long ip, int port) {
				peerIp = ip;
				peerPort = port;
			}

			virtual void onRead();
			virtual void onWrite();
			virtual std::ostream & trace(std::ostream & os) const;
			virtual void onConnected(const std::string & errstr);

			InputBuffer m_input;
			OutputBuffer m_output;
	};

	class InnerConnCreator: public IClientConnCreator, public IServerConnCreator{
		public:
			InnerConnCreator(){}
			IConn *createConnect(const std::string& ip, uint32_t port, ILinkHandler *handler, ILinkEvent *eh, CreateCallback *);
			virtual IConn *creatConnect(SOCKET, uint32_t ip, int port, ILinkHandler *h, ILinkEvent *eH, CreateCallback *);
	};
} // namespace daemon

#endif // __POPO3_INNERCONN_HPP__
