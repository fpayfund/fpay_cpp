#ifndef __FPAY_PROTOCOL_H_
#define __FPAY_PROTOCOL_H_
#include "common/core/base_svid.h"
#include "common/packet.h"
#include "common/byte.h"
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
		Byte25 current_address;  //当前确认节点的地址
		Byte32 public_key;       //当前确认节点的公钥
        Byte32 private_key;      //当前确认节点的私钥，不参与编码，只参与计算签名
		uint32_t timestamp;      //当前节点确认时间戳
		uint32_t balance;
		Byte32 payment_id;
		Byte25 next_address;     //下一个确认节点的地址
		Byte32 sign;             //前置数据的签名

		void operator=(const _confirmation_info& r)
		{
			this.current_address = r.current_address;
			this.public_key = r.public_key;
			this.private_key = r.private_key;
			this.timestamp = r.timestamp;
			this.balance = r.balance;
			this.payment_id = r.payment_id;
			this.next_address = r.next_address;
			this.sign = r.sign;
		}
		//生成签名
		void genSign();   
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
		Byte25 from_address;           //钱包的地址 
		Byte32 public_key;             //钱包的公钥
        Byte32 private_key;            //钱包的私钥
		Byte25 to_address;             //目标地址
		uint64_t amount;               //64位无符号整数（后18位为小数）。转账金额
		uint64_t balance;              //账户当前余额 (后18位为小数）
		Byte32 balance_payment_id;     //支付者余额对应的最新支付id
		Byte25 accept_address;         //受理节点地址,就是接入的矿工节点地址
		Byte32 sign;                   //钱包签名。对前置数据进行签名

		void operator=(const _pay& r)
		{
			this.id = r.id;
			this.from_address = r.from_address;
			this.public_key = r.public_key;
			this.private_key = r.private_key;
			this.to_address = r.to_address;
			this.amount = r.amount;
			this.balance = r.balance;
			this.balance_payment_id = r.balance_payment_id;
			this.accpet_address = r.accept_address;
			this.sign = r.sign;
		}
		//生成签名
		void genSign();
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

		void operator=(const _payment_info& r)
		{
			this.pay = r.pay;
			for( uint32_t i = 0; i < r.comfirmations.size(); i++ ) {
				this.confirmations.push_back(r.comfirmations[i]);
			}
		}
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
		Byte25 root_address; //根节点地址
		Byte32 public_key; //根节点公钥
		Byte32 private_key;//根节点私钥，不参与编码,只参与计算签名
		uint32_t timestamp; //出块时间戳。仅用于记录	
		vector<payment_info_t> payments; //支付数组。存放已经经过确认的支付信息
		Byte32 sign;  //根节点确认签名
		
		void operator=(const _block_info& r) {
			this.idx = r.idx;
			this.pre_id = r.pre_id;
			this.root_address = r.root_address;
			this.public_key = r.public_key;
			this.private_key = r.private_key;
			this.timestamp = r.timestamp;
			for( uint32_t i = 0; i < r.payments.size(); i++ ) {
				this.payments.push_back(r.payments[i]);
			}
			this.sign = r.sign;
		}
		
		//生成签名
		void genSign();
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
		Byte25 address;      //节点地址
		string ip;           //节点ip地址,兼容IPV4 IPV6 地址，如果是IPV4就是前面四个字节表示，如果是ipv6则是16个字节
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
		bool operator<(const struct _node_info & right) const   //重载<运算符
		{
			if(this->address == right.address){     //根据id去重

				return false;
			}
			return this->address >  right.address; 
		}

	};
	typedef struct _node_info node_info_t;

	class nodeInfoCmp {  
		public:  
			bool operator() (const node_info_t& lc, const node_info_t& rc) {  
				return (lc.address < rc.address);  
			}	
	};

	struct RequestBase : public sox::Marshallable
	{
		uint64_t session;	
		version_info_t protocol_version;
		uint32_t timestamp; //时间戳
		//每个请求都带上自己的地址和公钥
		Byte25 address;
		Byte32 public_key;
		//私有key，制作签名用，不做封包处理
		Byte32 private_key;
		Byte32 sign;

		RequestBase& operator=(const RequestBase& rhs)
		{
			this->session = rhs.session;
			this->protocol_version = rhs.protocol_version;
			this->timestamp = rhs.timestamp;
			this->address = rhs.address;
			this->public_key = rhs.public_key;
			this->private_key = rhs.private_key;
			this->sign = rhs.sign;
			return *this;
		}
		//生成前面
        virtual void genSign() = 0;
		//数据签名验证
		virtual bool signValidate() = 0;

		virtual void marshal(sox::Pack &pk) const
		{
			pk << session << protocol_version << timestamp;
			pk << address << public_key << sign;
		}

		virtual void unmarshal(const sox::Unpack &up)
		{
			up >> session >> protocol_version >> timestamp;
			up >> address >> public_key >> sign;
		}
	};


	struct ResponseBase : public sox::Marshallable
	{
	    uint64_t session;	
		version_info_t protocol_version;
		uint32_t timestamp;
		uint32_t resp_code;
		Byte25 address;
		Byte32 public_key;
        Byte32 private_key;
		Byte32 sign;
		
		//生成签名
		virtual void genSign() = 0;
		//数据签名验证
		virtual bool signValidate() = 0;
		ResponseBase& operator=(const ResponseBase& rhs)
		{
			this->session = rhs.session;
			this->protocol_version = rhs.protocol_version;
			this->timestamp = rhs.timestamp;
			this->resp_code = rhs.resp_code;
			this->address = rhs.address;
			this->public_key = rhs.public_key;
			this->sign = rhs.sign;
			return *this;
		}
		virtual void marshal(sox::Pack &pk) const
		{
			pk << session << protocol_version << timestamp << resp_code ;
			pk << address << public_key << sign;
		}

		virtual void unmarshal(const sox::Unpack &up)
		{
			up >> session >> protocol_version >> timestamp >> resp_code;
			up >> address >> public_key >> sign;
		}
	};

	struct BroadcastBase : public sox::Marshallable
	{
		uint64_t  session;   
		version_info_t protocol_version;
		uint32_t timestamp;
		Byte25 address;
		Byte32 public_key;
		Byte32 private_key;
	    Byte32 sign;

		virtual void genSign() = 0;
		//数据签名验证
		virtual bool signValidate() = 0;
	
		BroadcastBase& operator=(const BroadcastBase& rhs)
		{
			this->session = rhs.session;
			this->protocol_version = rhs.protocol_version;
			this->timestamp = rhs.timestamp;
			this->address = rhs.address;
			this->public_key = rhs.public_key;
			this->sign = rhs.sign;
			return *this;
		}
		virtual void marshal(sox::Pack &pk) const
		{
			pk << session << protocol_version << timestamp;
			pk << address << public_key << sign;
		}

		virtual void unmarshal(const sox::Unpack &up)
		{
			up >> session >> protocol_version >> timestamp;
			up >> address >> public_key >> sign;
		}
	};


	//字节的注册到网络
	//地址未空是0x0000000000000000000000000000000000000000000000000000000000000000
	struct NodeRegisterReq: public RequestBase
	{
		enum {uri = NODE_REGISTER_PROTO_REQ << 8 | FPAY_SID };
	
		//生成签名
		virtual void genSign();
		//数据签名验证
		virtual bool signValidate();

		virtual void marshal(sox::Pack &pk) const
		{
			RequestBase::marshal(pk);
		}
		virtual void unmarshal(const sox::Unpack &up)
		{
			RequestBase::unmarshal(up);
		}
	};

	struct NodeRegisterRes: public ResponseBase
	{
		enum {uri = NODE_REGISTER_PROTO_RES << 8 | FPAY_SID };
	
		uint8_t tree_level;                  //本节点树的层级，0表示根节点，1表示一级节点，2表示二级节点
		uint64_t last_block_idx;             //区块索引id
		Byte32 last_block_id;                //本节点最后的区块id
        Byte25 first_root_node_address;      //第一个根节点地址，创始区块中的根节点地址
		Byte25 last_root_node_address;       //最后一个根节点地址,截至到当前时间的根节点地址
		node_info_t parent;                  //本节点的父节点
		vector<node_info_t> children;        //本节点的子节点（目前最多给5个）

        virtual void genSign();
		virtual bool signValidate();

		virtual void marshal(sox::Pack &pk) const
		{
			ResponseBase::marshal(pk);
			pk << tree_level << last_block_idx << last_block_id;
			pk << first_root_node_address << last_root_node_address << parent;
			marshal_container(pk, children);
		}
		virtual void unmarshal(const sox::Unpack &up)
		{
			ResponseBase::unmarshal(up);
			up >> tree_level >> last_block_idx >> last_block_id;
			up >> first_root_node_address >> last_root_node_address >> parent;
			unmarshal_container(up, std::back_inserter(children));	
		}
	};

	//节点就绪上报(P2P网络发现，这里上报给P2P网络节点发现服务器）
	struct NodeReadyReq: public RequestBase
	{
		enum {uri = NODE_READY_PROTO_REQ << 8 | FPAY_SID };

		node_info_t node_info;

		virtual void genSign();
		//数据签名验证
		virtual bool signValidate();

		virtual void marshal(sox::Pack &pk) const
		{
			RequestBase::marshal(pk);
			pk << node_info;
		}

		virtual void unmarshal(const sox::Unpack &up)
		{
			RequestBase::unmarshal(up);
			up >> node_info;	
		}
	};

	//节点就绪上报回应
	struct NodeReadyRes: public ResponseBase
	{
		enum {uri = NODE_READY_PROTO_RES << 8 | FPAY_SID };
		virtual void genSign();
		virtual bool signValidate();
		virtual void marshal(sox::Pack &pk) const
		{
			ResponseBase::marshal(pk);
		}

		virtual void unmarshal(const sox::Unpack &up)
		{
			ResponseBase::unmarshal(up);	
		}
		
	};


	//支付请求
	struct PayReq : public RequestBase
	{
		enum {uri = PAY_PROTO_REQ << 8 | FPAY_SID };
		pay_t pay;
		//确认链
		vector<confirmation_info_t> confirm_link;  //支付确认的链条，从叶子矿工节点到根节点，每次转发都多加一条confirm信息
        
		virtual void genSign();
		//支付数据签名验证
		virtual bool signValidate(); //两个验证：支付请求数据的签名验证，确认数据的签名确认
		virtual void marshal(sox::Pack &pk) const
		{
	        RequestBase::marshal(pk);	
			pk << pay;
			marshal_container(pk, confirm_link);

		}
		virtual void unmarshal(const sox::Unpack &up)
		{
	        RequestBase::unmarshal(up);
			up >> pay;
			unmarshal_container(up, std::back_inserter(confirm_link));
		}
	};

	//确认应答
	struct PayRes: public ResponseBase
	{
		enum {uri = PAY_PROTO_RES << 8 | FPAY_SID };
		virtual void genSign();
		virtual bool signValidate();	
		virtual void marshal(sox::Pack &pk) const
		{
	        ResponseBase::marshal(pk);	
		}
		virtual void unmarshal(const sox::Unpack &up)
		{
	        ResponseBase::unmarshal(up);
		}
	
	};

	//最新形成的区块广播报文
	struct BlockBroadcast : public BroadcastBase
	{
		enum {uri = BLOCK_PROTO_BROADCAST << 8 | FPAY_SID };
		block_info_t block;


		virtual void genSign();
		//数据签名验证
		virtual bool signValidate() {
			return block.signValidate();
		}
		virtual void marshal(sox::Pack &pk) const
		{
			BroadcastBase::marshal(pk);
			pk << block;
		}
		virtual void unmarshal(const sox::Unpack &up)
		{
			BroadcastBase::unmarshal(up);
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
       
		virtual void genSign();
		bool signValidate(){
			return true;
		}
		virtual void marshal(sox::Pack &pk) const
		{
			RequestBase::marshal(pk);
			pk << from_block_id << from_block_idx << block_num;
		}
		virtual void unmarshal(const sox::Unpack &up)
		{
			RequestBase::unmarshal(up);
			up >> from_block_id >> from_block_idx >> block_num;
		}
	};

	//同步多个区块回应
	struct SyncBlocksRes : public ResponseBase
	{
		enum {uri = SYNC_BLOCKS_PROTO_RES << 8 | FPAY_SID };
		//总区块数
		uint64_t total_blocks;
		uint8_t continue_flag;
		vector<block_info_t> blocks;

		virtual void genSign();
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
			ResponseBase::marshal(pk);
			pk << total_blocks << continue_flag;
			marshal_container(pk, blocks);
		}
		virtual void unmarshal(const sox::Unpack &up)
		{
			ResponseBase::unmarshal(up);
			up >> total_blocks >> continue_flag;
			unmarshal_container(up, std::back_inserter(blocks));
		}
	};

	//查询相邻节点，返回5个随机子节点对象
	struct GetRelativesReq : public RequestBase 
	{
		enum {uri = GET_RELATIVES_PROTO_REQ << 8 | FPAY_SID };	

		virtual void genSign();
		//数据签名验证
		virtual bool signValidate() {
			return true;
		}
		virtual void marshal(sox::Pack &pk)
		{
			RequestBase::marshal(pk);
		}
		virtual void unmarshal(const sox::Unpack& up)
		{
			RequestBase::unmarshal(up);
		}
	};

	//返回
	struct GetRelativesRes : public ResponseBase 
	{
		enum {uri = GET_RELATIVES_PROTO_RES << 8 | FPAY_SID };		
		vector<node_info_t> nodes;
		virtual void genSign();
		//数据签名验证
		virtual bool signValidate();
		virtual void marshal(sox::Pack &pk) cuponst
		{
			ResponseBase::marshal(pk);
			marshal_container(pk, nodes);
		}
		virtual void unmarshal(const sox::Unpack &up)
		{
            ResponseBase::unmarshal(up);
			unmarshal_container(up, std::back_inserter(nodes));
		}
	};

	struct PingReq : public RequestBase 
	{
		enum {uri = PING_REQ << 8 | FPAY_SID };
		uint8_t tree_level;
		void genSign();
		bool signValidate(){
			return true;
		}
		virtual void marshal(sox::Pack& pk) const
		{
			RequestBase::marshal(pk);
			pk << tree_level;
		}

		virtual void unmarshal(const sox::Unpack& up)
		{
			RequestBase::unmarshal(up);
			up >> tree_level;
		}

	};
	//返回
	struct PingRes : public ResponseBase 
	{
		enum {uri = GET_RELATIVES_PROTO_RES << 8 | FPAY_SID };
		uint8_t tree_level;
		void genSign();
		bool signValidate() {
			return true;
		}
		virtual void marshal(sox::Pack& pk)
		{
			ResponseBase::marshal(pk);
			pk << tree_level;
		}
		virtual void unmarshal(const sox::Unpack& up)
		{
			ResponseBase::unmarshal(up);
			up >> tree_level;
		}
	};

}

}
#endif /*FPAY_PROTOCOL_H_*/
