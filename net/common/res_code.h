#ifndef  RESCODE_COMMON_H
#define  RESCODE_COMMON_H

/* -------------------------------------------------------------------------
★ 0 ~ 99 验证模块错误码;

★ 1xx 说明性的返回;

★ 200 表示处理成功;

★ 3xx是告诉客户端需要进一步的动作才能完成功能;
★ 4xx是客户端的原因导致的出错;
★ 5xx是服务器端的原因导致的出错.
	500 ~ 549 通用错误
	550 ~ 599 locate 定位服务中转错误

★ 6xx是 popoc 使用

★ 700 ~ 800 会话 session 错误码字段

★ 801 ~ 900 短信服务 错误码字段

★ 返回代码使用uint16_t的数据类型.
   ------------------------------------------------------------------------- */


/*
login 登陆事件
AuthService 验证服务事件
验证错误不能忽略，会引起登陆流程中断。
忽略错误
*/

#define AUTH_TICKET_OK                  0   /*密码验证，ticket ok 原来的 AUTH_READY0*/
#define AUTH_LINK_OK                    1   /*connection ok  AUTH_READY*/

// AuthService 与 srvbase 交互，用来拆除绑定连接
#define AUTH_LINK_DESTROY               2   /* 链接已经销毁 */
#define AUTH_LINK_DESTROY_TIMEOUT       3   /* 链接由于idle-timeou而销毁, 主要用于服务器 */

// 流程相关
#define AUTH_EUIDPASS                   4   /* 用户名密码错误 */
#define AUTH_EPROXYUIDPASS              5   /* 代理用户名密码错误 */

// 其他错误
#define AUTH_ENET                       6   /* 网络错误 */
#define AUTH_EPROCESS                   7   /* 处理错误 */
#define AUTH_DESTROY                    8   /* Auth服务Destroy，XXX内部使用作其他作用 */

#define AUTH_ENET_TICKET_TIMO           9   /*客户端获取ticket超时*/
#define AUTH_ENET_LINK_TIMO             10  /*客户端链接linkd超时*/
#define AUTH_EPROXY_NET_TICKET_TIMO     11  /*客户端代理获取ticket超时*/
#define AUTH_EPROXY_NET_LINK_TIMO       12  /*客户端代理获取链接超时*/
#define AUTH_ETICKET                    14  /*服务器拒绝提交的Ticket*/

#define AUTH_EMAKECOOKIE                20  /*服务器拒绝提交的Ticket*/

// srvbase 的登陆细分事件
#define PP_MYINFO_OK                    50  // 自己的资料，私有配置获取成功
#define PP_MYLIST_OK                    51  // 好友列表同步完成
#define PP_LOGIN_OK                     52  // 登陆完成

//////////////////////////////////////////////////////////////////

#define RES_SUCCESS     200     /* 功能成功完成,一切正常,不必担心 */
#define RES_ACCEPTED    202     /* 功能请求已接受,等候处理 */
#define RES_NOCHANGED   204     /* 结果没有变，用于CHECKSUM */

#define RES_ERETRY      300     /* 暂时无法受理,建议稍后再试 */

#define RES_EREQUEST    400     /* 碰到无法理解的功能请求 */
#define RES_EAUTH       401     /* 请求者身份未经认证,因此后台拒绝完成功能 */
#define RES_EPERM       403     /* 对方实体没授权(可不是后台不受理) */
#define RES_ENONEXIST   404     /* 目标(对象或用户)不存在 */
#define RES_EACCESS     405     /* 无权限请求此项功能 */
#define RES_EQUOTA      406     /* 用户数据存储超过了限额 */
#define RES_EVOLATILE   407     /* 某些有生存时间限制的资源已经超时不可用 */
#define RES_ETIMEOUT    408     /* 请求过程超时 */
#define RES_ECONFLICT   409     /* 资源或者对象冲突(比如重名) */
#define RES_EPARAM      414     /* 参数数据出错.(越界,超长,不完整等原因) */
#define RES_EDBERROR    415     /* 数据库操作失败 */
#define RES_EDBNVALID   416     /* 数据库暂时不可用，可能是正在维护 */
#define RES_ECONNURS    417     /* 连接URS的错误 */
#define RES_NODEMOVED   418     /* 节点已经被转移*/
#define RES_BUFOVERFLOW 419     /* 接收缓冲区溢出*/
#define RES_EUNKNOWN    500     /* 出错了.但是原因未明,或是不便透露给你 */
#define RES_EBUSY       504     /* 后台忙,拒绝处理 */
#define RES_EPROT       505     /* 后台不支持此协议版本 */
#define RES_EOVERTIMES  453     /* 操作次数太多了 */
#define RES_EDATANOSYNC	506     /* 与数据库数据不同步,client需要重新获得数据 */
#define RES_ENOTENOUGH	507     /* 数量不够 */
#define RES_ENONEXIST_REAL   508     /* 目标(对象或用户)不存在 */

