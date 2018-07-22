#ifndef  RESCODE_COMMON_H
#define  RESCODE_COMMON_H

/* -------------------------------------------------------------------------
�� 0 ~ 99 ��֤ģ�������;

�� 1xx ˵���Եķ���;

�� 200 ��ʾ����ɹ�;

�� 3xx�Ǹ��߿ͻ�����Ҫ��һ���Ķ���������ɹ���;
�� 4xx�ǿͻ��˵�ԭ���µĳ���;
�� 5xx�Ƿ������˵�ԭ���µĳ���.
	500 ~ 549 ͨ�ô���
	550 ~ 599 locate ��λ������ת����

�� 6xx�� popoc ʹ��

�� 700 ~ 800 �Ự session �������ֶ�

�� 801 ~ 900 ���ŷ��� �������ֶ�

�� ���ش���ʹ��uint16_t����������.
   ------------------------------------------------------------------------- */


/*
login ��½�¼�
AuthService ��֤�����¼�
��֤�����ܺ��ԣ��������½�����жϡ�
���Դ���
*/

#define AUTH_TICKET_OK                  0   /*������֤��ticket ok ԭ���� AUTH_READY0*/
#define AUTH_LINK_OK                    1   /*connection ok  AUTH_READY*/

// AuthService �� srvbase �������������������
#define AUTH_LINK_DESTROY               2   /* �����Ѿ����� */
#define AUTH_LINK_DESTROY_TIMEOUT       3   /* ��������idle-timeou������, ��Ҫ���ڷ����� */

// �������
#define AUTH_EUIDPASS                   4   /* �û���������� */
#define AUTH_EPROXYUIDPASS              5   /* �����û���������� */

// ��������
#define AUTH_ENET                       6   /* ������� */
#define AUTH_EPROCESS                   7   /* ������� */
#define AUTH_DESTROY                    8   /* Auth����Destroy��XXX�ڲ�ʹ������������ */

#define AUTH_ENET_TICKET_TIMO           9   /*�ͻ��˻�ȡticket��ʱ*/
#define AUTH_ENET_LINK_TIMO             10  /*�ͻ�������linkd��ʱ*/
#define AUTH_EPROXY_NET_TICKET_TIMO     11  /*�ͻ��˴����ȡticket��ʱ*/
#define AUTH_EPROXY_NET_LINK_TIMO       12  /*�ͻ��˴����ȡ���ӳ�ʱ*/
#define AUTH_ETICKET                    14  /*�������ܾ��ύ��Ticket*/

#define AUTH_EMAKECOOKIE                20  /*�������ܾ��ύ��Ticket*/

// srvbase �ĵ�½ϸ���¼�
#define PP_MYINFO_OK                    50  // �Լ������ϣ�˽�����û�ȡ�ɹ�
#define PP_MYLIST_OK                    51  // �����б�ͬ�����
#define PP_LOGIN_OK                     52  // ��½���

//////////////////////////////////////////////////////////////////

#define RES_SUCCESS     200     /* ���ܳɹ����,һ������,���ص��� */
#define RES_ACCEPTED    202     /* ���������ѽ���,�Ⱥ��� */
#define RES_NOCHANGED   204     /* ���û�б䣬����CHECKSUM */

#define RES_ERETRY      300     /* ��ʱ�޷�����,�����Ժ����� */

#define RES_EREQUEST    400     /* �����޷����Ĺ������� */
#define RES_EAUTH       401     /* ���������δ����֤,��˺�̨�ܾ���ɹ��� */
#define RES_EPERM       403     /* �Է�ʵ��û��Ȩ(�ɲ��Ǻ�̨������) */
#define RES_ENONEXIST   404     /* Ŀ��(������û�)������ */
#define RES_EACCESS     405     /* ��Ȩ���������� */
#define RES_EQUOTA      406     /* �û����ݴ洢�������޶� */
#define RES_EVOLATILE   407     /* ĳЩ������ʱ�����Ƶ���Դ�Ѿ���ʱ������ */
#define RES_ETIMEOUT    408     /* ������̳�ʱ */
#define RES_ECONFLICT   409     /* ��Դ���߶����ͻ(��������) */
#define RES_EPARAM      414     /* �������ݳ���.(Խ��,����,��������ԭ��) */
#define RES_EDBERROR    415     /* ���ݿ����ʧ�� */
#define RES_EDBNVALID   416     /* ���ݿ���ʱ�����ã�����������ά�� */
#define RES_ECONNURS    417     /* ����URS�Ĵ��� */
#define RES_NODEMOVED   418     /* �ڵ��Ѿ���ת��*/
#define RES_BUFOVERFLOW 419     /* ���ջ��������*/
#define RES_EUNKNOWN    500     /* ������.����ԭ��δ��,���ǲ���͸¶���� */
#define RES_EBUSY       504     /* ��̨æ,�ܾ����� */
#define RES_EPROT       505     /* ��̨��֧�ִ�Э��汾 */
#define RES_EOVERTIMES  453     /* ��������̫���� */
#define RES_EDATANOSYNC	506     /* �����ݿ����ݲ�ͬ��,client��Ҫ���»������ */
#define RES_ENOTENOUGH	507     /* �������� */
#define RES_ENONEXIST_REAL   508     /* Ŀ��(������û�)������ */

