#include "FPayServerCore.h"
#include "common/core/form.h"
#include "core/sox/logger.h"
#include "common/core/ibase.h"
#include "packet.h"


using namespace core;
using namespace sdaemon;

const uint32_t TIMER_CHECK_CONN_TIMEOUT = 1000 * 5;
const uint32_t CONN_TIMEOUT = 60;  //链路超时事件

BEGIN_FORM_MAP(FPayServerCore)
    ON_LINK(NodeRegisterReq, &FPayServerCore::onNodeRegister)
    ON_LINK(PingReq, &FPayServerCore::onPing)
    ON_LINK(PayReq, &FPayServerCore::onPay)
    ON_LINK(ConfirmReq, &FPayServerCore::onConfirm)
    ON_LINK(SyncBlocksReq, &FPayServerCore::onSyncBlocks)
    ON_LINK(GetRelativesReq, &FPayServerCore::onGetRelatives)
END_FORM_MAP()

		
FPayServerCore::FPayServerCore(IServerCallbackIf* If):
	timer_check_conn_timeout(this),
	proxy(If)
{
	timer_check_conn_timeout.start(TIMER_CHECK_CONN_TIMEOUT);
}

FPayServerCore::~FPayServerCore()
{
}

//底层往上抛出的链路断开事件
void FPayServerCore::eraseConnect(IConn *conn)
{
	uint32_t cid = conn->getConnId();

	//删除对应链路的子节点信息
	map<uint32_t,child_info_t>::iterator it;
	it = child_infos.find(cid);
	if( it != child_infos.end() ) { 
		address_2_connid.erase(it->address); 
	}
	child_infos.erase(cid);

	//删除底层链路信息
	MultiConnManagerImp::eraseConnect(conn);
}


//子节点注册请求处理
void FPayServerCore::onNodeRegister(NodeRegisterReq *reg, IConn* c)
{
	//做基本数据签名验证
	if( reg->signValidate() ) {
	    //抛给上层模块
	    proxy->onReceiveRegister(*reg);
	    //保存连接信息
	    child_info_t child(c->getConnId(),reg.address,1); 
	    child_infos[child.cid] = child;
        //保存子节点和连接id对应关系
        address_2_connid[reg.addres] = child.cid;
	} else { //签名无效
		//直接断开连接，并且将IP加入黑名单
		eraseConnectById(c->getConnId());
		//todo
	}
}


//子节点心跳
void FPayServerCore::onPing(PingReq * ping, IConn* c)
{
	if(ping->signValidate() ) {
	    connHeartbeat(c->getConnId());
	} else {
		//直接断开连接，并且将IP加入黑名单
		eraseConnectById(c->getConnId());
	}
}


//验证链路
bool FPayServerCore::vaildChild(uint32_t cid) 
{
	map<uint32_t,child_info_t>::iterator it;
	it = child_infos.find(cid);
	if( it != child_infos.end() ) { 
		return !(it->second.vaild < 3);
	}
	
	return false;
}
		

void FPayServerCore::response(uint32_t uri,sox::Marshallable& marshal)
{
	Sender rsp_send;
	rsp_send.marshall(uri,marshal);
	rsp_send.endPack();
	
	dispatchById( c->getConnId(), rsp_send );
}

//受理支付请求
void FPayServerCore::onPay(PayReq* pay,core::IConn* c)
{

	//todo 做基本的数据签名验证
	bool signVaild = pay->signValidate();
	if( signVaild ) {
		//看连接是不是已经验证成功
        if( vaildChild(c->getConnId()) ) {
			//抛给上层模块处理
			proxy->onReceivePay(*pay);
		} else {
            PayRes res;
			res.resp_code = 2001; //链路还没有验证完成或者未验证,不接受任何请求
			response(PayRes::uri,res);
		}
        connHeartbeat(c->getConnId());

	}else {
		//直接断开连接，并且将IP加入黑名单
		eraseConnectById(c->getConnId());
	}

}

//更新链路心跳时间戳
void FPayServerCore::childHeartbeat(uint32_t cid)
{
	map<uint32_t,child_info_t>::iterator it;
	it = child_infos.find(cid);
	if( it != child_infos.end() ) { 
		it->second.last_ping_time = time(NULL);
	}
}


//定时检测超时子节点
bool FPayServerCore::checkChildTimeout()
{
	time_t now = time(NULL);
	set<uint32_t> bad_conns;
	map<uint32_t,child_info_t>::iterator cit;
	for( cit = child_infos.begin(); cit != child_infos.end(); ++cit )
	{
		if( now - cit->last_ping_time > CONN_TIMEOUT )
		{	
			bad_conns.insert( cit->cid );
			address_2_conn.erase(cit->address);
		}

	}
	set<uint32_t>::iterator bad_it;
	for( bad_it = bad_conns.begin(); bad_it != bad_conns.end(); ++bad_it )
	{
		eraseConnectById(*bad_it);	
	}
	return true;
}