//memcache
#define RES_MCNOEXISIT 509
#define RES_EADDBUDDYTOOMUCHTODAY   510     /* 本人今天添加好友过多, 需要用户第二天再添加 */
#define RES_EADDBUDDYTOOMUCHFORME   511     /* 本人好友总数已达到上限 */
#define RES_EADDBUDDYTOOMUCHFORBUDDY   512     /* 对方好友总数已达到上限 */

/* NOTE:针对某些特定功能的特殊含义待扩充 */

/* 550~599 locate 中转错误 begin */
#define RES_ESERVICE    550     /* 不支持的服务 */
#define RES_EDAEMON     551     /* 服务器未找到 */
#define RES_EUNUSABLE   552     /* 服务暂时不可用 */
#define RES_ECONNMISS   553     /* 内部错误，根据connid找不到连接 */
#define RES_EBUFOVR     554     /* 发送请求给内部服务器时，缓冲不够，瓶颈保护用。*/
#define RES_ECONNECT    555     /* 内部服务连接不上,
                                   通过 IConnectErrorHandle 报告给应用，由应用填写 */
#define RES_ESENDREQ    556     /* 发送请求发生异常 */
#define RES_EHASHTYPE   557     /* 内部配置错误 */
#define RES_EPACKET     558     /* 请求数据包错误 */

#define RES_ELOCATE     559     /* 定位错误，服务器收到不属于自己的请求 */

#define RES_LIMITUSER	580		/* 不允许一台机器上登录两个相同账号 */

/* 599 linkd 错误 */
#define RES_ETRUNKCLI   599     /* 不允许客户端发送 trunk */
/* locate 中转错误 end */

/* 600~699 local client error begin */

// general
#define PP_ENETBROKEN      600     /* 连接断开 */
#define PP_EPROCESSEXP     601     /* 网络回调处理过程中发生异常 */
#define PP_ETIMEOUTCTX     602     /* 上下文等待请求结果超时 */
#define PP_EAPPLYCTX       603     /* XXX. apply 异常，会使用这个错误码，再一次apply */
#define PP_ESRVDESTROY     604     /* 服务销毁的时候，对于某些还存在的上下文，报告这个错误 */

// session
#define PP_ESSOPENING      609     /* session opening */
#define PP_ESSNOTREADY     610     /* session not ready */
#define PP_ESSCREATEVIEW   611     /* 会话，创建UI-View 失败 */
#define PP_ESSSAYSELF      612     /* session 没有任何下接收这，自言自语 */
#define PP_ESSCTXINDESTROY 613     /* 会话销毁中，但上下文还存在 */
#define PP_ECTXINPASSIVE   614     /* 被动模式不能有上下文 */
#define PP_ESSIDMISSMATCH  615     /* 加入会话的 ssid 和返回的 ssid 不匹配 */
#define PP_EDUPSSID        616     /* 创建会话返回的ssid重复了 */
#define PP_ECREATESSID     617     /* 创建会话返回不正确的 ssid */
#define PP_ESSNOTEXIST     618     /* 会话不存在了，一般是群，聊天室 */
#define PP_ESSIMNOTTHERE   619     /* 自己不在列表里面，一般是被踢出去了 */
#define PP_ESSIDUNKNOWN    620     /* 不认识的 ssid */
#define PP_EFORUMDISMISSED 621     /* 话题被解散了 */
#define PP_ENOTMEMBER      622     /* 不是成员，话题，群等，加入失败 */
#define PP_ESSDISMISSED    623     /* 会话被解散了 */
#define PP_ESSNOTOPEN      624     /* 会话没有实例化，根本没有被打开 */

//session manager
#define RES_CHANNEL_MOVED 625 //频道已经迁移到新版
#define RES_CHANNEL_NOT_MOVED	626 //频道未迁移到新版



