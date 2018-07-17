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
		    ON_LINK(PingRes,&FPayClientCore::onPingRes)
END_FORM_MAP()

FPayClientCore *FPayClientCore::instance = NULL;
void FPayClientCore::init(
			const Byte32& address,
			const Byte32& public_key,
			const Byte32& private_key,
			IClientCallbackIf* cif,
			IClientTimerIf* tif)
{
	init_flag = 0;
	tree_level = 255;
    local_address = address;
	local_public_key = public_key;
	local_private_key = private_key;
	net_proxy = cif;
	timer_proxy = tif;
}


FPayClientCore::FPayClientCore():
	timer_link_check(this),
	timer_ping(this),
	timer_check_root_switch(this),
	timer_check_blocks_full(this),
	timer_check_best_route(this) 
{

}

FPayClientCore::~FPayClientCore(){

}


//链接错误
void FPayClientCore::onError(int ev, const char *msg, IConn *conn)
{
	log( Info, "FPayClientCore::onError, conn:%d to node error:%s", 
				conn->getConnId(), msg );
	MultiConnManagerImp::onError(ev,msg,conn);
}


//选举新的父节点
void FPayClientCore::voteParentNode()
{
	map<uint32_t,up_conn_info_t>::iterator it;
	for( it = up_conn_infos.begin(); it != up_conn_infos.end(); it++ ) {
		current_parent_address = it->second.node.address;
		break;
	}
}


//链接断开
void FPayClientCore::eraseConnect(IConn *conn)
{
	Byte32 address;
	map<uint32_t,up_conn_info_t>::iterator it;
	for( it = up_conn_infos.begin(); it != up_conn_infos.end(); ) {
		if( conn->getConnId() == it->first ) {
			address = it->second.node.address;
			log( Warn, "FPayClientCore::eraseConnect, delete conn(%d) to up node(%s,%u)",
				conn->getConnId(), it->second.node.ip.c_str(),it->second.node.port);
			up_conn_infos.erase(it);
			break;
		}
	}

	//如果断开连接的是当前父节点,选取一个作为新的父节点
	if( address == current_parent_address ) {
		voteParentNode();
	}
	MultiConnManagerImp::eraseConnect(conn);
}


void FPayClientCore::send(uint32_t cid, uint32_t uri, sox::Marshallable& marshal)
{
	Sender rsp_send;
	rsp_send.marshall(uri,marshal);
	rsp_send.endPack();
	dispatchById( cid, rsp_send );
}


void FPayClientCore::registerUpNode(const node_info_t& up_node_info)
{
	//连接该up node
	IConn* conn = connectNode(up_node_info->ip,up_node_info->port);
	NodeRegisterReq reg;
	reg.address = local_address;
	reg.public_key = local_public_key;
	reg.private_key = local_private_key;
	reg.genSign();

	//发送网络注册请求
	send(conn->getConnId(),NodeRegisterReq::uri,reg);
}


void FPayClientCore::startSV( const set<node_info_t,nodeInfoCmp>& init_nodes )
{
	log(Info, "FPayClientCore::startSV,init up nodes size:%Zu", init_nodes.size() );

	if( init_nodes.size() > 0 ){

		backup_node_infos = init_nodes;

		set<node_info_t,nodeInfoCmp>::iterator it = init_nodes.begin();
		for( ;it != init_nodes.end(); ++it ) {
			registerUpNode(*it);
		}
        	
		//启动链路检测定时器
		timer_link_check.start(TIMER_LINK_CHECK);
		//启动ping定时器
	    timer_ping.start(TIMER_PING);
        //启动根节点切换时机检查定时器
		timer_check_root_switch(TIMER_CHECK_ROOT_SWITCH_INTERVAL);
		//启动检查区块同步定时器
		timer_check_blocks_full(TIMER_CHECK_BLOCKS_FULL_INTERVAL);
        //启动检查最佳路由定时器
		timer_check_best_route(TIMER_CHECK_BEST_ROUTE_INTERVAL);
	} else { //本节点为根节点
		tree_level = 0;
		init_flag = 0xFFFFFFFFFFFFFFFF; //初始化完成
		//启动根节点选举定时器
        timer_check_root_switch(TIMER_CHECK_ROOT_SWITCH_INTERVAL);
	}
}


