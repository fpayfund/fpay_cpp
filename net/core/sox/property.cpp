
#include "property.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <stdlib.h>
namespace sox
{

void Property::parse_insert(const std::string& line)
{
	std::string::size_type pos = 0;
	if ((pos = line.find('=', pos)) != std::string::npos)
	{
		std::string name(line, 0, pos);
		std::string value(line, pos + 1, line.size() - pos);
		m_map[name] = value;
	}
}

void Property::load(const char * file)
{
	std::ifstream ifile(file);
	std::string line;
	while (std::getline(ifile, line))
	{
		if (line.empty() || line[0] == '#' || line[0] == ';')
			continue;
		parse_insert(line);
	}
}

void Property::save(const char * file)
{
	typedef std::vector<std::string> AllLine;
	AllLine allline;
	{
		std::ifstream ifile(file);
		std::string in_line;
		while (std::getline(ifile, in_line))
		{
			allline.push_back(in_line);
			if (in_line.empty() || in_line[0] == '#' || in_line[0] == ';')
				continue;

			std::string::size_type pos = 0;
			if ((pos = in_line.find('=', pos)) != std::string::npos)
			{
				std::string name(in_line, 0, pos);
				iterator it = m_map.find(name);
				if (it != m_map.end())
				{
					allline.back() = name + "=" + it->second;
					m_map.erase(it);
				}
			}
		}
	}
	std::ofstream ofile(file);
	for (AllLine::iterator it = allline.begin(); it != allline.end(); ++it)
		ofile << *it << "\n";

	for (iterator i = m_map.begin(); i != m_map.end(); ++i)
		ofile << i->first << "=" << i->second << "\n";
}

int Property::getint(const std::string & name, int defvalue) const
{
	const_iterator it = m_map.find(name);
	if (m_map.end() == it)
		return defvalue;

	char * endptr;
	int result = ::strtoul(it->second.c_str(), &endptr, 0);
	if (*endptr == '\0' || isspace(*endptr))
		return result;
	else
		return defvalue;
}

std::string Property::getstring(const std::string & name, const std::string & defvalue) const
{
	const_iterator it = m_map.find(name);
	if (m_map.end() == it)
		return defvalue;
	return it->second;
}

void Property::setint(const std::string & name, int value)
{
	std::ostringstream sstr;
	sstr << value;
	setstring(name, sstr.str());
}

void Property::setstring(const std::string & name, const std::string & value)
{
	m_map[name] = value;
}

void Property::parse(int argc, char * argv[])
{
	for (int i=1; i<argc; ++i)
		parse_insert(argv[i]);
}

} // namespace sox