// parameter
#define PP_EBADUID         630     /* 错误的uid */
#define PP_EUNKNOWNEXP     631     /* 不知道类型的异常 */
#define PP_ENWPRES         632     /* NofityService 不能注册状态的通知 */
#define PP_ENWEXIST        633     /* NofityService 通知(nid) 已经被注册 */
#define PP_ENWREVOKE       634     /* NofityService 注销失败，参数不匹配 */
#define PP_EWITHPARENT     635     /* Parent 必须单独修改，不能和其他属性一起修改 */
#define PP_ESSPARAM        636     /* 一般的参数错误 */
#define PP_ESUPEREXIT      637     /* Group :: Super can't exit */

#define PP_ENOTIMPLEMENT   640     /* 操作没有实现 */

// operate
#define PP_OPERR(op, err)  ((op<<16)|err)
#define PP_GETOP(err)      (((unsigned long)(err))>>16)
#define PP_GETERR(err)     (err & 0xffff)

#define OP_CREATEGINFO     660     /* 创建群的第一步，Create Ginfo */
#define OP_JOINGROUP       661     /* uglist.add ， 加入群 */
#define OP_OPENGLIST       662     /* OpenGroupList */

/* local client error end */

//700~800 会话session错误码字段
//#define RES_SS_    7xx     /*  */
//701~739  session公有错误
//741~759 standard session错误
//761~779 room session错误
//781~799 group错误

// 严重错误，客户端应该禁用会话
// unprotected error for client
#define SS_ESNOTEXIST   701 /*  · */
#define SS_EUNOTEXIST   702 /* 用户不在会话中 */
#define SS_ETIMEOUT     705 /* 定时器操时 */
#define SS_EBANPENANCE  765 /* 禁闭室 */
#define SS_EJOINTOOFAST 766 /* 加入退出聊天室过于频繁*/

// 非致命错误，一般是操作引起的，仅表示当前操作失败
// error can be proccessed easily by client
#define SS_EDELIVER         703	 /* 后台服务器传输错误 */
#define SS_ENOAUTH          704	 /* 请求越权了 */
#define SS_EINVALIDOBJ      706  /* 不合法的操作对象*/
#define SS_ETOOMANYUSER     707  /* 已经达到会话人数上限*/
#define SS_EUNOTOPEN        708  /* 用户没有打开会话就发送其它请求*/
#define SS_EBANNED          761  /* 黑名单 */
#define SS_ENESTFORUM       763  /* 嵌套的话题 */
#define SS_EFOLDERNOTEXIST  764  /* 目录或者话题不存在 */
#define SS_ECHATTOOFAST     767  /* 聊天室里说话太快*/
#define SS_EPRIVATEFORUM    783  /* 私有的话题 */
#define SS_EFOLDERNEMPTY    784  /* 组织结构非空 */
#define SS_EPROTECTEDFORUM  785  /* 受保护的话题 */
#define SS_EREJECTAUTO      786  /* 被设置自动拒绝*/
#define SS_EREJECTADMIN     787  /* 被管理员自动拒绝*/
#define SS_ETOGROUPLIMIT    788  /* 已经达到群人数上限,无法再添加用户*/

/*
srvbase 会话错误处理(从严)
    对于 "非致命错误" 仅仅报告。
	其他错误全部禁用会话。

	TODO 识别更多的非致命错误
	see protocol/popoc/Session.cpp::onError
*/

//800~900 短信sms错误码字段
#define RES_SMS_ELIMIT           801   /* 超过发送限制 */
#define RES_SMS_ETOOLONG         802   /* 文字太长     */
#define RES_SMS_EBANNED          803   /* 手机被禁用   */
#define RES_SMS_EINACTIVE        804   /* 对方没有注册手机 */
#define RES_SMS_EINACTIVE_SELF   805   /* 用户没有注册手机 */
#define RES_SMS_ENETWORK         806   /* 网络错误 */
#define RES_SMS_EPEERPERMIT      807   /* 对方禁止接收 */
#define RES_SMS_EFREQ            808   /* 发送过快 */
#define RES_SMS_EBANQF           809   /* 欠费 */
#define RES_SMS_ELIANTONG        810   /* 对方是联通手机，没有注册 */
#define RES_SMS_EORDER           811   /* 定制没有成功 */
#define RES_SMS_ENOTSAME_NET     812   /* 联通移动不能互通 */
#define RES_SMS_ENOENOUGH_PAOBI  813   /* 泡币不足 */
#define RES_SMS_EBADWORD         814   /* */
#define RES_SMS_EPARAMETER       815   /* 参数错误 */
#define RES_SMS_EOTHER_ERROR     816   /* 其它错误 */

