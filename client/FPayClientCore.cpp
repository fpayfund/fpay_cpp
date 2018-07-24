#include "FPayClientCore.h"
#include "common/core/form.h"
#include "core/sox/sockethelper.h"
#include "core/sox/logger.h"
#include "core/sox/udpsock.h"
#include "core/corelib/AbstractConn.h"
#include "core/corelib/InnerConn.h"

using namespace core;
using namespace sox;

//广播监听IP
DECLARE_string(broadcast_ip)
//广播监听POrt
DECLARE_int(broadcast_port)

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
			const Byte20& address,
			const Byte32& public_key,
			const Byte32& private_key)
{
	init_flag = 0;
	tree_level = 255;
    local_address = address;
	local_public_key = public_key;
	local_private_key = private_key;
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

//报文发送函数
void FPayClientCore::send(uint32_t cid, uint32_t uri, sox::Marshallable& marshal)
{
	Sender send;
	send.marshall(uri,marshal);
	send.endPack();
	dispatchById( cid, send );
}


//注册进入网络
void FPayClientCore::registerIn(const string& ip, uint16_t port)
{
	//连接该up node
	IConn* conn = connectNode(ip,port);
	
	NodeRegisterReq reg;
	reg.address = local_address;
	reg.public_key = local_public_key;
	reg.ip = FLAG_broadcast_ip;
	reg.port = FLAG_broadcast_port;
	reg.genSign(local_private_key);
	//发送网络注册请求
	send(conn->getConnId(),NodeRegisterReq::uri,reg);

    //加入本地连接映射	
	up_conn_info_t up_conn_info;
	up_conn_info.cid = conn->getConnId();
	up_conn_info.ip = ip;
	up_conn_info.port = port;
	up_conn_infos[conn->getConnId()] = up_conn_info;

}

//客户端模块启动
void FPayClientCore::start( const vector< pair<string,uint16_t> >& init_nodes )
{
	log(Info, "FPayClientCore::start,init up nodes size:%Zu", init_nodes.size() );

	if( init_nodes.size() > 0 ){

		//注册进网络
		vector< pair<string,uint16_t> >::iterator it = init_nodes.begin();
		for( ;it != init_nodes.end(); ++it ) {
			registerIn(it->first,it->second);
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
	sync.from_block_id = from_block_id;
	sync.from_block_idx = from_block_idx;
	sync.block_num = count;
	sync.genSign(local_private_key);
	//发送同步请求
	send(cid,SyncBlocksReq::uri,sync);
}


void FPayClientCore::onNodeRegisterRes(NodeRegisterRes* res, IConn* c)
{
	if( res->signValidate() ) {
	    if( init_flag & BIT_CLIENT_INIT_REGISTER_OVER != BIT_CLIENT_INIT_REGISTER_OVER ) {
			init_flag = init_flag | BIT_CLIENT_INIT_REGISTER_OVER;
		}

		//如果此节点返回的角色比本节点高出两个及以上，则将父节点切入当前节点地址
		if( tree_level > res->tree_level + 1 ) {
			tree_level = res->tree_level + 1; //设置本节点的tree level
            current_parent_address = res->address; //更改父节点地址	
		}

		up_conn_infos[c->getConnId()].address = res->address;
	
		//将该节点加入备份节点管理
		node_info_t node;
		node.address = res->address;
		node.ip = up_conn_infos[c->getConnId()].ip;
		node.port = up_conn_infos[c->getConnId()].port;
        backup_node_infos.insert(node);


		//调用区块模块获取当前的最后一个区块id idx
        //todo
		Byte32 from_block_id;
		uint64_t from_block_id;
        uint8_t count = 2;
		//发送同步区块请求
		syncBlocks(c->getConnId(), from_block_id, from_block_idx,count);

	}else{
		//断开连接
		eraseConnectById(c->getConnId());
	}

}

//区块同步回应
void FPayClientCore::onSyncBlocksRes(SyncBlocksRes* res, IConn* c)
{
	if( res->signValidate() ) {
		
		for( uint32_t i = 0; i < res->blocks.size(); i++ ) {
			//调用区块模块将block存储:
		    //todo	
		}	
		//判断是否有后续区块
		if(res->continue_flag == 1) {
			//调用区块模块获取最后的区块id和idx
			//todo
			Byte32 from_block_id;
			uint64_t from_block_id;
			uint8_t count = 2;
			//发送同步区块请求
			syncBlocks(c->getConnId(), from_block_id, from_block_idx,count);
		}else {
			if( init_flag & BIT_CLIENT_INIT_BLOCKS_FULL != BIT_CLIENT_INIT_BLOCKS_FULL ) {
				init_flag = init_flag | BIT_CLIENT_INIT_BLOCKS_FULL; //表示区块同步完成
			}
		}	
	}else {
		//断开连接
		eraseConnectById(c->getConnId());
	}
}

//支付回应
void FPayClientCore::onPayRes(PayRes* res, IConn* c)
{
	if( res->signValidate() ) {
        
		//todo
	}else{
		//断开连接
		eraseConnectById(c->getConnId());
	}

}

//ping 回应
void FPayClientCore::onPingRes(PingRes* res, IConn* c)
{
	if( res->signValidate() ) {
		if( tree_level  > res->tree_level + 1 ) { //换父节点
            tree_level = res->tree_level + 1;
			current_parent_address = up_conn_infos[c->getConnId()].address;
		}

	}else {
		//断开
		eraseConnectById(c->getConnId());
	}

}


//收到下发的区块广播
void FPayClientCore::onBlockBroadcast(BlockBroadcast* broadcast, IConn* c)
{
	if( broadcast->signValidate() ) {
		//调用区块模块，计算UTXO，并将区块存储起来
		//todo
		FPayServerCore::broadcastBlock(broadcast->block);
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
			registerIn(it->ip,it->port);
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
		req.public_key = local_public_key;
		req.tree_level = tree_level;
		
		req.genSign(local_private_key);
		send(it->second.cid,PingReq::uri,req);
	}
	return true;
}


//根节点切换定时器
bool FPayClientCore::checkRootSwitch()
{

	return true;
}


//区块完整性检查定时器
bool FPayClientCore::checkBlocksFull()
{

	return true;
}


//路由优化检查定时器
bool FPayClientCore::checkBestRoute()
{

	return true;
}


//根据地址找到连接
uint32_t FPayClientCore::findConnByAddress(const Byte20& address)
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
int FPayClientCore::dispatchPay( const PayReq& pay )
{
	uint32_t cid = findConnByAddress(current_parent_address);
	if( cid != 0 ) {
		send(cid,PayReq::uri,pay);
		return 0;
	}
	return -1;
}