void FPayClientCore::syncBlocks(uint32_t cid, 
			const Byte32& from_block_id, 
			uint64_t from_block_idx,
			uint8_t count)
{
	SyncBlocksReq sync;
	sync.public_key = local_public_key;
	sync.private_key = local_private_key;
	sync.from_block_id = from_block_id;
	sync.from_block_idx = from_block_idx;
	sync.block_num = count;
	sync.genSign();
	//发送同步请求
	send(cid,SyncBlocksReq::uri,sync);
}


void FPayClientCore::onNodeRegisterRes(NodeRegisterRes* res, IConn* c)
{
	if( res->signValidate() ) {
	    if( init_flag & BIT_CLIENT_INIT_REGISTER_OVER != BIT_CLIENT_INIT_REGISTER_OVER ) {
			init_flag = init_flag | BIT_CLIENT_INIT_REGISTER_OVER;
		}

		if( tree_level > res->tree_level + 1 ) {
			tree_level = res->tree_level + 1; //设置本节点的tree level
            current_parent_address = res->address; //更改父节点地址	
		}
		up_conn_info_t up_conn;
		up_conn.cid = c->getConnId();
		up_conn.address = res->address;
        up_conn_infos[c->getConnId()] = up_conn;


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
		}

	}else {
		//断开连接
		eraseConnectById(c->getConnId());
	}
}


void FPayClientCore::onPayRes(PayRes* res, IConn* c)
{
	if( res->signValidate() ) {
        net_proxy->onReceivePayRes(res);
	}else{
		//断开连接
		eraseConnectById(c->getConnId());
	}

}


void FPayClientCore::onPingRes(PingRes* res, IConn* c)
{


}


//收到下发的区块广播
void FPayClientCore::onBlockBroadcast(BlockBroadcast* broadcast, IConn* c)
{
	if( broadcast->signValidate() ) {
		net_proxy->onReceiveBlockBroadcast(broadcast->block);
	}else {
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

	set<node_info_t,nodeInfoCmp>::iterator it;
	for( it = backup_node_infos.begin(); it != backup_node_infos.end(); ++it ) {
		if( findConnByAddress(it->address) == 0 ) {
			registerUpNode(*it);
		}
	}

	return true;
}


bool FPayClientCore::ping()
{
	log( Info, "FPayClientCore::ping to all up node");

	map<uint32_t,up_node_info_t>::iterator it;
	for( it = up_node_infos.begin(); it != up_node_infos.end(); ++it ) {
		PingReq req;
		req.private_key = local_private_key;
		req.genSign();
		send(it->second.cid,PingReq::uri,req);
	}
	return true;
}


//根节点切换定时器
bool FPayClientCore::checkRootSwitch()
{
	timer_proxy->onTimerRootSwitchCheck();	
	return true;
}


//区块完整性检查定时器
bool FPayClientCore::checkBlocksFull()
{
	timer_proxy->onTimerBlocksFullCheck();
	return true;
}


//路由优化检查定时器
bool FPayClientCore::checkBestRoute()
{
	//timer_proxy->onTimerBestRouteCheck();
	return true;
}


//根据地址找到连接
uint32_t FPayClientCore::findConnByAddress(Byte32 address)
{
	map<uint32_t,up_node_info_t>::iterator it;
	for( it = up_node_infos.begin(); it != up_node_infos.end(); ++it ) {
		if( it->second.address == address ){
			return it->second.cid;
		}
	}
	return 0;
}


//转发支付请求
int FPayClientCore::dispatchPayReq( const PayReq& pay )
{
	uint32_t cid = findConnByAddress(current_parent_address);
	if( cid != 0 ) {
		send(cid,PayReq::uri,pay);
		return 0;
	}
	return -1;
}


//同步区块
int FPayClientCore::dispatchSyncBlocksReq( 
			const Byte32& from_block_id, 
			uint64_t from_block_idx, 
			uint8_t count )
{
	//轮询当前的所有up node
	static uint64_t roll = 0;
	roll++;

	//如果count为0，表示已经同步完成
	if( count == 0 ) {
		if( init_flag & BIT_CLIENT_INIT_BLOCKS_FULL != BIT_CLIENT_INIT_BLOCKS_FULL ) {
			init_flag = init_flag | BIT_CLIENT_INIT_BLOCKS_FULL; //表示区块同步完成
		}
		return 0;
	}

	uint64_t idx = roll % up_node_infos.size();
	map<uint32_t,up_node_info_t>::iterator it;
	uint64_t num = 0;
	for( it = up_node_infos.begin(); it != up_node_infos.end(); ++it ) {
		if( num == idx ){
			syncBlocks(it->second.cid,from_block_id,from_block_idx,count);
			break;
		}
		num++;
	}
	return 0;
}