/* roster 900~950 */
#define ROSTER_NOTREADY          901   /* Roster 没有准备好, 一般是转载数据失败 */
#define ROSTER_ERRLOGIN          902   /* NotLogin or NotAnswer or NotOwner */

/*-------------------------------------------
	用最高一位区分操作是否成功，其他位代表原因。
---------------------------------------------*/

#define ISOK(rc)   (0x8000 & rc) /* 判断返回是否成功 */
#define RSCODE(rc) (0x7fff & rc) /* 提取返回码 */
#define RSOK(rc)   (0x8000 | rc) /* 生成成功返回码 */
#define RSERR(rc)  (0x7fff & rc) /* 生成失败返回码 */
#define RES_NULL   0


//////////////////////////////////////////////////////////
// 大于 10000 都保留给客户端

//通用错误
#define RES_ACCOUNT_TOOSHORT  30000 // 帐号太短了，必须大于３个字
#define RES_ACCOUNT_LIMIT     30001 // 帐号超长
#define RES_ACCOUNT_EMPTY     30002 // 帐号为空
#define RES_ACCOUNT_INVALID   30003 // 帐号格式出错

#define RES_PASS_EMPTY        30004 // 密码为空
#define RES_PASS_LIMIT        30005 // 密码超长

#define RES_NICK_EMPTY        30006 // 昵称为空
#define RES_NICK_INVALID      30007 // 昵称格式出错
#define RES_NICK_LIMIT        30008 // 昵称超长

#define RES_YEAR_INVALID      30009 // 年份有误
#define RES_MONTH_INVALID     30010 // 月份有误
#define RES_DAY_INVALID       30011 // 日期有误


//登录的专有错误
#define RES_LOGIN_OFFLINENOPASS	30012	//离线登录但没保存密码


//专有错误

//代理服务器设置
#define RES_PROXY_SERVEREMPTY    10009 // 代理服务器的地址为空
#define RES_PROXY_SERVER_INVALID 10010 // 代理服务器的地址出错
#define RES_PROXY_PORTEMPTY      10011 // 代理服务器的端口为空

//个人信息框错误
#define RES_UINFO_PROVINCE_EMPTY   10012 // 个人信息框的省分空白
#define RES_UINFO_PROVINCE_INVALID 10013 // 个人信息框的省分格式出错
#define RES_UINFO_PROVINCE_LIMIT   10014 // 个人信息框的省分超长

#define	RES_UINFO_NOTENICK_INVALID 10015 // 个人信息框的备注昵称出错
#define RES_UINFO_NOTENICK_LIMIT   10016 // 个人信息框的备注昵称超长

//系统配置框错误

//创建群的错误
#define RES_CLUSTE_NAME_EMPTY        10017 // 群名称为空
#define RES_CLUSTE_NAME_INVALID      10018 // 群名称格式出错
#define RES_CLUSTE_NAME_LIMIT        10019 // 群名称超长
#define	RES_CLUSTE_TYPE_EMPTY        10020 // 群类型为空
#define	RES_CLUSTE_CARD_NICK_INVALID 10021 // 群名片的昵称出错
#define	RES_CLUSTE_CARD_NICK_LIMIT   10022 // 群名片的昵称超长
#define	RES_CLUSTE_CARD_NOTE_INVALID 10023 // 群名片中的备注出错
#define RES_CLUSTE_CARD_NOTE_LIMIT   10024 // 群名片中的备注超长

//查找群的错误
#define RES_FINDCLUSTE_ACCOUNT_EMPTY 10025 // 群搜索的帐号为空
#define RES_FINDCLUSTE_ACCOUNT_LIMIT 10026 // 群搜索的帐号超长

#define RES_FINDCLUSTE_NAME_EMPTY   10027 // 群搜索的名字为空
#define RES_FINDCLUSTE_NAME_LIMIT   10028 // 群搜索的名字超长

#define	RES_FINDCLUSTE_TYPE_INVALID 10029 // 群搜索的类型有误


