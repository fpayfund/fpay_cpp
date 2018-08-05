#include "InnerConn.h"
#include "core/sox/util.h"
#include "core/sox/snox.h"
#include "core/sox/logger.h"
#include <errno.h>
#include <stdio.h>
using namespace core;

InnerConn::~InnerConn() {
	log(Info, "Destroy: %s", toString().data());
}

void InnerConn::sendBin(const char * data, size_t size, uint32_t uri) {
	//char * for encrypto
	if(bEnable){
		char *dt = const_cast<char *>(data);

		if (socket().isConnected()) {
			m_output.write(socket(), dt, size);
			if (!m_output.empty()){
#ifndef    NO_SEND_BLOCKED_LOG
				log(Info, "send blocked output size: %u connid:%u peer:%s:%u", m_output.size(), getConnId(), sox::addr_ntoa(getPeerIp()).c_str(), getPeerPort());
#endif
				select(0, sox::SEL_WRITE);
			}
		} else {
			m_output.write(dt, size);
			log(Info, "socket not conntected !");
			//        m_hdl->onError(-1, m_output.data(), this);
		}
	}else{
		log(Info, "socket not enable: %s", sox::addr_ntoa(peerIp).data());
	} 
}

void InnerConn::onConnected(const std::string & errstr) {
	if (socket().isConnected()) {
		log(Info, "connected");
		select_timeout();
		int add = sox::SEL_READ;
		if (!m_output.empty()) {
			m_output.flush(socket());
			if (!m_output.empty())
			  add |= sox::SEL_WRITE;
		}
		select(sox::SEL_ALL, add);
		eHandler->onConnected(this);
	} else {
		log(Error, "Connect %s, on socket %s", errstr.data(), toString().data());
	    fprintf(stderr, "InnerConn::onConnected,Connect %s, on socket %s\n", errstr.data(), toString().data());
		
		eHandler->onError(-1, "", this);
	}
}


void InnerConn::onRead() {
	try {
		if (m_input.pump(socket()) > 0)
		{
			int ret = dHandler->onData(m_input.data(), m_input.size(), this, m_input.isEncrypto()? TCP_ENCODE : TCP_UNCODE);

			if(ret != -1){
				m_input.erase(0, ret);
			}else{                
				eHandler->onInitiativeClose(this);
			}
		}
		else {
			eHandler->onClose(this);
		}
	}  catch(sox::socket_error &se) {
		if(se.what_errno() == EAGAIN || se.what_errno() == EINTR) {
			return;
		} else {
			try {
				eHandler->onError(sox::env::SocketErrorLogLevel, (std::string("Inner Conn read error:") + se.what()).data(), this);
			} catch(std::exception &err) {
				log(Warn, "ignore exception in socket_error: %s", err.what());
			}
		}
	} catch(std::exception &ex) {
		try {
			eHandler->onError(sox::env::SocketErrorLogLevel, (std::string("Inner Conn read error:") + ex.what()).data(), this);
		} catch(std::exception &err) {
			log(Warn, "ignore exception in exception: %s", err.what());
		}
	}
}

void InnerConn::onWrite() {
	try {
		m_output.flush(socket());
		if (m_output.empty())
		  select(sox::SEL_WRITE, 0);
	} catch(sox::socket_error &se) {
		if(se.what_errno() == EAGAIN || se.what_errno() == EINTR) {
			return;
		} else {
			eHandler->onError(sox::env::SocketErrorLogLevel, (std::string("Inner Conn write error:") + se.what()).data(), this);
		}
	} catch(std::exception &ex) {
		eHandler->onError(sox::env::SocketErrorLogLevel, (std::string("Inner Conn write error:") + ex.what()).data(), this);
		//throw ex;
	}
}

std::ostream & InnerConn::trace(std::ostream & os) const {
	return os << " "<< sox::addr_ntoa(peerIp) << ":"<< peerPort;
}

