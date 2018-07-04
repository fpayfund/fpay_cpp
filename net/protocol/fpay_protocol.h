#ifndef __FPAY_PROTOCOL_H_
#define __FPAY_PROTOCOL_H_
#include "common/core/base_svid.h"
#include "common/packet.h"
#include "common/protocol/const.h"
#include <vector>
/**
 *@author:wisefan
 *@date:2018-06-01
 *@last-update:2018-06-13
*/

namespace fpay { namespace protocol {

	using namespace std;

	enum
	{
		FPAY_SID = 199
	};

	enum FPayProtoURI
	{
		NODE_REGISTER_PROTO_REQ                  = 1,        //子节点注册请求
		NODE_REGISTER_PROTO_RES                  = 2,        //子节点注册回应

		NODE_READY_PROTO_REQ                     = 3,        //子节点就绪上报
		NODE_READY_PROTO_RES                     = 4,        //子节点就绪回应

		GET_RELATIVES_PROTO_REQ                  = 5,        //查询相邻节点
		GET_RELATIVES_PROTO_RES                  = 6,        //返回父节点及最多5个子节点

		SYNC_BLOCKS_PROTO_REQ                    = 7,        //同步区块s
		SYNC_BLOCKS_PROTO_RES                    = 8,        //返回区块s

		PAY_PROTO_REQ                            = 9,        //支付请求
		PAY_PROTO_RES                            = 10,        //支付回应

        PING_REQ                                 = 13,        //ping请求
		PING_RES                                 = 14,

		BLOCK_PROTO_BROADCAST                    = 15         //区块广播
	};

	//版本号。用于协议和节点
	struct _version_info : public sox::Marshallable {
		uint8_t main;
		uint8_t vice;
		uint8_t development;
		uint8_t compatibility;
		virtual void marshal(sox::Pack &pk) const
		{
			pk << main << vice << development << compatibility;
		}

		virtual void unmarshal(const sox::Unpack &up)
		{
			up >> main >> vice >> development >> compatibility;
		}
	};
	typedef struct _version_info  version_info_t;

	//支付确认对象。记录节点对支付的确认信息
	struct _confirmation_info : public sox::Marshallable  {
		Byte32 current_address;  //当前确认节点的地址
		Byte32 public_key;       //当前确认节点的公钥
		uint32_t timestamp;      //当前节点确认时间戳
		uint32_t balance;
		Byte32 payment_id;
		Byte32 next_address;     //下一个确认节点的地址
		Byte32 sign;             //前置数据的签名
	
		//确认数据签名验证
		bool signValidate();   //current_address&public_key&timestamp&balance&payment_id&next_address
		virtual void marshal(sox::Pack &pk) const
		{
			pk << current_address << public_key << timestamp << balance << payment_id << next_address << sign;
		}

		virtual void unmarshal(const sox::Unpack &up)
		{
			up >> current_address >> public_key >> timestamp >> balance >> payment_id >> next_address >> sign;
		}
	};
	typedef struct _confirmation_info confirmation_info_t;

    //支付请求数据
	struct _pay : public sox::Marshallable {
		Byte32 id;                     //支付ID
		Byte32 from_address;           //钱包的地址 
		Byte32 public_key;             //钱包的公钥  	
		Byte32 to_address;             //目标地址
		uint64_t amount;               //64位无符号整数（后18位为小数）。转账金额
		uint64_t balance;              //账户当前余额 (后18位为小数）
		Byte32 balance_payment_id;     //支付者余额对应的最新支付id
		Byte32 accept_address;         //受理节点地址,就是接入的矿工节点地址
		Byte32 sign;                   //钱包签名。对前置数据进行签名
	
		//数据签名验证
		bool signValidate(); //id&from_address@public_key@to_address@amount@balance@balace_payment_id@accept_address
		virtual void marshal(sox::Pack &pk) const
		{
			pk << id << from_address << public_key << timestamp << to_address << amount << balance << balance_payment_id;
			pk << accept_address << sign;
	
		}

		virtual void unmarshal(const sox::Unpack &up)
		{
			up >> id >> from_address >> public_key >> timestamp >> to_address >> amount >> balance >> balance_payment_id;
			up >> accept_address >> sign;
		}
	};
	typedef struct _pay pay_t;

