
#include "snox.h"
#include "logger.h"
#include <sstream>
namespace sox
{

Selector * env::m_sel = NULL;
int env::SocketErrorLogLevel = Debug;
Handler * env::s_CurrentHandler = NULL;
time_t env::now = time(NULL);
std::string env::strTime = "";
unsigned int  env::haomiao = 0;
void env::finalize()
{
	delete m_sel; m_sel = NULL;
	Sockethelper::finalize();
}

///////////////////////////////////////////////////////////////////////////////
// Socket

void Socket::select(int remove, int add)
{
	if (!socket().isValid())
		throw std::runtime_error("select with invalid socket");

	// TODO XXX uncommemt
	//if (isBlocking()) socket().setblocking(false);

	// clear invalid add
	if (socket().isShutdownSends())     add &= ~SEL_WRITE;
	if (socket().isShutdownReceives())  add &= ~SEL_READ;

	remove = remove & ~add; // do not remove that will add

	// clear not need
	remove &= socket().m_sock_flags.selevent;
	add &= ~socket().m_sock_flags.selevent;

	if ((remove + add) != SEL_NONE)
	{
		env::selector()->select(this, remove, add); // MUST HAVE selector
		// save event after select ok.
		socket().m_sock_flags.selevent = (socket().m_sock_flags.selevent & ~remove) | add;
		socket().m_sock_flags.selected = 1;
	}
}

void Socket::remove()
{
	if (socket().m_sock_flags.selected == 1)
	{
		env::selector()->remove(this);
		socket().m_sock_flags.selected = 0;
	}
}

/*
void Socket::close()
{
	remove();
	Sockethelper::close();
}
*/

Socket::~Socket()
{
	// ~Sockethelper call Sockethelper::close() only
	//log(Debug, "~Socket ", getsocket());
	remove(); // close();
}

std::ostream & Socket::trace(std::ostream & os) const
{
	try
	{
		std::string ip;
		int port = 0;
		if (m_socket.isConnected())
			ip = m_socket.getpeerip(&port);

		return os << '-' << (unsigned int)m_socket.getsocket() << ' ' << ip << ':' << port;
	}
	catch ( std::exception & ex)
	{
		return os << ex.what();
	}
}

std::ostream & Handler::trace(std::ostream & os) const
{
	return os;
}

Handler::~Handler()
{
	if (Selector * sel = env::selector())
	{
		select_timeout(); // remove
		sel->record_removed(this);
	}
}

std::string Handler::toString() const{
	std::ostringstream ostr;
	trace(ostr);
	return ostr.str();
}
} // namespace sox

