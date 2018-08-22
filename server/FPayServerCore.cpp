#include "FPayServerCore.h"
#include "common/core/form.h"
#include "core/sox/logger.h"
#include "common/core/ibase.h"
#include "common/packet.h"
#include "ecc_helper.h"
#include "FPayBlockService.h"
#include "FPayTXService.h"
#include "FPayClientCore.h"
#include "FPayConfig.h"

using namespace core;

const uint32_t TIMER_CHECK_CONN_TIMEOUT_INTERVAL         = 1000 * 5;
//定时打包区块的时间间隔  1秒
const uint32_t TIMER_CHECK_PRODUCE_BLOCK_INTERVAL        = 1000 * 1000;
const uint32_t CONN_TIMEOUT = 60;  //链路超时事件

BEGIN_FORM_MAP(FPayServerCore)
    ON_LINK(NodeRegisterReq, &FPayServerCore::onNodeRegister)
    ON_LINK(PingReq, &FPayServerCore::onPing)
    ON_LINK(PayReq, &FPayServerCore::onPay) 
    ON_LINK(SyncBlocksReq, &FPayServerCore::onSyncBlocks)
    ON_LINK(GetRelativesReq, &FPayServerCore::onGetRelatives)
END_FORM_MAP()

FPayServerCore* FPayServerCore::_instance = NULL;

void FPayServerCore::init(const Byte20& address,
					const Byte32& public_key,
					const Byte32& private_key)
{
	_localAddress = address;
	_localPublicKey = public_key;
	_localPrivateKey = private_key;


    Byte32 block_id = FPayConfig::getInstance()->initBlockId;

	fprintf(stderr,"FPayServerCore::init,bigen\n");

	Byte32 last_block_id;
	while (!block_id.isEmpty()) {
        block_info_t block;
		if( FPayBlockService::getInstance()->getBlock(block_id, block) ) {
		    FPayTXService::getInstance()->updateBalanceByBlock(block);
		}
		last_block_id = block_id;
		block_id  = block.next_id;	
	}
	FPayBlockService::getInstance()->storeLastBlockId(last_block_id);
	fprintf(stderr,"FPayServerCore::init,end\n");
}


FPayServerCore::FPayServerCore():
	_timerCheckChildTimeout(this),
	_timerCheckCreateBlock(this)
{
	_timerCheckChildTimeout.start(TIMER_CHECK_CONN_TIMEOUT_INTERVAL);
	_timerCheckCreateBlock.start(TIMER_CHECK_PRODUCE_BLOCK_INTERVAL);
}


FPayServerCore::~FPayServerCore()
{
}


//底层往上抛出的链路断开事件
void FPayServerCore::eraseConnect(IConn *conn)
{
	fprintf(stderr,"FPayServerCore::eraseConnect\n");
	uint32_t cid = conn->getConnId();	
	_childInfos.erase(cid);
	//删除底层链路信息
	MultiConnManagerImp::eraseConnect(conn); 
}


//子节点注册请求处理
void FPayServerCore::onNodeRegister(NodeRegisterReq *reg, IConn* c)
{
	fprintf(stderr,"FPayServerCore::onNodeRegister\n");
	//做基本数据签名验证
	if( reg->signValidate() ) {
	    
		//回应
		NodeRegisterRes res;
		res.public_key = _localPublicKey;
        res.tree_level = FPayClientCore::getInstance()->getTreeLevel();
		res.genSign(_localPrivateKey);
	    response(c->getConnId(),NodeRegisterRes::uri,res);
	        
		//保存连接信息
	    child_info_t child(c->getConnId(),reg->address); 
	    _childInfos[child.cid] = child;
		
	} else { //签名无效
		//直接断开连接
		eraseConnectById(c->getConnId());	
	}
}


