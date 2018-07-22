#ifndef WRAPFORWARDBUFFER_H
#define WRAPFORWARDBUFFER_H
namespace core{
class WrapForwardBuffer {
public:
	WrapForwardBuffer(const char * data, size_t size) :
	  m_data(data), m_size(size), m_pos(0) {
	  }

	  const char * data() {
		  return m_data + m_pos;
	  }
	  size_t size() {
		  return m_size - m_pos;
	  }

	  void erase(size_t n) {
		  m_pos += n;
		  assert(m_pos <= m_size);
	  }
	  bool empty() const {
		  return m_size == m_pos;
	  }

	  size_t offset() const {
		  return m_pos;
	  }

private:
	const char * m_data;
	size_t m_size;
	size_t m_pos;
};
}

#endif

