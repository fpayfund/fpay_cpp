
#ifndef __SOX_HTTP_PARSER_H_INCLUDE__
#define __SOX_HTTP_PARSER_H_INCLUDE__

#include "soxhelper.h"

namespace sox
{

/* ICallback interface
class ICallback
{
	// firstline: Request-Line or Status-Line
	bool onHttpRequestLine(char * data, size_t size);
	bool onHttpUnknownField(char * name, char * value);
	bool onHttpHeadersEnd();
	// multipart and connection close never invoke this. length_detectable only
	bool onHttpRequestEnd(char * data, size_t size);
	// the number of bytes processed is returned. return -1 to stop parse
	int onHttpContent(char * data, size_t size);
};
*/

template < typename ForwardBuffer, typename ICallback >
class THttpParser
{
public:
	enum // Body-Type: rfc2616#4.4 Message Length
	{
		// XXX 有判断依赖常量大小顺序，
		// 1. response message which "MUST NOT" include a message-body.
		BODY_NOCONTENT = '1',
		// 2.3. latter is ignore, 当前处理：BODY_CHUNKED 具有优先权利.
		BODY_CHUNKED = '2', BODY_LENGTH = '3',
		// 4. multipart and the transfer-length is not otherwise specified
		BODY_MULTIPART = '4',
		// 5. (client only) By the server closing the connection.
		BODY_CONNECTION_CLOSE = '5',
	};

	THttpParser() { reset(); }

	int parse(ForwardBuffer & cb, ICallback * conn);

	// BODY_NOCONTENT 算是知道Content-Length的
	bool length_detectable() const { return m_http_body_type < BODY_MULTIPART; }
	size_t content_length() const  { return m_content_length; }
	char body_type() const	       { return m_http_body_type; }

	// XXX
	enum ParseFlag { FLG_MULTIPART_END = 1 };
	void set_parse_flag(ParseFlag c) { m_parse_flag |= c; }

	void reset()
	{
		m_http_parse_state = HTTP_REQUEST_LINE;
		m_http_body_type = BODY_NOCONTENT;
		m_content_length = 0;
		m_chunk_size = 0;
		m_chunk_offset = 0;
		m_parse_flag = 0;
	}
	std::ostream & trace(std::ostream & os) const
	{
		return os << "[S" << int(m_http_parse_state) << " body=" << body_type()
			<< " cl=" << content_length() << " size=" << m_chunk_size
			<< " offset=" << m_chunk_offset << "]";
	}

private:
	int http_request_line(ForwardBuffer &cb, ICallback * conn);

	int http_header_fields(ForwardBuffer &cb, ICallback * conn);
	bool parse_header_fields(char * header, size_t len, ICallback * conn);
	bool parse_field(char * data, size_t size, ICallback * conn);

	int http_parse_chunksize(ForwardBuffer &cb, ICallback * conn);
	int http_process_content(ForwardBuffer &cb, ICallback * conn);
	// XXX skip trailer, which is terminated by an empty line.
	int http_parse_trailer(ForwardBuffer &cb, ICallback * conn);
	int http_finish_chunk(ForwardBuffer &cb, ICallback * conn);

	enum // State
	{
		HTTP_REQUEST_LINE, HTTP_HEADER_FIELDS,
		HTTP_PARSE_CHUNKSIZE, HTTP_PROCESS_CONTENT, HTTP_FINISH_CHUNK,
		HTTP_PARSE_TRAILER,
		// temp
		HTTP_NEEDMOREDATA, HTTP_PARSE_ERROR, // detail error
	};

	bool is_multipart_end() const  { return (m_parse_flag & FLG_MULTIPART_END); }
	bool end_of_content() const { return content_length() == 0; }

	char m_http_body_type;
	char m_http_parse_state;
	short m_parse_flag;

