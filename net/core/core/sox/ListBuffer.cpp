#include "ListBuffer.h"

namespace sox
{

#define LIST_BLOCK_ALLOC_TIMER	1000

ListBlockAllocator::ListBlockAllocator()
{
	for(uint32_t size = DEF_BLOCK_SIZE;size <= MAX_BLOCK_SIZE;size <<= 1)
	{
		m_lbFreeQ[size];
		m_lbAllocate[size] = 0;
	}

	m_totalSize   = 0;
	m_historySize = 0;
	m_inuseSize   = 0;

	m_maxBufferSize = 400*1024*1024;
	m_overflow      = false;

	select_timeout(LIST_BLOCK_ALLOC_TIMER);
}

ListBlockAllocator::~ListBlockAllocator()
{
	for(LBFreeQueue_t::iterator it = m_lbFreeQ.begin();it != m_lbFreeQ.end();++it)
	{
		for(std::list<ListBlock*>::iterator lIt = it->second.begin();lIt != it->second.end();++lIt)
		{
			ListBlock* p = *lIt;
			if(p->m_data)
				delete p->m_data;
			delete p;
		}
	}
}

ListBlock* ListBlockAllocator::ordered_malloc(size_t n)
{
	ListBlock* ret = NULL;

	for(LBFreeQueue_t::iterator it = m_lbFreeQ.begin();it != m_lbFreeQ.end();++it)
	{
		if(it->first >= n)
		{
			if(it->second.empty())
			{
				ListBlock* p = new ListBlock();
				if(p)
				{
					p->m_data = (char*)malloc(it->first);
					if(p->m_data)
					{
						p->m_size = it->first;
						ret = p;
						++m_lbAllocate[p->m_size];
						m_totalSize += p->m_size;
						if(m_totalSize > m_historySize)
							m_historySize = m_totalSize;
					}
					else {
						delete p;
					}

					log(Debug, "ListBlockAllocator::ordered_malloc:%p m_inuseSize:%u", ret, m_inuseSize);
				}
			}
			else
			{
				ret = it->second.front();
				it->second.pop_front();
			}
			break;
		}
	}

	if(ret)
	{
		m_inuseSize += ret->m_size;
	}
	if(m_inuseSize > m_maxBufferSize)
		m_overflow = true;

	return ret;
}

void ListBlockAllocator::ordered_free(ListBlock* p)
{
	p->reset();
	m_lbFreeQ[p->m_size].push_back(p);

	assert(m_inuseSize >= p->m_size);
	m_inuseSize -= p->m_size;
	if(m_inuseSize <= m_maxBufferSize)
		m_overflow = false;
}

void ListBlockAllocator::dump()
{
  log(Info, "ListBlockAllocator::dump total: %u, history: %u inuse: %u", m_totalSize, m_historySize, m_inuseSize);

//  for(LBFreeQueue_t::iterator it = m_lbFreeQ.begin();it != m_lbFreeQ.end();++it)
//  {
//    log(Debug, "block size: %7u alloc: %u free: %u", it->first, m_lbAllocate[it->first], it->second.size());
//  }
}

void ListBlockAllocator::recycle()
{
	uint32_t curr = sox::env::now;
	static uint32_t last = curr;
	if(last + 60 > curr)
		return;

	last = curr;

	for(LBFreeQueue_t::iterator it = m_lbFreeQ.begin();it != m_lbFreeQ.end();++it)
	{
		uint32_t alloc = m_lbAllocate[it->first];
		uint32_t free  = it->second.size();
		assert(free <= alloc);
		uint32_t inuse = alloc - free;
		uint32_t count = 0;

		// block越小，变动越频繁，越小的block可以预留多一些空间

		while(free > inuse+1)
		{
			// 预留与使用值相等的block作为buffer，至少一个，其余的回收
			ListBlock* p = it->second.back();
			assert(p != NULL);
			delete p->m_data;
			delete p;
			it->second.pop_back();

			--free;
			++count;
			//
			m_totalSize -= p->m_size;
		}

		// update counter
		m_lbAllocate[it->first] = free + inuse;

		if(count > 0)
			log(Info, "ListBlockAllocator::recycle size: %u count: %u", it->first, count);
	}
}

void ListBlockAllocator::handle(int)
{
	try {
		dump();

		recycle();
	}
	catch (...)
	{
		log(Error, "ListBlockAllocator::handle timer err!");
	}

	select_timeout(LIST_BLOCK_ALLOC_TIMER);
}

ListBuffer::ListBuffer()
{
	m_blocks = 0;
}

ListBuffer::~ListBuffer()
{
	// 
	for(std::list<ListBlock*>::iterator it = m_listBlocks.begin();it != m_listBlocks.end();++it)
	{
		ListBlockAllocator::Instance()->ordered_free(*it);
	}
}

bool ListBuffer::append(const char* app, size_t len)
{
	if(len == 0)
		return true;

	if(m_listBlocks.empty())
	{
		ListBlock* p = ListBlockAllocator::Instance()->ordered_malloc(len);
		if(!p)
			return false;

		m_listBlocks.push_back(p);
		m_blocks += (p->m_size >> DEF_BLOCK_MASK);
	}

	ListBlock* p = m_listBlocks.back();
	uint32_t ret = p->write(app, len);
	if(ret < len)
	{
		if(m_blocks >= MAX_BLOCK_NUM)
			return false;

		ListBlock* newBlock = ListBlockAllocator::Instance()->ordered_malloc(len-ret);
		if(!newBlock)
			return false;

		newBlock->write(app+ret, len-ret);

		m_listBlocks.push_back(newBlock);
		m_blocks += (newBlock->m_size >> DEF_BLOCK_MASK);
	}

	return true;
}

void ListBuffer::erase(uint32_t len)
{
	// 删除第一个block的数据，len必须全部在第一个block中，不跨block删除
	if(m_listBlocks.empty())
		return;

	ListBlock* p = m_listBlocks.front();
	p->erase(len);
	if(p->empty())
	{
		m_listBlocks.pop_front();
		ListBlockAllocator::Instance()->ordered_free(p);
		m_blocks -= (p->m_size >> DEF_BLOCK_MASK);
	}
}

}