//memcache
#define RES_MCNOEXISIT 509
#define RES_EADDBUDDYTOOMUCHTODAY   510     /* ���˽�����Ӻ��ѹ���, ��Ҫ�û��ڶ�������� */
#define RES_EADDBUDDYTOOMUCHFORME   511     /* ���˺��������Ѵﵽ���� */
#define RES_EADDBUDDYTOOMUCHFORBUDDY   512     /* �Է����������Ѵﵽ���� */

/* NOTE:���ĳЩ�ض����ܵ����⺬������� */

/* 550~599 locate ��ת���� begin */
#define RES_ESERVICE    550     /* ��֧�ֵķ��� */
#define RES_EDAEMON     551     /* ������δ�ҵ� */
#define RES_EUNUSABLE   552     /* ������ʱ������ */
#define RES_ECONNMISS   553     /* �ڲ����󣬸���connid�Ҳ������� */
#define RES_EBUFOVR     554     /* ����������ڲ�������ʱ�����岻����ƿ�������á�*/
#define RES_ECONNECT    555     /* �ڲ��������Ӳ���,
                                   ͨ�� IConnectErrorHandle �����Ӧ�ã���Ӧ����д */
#define RES_ESENDREQ    556     /* �����������쳣 */
#define RES_EHASHTYPE   557     /* �ڲ����ô��� */
#define RES_EPACKET     558     /* �������ݰ����� */

#define RES_ELOCATE     559     /* ��λ���󣬷������յ��������Լ������� */

#define RES_LIMITUSER	580		/* ������һ̨�����ϵ�¼������ͬ�˺� */

/* 599 linkd ���� */
#define RES_ETRUNKCLI   599     /* ������ͻ��˷��� trunk */
/* locate ��ת���� end */

/* 600~699 local client error begin */

// general
#define PP_ENETBROKEN      600     /* ���ӶϿ� */
#define PP_EPROCESSEXP     601     /* ����ص���������з����쳣 */
#define PP_ETIMEOUTCTX     602     /* �����ĵȴ���������ʱ */
#define PP_EAPPLYCTX       603     /* XXX. apply �쳣����ʹ����������룬��һ��apply */
#define PP_ESRVDESTROY     604     /* �������ٵ�ʱ�򣬶���ĳЩ�����ڵ������ģ������������ */

// session
#define PP_ESSOPENING      609     /* session opening */
#define PP_ESSNOTREADY     610     /* session not ready */
#define PP_ESSCREATEVIEW   611     /* �Ự������UI-View ʧ�� */
#define PP_ESSSAYSELF      612     /* session û���κ��½����⣬�������� */
#define PP_ESSCTXINDESTROY 613     /* �Ự�����У��������Ļ����� */
#define PP_ECTXINPASSIVE   614     /* ����ģʽ������������ */
#define PP_ESSIDMISSMATCH  615     /* ����Ự�� ssid �ͷ��ص� ssid ��ƥ�� */
#define PP_EDUPSSID        616     /* �����Ự���ص�ssid�ظ��� */
#define PP_ECREATESSID     617     /* �����Ự���ز���ȷ�� ssid */
#define PP_ESSNOTEXIST     618     /* �Ự�������ˣ�һ����Ⱥ�������� */
#define PP_ESSIMNOTTHERE   619     /* �Լ������б����棬һ���Ǳ��߳�ȥ�� */
#define PP_ESSIDUNKNOWN    620     /* ����ʶ�� ssid */
#define PP_EFORUMDISMISSED 621     /* ���ⱻ��ɢ�� */
#define PP_ENOTMEMBER      622     /* ���ǳ�Ա�����⣬Ⱥ�ȣ�����ʧ�� */
#define PP_ESSDISMISSED    623     /* �Ự����ɢ�� */
#define PP_ESSNOTOPEN      624     /* �Ựû��ʵ����������û�б��� */

