#include "FPayClientCore.h"
#include "common/core/form.h"
#include "core/sox/sockethelper.h"
#include "core/sox/logger.h"
#include "core/sox/udpsock.h"
#include "core/corelib/AbstractConn.h"
#include "core/corelib/InnerConn.h"
#include "server/FPayBlockService.h"
#include "server/FPayServerCore.h"
#include "server/FPayTXService.h"
using namespace core;
using namespace sox;

//广播监听IP
//DECLARE_string(broadcast_ip)
//广播监听POrt
//DECLARE_int(broadcast_port)

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

FPayClientCore *FPayClientCore::_instance = NULL;
void FPayClientCore::init(
			const Byte20& address,
			const Byte32& public_key,
			const Byte32& private_key)
{
	_initFlag = 0;
	_treeLevel = 255;
    _localAddress = address;
	_localPublicKey = public_key;
	_localPrivateKey = private_key;
}


FPayClientCore::FPayClientCore():
	_timerLinkCheck(this),
	_timerPing(this),
	_timerCheckRootSwitch(this),
	_timerCheckBlocksFull(this),
	_timerCheckBestRoute(this) 
{

}

FPayClientCore::~FPayClientCore(){

}


//链接错误
void FPayClientCore::onError(int ev, const char *msg, IConn *conn)
{
	fprintf( stderr, "FPayClientCore::onError, conn:%d to node error:%s\n", 
				conn->getConnId(), msg );
	
	log( Info, "FPayClientCore::onError, conn:%d to node error:%s", 
				conn->getConnId(), msg );
	//MultiConnManagerImp::onError(ev,msg,conn);
}


//选举新的父节点
void FPayClientCore::voteParentNode()
{
	log( Debug, "FPayClientCore::voteParentNode");
	map<uint32_t,up_conn_info_t>::iterator it;
	for( it = _upConnInfos.begin(); it != _upConnInfos.end(); it++ ) {
		_currentParentAddress = it->second.address;
		break;
	}
}


//链接断开
void FPayClientCore::eraseConnect(IConn *conn)
{
	fprintf(stderr,"FPayClientCore::eraseConnect\n");
	log( Info,"FPayClientCore::eraseConnect");

	Byte20 address;
	map<uint32_t,up_conn_info_t>::iterator it;
	for( it = _upConnInfos.begin(); it != _upConnInfos.end(); ) {
		if( conn->getConnId() == it->first ) {
			address = it->second.address;
			log( Warn, "FPayClientCore::eraseConnect, delete conn(%d) to up node(%s,%u)",
				conn->getConnId(), it->second.ip.c_str(),it->second.port);
			_upConnInfos.erase(it);
			break;
		}
	}

	//如果断开连接的是当前父节点,选取一个作为新的父节点
	if( address == _currentParentAddress ) {
		voteParentNode();
	}
	MultiConnManagerImp::eraseConnect(conn);
}

//报文发送函数
void FPayClientCore::send(uint32_t cid, uint32_t uri, const sox::Marshallable& marshal)
{
	Sender send;
	send.marshall(uri,marshal);
	send.endPack();
	dispatchById( cid, send );
}


//注册进入网络
void FPayClientCore::registerIn(const string& ip, uint16_t port)
{
	fprintf(stderr,"FPayClientCore::registerIn,ip:%s,port:%u\n",ip.c_str(),port);
	log( Info, "FPayClientCore::registerIn,ip:%s,port:%u",ip.c_str(),port);

	//连接该up node
	IConn* conn = connectNode(ip,port);
	
	NodeRegisterReq reg;
	reg.address = _localAddress;
	reg.public_key = _localPublicKey;
	//reg.ip = FLAG_broadcast_ip;
	//reg.port = FLAG_broadcast_port;
	reg.genSign(_localPrivateKey);
	//发送网络注册请求
	send(conn->getConnId(),NodeRegisterReq::uri,reg);

    //加入本地连接映射	
	up_conn_info_t up_conn_info;
	up_conn_info.cid = conn->getConnId();
	up_conn_info.ip = ip;
	up_conn_info.port = port;
	_upConnInfos[conn->getConnId()] = up_conn_info;

}