//IM错误
#define RES_REACHE_MAX_OFFLINEMSG	10030 //到达最大离线消息数量
#define RES_ALREDY_BUDDY			10031 //已经存在于自己的好友列表中
#define RES_CANNOT_ADD_SELE			10032 //不能自己加自己为好友
#define RES_SNEDMSG_FAIL_NOT_BUDDY	10033 //不是自己的好友
//wuji start
#define RES_IM_ANSWER_NOT_RIGHT		10034 //回答问题不对
#define RES_IM_JIFEN_NOT_RIGHT		10035 //积分不够
#define RES_IM_RECEIVER_NOT_ONLINE		10036 //接收人不在线
// 注册多玩UDB的返回结果
#define RES_REG_UDB_SUCCESS				200				// 成功注册
#define RES_REG_UDB_TOO_OFTEN			10099			// 同ip注册数过大，拒绝服务
#define RES_REG_UDB_LINK_UDB_FAIL		10100			// 连不上udb
#define RES_REG_UDB_NO_AUTH				10101			// 没有访问权限
#define RES_REG_UDB_ILLEGAL_USERNAME	10102			// 用户名不合格
#define RES_REG_UDB_DUP_USERNAME		10103			// 用户名重复
#define RES_REG_UDB_WRONG_PASSWD		10104			// 密码错误
#define RES_REG_UDB_WRONG_EMAIL			10105			// email错误
#define RES_REG_UDB_DUP_EMAIL			10106			// email重复
#define RES_REG_UDB_WRONG_BIRTHDAY		10107			// 生日错误
#define RES_REG_UDB_INS_USERINFO_ERR	10108			// 用户信息入库错误
#define RES_REG_UDB_INS_USERINFO_EX_ERR	10109			// 用户扩展信息入库错误
#define RES_REG_UDB_WRONG_MAC			10110			// mac 验证错误
#define RES_REG_UDB_UNKNOWN				10111			// 未知错误

#define RES_GINFO_NO_PERMISSION     10100 //Requester is not an OW/admin of the group/folder
#define RES_GINFO_DB_ERROR          10101 //General SQL failure
#define RES_GINFO_NO_CHANGE         10102 //Nothing is changed in DB by the operation (eg, deleted nothing)
#define RES_GINFO_GEN_FAIL          10103 //General failure
#define RES_GINFO_EXCEEDED          10104 //# of owned group exceeds, # of admins exceeds, etc
#define RES_GINFO_ON_GOING          10105 //Last same operation hasn't been finished
#define RES_GINFO_OW_NOT_ALLOWED    10110 //Group owner cannot quit a group/be added as admin
#define RES_GINFO_NOT_MEMBER        10111 //New owner is not a group member
#define RES_GINFO_INVALID_FOLDER    10112 //Target folder is invalid
#define RES_GINFO_COOKIE_ERROR      10113 //Cookie(checksum) in the invitation message is incorrect
#define RES_GINFO_ADMIN_REJECT      10114 //Request is rejected by the admin
#define RES_GINFO_EXPIRED           10115 //The invitation code is expired or the count is used up
#define RES_GINFO_BAD_ACT_CODE      10116 //The activation code is expired or fake
#define RES_GINFO_NOT_EMPTY         10117 //Group/folder is not empty
#define RES_GINFO_INVALID_CHANNEL   10118 //Channel doesn't exist
#define RES_GINFO_NOT_CH_MEMBER     10119 //Requester is not a role of the channel
#define RES_GINFO_IMPORT_EXCEEDED   10120

#define RES_GPROP_DB_ERROR          20100 // 数据库错误
#define RES_GPROP_NO_GROUP_INF      20101 // 没有查找到群的属性信息
#define RES_GPROP_NO_FOLDER_INF     20102 // 没有查找到相应组的信息
#define RES_GPROP_INVALID_CHANNEL   RES_GINFO_INVALID_CHANNEL //Just an alias

#define RES_IM_CHAT_FAIL            10118 //好友聊天失败
#define RES_GROUP_CHAT_FAIL         10119 //群临时聊天失败
#define RES_GVERIFYCODE_NEED        10200 //需要验证码
#define RES_GVERIFYCODE_PASS        10201 //验证码验证通过
#define RES_GVERIFYCODE_FAIL        10202 //验证码验证失败
#define RES_GVERIFYCODE_TIMEOUT     10203 //验证码超时

typedef unsigned short RES_CODE;

#endif  /* !_RES_CODE_H_ */

/*
    vim: set et ts=4 sts=4 syn=cpp :
 */