void InnerConn::setTimeout(int tm) {
	AbstractConn::setTimeout(tm);
	select_timeout(tm);
}

void InnerConn::onTimeout() {
	if (socket().clearRecvTag()) {
		//        log(Debug, "timeout is:", tmo);
		select_timeout(tmo);
	} else {
		try {
			eHandler->onTimeout(this);
		} catch(sox::socket_error &se) {
			if(se.what_errno() == EAGAIN || se.what_errno() == EINTR) {
				return;
			} else {
				try {
					eHandler->onError(sox::env::SocketErrorLogLevel, (std::string("Inner Conn read error:") + se.what()).data(), this);
				}catch(std::exception &err) {
					log(Warn, "ignore exception in socket_error: %s", err.what());
				}
			}
		} catch(std::exception &ex) {
			try {
				eHandler->onError(sox::env::SocketErrorLogLevel, (std::string("Inner Conn read error:") + ex.what()).data(), this);
			}catch(std::exception &err) {
				log(Warn, "ignore exception in exception: %s", err.what());
			}
		}
	}
}

void InnerConn::setEncKey(const unsigned char *key, size_t len) {
	m_output.setRC4Key(key, len);
	m_input.setRC4Key(key, len);
}

size_t InnerConn::get_input_buffer_size()
{
	return m_input.size();
}
size_t InnerConn::get_output_buffer_size()
{
	return m_output.size();
}

#define ENCODE_BUFFER 1024
RC4Alloc::RC4Alloc(){
	writeBuffer = new unsigned char[ENCODE_BUFFER];
	curSize = ENCODE_BUFFER;
}

RC4Alloc::~RC4Alloc(){
	delete[] writeBuffer;
}

void  RC4Alloc::realloc(size_t sz){
	delete[] writeBuffer;
	writeBuffer = new unsigned char[sz];
	curSize = sz;
}

RC4Alloc RC4Filter::writeBuffer;


RC4Filter::RC4Filter() {
	encrypted = false;
}

RC4Filter::~RC4Filter() {

}



void RC4Filter::filterRead(char *data, size_t sz) {
	if (encrypted) {
		RC4(&rc4, (unsigned long)sz, (unsigned char *)data, (unsigned char *)data);
	}
}

char* RC4Filter::filterWrite(char *data, size_t sz) {
	//xxx thread unsafe
	if (encrypted) {
		if (sz > writeBuffer.getCurSize()) {
			writeBuffer.realloc(sz);
		}
		RC4(&rc4, (unsigned long)sz, (unsigned char *)data, writeBuffer.getWriteBuffer());
		return (char *)writeBuffer.getWriteBuffer();
	} else {
		return data;
	}

}

bool RC4Filter::isEncrypto() const {
	return encrypted;
}

void RC4Filter::setRC4Key(const unsigned char *data, size_t len) {
	RC4_set_key(&rc4, (int)len, data);
	encrypted = true;
}

IConn *InnerConnCreator::createConnect(const std::string& ip, uint32_t port, ILinkHandler *handler, ILinkEvent *eh, CreateCallback *cc) {
	try {
		InnerConn *conn = new InnerConn(ip, port, 2000, handler, eh);

		conn->setHandler(handler);
		conn->setLinkEvent(eh);

		if(cc){
			cc->onConnCreate(conn);
		}
		conn->select(0, sox::SEL_CONNECTING);
		return conn;
	}catch (sox::socket_error & se)
	{
		log(Warn, "InnerCreate SocketError: %s", se.what());
		return NULL;
	}

}

IConn *InnerConnCreator::creatConnect(SOCKET so, uint32_t ip, int port, ILinkHandler *h, ILinkEvent *eH, CreateCallback *cc){
	InnerConn *conn = new InnerConn(so, (u_long)ip, port, h, eH);

	conn->setHandler(h);
	conn->setLinkEvent(eH);

	if(cc){
		cc->onConnCreate(conn);
	}
	conn->select(0, sox::SEL_READ);
	return conn;
}