//客户端模块启动
void FPayClientCore::start( const vector< pair<string,uint16_t> >& init_nodes )
{
	log(Info, "FPayClientCore::start,init up nodes size:%Zu", init_nodes.size() );

	if( init_nodes.size() > 0 ){

		//注册进网络
		vector< pair<string,uint16_t> >::const_iterator it = init_nodes.begin();
		for( ;it != init_nodes.end(); ++it ) {
			registerIn(it->first,it->second);
		}
        	
		//启动链路检测定时器
		_timerLinkCheck.start(TIMER_LINK_CHECK);
		//启动ping定时器
	    _timerPing.start(TIMER_PING);
        //启动根节点切换时机检查定时器
		_timerCheckRootSwitch.start(TIMER_CHECK_ROOT_SWITCH_INTERVAL);
		//启动检查区块同步定时器
		_timerCheckBlocksFull.start(TIMER_CHECK_BLOCKS_FULL_INTERVAL);
        //启动检查最佳路由定时器
		_timerCheckBestRoute.start(TIMER_CHECK_BEST_ROUTE_INTERVAL);
	} else { //本节点为根节点
		_treeLevel = 0;
		_currentParentAddress = _localAddress;
		//_initFlag = 0xFFFFFFFFFFFFFFFF; //初始化完成
 		//启动根节点选举定时器
        _timerCheckRootSwitch.start(TIMER_CHECK_ROOT_SWITCH_INTERVAL);
	}
}


void FPayClientCore::syncBlocks(uint32_t cid)
{
	fprintf(stderr,"FPayClientCore::syscBlocks,cid:%u\n",cid);	
	//调用区块模块获取当前的最后一个区块id idx
	Byte32 from_block_id;
	uint64_t from_block_idx;
	block_info_t block;
	if( FPayBlockService::getInstance()->getLastBlock(block) ) {
		from_block_id = block.id;
		from_block_idx = block.idx;

	} else {
		from_block_idx = 0; //从传世区块开始取	
	}
	fprintf(stderr,"FPayClientCore::syncBlocks,from block idx:%lu\n",from_block_idx);
   
	SyncBlocksReq sync;
	sync.public_key = _localPublicKey;
	sync.from_block_id = from_block_id;
	sync.from_block_idx = from_block_idx;
	sync.block_num = 2;
	sync.genSign(_localPrivateKey);

	//发送同步请求
	send(cid,SyncBlocksReq::uri,sync);
}


void FPayClientCore::onNodeRegisterRes(NodeRegisterRes* res, IConn* c)
{
	fprintf(stderr,"FPayClientCore::onNodeRegisterRes\n");
	if( res->signValidate() ) {

		fprintf(stderr,"FPayClientCore::onNodeRegisterRes,sign validate success,node tree level:%d, local tree level:%d\n",res->tree_level,_treeLevel);
	
	    if( (_initFlag & BIT_CLIENT_INIT_REGISTER_OVER) != BIT_CLIENT_INIT_REGISTER_OVER ) {
			_initFlag = _initFlag | BIT_CLIENT_INIT_REGISTER_OVER;
		}

		//如果此节点返回的角色比本节点高出两个及以上，则将父节点切入当前节点地址
		if( _treeLevel > res->tree_level + 1 ) {
			_treeLevel = res->tree_level + 1; //设置本节点的tree level
            _currentParentAddress = res->address; //更改父节点地址	
		}

		_upConnInfos[c->getConnId()].address = res->address;
	
		//将该节点加入备份节点管理
		node_info_t node;
		node.address = res->address;
		node.ip = _upConnInfos[c->getConnId()].ip;
		node.port = _upConnInfos[c->getConnId()].port;
        _backupNodeInfos.insert(node);

		//同步区块
		syncBlocks(c->getConnId());
	}else{
		//断开连接
		eraseConnectById(c->getConnId());
	}

}