//子节点心跳
void FPayServerCore::onPing(PingReq * ping, IConn* c)
{
    //fprintf(stderr,"FPayServerCore::onPing\n");
	if(ping->signValidate() ) {
	    connHeartbeat(c->getConnId());
		
		PingRes res;
		res.public_key = _localPublicKey;
		res.tree_level = FPayClientCore::getInstance()->getTreeLevel();
	   
		block_info_t block;
		if( FPayBlockService::getInstance()->getLastBlock(block) ) {
			res.last_block_id = block.id;
	        res.last_block_idx = block.idx;
		}
		res.genSign(_localPrivateKey);
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

		PayRes res;

		uint64_t balance = 0;
		res.resp_code = 10001;
        bool get_balance_ret = FPayTXService::getInstance()->getBalance(pay->payment.pay.from_address,balance); 
		
		fprintf(stderr,"FPayServerCore::onPay,ret:%s,balance:%lu,pay amount:%lu\n",get_balance_ret? "success":"failed",balance,pay->payment.pay.amount);
		if (!get_balance_ret || (balance > pay->payment.pay.amount) ) {
			//给pay增加确认信息
			confirmation_info_t confirm;
			confirm.current_address = _localAddress;
			confirm.public_key = _localPublicKey;
			confirm.next_address = FPayClientCore::getInstance()->getParentAddress();
			confirm.genSign(_localPrivateKey);
			pay->payment.confirmations.push_back(confirm);

			if ( FPayClientCore::getInstance()->getTreeLevel() == 0 ) {
				bool pay_ret = FPayTXService::getInstance()->handlePayment(pay->payment);
		        res.resp_code = pay_ret ? 0 : 10001;
				fprintf(stderr,"FPayServerCore::onPay, handlePayment failed\n");
			} else {
				FPayClientCore::getInstance()->dispatchPay(*pay);
		        res.resp_code = 0;
			}
		}

		res.public_key = _localPublicKey;
		res.id = pay->payment.pay.id;
		res.genSign(_localPrivateKey);	
		response(c->getConnId(),PayRes::uri,res);

		connHeartbeat(c->getConnId());
	}else {
		//直接断开连接，并且将IP加入黑名单
		eraseConnectById(c->getConnId());
	}

}

//同步区块请求
void FPayServerCore::onSyncBlocks(SyncBlocksReq* sync, core::IConn* c)
{
	fprintf(stderr,"FPayServerCore::onSyncBlocks\n");	
	if( sync->signValidate() ) {	
		//调用区块模块，并传参from id返回区块，是否有后续区块标志位		
		SyncBlocksRes res;
		res.resp_code = 0;
		res.continue_flag = 1;
		Byte32 from_block_id = sync->from_block_id;
		if( from_block_id.isEmpty() ) {
			block_info_t block;

			if( FPayBlockService::getInstance()->getInitBlock(block) ) {
				from_block_id = block.id;
			}
		} else {
			block_info_t block;
			if (FPayBlockService::getInstance()->getBlock(from_block_id,block)) {
				from_block_id = block.next_id;
			}else {
				res.resp_code = 10001;
			}
		}
	
		if( res.resp_code == 0 ) {
			for( uint32_t i = 0; i < sync->block_num; i++ ) {
				block_info_t block;
				bool ret = FPayBlockService::getInstance()->getBlock(from_block_id,block);
				if( ret == false ){
					res.continue_flag = 0;
					break;
				}
				res.blocks.push_back(block);
				from_block_id = block.next_id;
			}
		}
		res.public_key = _localPublicKey;
	    res.genSign(_localPrivateKey);
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
	fprintf(stderr, "FPayServerCore::onGetRelatives\n");
	if( req->signValidate() ){
		GetRelativesRes res;
		res.public_key = _localPublicKey;
		res.genSign(_localPrivateKey);
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
	it = _childInfos.find(cid);
	if( it != _childInfos.end() ) { 
		it->second.last_ping_time = time(NULL);
	}
}


//定时检测超时子节点
bool FPayServerCore::checkChildTimeout()
{
	time_t now = time(NULL);
	set<uint32_t> bad_conns;
	map<uint32_t,child_info_t>::iterator cit;
	for( cit = _childInfos.begin(); cit != _childInfos.end(); ++cit )
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
bool FPayServerCore::checkCreateBlock()
{
	if( FPayClientCore::getInstance()->getTreeLevel() == 0 ) {
		fprintf(stderr,"FPayServerCore::checkProduceBlock,create block\n");
		//调用区块模块生成区块
		block_info_t block;
		block.root_address = _localAddress;
		block.public_key = _localPublicKey;
		if ( FPayBlockService::getInstance()->createBlock(block,_localPrivateKey) ) {
			fprintf(stderr,"FPayServerCore::checkCreateBlock,create block ok,block idx:%lu\n",block.idx);
			//广播
			broadcastBlock(block);
		} else {
			fprintf(stderr,"FPayServerCore::checkCreateBlock,create block failed\n");
		}
	}
    return true;
}


//区块广播
void FPayServerCore::broadcastBlock(const block_info_t & block)
{
	BlockBroadcast broadcast;
	broadcast.public_key = _localPublicKey;
	broadcast.block = block;

	map<uint32_t,child_info_t>::iterator cit;
	for( cit = _childInfos.begin(); cit != _childInfos.end(); ++cit )
	{
		response(cid,BlockBroadcast::uri,broadcast);
	}
}

