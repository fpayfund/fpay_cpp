#include "FPayServerCore.h"
#include "common/core/form.h"
#include "core/sox/logger.h"
#include "common/core/ibase.h"
#include "common/packet.h"
#include "FPayBlockSerivce.h"
#include "FPayTXService.h"
#include "FPayClientCore.h"

using namespace core;

const uint32_t TIMER_CHECK_CONN_TIMEOUT_INTERVAL         = 1000 * 5;
//定时打包区块的时间间隔  1秒
const uint32_t TIMER_CHECK_PRODUCE_BLOCK_INTERVAL        = 1000 * 1;
const uint32_t CONN_TIMEOUT = 60;  //链路超时事件

BEGIN_FORM_MAP(FPayServerCore)
    ON_LINK(NodeRegisterReq, &FPayServerCore::onNodeRegister)
    ON_LINK(PingReq, &FPayServerCore::onPing)
    ON_LINK(PayReq, &FPayServerCore::onPay) 
    ON_LINK(SyncBlocksReq, &FPayServerCore::onSyncBlocks)
    ON_LINK(GetRelativesReq, &FPayServerCore::onGetRelatives)
END_FORM_MAP()

FPayServerCore* FPayServerCore::instance = NULL;

void FPayServerCore::init(const Byte20& address,
					const Byte32& public_key,
					const Byte32& private_key)
{
	local_address = address;
	local_public_key = public_key;
	local_private_key = private_key;
}


FPayServerCore::FPayServerCore():
	timer_check_child_timeout(this),
	timer_check_produce_block(this)
{
	timer_check_child_timeout.start(TIMER_CHECK_CONN_TIMEOUT_INTERVAL);
	timer_check_produce_block.start(TIMER_CHECK_PRODUCE_BLOCK_INTERVAL);
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
	    
        //存储对应的广播监听地址
		//todo

		//回应
		NodeRegisterRes res;
		res.public_key = local_public_key;
        res.tree_level = FPayClientCore::getInstance()->getTreeLevel();
		res.genSign(local_private_key);
	    response(c->getConnId(),NodeRegisterRes::uri,res);
	        
		//保存连接信息
	    child_info_t child(c->getConnId(),reg->address); 
	    child_infos[child.cid] = child;
		
	} else { //签名无效
		//直接断开连接
		eraseConnectById(c->getConnId());	
	}
}


//子节点心跳
void FPayServerCore::onPing(PingReq * ping, IConn* c)
{
	if(ping->signValidate() ) {
	    connHeartbeat(c->getConnId());
		PingRes res;
		res.public_key = local_public_key;
		res.tree_level = FPayClientCore::getInstance()->getTreeLevel();
	    //调用区块模块获取最后一个区块id idx
		block_info_t block;
		if( FPayBlockService::getInstance()->getLastBlock(block) ) {
			res.last_block_id = block.id;
	        res.last_block_idx = block.idx;
		}
		res.genSign(local_private_key);
		response(c->getConnId(),PingRes::uri,res);
	} else {
		//直接断开连接
		eraseConnectById(c->getConnId());
	}
}

//回应
void FPayServerCore::response(uint32_t cid, uint32_t uri, sox::Marshallable& marshal)
{
	Sender rsp_send;
	rsp_send.marshall(uri,marshal);
	rsp_send.endPack();
	
	dispatchById( cid, rsp_send );
}

//受理支付请求
void FPayServerCore::onPay(PayReq* pay,core::IConn* c)
{

	//做基本的数据签名验证
	if( pay->signValidate() ) {
        //确定角色
		bool root = (FPayClientCore::getInstance()->getTreeLevel() == 0);
		if( !root ) {
		    //给pay增加确认信息
		    confirmation_info_t confirm;
		    confirm.current_address = local_address;
		    confirm.public_key = local_public_key;
		    confirm.next_address = FPayClientCore::getInstance()->getParentAddress();
			confirm.genSign(local_private_key);
			pay->payment.confirmations.push_back(confirm);
		}
		//调用支付模块，计算支付是否ok
		bool pay_ret = FPayTxService::getInstance()->handlePayment(pay->payment);

		PayRes res;
		res.resp_code = pay_ret? 0 : 10001; //如果不ok，则返回10001
		res.public_key = local_public_key;
		res.id = pay->payment.pay.id;
		res.genSign(local_private_key);	
		response(c->getConnId(),PayRes::uri,res);

		//如果不是根节点，都要转发支付请求
		if( !root ) {
			FPayClientCore::getInstance()->dispatchPay(*pay);
		}
        connHeartbeat(c->getConnId());
	}else {
		//直接断开连接，并且将IP加入黑名单
		eraseConnectById(c->getConnId());
	}

}

//同步区块请求
void FPayServerCore::onSyncBlocks(SyncBlocksReq* sync, core::IConn* c)
{
	if( sync->signValidate() ) {
		//调用区块模块，并传参from id返回区块，是否有后续区块标志位		
		SyncBlocksRes res;
		res.continue_flag = 1;
		Byte32 from_block_id = sync->from_block_id;
		for( uint32_t i = 0; i < sync->block_num; i++ ) {
			block_info_t block;
			bool ret = FPayBlockService::getInstance()->getBlock(from_block_id,block);
			if( ret == false ){
				res.continue_flag = 0;
				break;
			}
			res.blocks.push_back(block);
			from_block_id = block.id;
		}
		res.public_key = local_public_key;
	    res.genSign(local_private_key);	
		response(c->getConnId(),SyncBlocksRes::uri,res);
		connHeartbeat(c->getConnId());
	}else {
		//直接断开连接，并且将IP加入黑名单
		eraseConnectById(c->getConnId());
	}
}

//获取亲属节点请求
void FPayServerCore::onGetRelatives(GetRelativesReq* req, core::IConn* c)
{
	if( req->signValidate() ){
		GetRelativesRes res;
		res.public_key = local_public_key;
		res.genSign(local_private_key);
		response(c->getConnId(),GetRelativesRes::uri,res);
		connHeartbeat(c->getConnId());
	}else {
		//直接断开连接
		eraseConnectById(c->getConnId());
	}

}


//更新链路心跳时间戳
void FPayServerCore::connHeartbeat(uint32_t cid)
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
		if( now - cit->second.last_ping_time > CONN_TIMEOUT )
		{	
			bad_conns.insert( cit->second.cid );	
		}

	}
	set<uint32_t>::iterator bad_it;
	for( bad_it = bad_conns.begin(); bad_it != bad_conns.end(); ++bad_it )
	{
		eraseConnectById(*bad_it);	
	}
	return true;
}


//区块打包定时器
bool FPayServerCore::checkProduceBlock()
{
	//调用区块模块生成区块
	//todo
	block_info_t block;
	broadcastBlock(block);
    return true;
}


//区块广播
void FPayServerCore::broadcastBlock(const block_info_t & block)
{
	BlockBroadcast broadcast;
	broadcast.public_key = local_public_key;
	broadcast.block = block;
	broadcast.genSign(local_private_key);
	map<uint32_t,child_info_t>::iterator cit;
	for( cit = child_infos.begin(); cit != child_infos.end(); ++cit )
	{
		response(cid,BlockBroadcast::uri,broadcast);
	}
}