//区块同步回应
void FPayClientCore::onSyncBlocksRes(SyncBlocksRes* res, IConn* c)
{
	fprintf(stderr,"FPayClientCore::onSyncBlockRes\n");	
	if( res->signValidate() ) {
	
		log( Info, "FPayClientCore::onSyncBlockRes, sign validate success,node block size:%Zu,continue flag:%d",res->blocks.size(),res->continue_flag);	
	    fprintf(stderr,"FPayClientCore::onSyncBlockRes, sign validate success,node block size:%Zu,continue flag:%d\n",res->blocks.size(),res->continue_flag);	
	  
		if( res->resp_code != 0 ) {
			//todo	
		} else {
			for( uint32_t i = 0; i < res->blocks.size(); i++ ) {
				//调用区块模块将block存储:
				FPayBlockService::getInstance()->storeBlock(res->blocks[i]);
				FPayTXService::getInstance()->updateBalanceByBlock(res->blocks[i]);
			}	
			//判断是否有后续区块
			if(res->continue_flag == 1) {
				syncBlocks(c->getConnId());
			}else {
				if( (_initFlag & BIT_CLIENT_INIT_BLOCKS_FULL) != BIT_CLIENT_INIT_BLOCKS_FULL ) {
					_initFlag = _initFlag | BIT_CLIENT_INIT_BLOCKS_FULL; //表示区块同步完成
				}
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
	//fprintf(stderr,"FPayClientCore::onPingRes\n");
	if( res->signValidate() ) {
		if( _treeLevel  > res->tree_level + 1 ) { //换父节点
            _treeLevel = res->tree_level + 1;
			_currentParentAddress = _upConnInfos[c->getConnId()].address;
		}

	}else {
		//断开
		eraseConnectById(c->getConnId());
	}

}

	
void FPayClientCore::onGetRelativesRes(GetRelativesRes* rela_res,core::IConn* c)
{


}

//收到下发的区块广播
void FPayClientCore::onBlockBroadcast(BlockBroadcast* broadcast, IConn* c)
{
	fprintf(stderr,"FPayClientCore::onBlockBroadcast\n");
	if( broadcast->signValidate() ) {
		fprintf(stderr,"FPayClientCore::onBlockBroadcast,sign validate success,block idx:%lu\n",broadcast->block.idx);
		//调用区块模块存储起来
		FPayBlockService::getInstance()->storeBlock(broadcast->block);
		FPayServerCore::getInstance()->broadcastBlock(broadcast->block);
		FPayTXService::getInstance()->updateBalanceByBlock(broadcast->block);
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
	//log( Info, "FPayClientCore::linkCheck, check link wheath alive" );
	//fprintf(stderr,"FPayClientCore::linkCheck, check link wheath alive\n" );
    
	set<node_info_t,nodeInfoCmp>::iterator it;
	for( it = _backupNodeInfos.begin(); it != _backupNodeInfos.end(); ++it ) {
		if( findConnByAddress(it->address) == 0 ) {
			registerIn(it->ip,it->port);
		}
	}

	return true;
}


bool FPayClientCore::ping()
{
	//log( Info, "FPayClientCore::ping to all up node");
    //fprintf( stderr, "FPayClientCore::ping to all up node\n");
    
	map<uint32_t,up_conn_info_t>::iterator it;
	for( it = _upConnInfos.begin(); it != _upConnInfos.end(); ++it ) {
		PingReq req;
		req.public_key = _localPublicKey;		
		req.genSign(_localPrivateKey);
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
	map<uint32_t,up_conn_info_t>::iterator it;
	for( it = _upConnInfos.begin(); it != _upConnInfos.end(); ++it ) {
		if( it->second.address == address ){
			return it->second.cid;
		}
	}
	return 0;
}


//转发支付请求
void FPayClientCore::dispatchPay( const PayReq& pay )
{
	if( _treeLevel > 0 ) {
		uint32_t cid = findConnByAddress(_currentParentAddress);
		if( cid != 0 ) {
			send(cid,PayReq::uri,pay);	
		}
	}
}


