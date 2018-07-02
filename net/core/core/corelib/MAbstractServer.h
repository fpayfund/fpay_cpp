#ifndef MABSTRACTSERVER_H_
#define MABSTRACTSERVER_H_

#include <common/core/iserver.h>
#include <common/core/ilink.h>

namespace core
{

class MAbstractServer : public virtual IServer,
						public IConnManagerAware
{
private:
	ILinkEvent* _link_event;

protected:
	std::string _name;
	volatile uint32_t _server_id;
	int _net_type;

	uint32_t _group_id;
public:
	MAbstractServer();
	
	virtual void setName(const std::string &n);

	virtual std::string getName(){
		return _name;
	}

	virtual uint32_t getServerId(){
		return _server_id;
	}

	virtual int getNetType(){
		return _net_type;
	}

	virtual void getFullName(std::string& get_fullname);

	void setLinkEvent(ILinkEvent *lv){
		_link_event = lv;
	}

	ILinkEvent *getLinkEvent(){
		return _link_event;
	}

	virtual uint32_t getGroupId() {
		return _group_id;
	}

	void setGroupId(uint32_t g){
		_group_id = g;
	}
	

	virtual bool setInitStatus(int){
		return false;
	}

};



}

#endif /*MABSTRACTSERVER_H_*/