	//支付对象。钱包用于支付的信息
	struct _payment_info : public sox::Marshallable {
		
        pay_t pay;
        vector<confirmation_info_t> confirmations;//支付确认数组。存放已经经过的节点确认信息
		//数据签名验证
		bool signValidate();
	
		virtual void marshal(sox::Pack &pk) const
		{	
			pk << pay;
			marshal_container(pk, confirmations);
		}

		virtual void unmarshal(const sox::Unpack &up)
		{
			up >> pay;
			unmarshal_container(up, std::back_inserter(confirmations));
		}

	}
	typedef struct _payment_info payment_info_t;


	//区块。由根节点把若干支付信息打包而成
	struct _block_info : public sox::Marshallable {
		uint64_t idx; //当前区块的索引idx，创世区块idx 为0，后面的是累加
		Byte32 id;  //当前区块ID。256位无符号整数。由上一区块的ID与本区块最后一笔支付的签名计算而成
        Byte32 pre_id; //上一个区块的id
		Byte32 root_address; //根节点地址
		Byte32 public_key; //根节点公钥
		uint32_t timestamp; //出块时间戳。仅用于记录	
		vector<payment_info_t> payments; //支付数组。存放已经经过确认的支付信息
		Byte32 sign;  //根节点确认签名

		//签名验证
		bool signValidate();
		virtual void marshal(sox::Pack &pk) const
		{
			pk << idx << id << pre_id << address << public_key << timestamp;
			marshal_container(pk, confirmations);
			pk  << sign;
		}
		virtual void unmarshal(const sox::Unpack &up)
		{
			up >> idx >> id >> pre_id >> address >> public_key >> timestamp;
			unmarshal_container(up, std::back_inserter(confirmations));
			up  >> sign;
		}

	};
	typedef struct _block_info block_info_t;


	//节点对象
	typedef struct _node_info : public sox::Marshallable {
		Byte32 address;      //节点地址
		Byte16 ip;           //节点ip地址,兼容IPV4 IPV6 地址，如果是IPV4就是前面四个字节表示，如果是ipv6则是16个字节
		uint16_t port;       //节点端口
		uint8_t ip_version;  //ip版本 0 表示IPV4 1表示IPV6
		virtual void marshal(sox::Pack &pk) const
		{
			pk << address << ip << port << ip_version;
		}

		virtual void unmarshal(const sox::Unpack &up)
		{
			up >> address >> ip >> port >> ip_version;
		}
	};
	typedef struct _node_info node_info_t;



	struct RequestBase : public sox::Marshallable
	{
		uint32_t session;
		Byte4 header;   
		version_info_t protocol_version;
		uint32_t timestamp;
		RequestBase():
		{
		}
		RequestBase& operator=(const RequestBase& rhs)
		{
			this->header = rhs.header;
			this->protocol_version = rhs.protocol_version;
			this->timestamp = rhs.timestamp;
			return *this;
		}

		//数据签名验证
		virtual bool signValidate() = 0;

		virtual void marshal(sox::Pack &pk) const
		{
			pk << session << header << protocol_version << timestamp;
		}

		virtual void unmarshal(const sox::Unpack &up)
		{
			up >> session >> header >> protocol_version >> timestamp;
		}
	};


	struct ResponseBase : public sox::Marshallable
	{
		Byte4 header;   
		version_info_t protocol_version;
		uint32_t timestamp;
		uint32_t resp_code;
		ResponseBase():
		{
		}
		//数据签名验证
		virtual bool signValidate() = 0;
		ResponseBase& operator=(const ResponseBase& rhs)
		{
			this->header = rhs.header;
			this->protocol_version = rhs.protocol_version;
			this->timestamp = rhs.timestamp;
			this->resp_code = rhs.resp_code;
			return *this;
		}
		virtual void marshal(sox::Pack &pk) const
		{
			pk << header << protocol_version << timestamp << resp_code ;
		}

		virtual void unmarshal(const sox::Unpack &up)
		{
			up >> header >> protocol_version >> timestamp >> resp_code;
		}
	};

