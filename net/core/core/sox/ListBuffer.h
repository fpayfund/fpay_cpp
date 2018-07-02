#pragma once

#include <new>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <list>
#include <map>

#include "singleton2.h"
#include "snox.h"
#include "logger.h"

#define DEF_BLOCK_MASK 13
#define DEF_BLOCK_SIZE (1 << DEF_BLOCK_MASK)
#define MAX_BLOCK_SIZE (8*1024*1024)
// �ܹ���������block size��2��ת���õ�����block����
#define MAX_BLOCK_NUM  (MAX_BLOCK_SIZE*2 >> DEF_BLOCK_MASK)

namespace sox
{

struct ListBlock
{
  char*    m_data; // block data
  uint32_t m_size; // block size
  uint32_t m_rpos; // read pointer
  uint32_t m_wpos; // write pointer
  ListBlock()
  {
    m_data = NULL;
    m_size = 0;
    m_rpos = 0;
    m_wpos = 0;
  }
  uint32_t freeSpace()
  {
    assert(m_wpos <= m_size);
    return m_size - m_wpos;
  }
  char* tail()
  {
    return m_data+m_wpos;
  }
  char* data()
  {
    return m_data+m_rpos;
  }
  void reset()
  {
    m_rpos = 0;
    m_wpos = 0;
  }
  uint32_t write(const char* a, size_t len)
  {
    uint32_t free = freeSpace();
    uint32_t size = (free >= len ? len : free);
    memcpy(tail(), a, size);
    m_wpos += size;

    return size;
  }
  void erase(size_t len)
  {
    // ɾ������
    assert(len + m_rpos <= m_wpos);
    m_rpos += len;
  }
  bool empty()
  {
    // block��������
    return m_rpos == m_wpos;
  }
  uint32_t size()
  {
    // block������size
    assert(m_rpos <= m_wpos);
    return m_wpos - m_rpos;
  }
};

struct ListBlockAllocator: public Singleton <ListBlockAllocator>, public sox::Handler
{
public:
  ListBlockAllocator();
  ~ListBlockAllocator();
  virtual void handle(int);

  ListBlock* ordered_malloc(size_t n);
  void ordered_free(ListBlock* p);

  //
  void dump();
  void recycle();
  void setMaxBufferSize(uint32_t size) { m_maxBufferSize = size; }
  bool isOverflow() { return m_overflow; }

  bool m_overflow;
  uint32_t m_maxBufferSize;

private:
  typedef std::map<uint32_t, std::list<ListBlock*> > LBFreeQueue_t;
  LBFreeQueue_t m_lbFreeQ;
  // ͳ�Ʒ���ĸ���size��block��
  typedef std::map<uint32_t, uint32_t> LBCounter_t;
  LBCounter_t m_lbAllocate;
  // ����size��block�ı�����
  LBCounter_t m_lbReserve;
  // ������ܳ���
  uint32_t m_totalSize;
  // ��ʷ��������
  uint32_t m_historySize;
  // ��ǰʹ�õ�size
  uint32_t m_inuseSize;
};

class ListBuffer
{
public:
  enum { npos = size_t(-1) };
  enum { max_blocks = MAX_BLOCK_NUM };

  ListBuffer();
  virtual ~ListBuffer();

  // buffer�Ƿ�Ϊ��
  inline bool empty() const
  {
    return size() == 0;
  }

  inline size_t size() const
  {
    if(m_listBlocks.empty())
      return 0;
    ListBlock* p = m_listBlocks.front();
    return p->size();
  }

  inline char* data()
  {
    if(m_listBlocks.empty())
      return NULL;
    ListBlock* p = m_listBlocks.front();
    return p->data();
  }

  inline uint32_t block()
  {
    return m_blocks;
  }

  inline uint32_t allSize()
  {
    return m_blocks * DEF_BLOCK_SIZE;
  }

  inline uint32_t dataSize()
  {
  	if (m_listBlocks.empty())
		return 0;

	ListBlock * pf = m_listBlocks.front();
  	ListBlock * pb = m_listBlocks.back();
		
  	return (allSize() - (pf->m_rpos + pb->freeSpace()));
  }

  bool append(const char* app, size_t len);
  void erase(uint32_t len);

private:

  // û�����ݵ�block���������list��
  std::list<ListBlock*> m_listBlocks;
  uint32_t m_blocks;

  // 
  ListBuffer(const ListBuffer&);
  void operator = (const ListBuffer &);
};

}
