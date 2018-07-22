
#ifndef __SOX_HOSTS_H_INCLUDE__
#define __SOX_HOSTS_H_INCLUDE__

#include <set>
#include "sockethelper.h"

namespace sox
{

class Hosts
{
public:
	Hosts() { }
	Hosts(const std::string & file) { load_add(file); }

	void reload(const std::string & file)   { m_hosts.clear(); load_add(file); }
	void load_add(const std::string & file);
	std::ostream & trace(std::ostream & os) const;

	void add(const std::string &host) { parse_add(host); }
	bool find(u_long ip) { return m_hosts.find(ip) != m_hosts.end(); }

	bool empty() { return m_hosts.empty();}
protected:
	void add(u_long host) { if (valid_addr(host)) m_hosts.insert(host); }
	void add(u_long h1, u_long h2);
	void parse_add(const std::string & host);
	typedef std::set<u_long> Hostset;
	Hostset m_hosts;
};

} // namespace sox

#endif // __SOX_HOSTS_H_INCLUDE__
