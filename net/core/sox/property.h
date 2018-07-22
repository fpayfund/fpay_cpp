
#ifndef __SOX_PROPERTY_H_INCLUDE__
#define __SOX_PROPERTY_H_INCLUDE__

#include <string>
#include <map>

namespace sox
{

class Property
{
public:
	void load(const char * file);
	void save(const char * file);
	void parse(int argc, char * argv[]);

	int getint(const std::string & name, int defvalue) const;
	std::string getstring(const std::string & name, const std::string & defvalue) const;

	void setint(const std::string & name, int value);
	void setstring(const std::string & name, const std::string & value);

	typedef std::map<std::string, std::string> SSMap;
	typedef SSMap::iterator iterator;
	typedef SSMap::const_iterator const_iterator;
	SSMap m_map;

	void parse_insert(const std::string& line);
};

} // namespace sox
#endif // __SOX_PROPERTY_H_INCLUDE__