//session manager
#define RES_CHANNEL_MOVED 625 //Ƶ���Ѿ�Ǩ�Ƶ��°�
#define RES_CHANNEL_NOT_MOVED	626 //Ƶ��δǨ�Ƶ��°�



// parameter
#define PP_EBADUID         630     /* �����uid */
#define PP_EUNKNOWNEXP     631     /* ��֪�����͵��쳣 */
#define PP_ENWPRES         632     /* NofityService ����ע��״̬��֪ͨ */
#define PP_ENWEXIST        633     /* NofityService ֪ͨ(nid) �Ѿ���ע�� */
#define PP_ENWREVOKE       634     /* NofityService ע��ʧ�ܣ�������ƥ�� */
#define PP_EWITHPARENT     635     /* Parent ���뵥���޸ģ����ܺ���������һ���޸� */
#define PP_ESSPARAM        636     /* һ��Ĳ������� */
#define PP_ESUPEREXIT      637     /* Group :: Super can't exit */

#define PP_ENOTIMPLEMENT   640     /* ����û��ʵ�� */

// operate
#define PP_OPERR(op, err)  ((op<<16)|err)
#define PP_GETOP(err)      (((unsigned long)(err))>>16)
#define PP_GETERR(err)     (err & 0xffff)

#define OP_CREATEGINFO     660     /* ����Ⱥ�ĵ�һ����Create Ginfo */
#define OP_JOINGROUP       661     /* uglist.add �� ����Ⱥ */
#define OP_OPENGLIST       662     /* OpenGroupList */

/* local client error end */

//700~800 �Ựsession�������ֶ�
//#define RES_SS_    7xx     /*  */
//701~739  session���д���
//741~759 standard session����
//761~779 room session����
//781~799 group����

// ���ش��󣬿ͻ���Ӧ�ý��ûỰ
// unprotected error for client
#define SS_ESNOTEXIST   701 /*  �� */
#define SS_EUNOTEXIST   702 /* �û����ڻỰ�� */
#define SS_ETIMEOUT     705 /* ��ʱ����ʱ */
#define SS_EBANPENANCE  765 /* ������ */
#define SS_EJOINTOOFAST 766 /* �����˳������ҹ���Ƶ��*/

// ����������һ���ǲ�������ģ�����ʾ��ǰ����ʧ��
// error can be proccessed easily by client
#define SS_EDELIVER         703	 /* ��̨������������� */
#define SS_ENOAUTH          704	 /* ����ԽȨ�� */
#define SS_EINVALIDOBJ      706  /* ���Ϸ��Ĳ�������*/
#define SS_ETOOMANYUSER     707  /* �Ѿ��ﵽ�Ự��������*/
#define SS_EUNOTOPEN        708  /* �û�û�д򿪻Ự�ͷ�����������*/
#define SS_EBANNED          761  /* ������ */
#define SS_ENESTFORUM       763  /* Ƕ�׵Ļ��� */
#define SS_EFOLDERNOTEXIST  764  /* Ŀ¼���߻��ⲻ���� */
#define SS_ECHATTOOFAST     767  /* ��������˵��̫��*/
#define SS_EPRIVATEFORUM    783  /* ˽�еĻ��� */
#define SS_EFOLDERNEMPTY    784  /* ��֯�ṹ�ǿ� */
#define SS_EPROTECTEDFORUM  785  /* �ܱ����Ļ��� */
#define SS_EREJECTAUTO      786  /* �������Զ��ܾ�*/
#define SS_EREJECTADMIN     787  /* ������Ա�Զ��ܾ�*/
#define SS_ETOGROUPLIMIT    788  /* �Ѿ��ﵽȺ��������,�޷�������û�*/

/*
srvbase �Ự������(����)
    ���� "����������" �������档
	��������ȫ�����ûỰ��

	TODO ʶ�����ķ���������
	see protocol/popoc/Session.cpp::onError
*/