	size_t m_content_length; // content-length 时，表示整个大小
	size_t m_chunk_size;     // 当前chunk剩下多少，content-length 当作一个chunk处理
	size_t m_chunk_offset;   // chunked时，记录上一个chunk剩下的数据偏移，用于合并chunk
};

template < typename ForwardBuffer, typename ICallback >
int THttpParser<ForwardBuffer, ICallback>::parse(ForwardBuffer &cb, ICallback * conn)
{
	while (!cb.empty())
	{
		int n = HTTP_PARSE_ERROR;
		switch (m_http_parse_state)
		{
		case HTTP_PROCESS_CONTENT: n = http_process_content(cb, conn); break;
		case HTTP_PARSE_CHUNKSIZE: n = http_parse_chunksize(cb, conn); break;
		case HTTP_FINISH_CHUNK:    n = http_finish_chunk(cb, conn); break;
		case HTTP_REQUEST_LINE:    n = http_request_line(cb, conn); break;
		case HTTP_HEADER_FIELDS:   n = http_header_fields(cb, conn); break;
		case HTTP_PARSE_TRAILER:   n = http_parse_trailer(cb, conn); break;
		}

		if (n == HTTP_PARSE_ERROR)
			return -1;

		if (n == HTTP_NEEDMOREDATA)
			break; // need more

		if (n != HTTP_REQUEST_LINE)
			m_http_parse_state = n;
		else if (n != m_http_parse_state) // 开始解析新请求，忽略请求间的空白不需要reset
			reset();
	}
	return 0;
}

template < typename ForwardBuffer, typename ICallback >
int THttpParser<ForwardBuffer, ICallback>::http_request_line(ForwardBuffer &cb, ICallback * conn)
{
	char * data = cb.data();
	size_t n = sox::find(data, cb.size(), '\n');
	if (n == 0) return HTTP_NEEDMOREDATA; // No Line

	size_t size = n;
	// skip space
	for (; size > 0 && isspace(*data); ++data, --size) {}
	cb.erase(0, n - size); // erase space first

	if (cb.empty()) return HTTP_NEEDMOREDATA; // no more dat
	if (size == 0) return HTTP_REQUEST_LINE; // all isspace but remain data

	if (!conn->onHttpRequestLine(cb.data(), size))
		return HTTP_PARSE_ERROR;

	cb.erase(0, size - 1); *cb.data() = '\n';  // 不全部删除，保留一个字节, ungetc

	return HTTP_HEADER_FIELDS;
}

template < typename ForwardBuffer, typename ICallback >
int THttpParser<ForwardBuffer, ICallback>::http_header_fields(ForwardBuffer &cb, ICallback * conn)
{
	size_t n = headersEnd(cb.data(), cb.size()); // check header end "\n\n"
	if (n == 0) return HTTP_NEEDMOREDATA;

	if (!parse_header_fields(cb.data(), n, conn))
		return HTTP_PARSE_ERROR;

	cb.erase(0, n); // erase all header
	if (!conn->onHttpHeadersEnd())
		return HTTP_PARSE_ERROR;

	switch (body_type())
	{
	case BODY_CHUNKED:   return HTTP_PARSE_CHUNKSIZE;
	case BODY_NOCONTENT: assert(end_of_content()); // falldown
	case BODY_LENGTH:
		if (end_of_content()) // finish check. see ICallback interface
		{
			if (!conn->onHttpRequestEnd("", 0)) return HTTP_PARSE_ERROR;
			return HTTP_REQUEST_LINE;
		}
		m_chunk_size = content_length(); // init work counter falldown
	default:
		return HTTP_PROCESS_CONTENT;
	}
}

template < typename ForwardBuffer, typename ICallback >
bool THttpParser<ForwardBuffer, ICallback>::parse_header_fields(char * header, size_t len, ICallback * conn)
{
	for (; len > 0 && isspace(*header); ++header, --len) {}
	if (len == 0) return true; // empty header

	// 记住第一行
	char * field = header;
	size_t fieldsize = sox::find(header, len, '\n');
	header += fieldsize, len -= fieldsize;
	while (len > 0)
	{
		size_t n = sox::find(header, len, '\n');
		if (isspace(*header)) // 续行，最后的\n or \r\n也当作续行
			fieldsize += n;
		else
		{
			if (!parse_field(field, fieldsize, conn))
				return false;
			field = header;
			fieldsize = n;
		}
		header += n, len -= n;
	}
	return parse_field(field, fieldsize, conn);
}

template < typename ForwardBuffer, typename ICallback >
bool THttpParser<ForwardBuffer, ICallback>::parse_field(char * data, size_t size, ICallback * conn)
{
	size_t n = sox::find(data, size, ':');
	if (n == 0) return true; // XXX skip error

	data[n - 1] = 0;
	data[size - 1] = 0;

	char * name = data;
	char * value = name + n;
	for (; *value && isspace(*value); ++value); // skip space

	// see Body-Type
	if (sox::strcasecmp(name, "content-length") == 0)
	{
		if (body_type() != BODY_CHUNKED)
			m_http_body_type = BODY_LENGTH;
		m_content_length = atoi(value);
		return true;
	}
	else if (sox::strcasecmp(name, "transfer-encoding") == 0)
	{
		//if (body_type() != BODY_LENGTH)
		{
			if (sox::strncasecmp(value, "chunked", 7) == 0)
				m_http_body_type = BODY_CHUNKED;
			else
				return false;
		}
		return true;
	}

	if (sox::strcasecmp(name, "connection") == 0)
	{
		if (BODY_NOCONTENT == body_type() // only setup if nocontent
			&& sox::strncasecmp(value, "close", 5) == 0)
			m_http_body_type = BODY_CONNECTION_CLOSE;
	}
	else if (sox::strcasecmp(name, "content-type") == 0)
	{
		if ((BODY_NOCONTENT == body_type() || BODY_CONNECTION_CLOSE == body_type())
			&& sox::strncasecmp(value, "multipart", 9) == 0)
			m_http_body_type = BODY_MULTIPART;
	}
	// call handler
	return conn->onHttpUnknownField(name, value);
}

template < typename ForwardBuffer, typename ICallback >
int THttpParser<ForwardBuffer, ICallback>::http_process_content(ForwardBuffer &cb, ICallback * conn)
{
	size_t cl = cb.size();
	if (length_detectable() && m_chunk_size < cl)
		cl = m_chunk_size;

	int n = conn->onHttpContent(cb.data(), cl);
	if (n < 0) return HTTP_PARSE_ERROR;
	cb.erase(0, n);
	if (length_detectable())
		m_chunk_size -= n;

	switch (body_type())
	{
	case BODY_CHUNKED: // check chunk end.
		if (m_chunk_size == 0 || m_chunk_size < cb.size())
		{
			// record offsetbegin process next chunk
			m_chunk_offset = m_chunk_size;
			m_chunk_size = 0;
			return HTTP_FINISH_CHUNK;
		}
		break;

	case BODY_LENGTH: // check content end
		if (m_chunk_size <= cb.size()) // include (m_chunk_size == 0)
		{
			bool keep = conn->onHttpRequestEnd(cb.data(), m_chunk_size);
			// request ended, and keep-alive, next request
			cb.erase(0, m_chunk_size);
			return keep ? HTTP_REQUEST_LINE : HTTP_PARSE_ERROR;
		}
		break;

	// 以下类型不知道明确长度
	case BODY_MULTIPART: // XXX check inner?
		// 为了keep-alive，需要知道结束并且重置解析状态，当前有外部判断并设置标志。
		if (is_multipart_end())
			// in this case, onHttpContent have done everything. no onHttpRequestEnd
			return HTTP_REQUEST_LINE;
		break;
	case BODY_CONNECTION_CLOSE: // 不需要知道结束，解析器无法知道连接的事情，外部自己判断
		break;

	default:
		assert(false);
	}
	return HTTP_NEEDMOREDATA;
}

template < typename ForwardBuffer, typename ICallback >
int THttpParser<ForwardBuffer, ICallback>::http_parse_chunksize(ForwardBuffer &cb, ICallback * conn)
{
	//assert(body_type() == BODY_CHUNKED);
	//assert(m_chunk_size == 0);

	char * data = cb.data() + m_chunk_offset;
	size_t size = cb.size() - m_chunk_offset;

	size_t n = sox::find(data, size, '\n');
	if (n == 0) return HTTP_NEEDMOREDATA;

	data[n - 1] = 0;
	char * endptr;
	size_t chunk_size = strtoul(data, &endptr, 16);
	// TODO check if valid. if (!isspace(*endptr)) return HTTP_PARSE_ERROR;

	if (chunk_size == 0) // last chunk
	{
		m_chunk_size = n; // last chunk 的大小
		return HTTP_PARSE_TRAILER; // 开始查找 chunk-trailer 的尾巴
	}

	cb.erase(m_chunk_offset, n);

	m_chunk_size = chunk_size + m_chunk_offset;
	m_chunk_offset = 0;

	return HTTP_PROCESS_CONTENT;
}

template < typename ForwardBuffer, typename ICallback >
inline int THttpParser<ForwardBuffer, ICallback>::http_parse_trailer(ForwardBuffer &cb, ICallback * conn)
{
	char * line = cb.data() + m_chunk_size + m_chunk_offset;
	size_t size = cb.size() - m_chunk_offset - m_chunk_size;

	size_t n;
	while ((n = sox::find(line, size, '\n')) > 0)
	{
		char c = *line;
		m_chunk_size += n; line += n; size -= n;
		if (n == 1 || (n == 2 && c == '\r')) // empty line: '\n' or "\r\n"
		{
			bool keep = conn->onHttpRequestEnd(cb.data(), m_chunk_offset);
			cb.erase(0, m_chunk_offset + m_chunk_size);
			return keep ? HTTP_REQUEST_LINE : HTTP_PARSE_ERROR;
		}
	}
	return HTTP_NEEDMOREDATA;
}

template < typename ForwardBuffer, typename ICallback >
inline int THttpParser<ForwardBuffer, ICallback>::http_finish_chunk(ForwardBuffer &cb, ICallback * conn)
{
	size_t n = sox::find(cb.data()+m_chunk_offset, cb.size()-m_chunk_offset, '\n');
	if (n == 0) return HTTP_NEEDMOREDATA;

	cb.erase(m_chunk_offset, n);
	return HTTP_PARSE_CHUNKSIZE;
}

} // namespace sox

#endif // __SOX_HTTP_PARSER_H_INCLUDE__
