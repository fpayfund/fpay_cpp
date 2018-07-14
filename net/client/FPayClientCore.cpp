#include "FPayClientCore.h"
#include "common/core/form.h"
#include "core/sox/sockethelper.h"
#include "core/sox/logger.h"
#include "core/sox/udpsock.h"
#include "core/corelib/AbstractConn.h"
#include "core/corelib/InnerConn.h"
#include "doublelink_protocol.h"

using namespace core;
using namespace sox;
using namespace sdaemon;
using namespace protocol;

const uint32_t TIMER_LINK_CHECK  = 1000 * 1;
const uint32_t TIMER_PING   = 1000 * 2;
const uint32_t TIMER_CHECK_CONN_TIMEOUT_INTERVAL         = 1000 * 5;
//定时检测根节点选举时机的时间间隔，9秒
const uint32_t TIMER_CHECK_ROOT_SWITCH_INTERVAL          = 1000 * 9;
//定时检测包是否完整的时间间隔，3秒
const uint32_t TIMER_CHECK_BLOCKS_FULL_INTERVAL          = 1000 * 3;
const uint32_t TIMER_CHECK_BEST_ROUTE_INTERVAL           = 1000 * 11;


BEGIN_FORM_MAP(FPayClientCore)
		    ON_LINK(NodeRegisterRes, &FPayClientCore::onNodeRegisterRes) 
		    ON_LINK(PayRes, &FPayClientCore::onPayRes) 
		    ON_LINK(SyncBlocksRes, &FPayClientCore::onSyncBlocksRes)
		    ON_LINK(GetRelativesRes, &FPayClientCore::onGetRelativesRes)
		    ON_LINK(BlockBroadcast, &FPayClientCore::onBlockBroadcast)
END_FORM_MAP()

FPayClientCore::FPayClientCore(IClientCallbackIf* cif,IClientTimerIf* tif):
    init_flag(0),
	tree_level(255),
	timer_link_check(this),
	timer_ping(this),
	net_proxy(cif),
	timer_proxy(tif)
{

}

FPayClientCore::~FPayClientCore(){

}

void FPayClientCore::Init(
			const Byte32& address,
			const Byte32& public_key,
			const Byte32& private_key,
			uint64_t last_block_idx,
			const Byte32& last_block_id, 
			const Byte32& first_root_addr);
{
	local_address = address;
	local_public_key = public_key;
	local_private_key = private_key;
	local_last_block_idx = last_block_idx;
	local_last_block_id = last_block_id;
	local_first_root_address = first_root_addr;
}

//链接错误
void FPayClientCore::onError(int ev, const char *msg, IConn *conn)
{
	log( Info, "FPayClientCore::onError, conn:%d to node error:%s", 
				conn->getConnId(), msg );
	MultiConnManagerImp::onError(ev,msg,conn);
}


//链接断开
void FPayClientCore::eraseConnect(IConn *conn)
{
		
	log( Warn, "FPayClientCore::eraseConnect, delete conn(%d) to proxy(%s)",
				conn->getConnId(), proxy.c_str());
	MultiConnManagerImp::eraseConnect(conn);
}


void FPayClientCore::send(uint32_t cid, uint32_t uri, sox::Marshallable& marshal)
{
	Sender rsp_send;
	rsp_send.marshall(uri,marshal);
	rsp_send.endPack();
	dispatchById( cid, rsp_send );
}


