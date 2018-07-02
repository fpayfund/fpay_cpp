
#include "hostsox.h"
#include <fstream>
#include <stdlib.h>
namespace sox
{

// 没有报告错误
// XXX
// 192.168.0.1
// 192.168.0.0/24
// 192.168.0.1-192.168.0.3

void Hosts::add(u_long h1, u_long h2)
{
	if (!valid_addr(h1) || !valid_addr(h2))
		return;

	u_long i1 = ntohl(h1);
	u_long i2 = ntohl(h2);

	if (i2 - i1 > 64 * 1024) return; // guard

	for (u_long i = i1; i <= i2; ++i)
		m_hosts.insert(htonl(i));
}

void Hosts::parse_add(const std::string & host)
{
	const char * h = host.c_str();
	const char * c = strpbrk(h, "/-");

	if (NULL == c) // not found
	{
		add(aton_addr(h));
	}
	else
	{
		u_long ip1 = aton_addr(host.substr(0, c - h));
		if (!valid_addr(ip1)) return;

		switch (*c)
		{
		case '/':
			{
				int len = atoi(c+1);
				if (len <= 0 || len > 32)
					return;
				u_long mask = 0x80000000;
				while (--len > 0) mask = mask >> 1 | 0x80000000;
				mask = htonl(mask);
				add(ip1 & mask, ip1 | ~mask);
			}
			break;

		case '-':
			add(ip1, aton_addr(host.substr(c - h + 1)));
			break;
		}
	}
}

void Hosts::load_add(const std::string & file)
{
	std::ifstream ifile(file.c_str());
	std::string line;
	while (std::getline(ifile, line))
	{
		if (line.empty() || line[0] == '#')
			continue;
		add(line);
	}
}

std::ostream & Hosts::trace(std::ostream & os) const
{
	for (Hostset::const_iterator i = m_hosts.begin(); i != m_hosts.end(); ++i)
		os << addr_ntoa(*i) << '\n';
	return os;
}

} // namespace sox

#ifdef _TEST_

main()
{
	sox::Hosts hosts("hosts");
	hosts.trace(std::cout);
	std::string line;
	while (std::getline(std::cin, line))
	{
		hosts.add(line);
		std::cout << "trace\n";
		hosts.trace(std::cout);
	}
}
// g++ -o test_hosts -D_TEST_ hostsox.cpp sox.a

#endif