	struct BroadcastBase : public sox::Marshallable
	{
		Byte4 header;   
		version_info_t protocol_version;
		uint32_t timestamp;
		BroadcastBase():
		{
		}
		BroadcastBase& operator=(const BroadcastBase& rhs)
		{
			this->header = rhs.header;
			this->protocol_version = rhs.protocol_version;
			this->timestamp = rhs.timestamp;
			return *this;
		}
		virtual void marshal(sox::Pack &pk) const
		{
			pk << header << protocol_version << timestamp;
		}

		virtual void unmarshal(const sox::Unpack &up)
		{
			up >> header >> protocol_version >> timestamp;
		}
	};


	//字节的注册到网络
	//地址未空是0x0000000000000000000000000000000000000000000000000000000000000000
	struct NodeRegisterReq: public RequestBase
	{
		enum {uri = NODE_REGISTER_PROTO_REQ << 8 | FPAY_SID };

		Byte32 address;                    //本节点的地址
		Byte32 public_key;                 //本节点的公钥	
		uint64_t last_block_idx;           //区块的索引id
		Byte32 last_block_id;              //本节点最后的区块id
		Byte32 first_root_node_address;    //第一个根节点地址,创始区块中的根节点地址 
		Byte32 last_root_node_address;     //最后一个根节点地址 
		Byte32 sign;                       //数据签名 ()
		
		//数据签名验证
		bool signValidate();

		virtual void marshal(sox::Pack &pk) const
		{
			pk << address << public_key << last_block_idx << last_block_id;
			pk << first_root_node_address << last_root_node_address << sign;

		}
		virtual void unmarshal(const sox::Unpack &up)
		{
			up >> address >> public_key >> last_block_idx >> last_block_id;
			up >> first_root_node_address >> last_root_node_address >> sign;
		}
	};

	struct NodeRegisterRes: public ResponseBase
	{
		enum {uri = NODE_REGISTER_PROTO_RES << 8 | FPAY_SID };

		Byte32 address;                      //本节点的地址 
		Byte32 public_key;                   //本节点的公钥  	
		uint64_t last_block_idx;             //区块索引id
		Byte32 last_block_id;                //本节点最后的区块id
        Byte32 first_root_node_address;      //第一个根节点地址，创始区块中的根节点地址
		Byte32 last_root_node_address;       //最后一个根节点地址,截至到当前时间的根节点地址
		node_info_t parent;                  //本节点的父节点
		vector<node_info_t> children;        //本节点的子节点（目前最多给5个）
		Byte32 sign;                         //对上面数据签名

		virtual bool signValidate(); //address@public_key@last_block_idx@last_block_id@first_root_node_address@last_root_node_address

		virtual void marshal(sox::Pack &pk) const
		{
			pk << address << public_key << last_block_idx << last_block_id;
			pk << first_root_node_address << last_root_node_address << parent;
			marshal_container(pk, children);
			pk << sign;

		}
		virtual void unmarshal(const sox::Unpack &up)
		{
			up >> address >> public_key  >> last_block_idx >> last_block_id;
			up >> first_root_node_address >> last_root_node_address >> parent;
			unmarshal_container(up, std::back_inserter(confirmations));
			up >> sign;
		}
	};

	//节点就绪上报(P2P网络发现，这里上报给P2P网络节点发现服务器）
	struct NodeReadyReq: public RequestBase
	{
		enum {uri = NODE_READY_PROTO_REQ << 8 | FPAY_SID };

		node_info_t node_info;
		Byte32 public_key;                 //本节点的公钥	
		Byte32 sign;                       //数据签名 ()
		
		//数据签名验证
		bool signValidate();

		virtual void marshal(sox::Pack &pk) const
		{
			pk << node_info << public_key << sign;
		}

		virtual void unmarshal(const sox::Unpack &up)
		{
			up >> node_info >> public_key >> sign;	
		}
	};

	//节点就绪上报回应
	struct NodeReadyRes: public ResponseBase
	{
		enum {uri = NODE_READY_PROTO_RES << 8 | FPAY_SID };
		virtual bool signValidate()
		{
			return true;
		}
	};