//800~900 ����sms�������ֶ�
#define RES_SMS_ELIMIT           801   /* ������������ */
#define RES_SMS_ETOOLONG         802   /* ����̫��     */
#define RES_SMS_EBANNED          803   /* �ֻ�������   */
#define RES_SMS_EINACTIVE        804   /* �Է�û��ע���ֻ� */
#define RES_SMS_EINACTIVE_SELF   805   /* �û�û��ע���ֻ� */
#define RES_SMS_ENETWORK         806   /* ������� */
#define RES_SMS_EPEERPERMIT      807   /* �Է���ֹ���� */
#define RES_SMS_EFREQ            808   /* ���͹��� */
#define RES_SMS_EBANQF           809   /* Ƿ�� */
#define RES_SMS_ELIANTONG        810   /* �Է�����ͨ�ֻ���û��ע�� */
#define RES_SMS_EORDER           811   /* ����û�гɹ� */
#define RES_SMS_ENOTSAME_NET     812   /* ��ͨ�ƶ����ܻ�ͨ */
#define RES_SMS_ENOENOUGH_PAOBI  813   /* �ݱҲ��� */
#define RES_SMS_EBADWORD         814   /* */
#define RES_SMS_EPARAMETER       815   /* �������� */
#define RES_SMS_EOTHER_ERROR     816   /* �������� */

/* roster 900~950 */
#define ROSTER_NOTREADY          901   /* Roster û��׼����, һ����ת������ʧ�� */
#define ROSTER_ERRLOGIN          902   /* NotLogin or NotAnswer or NotOwner */

/*-------------------------------------------
	�����һλ���ֲ����Ƿ�ɹ�������λ����ԭ��
---------------------------------------------*/

#define ISOK(rc)   (0x8000 & rc) /* �жϷ����Ƿ�ɹ� */
#define RSCODE(rc) (0x7fff & rc) /* ��ȡ������ */
#define RSOK(rc)   (0x8000 | rc) /* ���ɳɹ������� */
#define RSERR(rc)  (0x7fff & rc) /* ����ʧ�ܷ����� */
#define RES_NULL   0


//////////////////////////////////////////////////////////
// ���� 10000 ���������ͻ���

//ͨ�ô���
#define RES_ACCOUNT_TOOSHORT  30000 // �ʺ�̫���ˣ�������ڣ�����
#define RES_ACCOUNT_LIMIT     30001 // �ʺų���
#define RES_ACCOUNT_EMPTY     30002 // �ʺ�Ϊ��
#define RES_ACCOUNT_INVALID   30003 // �ʺŸ�ʽ����

#define RES_PASS_EMPTY        30004 // ����Ϊ��
#define RES_PASS_LIMIT        30005 // ���볬��

#define RES_NICK_EMPTY        30006 // �ǳ�Ϊ��
#define RES_NICK_INVALID      30007 // �ǳƸ�ʽ����
#define RES_NICK_LIMIT        30008 // �ǳƳ���

#define RES_YEAR_INVALID      30009 // �������
#define RES_MONTH_INVALID     30010 // �·�����
#define RES_DAY_INVALID       30011 // ��������


//��¼��ר�д���
#define RES_LOGIN_OFFLINENOPASS	30012	//���ߵ�¼��û��������


//ר�д���

//�������������
#define RES_PROXY_SERVEREMPTY    10009 // ����������ĵ�ַΪ��
#define RES_PROXY_SERVER_INVALID 10010 // ����������ĵ�ַ����
#define RES_PROXY_PORTEMPTY      10011 // ����������Ķ˿�Ϊ��

//������Ϣ�����
#define RES_UINFO_PROVINCE_EMPTY   10012 // ������Ϣ���ʡ�ֿհ�
#define RES_UINFO_PROVINCE_INVALID 10013 // ������Ϣ���ʡ�ָ�ʽ����
#define RES_UINFO_PROVINCE_LIMIT   10014 // ������Ϣ���ʡ�ֳ���

#define	RES_UINFO_NOTENICK_INVALID 10015 // ������Ϣ��ı�ע�ǳƳ���
#define RES_UINFO_NOTENICK_LIMIT   10016 // ������Ϣ��ı�ע�ǳƳ���

//ϵͳ���ÿ����

//����Ⱥ�Ĵ���
#define RES_CLUSTE_NAME_EMPTY        10017 // Ⱥ����Ϊ��
#define RES_CLUSTE_NAME_INVALID      10018 // Ⱥ���Ƹ�ʽ����
#define RES_CLUSTE_NAME_LIMIT        10019 // Ⱥ���Ƴ���
#define	RES_CLUSTE_TYPE_EMPTY        10020 // Ⱥ����Ϊ��
#define	RES_CLUSTE_CARD_NICK_INVALID 10021 // Ⱥ��Ƭ���ǳƳ���
#define	RES_CLUSTE_CARD_NICK_LIMIT   10022 // Ⱥ��Ƭ���ǳƳ���
#define	RES_CLUSTE_CARD_NOTE_INVALID 10023 // Ⱥ��Ƭ�еı�ע����
#define RES_CLUSTE_CARD_NOTE_LIMIT   10024 // Ⱥ��Ƭ�еı�ע����

//����Ⱥ�Ĵ���
#define RES_FINDCLUSTE_ACCOUNT_EMPTY 10025 // Ⱥ�������ʺ�Ϊ��
#define RES_FINDCLUSTE_ACCOUNT_LIMIT 10026 // Ⱥ�������ʺų���

#define RES_FINDCLUSTE_NAME_EMPTY   10027 // Ⱥ����������Ϊ��
#define RES_FINDCLUSTE_NAME_LIMIT   10028 // Ⱥ���������ֳ���

#define	RES_FINDCLUSTE_TYPE_INVALID 10029 // Ⱥ��������������


//IM����
#define RES_REACHE_MAX_OFFLINEMSG	10030 //�������������Ϣ����
#define RES_ALREDY_BUDDY			10031 //�Ѿ��������Լ��ĺ����б���
#define RES_CANNOT_ADD_SELE			10032 //�����Լ����Լ�Ϊ����
#define RES_SNEDMSG_FAIL_NOT_BUDDY	10033 //�����Լ��ĺ���
//wuji start
#define RES_IM_ANSWER_NOT_RIGHT		10034 //�ش����ⲻ��
#define RES_IM_JIFEN_NOT_RIGHT		10035 //���ֲ���
#define RES_IM_RECEIVER_NOT_ONLINE		10036 //�����˲�����
// ע�����UDB�ķ��ؽ��
#define RES_REG_UDB_SUCCESS				200				// �ɹ�ע��
#define RES_REG_UDB_TOO_OFTEN			10099			// ͬipע�������󣬾ܾ�����
#define RES_REG_UDB_LINK_UDB_FAIL		10100			// ������udb
#define RES_REG_UDB_NO_AUTH				10101			// û�з���Ȩ��
#define RES_REG_UDB_ILLEGAL_USERNAME	10102			// �û������ϸ�
#define RES_REG_UDB_DUP_USERNAME		10103			// �û����ظ�
#define RES_REG_UDB_WRONG_PASSWD		10104			// �������
#define RES_REG_UDB_WRONG_EMAIL			10105			// email����
#define RES_REG_UDB_DUP_EMAIL			10106			// email�ظ�
#define RES_REG_UDB_WRONG_BIRTHDAY		10107			// ���մ���
#define RES_REG_UDB_INS_USERINFO_ERR	10108			// �û���Ϣ������
#define RES_REG_UDB_INS_USERINFO_EX_ERR	10109			// �û���չ��Ϣ������
#define RES_REG_UDB_WRONG_MAC			10110			// mac ��֤����
#define RES_REG_UDB_UNKNOWN				10111			// δ֪����

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

#define RES_GPROP_DB_ERROR          20100 // ���ݿ����
#define RES_GPROP_NO_GROUP_INF      20101 // û�в��ҵ�Ⱥ��������Ϣ
#define RES_GPROP_NO_FOLDER_INF     20102 // û�в��ҵ���Ӧ�����Ϣ
#define RES_GPROP_INVALID_CHANNEL   RES_GINFO_INVALID_CHANNEL //Just an alias

#define RES_IM_CHAT_FAIL            10118 //��������ʧ��
#define RES_GROUP_CHAT_FAIL         10119 //Ⱥ��ʱ����ʧ��
#define RES_GVERIFYCODE_NEED        10200 //��Ҫ��֤��
#define RES_GVERIFYCODE_PASS        10201 //��֤����֤ͨ��
#define RES_GVERIFYCODE_FAIL        10202 //��֤����֤ʧ��
#define RES_GVERIFYCODE_TIMEOUT     10203 //��֤�볬ʱ

typedef unsigned short RES_CODE;

#endif  /* !_RES_CODE_H_ */

/*
    vim: set et ts=4 sts=4 syn=cpp :
 */
