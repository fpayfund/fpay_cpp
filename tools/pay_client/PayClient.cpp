#include "PayClient.h"
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

BEGIN_FORM_MAP(PayClient)
		    ON_LINK(NodeRegisterRes, &PayClient::onNodeRegisterRes) 
		    ON_LINK(PayRes, &PayClient::onPayRes) 
END_FORM_MAP()

PayClient *PayClient::_instance = NULL;
void PayClient::init(
			const Byte20& address,
			const Byte32& public_key,
			const Byte32& private_key)
{
    _localAddress = address;
	_localPublicKey = public_key;
	_localPrivateKey = private_key;
}


PayClient::PayClient()
{

}

PayClient::~PayClient(){

}


//链接错误
void PayClient::onError(int ev, const char *msg, IConn *conn)
{
	fprintf( stderr, "PayClient::onError, conn:%d to node error:%s\n", 
				conn->getConnId(), msg );
	
	log( Info, "PayClient::onError, conn:%d to node error:%s", 
				conn->getConnId(), msg );
	//MultiConnManagerImp::onError(ev,msg,conn);
}


//链接断开
void PayClient::eraseConnect(IConn *conn)
{
	fprintf(stderr,"PayClient::eraseConnect\n");
	log( Info,"PayClient::eraseConnect");

	MultiConnManagerImp::eraseConnect(conn);
}

//报文发送函数
void PayClient::send(uint32_t cid, uint32_t uri, const sox::Marshallable& marshal)
{
	Sender send;
	send.marshall(uri,marshal);
	send.endPack();
	dispatchById( cid, send );
}


//注册进入网络
void PayClient::registerIn(const string& ip, uint16_t port)
{
	fprintf(stderr,"PayClient::registerIn,ip:%s,port:%u\n",ip.c_str(),port);
	log( Info, "PayClient::registerIn,ip:%s,port:%u",ip.c_str(),port);

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
void PayClient::start( const vector< pair<string,uint16_t> >& init_nodes )
{
	log(Info, "PayClient::start,init up nodes size:%Zu", init_nodes.size() );

	if( init_nodes.size() > 0 ){

		//注册进网络
		vector< pair<string,uint16_t> >::const_iterator it = init_nodes.begin();
		for( ;it != init_nodes.end(); ++it ) {
			registerIn(it->first,it->second);
		}
        
	} else { //本节点为根节点

	}
}


void PayClient::onNodeRegisterRes(NodeRegisterRes* res, IConn* c)
{
	fprintf(stderr,"PayClient::onNodeRegisterRes\n");
	if( res->signValidate() ) {

		fprintf(stderr,"PayClient::onNodeRegisterRes,sign validate success,node tree level:%d, local tree level:%d\n",res->tree_level,_treeLevel);
	
		_upConnInfos[c->getConnId()].address = res->address;

		_currentParentAddress = res->address;
		//将该节点加入备份节点管理
		node_info_t node;
		node.address = res->address;
		node.ip = _upConnInfos[c->getConnId()].ip;
		node.port = _upConnInfos[c->getConnId()].port;
        _backupNodeInfos.insert(node);

		//发起支付请求
		pay();
	}else{
		//断开连接
		eraseConnectById(c->getConnId());
	}

}

//支付回应
void PayClient::onPayRes(PayRes* res, IConn* c)
{
	if( res->signValidate() ) {
        
		//todo
	}else{
		//断开连接
		eraseConnectById(c->getConnId());
	}

}


IConn *PayClient::connectNode( const string& ip, uint16_t port)
{
	log(Info, "PayClient::connectNode,host:[%s,%u]", ip.c_str(), port);
	return createClientConn(ip.data(), port, handler, this);

}

//根据地址找到连接
uint32_t PayClient::findConnByAddress(const Byte20& address)
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
void PayClient::pay()
{
	PayConfig* config = PayConfig::getInstance();

	PayReq req;
	
	ECKey_Rand(req.payment.pay.id.u8,32);
	req.payment.pay.from_address = config->fromAddr;
	req.payment.pay.public_key = config->publicKey;
	req.payment.pay.timestamp = time(NULL);
	req.payment.pay.to_address = config->toAddr;
	req.payment.pay.amount = config->amount;
	req.payment.pay.balance = 0;
	req.payment.pay.accept_address = _currentParentAddress;
	req.payment.pay.genSign(config->privatekey);
    
	uint32_t cid = findConnByAddress(_currentParentAddress);
	if( cid != 0 ) {
		send(cid,PayReq::uri,pay);	
	}

}