	//支付请求
	struct PayReq : public RequestBase
	{
		enum {uri = PAY_PROTO_REQ << 8 | FPAY_SID };
		pay_t pay;
		//确认链
		vector<confirmation_info_t> confirm_link;  //支付确认的链条，从叶子矿工节点到根节点，每次转发都多加一条confirm信息
        //支付数据签名验证
		virtual bool signValidate(); //两个验证：支付请求数据的签名验证，确认数据的签名确认
		virtual void marshal(sox::Pack &pk) const
		{
		
			pk << pay;
			marshal_container(pk, confirm_link);

		}
		virtual void unmarshal(const sox::Unpack &up)
		{
	
			up >> pay;
			unmarshal_container(up, std::back_inserter(confirm_link));
		}
	};

	//确认应答
	struct PayRes: public ResponseBase
	{
		enum {uri = PAY_PROTO_RES << 8 | FPAY_SID };
		virtual bool signValidate() {
			return true;
		}
	};

	//最新形成的区块广播报文
	struct BlockBroadcast : public BroadcastBase
	{
		enum {uri = BLOCK_PROTO_BROADCAST << 8 | FPAY_SID };
		block_info_t block;

		//数据签名验证
		virtual bool signValidate() {
			return block.signValidate();
		}
		virtual void marshal(sox::Pack &pk) const
		{
			pk << block;
		}
		virtual void unmarshal(const sox::Unpack &up)
		{
			up >> block;
		}
	};

	//同步多个区块(一次最多同步256个）实际情况是1个区块估计几兆，故一次最多一个区块，然后从多个节点同步
	struct SyncBlocksReq : public RequestBase
	{
		enum {uri = SYNC_BLOCKS_PROTO_REQ << 8 | FPAY_SID };		
		Byte32 from_block_id;
		uint64_t from_block_idx;
		uint8_t block_num;

		bool signValidate(){
			return true;
		}
		virtual void marshal(sox::Pack &pk) const
		{
			pk << from_block_id << from_block_idx << block_num;
		}
		virtual void unmarshal(const sox::Unpack &up)
		{
			up >> from_block_id >> from_block_idx >> block_num;
		}
	};

	//同步多个区块回应
	struct SyncBlocksRes : public ResponseBase
	{
		enum {uri = SYNC_BLOCKS_PROTO_RES << 8 | FPAY_SID };
		vector<block_info_t> blocks;
		//数据签名验证
		virtual bool signValidate() {
			bool ret = true;
			for( uint32_t i = 0; i < blocks.size(); i++ ) {
				if( !blocks[i].signValidate() ) {
					ret = false;
					break;
				}
			}
			return ret;
		}

		virtual void marshal(sox::Pack &pk) const
		{
			marshal_container(pk, blocks);
		}
		virtual void unmarshal(const sox::Unpack &up)
		{
			unmarshal_container(up, std::back_inserter(blocks));
		}
	};

	//查询相邻节点，返回5个随机子节点对象
	struct GetRelativesReq : public RequestBase 
	{
		enum {uri = GET_RELATIVES_PROTO_REQ << 8 | FPAY_SID };	

		//数据签名验证
		bool signValidate() {
			return true;
		}
	};

	//返回
	struct GetRelativesRes : public ResponseBase 
	{
		enum {uri = GET_RELATIVES_PROTO_RES << 8 | FPAY_SID };		
		vector<node_info_t> nodes;
		Byte32 public_key;
		Byte32 sign;
		//数据签名验证
		virtual bool signValidate();
		virtual void marshal(sox::Pack &pk) const
		{

			marshal_container(pk, nodes);
			pk << public_key << sign;
		}
		virtual void unmarshal(const sox::Unpack &up)
		{

			unmarshal_container(up, std::back_inserter(nodes));
			up >> public_key >> sign;
		}
	};

	struct PingReq : public RequestBase 
	{
		enum {uri = PING_REQ << 8 | FPAY_SID };
		bool signValidate(){
			return true;
		}

	};
	//返回
	struct PingRes : public ResponseBase 
	{
		enum {uri = GET_RELATIVES_PROTO_RES << 8 | FPAY_SID };
		bool signValidate() {
			return true;
		}
	};


}

}
#endif /*FPAY_PROTOCOL_H_*/