void FPayClientCore::startSV( const set<node_info_t,compByte32>& init_nodes )
{
	log(Info, "FPayClientCore::startSV,init up nodes size:%Zu", init_nodes.size() );

	if( init_nodes.size() > 0 ){

		backup_node_infos = init_nodes;
		set<node_info_t,compByte32>::iterator it = init_nodes.begin();
        //取一个地址作为当前的父节点
		current_parent_node.address = it->address;
		current_parent_node.ip = it->ip;
		current_parent_node.port = it->port;

		//连接该父节点
		IConn* conn = connectNode(it->ip,it->port);

		NodeRegisterReq reg;
		reg.address = local_address;
		reg.public_key = local_public_key;
		reg.private_key = local_private_key;
		reg.last_block_idx = local_last_block_idx;
		reg.last_block_id = local_last_block_id;
		reg.first_root_address = local_first_root_address;
		reg.genSign();

		//发送网络注册请求
		send(conn->getConnId(),NodeRegisterReq::uri,reg);

		    //启动链路检测定时器
		timer_link_check.start(TIMER_LINK_CHECK);
		//启动ping定时器
	    timer_ping.start(TIMER_PING);
	} else { //本节点为根节点
		tree_level = 0;
		init_flag = 0xFFFFFFFFFFFFFFFF; //初始化完成
		//启动根节点选举定时器
        timer_check_root_switch(TIMER_CHECK_ROOT_SWITCH_INTERVAL);
	}
}


void FPayClientCore::onNodeRegisterRes(NodeRegisterRes* res, IConn* c)
{
	if( res->signValidate() ) {
		
		if( res->address == current_parent_node.address ) {
			init_flag | 0x0000000000000001; //父节点验证完毕
			tree_level = res->tree_level + 1; //设置本节点的tree level

			//判断区块数是否相同
			if( local_last_block_idx == res->last_block_idx && 
						local_last_block_id == res->last_block_id &&
						local_first_root_address = res->first_root_address) {
				init_flag | 0x0000000000000002 //区块数 is full
			} else {
				//发送同步区块
				SyncBlocksReq sync;
				sync.public_key = local_public_key;
				sync.private_key = local_private_key;
				sync.from_block_id = local_last_block_id;
				sync.from_block_idx = local_last_block_idx;
				sync.block_num = 2;
				sync.genSign();
				//发送同步请求
				send(c->getConnId(),SyncBlocksReq::uri,sync);
			}

		} else { //切换父节点
			//todo
		}

	}else{
		//断开连接
		eraseConnectById(c->getConnId());

	}

}


void FPayClientCore::onSyncBlocksRes(SyncBlocksRes* res, IConn* c)
{
	if( res->signValidate() ) {
		
		for( uint32_t i = 0; i < res->blocks.size(); i++ ) {	
			net_proxy->onReceiveSyncBlock(res->blocks[i]);
			local_last_block_idx = res->blocks[i].idx;
			local_last_block_id = res->blocks[i].id;
		}
		if( res->continue_flag == 1 ) { //还有区块没有同步，继续发送同步请求
			//发送同步区块
			SyncBlocksReq sync;
			sync.public_key = local_public_key;
			sync.private_key = local_private_key;
			sync.from_block_id = local_last_block_id;
			sync.from_block_idx = local_last_block_idx;
			sync.block_num = 2;
			sync.genSign();
			//发送同步请求
			send(c->getConnId(),SyncBlocksReq::uri,sync);		
		} else {
			init_flag | 0x0000000000000002 //区块数 is full
		}

	}else {
		//断开连接
		eraseConnectById(c->getConnId());
	}
}


IConn* FPayClientCore::onPayRes(PayRes* res, IConn* c)
{
	if( res->signValidate() ) {
        net_proxy->onReceivePayRes(res);
	}else{
		//断开连接
		eraseConnectById(c->getConnId());
	}

}


IConn *FPayClientCore::connectNode( const string& ip, uint16_t port)
{
	log(Info, "FPayClientCore::connectNode,host:[%s,%u]", ip.c_str(), port);
	return createClientConn(ip.data(), port, handler, this);

}


bool FPayClientCore::linkCheck()
{
	log( Info, "FPayClientCore::linkCheck, check link wheath alive" );

	//connectAllProxy();
	return true;
}


bool FPayClientCore::ping()
{
	log( Info, "FPayClientCore::ping to all up node");
	return true;
}
