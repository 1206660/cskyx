/**********************************************************
   user.cpp
   mt-svr

   Created by cui on 13-11-27.
   Copyright (c) 2013年 Locojoy. All rights reserved.

   说明:
       玩家定义
************************************************************/

#include <stdint.h>
#include <algorithm>
#include "globalconfig.h"
#include "user.h"
//#include "table.h"
#include "tabdefs.h"
#include "log.h"
#include "const_sqlstatement.h"
#include "db_proxy_new.h"
#include "defines.h"
#include "gamecfg.h"
#include "gem.h"
#ifdef __GNUC__
#define stdext  __gnu_cxx
#endif

#include "commutil.h"
#if defined(_LINUX) || defined(__APPLE__)
#include <ext/hash_map>
#elif _WIN32
#include <hash_map>
#endif 
#include "util.h"
/*#include "encoding.h"*/
#include "combatflow.h"
#include "playertab.h"
#include "requestfriendmng.h"
#include "cdtimetab.h"
#include "viptable.h"
#include "buyfriendtab.h"
#include "awardtable.h"
#include "opsourceidmanager.h"
//#include "cs-reqequipsharpen.h"
//#include "cs-reqequipreplace.h"
#include "leveuptab.h"
#include "vipawardtab.h"
#include "buyincreasetab.h"
#include "consumedef.h"
#include "outputaward.h"
#include "mem_buffer.h"
//#include "arenasortmgr.h"
#include "sysmailmgr.h"
#include "arenarobottab.h"
#include "lootdef.h"
#include "memlock.h"
#include "herostab.h"
#include "activitytab.h"
#include "itembuytab.h"
#include "globalfunc.h"
#include "hotupdatetab.h"
#include "arena_asortmgr.h"

//将int8_t数组转换为int32
//将int8_t数组转换为int32

uint32_t DataDirtyHelper::sDirtyStamp = 0;

int32_t g_Int8ArrayToInt32(int8_t* ids, int8_t nsize)
{
	if((NULL == ids) || (nsize < 1) || (nsize > 4))
		return -1 ;
	int32_t nValue ;
	int8_t* pValue = (int8_t*)&nValue ;
	for(int i=0; i<nsize; i++)
	{
		pValue[i] = ids[i] ;
	}
	return nValue ;
}

//将int32数组转换为int8数组
int g_Int32ToInt8Array(int32_t nValue, int8_t* ids, int8_t nsize) 
{
	if((NULL == ids) || (nsize < 1) || (nsize > 4))
		return -1 ;
	int8_t* pValue = (int8_t*)&nValue ;
	for(int i=0; i<nsize; i++)
	{
		ids[i] = pValue[i];
	}
	return 0 ;
}

/*
	只用于机器人生成guid,此类机器人为巅峰对决使用
*/
int64_t	GeneratePeakVsRobotGuid()
{
	static int32_t Index = 0;
	Index++;

	user_id_t userid;

	userid.setSvrId(server.server_serial_number);
	userid.setUserId(Index);	//userId从1开始

	return userid.toInt64();
}

/*
	只用于机器人生成guid
*/
int64_t	GenerateRobotGuid()
{
	static int32_t Index = 0;
	Index++;

	user_id_t userid;

	userid.setSvrId(server.server_serial_number);
	userid.setUserId(Index);	//userId从1开始

	return userid.toInt64();
}
 
int64_t	GetFirstRobotGuid()
{
	int32_t Index = 1;
	user_id_t userid;
	userid.setSvrId(server.server_serial_number);
	userid.setUserId(Index);	//userId从1开始
	return userid.toInt64();
}

bool IsRobot( int64_t userid )
{
	user_id_t _userid(userid); 
	if(_userid.getUserId()<BASEGUID_ROBOT) 
		return  true; 
	return false;
}

CUser* g_CurrUser = NULL ;
CUser::CUser(void) 
{
	func_start();
	clear();
	uoid = -1;
	func_end();
}

CUser::~CUser(void) {
    
}

void CUser::clear()
{
	func_start();
	key = -1;
	propertys.clear();
	//normal_mail_info.clear();
	propertys.clear();
	userrmb.clear() ;
	AccountPlatErrKey = -1;
	BindPlatCount = 0;

	heros.clear() ;

	FBStatus.clear();
	DailyMission.Clear() ;
	MissionData.Clear() ;

	m_UserFlag = userflag_normal;
	m_LanguageFlag = e_zh_CN ;
	_Dig.clear();

	//好友相关
	m_FriendMgr.Clear();
	invite_head = 0;
	invite_end  = EACH_REFURBISH_NUM-1;
	//RandInviteIndex();
	refurbishNum = 0;
	refPoolStatus = eRefUserPoolIndexStatus_UnDone;
	userEQLastRefTime = (uint32_t)time(NULL);
	m_InviteComabtMng.Clear();
	ClearPayment() ;

	m_nBindAccountPlatState = eBindAccountPlatState_uninit;
	m_Bag.Clear();
	m_SignMng.Clear();
	m_prepBattleMng.Clear();
// 	_maxcombatpower = 0;
	m_CombatFlow.Clear() ;

	_guideinfo.clear();
	userAward.clear();
	_arena.clear();
	_Dig.clear();
	m_LastPvpAttackRecord.clear() ;
	_update.clear();
	m_LoginAward.Clear();
	_herotrial.clear();
	peakvs.clear();
	smallendless.clear();
	endless.clear();
	m_ChargeInfoMng.Clear() ;
	m_ZodiacMng.Clear();
	m_ShopModule.clear();
	for(int i = 0; i < eShop_max; ++ i)
	{
		for(int j = 0; j < TOTAL_ITEM_CNT; ++ j)
		{
			for(int k = 0; k < MAX_GEM_MAIN_ATT_COUNT+MAX_GEM_ADDITIONAL_ATT_COUNT; ++ k)
			{
				m_ShopGemRandomData[i][j][k] = -1;
			}
		}
	}

	m_TradeMgr.clear();
	m_Loot.clear();
	//公告状态数据
	m_noticelastIndex = -1 ;
	m_noticelastId = -1 ;
	memset(m_ChatNoticeCheckMask, 0, sizeof(m_ChatNoticeCheckMask));
	m_ChatMsgId = -1;
	m_LastChatTime = 0;
	m_LastChatTimeLaba = 0;
	_FirstRecharge.clear();
	_VipAward.clear();
	_CommonActivity.clear();
	m_LoadFlag = eUserLoadFlag_TmpLoad;
	m_FriendAwardData.clear();
	m_HolidayItemExchange.clear();
	m_JYCombatCnt=0;
	m_JYDropHunShiCnt=0;
	func_end();
}

std::string CUser::getDebugString(void)
{
	std::string _rev ;
	char _tmpChars[1024] = {0};
	snprintf(_tmpChars, sizeof(_tmpChars), "userid:%lld, account=%s, devideid=%s, username=%s, channel=%s, platformId=%s", 
		GetUserID(), propertys.account.to_string(), propertys.devideid, propertys.username, propertys.Channel, propertys.PlatformId) ;
	_rev.append(_tmpChars) ; 
	return _rev ;
}

/*创建角色时初始化*/
int CUser::init(const char* szDeviceID,const char* username,uint8_t constellation,int64_t userid,const char* szChannel,const char* szPlatformId,int32_t serverId)
{
    func_start();

	if (0 == strlen(username))
	{
		LOGERR("CUser::init,username is null");
		return -1;
	}

	if (0 == strlen(szChannel))
	{
		LOGERR("CUser::init,szChannel is null");
		return -2;
	}

	if (0 == strlen(szPlatformId))
	{
		LOGERR("CUser::init,PlatformId is null");
		return -3;
	}

	if (!CheckUserName(username)) {
		return -4;
	}

	strncpy(propertys.devideid, szDeviceID, sizeof(propertys.devideid));
    strncpy(propertys.username, username, sizeof(propertys.username));
	propertys.username[MAX_USER_NAME_LEN]='\0';
	propertys.constellation = constellation;
	propertys.userid = userid;
	propertys.ServerId = serverId;
 
	strncpy(propertys.Channel, szChannel, sizeof(propertys.Channel));
	strncpy(propertys.PlatformId, szPlatformId, sizeof(propertys.PlatformId));


	if (server.DebugModel)
	{
		propertys.lvl = CGameConfigMgr::Instance()->GetConfigData().DefaultPlayerLvl;
	}
	else
	{
		propertys.lvl = 1;
	}
	LOG_USER_LEVEL_UP(this);
	
	propertys.register_time = (uint32_t)time(NULL);
	propertys.refurbish_physical_time = propertys.register_time ;
	propertys.cur_physical_strength = CGameConfigMgr::Instance()->GetConfigData().DefaultPhysicalStrength ;
	propertys.golds = CGameConfigMgr::Instance()->GetConfigData().DefaultGolds ;
	userrmb.m_donate = CGameConfigMgr::Instance()->GetConfigData().DefaultDiamonds;
	propertys.skillpoint = CGameConfigMgr::Instance()->GetConfigData().SkillPoint ;

	//重新设置战斗标志
	CombatEnd();

	m_nBindAccountPlatState = eBindAccountPlatState_uninit;
	m_Bag.Init(this);

	DailyMission.Init(this) ;
	MissionData.Init(this) ;
	m_SignMng.InitUser(this);
	m_prepBattleMng.init(this);
	m_ZodiacMng.OnInit(this);
	_arena.Init(this);
	m_ChargeInfoMng.Init(this) ;
	_Dig.Init(this);

	m_ShopModule.init(this);
	if (server.DebugModel)
	{
		for (int i = eShop_arena; i < eShop_max; ++i)
		{
			m_ShopModule.add_point(i,CGameConfigMgr::Instance()->GetConfigData().ShopPoint);
		}
	}

	m_TradeMgr.init(this);

	_herotrial.Init(this);
	peakvs.Init(this);
	smallendless.Init(this);
	endless.Init(this);
	CalcVipLevel(true);

	FBStatus.Init(this);
	_FirstRecharge.Init(this);
	_VipAward.Init(this);
	_CommonActivity.Init(this);
	m_FriendAwardData.Init(this);

	//新用户副本状态初始化
	if(MT_OK != FBStatus.InitFbStatus(DF_DEFAULT_OPEN_FB))
	{
		LOGERR("CUser::init(), new user init fb status fail!(UserId=%lld,DF_DEFAULT_OPEN_FB=%d)", propertys.userid, DF_DEFAULT_OPEN_FB);
		return -5;
	}

	return 0 ;
    func_end();
}
/*
服务器重启后, 用户首次登录后初始化
此函数只做各子模块指针初化不做数据处理
*/

int CUser::init()
{
	func_start();

	/*初始化用户基本信息*/
	if (0 == propertys.lvl)
	{
		return -1;
	}

	m_Bag.Init(this);
	DailyMission.SetUser(this) ;
	MissionData.SetUser(this) ;
	m_SignMng.InitUser(this);
	m_prepBattleMng.init(this);
	m_ZodiacMng.OnInit(this);
	_arena.Init(this);
	m_ChargeInfoMng.Init(this) ;
	_herotrial.Init(this);
	peakvs.Init(this);
	smallendless.Init(this);
	endless.Init(this);
	_Dig.Init(this);
	m_ShopModule.init(this);
	m_TradeMgr.init(this);
	FBStatus.Init(this);
	_FirstRecharge.Init(this);
	_VipAward.Init(this);
	_CommonActivity.Init(this);
	m_FriendAwardData.Init(this);
	CalcVipLevel(true);
	return 0;
	func_end();
}

/*设置玩家数据载入标识*/
void CUser::SetLoadFlag(eUserLoadFlag flag)
{
	m_LoadFlag = flag ;
}

eUserLoadFlag CUser::GetLoadFlag()
{
	return m_LoadFlag;
}

/*初始化基础信息*/
int CUser::InitUserBaseInfoToMem()
{
	func_start();

	req_friend_info ReqInfo;
	CDataMgr::Instance()->UpdateReqFriendData(propertys.userid,ReqInfo);

	MemSysMails sysmails;
	if (!CSysMailMgr::AddSysMail(propertys.userid,sysmails))
	{
		LOGERR("CUser::InitUserBaseInfoToMem AddSysMail()return failed. userid=%lld", propertys.userid);
		return MT_ERR;
	} 

	//帐号封停相关
	MemForbidUserInfo forbiduserinfo;
	if (MT_OK != CDataMgr::Instance()->UpdateForbidUserInfo(propertys.userid,forbiduserinfo))
	{
		LOGERR("CUser::InitUserBaseInfoToMem UpdateForbidUserInfo()return failed. userid=%lld", propertys.userid);
		return MT_ERR;
	}
	//禁言相关
	MemForbidSpeakInfo forbidspeakinfo;
	if (MT_OK != CDataMgr::Instance()->UpdateForbidSpeakInfo(propertys.userid,forbidspeakinfo))
	{
		LOGERR("CUser::InitUserBaseInfoToMem UpdateForbidSpeakInfo()return failed. userid=%lld", propertys.userid);
		return MT_ERR;
	}

	//设置玩家竞技场_临界数据
	{
		MemArena_aLockData _lockdata ;
		if(false == CArena_aSortMgr::Instance()->InitLockData(propertys.userid, _lockdata))
		{
			LOGERR("CUser::InitUserBaseInfoToMem() CArena_aSortMgr::Instance()->InitLockData() return fail!macaddr=%s,userid=%lld,userlvl=%u", 
				propertys.macaddr.to_string(), propertys.userid, propertys.lvl);
			return MT_ERR;
		}
	}

	return MT_OK;
	func_end();
}

/*注册时将新生成的用户信息写入数据库*/
int CUser::InitUserInfoToMem()
{
	func_start();

	if (MT_OK != InitUserBaseInfoToMem())
	{
		LOGERR("CUser::InitUserInfoToMem InitUserBaseInfoToMem()return failed. userid=%lld", propertys.userid);
		return MT_ERR;
	}

	if (MT_ERR == WriteInitHeroToDB())
		return MT_ERR;

	if (MT_ERR == WriteInitItemToDB())
		return MT_ERR;

	LOGINFO("init user data ok. userid=%lld,account=%s,macaddr=%s",propertys.userid, propertys.account.to_string(),propertys.macaddr.to_string());

	return MT_OK;
	func_end();
}

/*注册时将新生成的用户信息写入数据库*/
int CUser::WriteAdditionalDataToDB()
{
	//废弃
	return -1 ;
	func_start();

 	if (server.memcached_enable) return MT_OK;
// 	{
// 		req_friend_info ReqInfo;
// 		CDataMgr::Instance()->UpdateReqFriendData(propertys.userid,ReqInfo);
// 		
// 		MemSysMails sysmails;
// 		if (!CSysMailMgr::AddSysMail(propertys.userid,sysmails))
// 		{
// 			LOGERR("CUser::QueryUserAllInfo AddSysMail()return failed. userid=%lld", propertys.userid);
// 			return MT_ERR;
// 		} 
// 
// 		return MT_OK;
// 	}

	/*副本初始化*/
	if (MT_OK != s_dbproxy.WriteFBInfoToDB(propertys.userid, DF_DEFAULT_OPEN_FB, FBStatus.GetFBInfo(DF_DEFAULT_OPEN_FB)))
	{
		LOGERR("Write FB data Fail, account=%s,userid=%lld,macaddr=%s",
			propertys.account.to_string(), propertys.userid, propertys.macaddr.to_string());
		return MT_ERR;
	}

	/*挖宝数据初始化*/
	if (MT_OK != s_dbproxy.WriteDigInfoToDB(propertys.userid, _Dig))
	{
		LOGERR("Write Dig data Fail, account=%s,userid=%lld,macaddr=%s",
			propertys.account.to_string(), propertys.userid, propertys.macaddr.to_string());
		return MT_ERR;
	}

	/*好友数据初始化*/
	if (MT_OK != s_dbproxy.WriteFriendsInfoToDB(propertys.userid, this))
	{
		LOGERR("Write friends data Fail, account=%s,userid=%lld,macaddr=%s",
			propertys.account.to_string(), propertys.userid, propertys.macaddr.to_string());
		return MT_ERR;
	}

	/*钻石数据初始化*/
	if (MT_OK != s_dbproxy.WriteUserRMBToDB(propertys.userid, userrmb))
	{
		LOGERR("Write userrmb data Fail, account=%s,userid=%lld,macaddr=%s",
			propertys.account.to_string(), propertys.userid, propertys.macaddr.to_string());
		return MT_ERR;
	}
	//背包信息
	if (MT_OK != s_dbproxy.WriteBagInfoToDB(propertys.userid,this))
	{
		LOGERR("Write bag data Fail, account=%s,userid=%lld,macaddr=%s",
			propertys.account.to_string(), propertys.userid, propertys.macaddr.to_string());
		return MT_ERR;
	}
	if (MT_OK != s_dbproxy.UpDateUserGemsInfo(propertys.userid,this))
	{
		LOGERR("Write user gems Fail, account=%s,userid=%lld,macaddr=%s",
			propertys.account.to_string(), propertys.userid, propertys.macaddr.to_string());
		return MT_ERR;
	}
	//每日任务
	if (MT_OK != s_dbproxy.WriteDailyMissionToDB(propertys.userid,this))
	{
		LOGERR("Write dailymission data Fail, account=%s,userid=%lld,macaddr=%s",
			propertys.account.to_string(), propertys.userid, propertys.macaddr.to_string());
		return MT_ERR;
	}
	//任务
	if (MT_OK != s_dbproxy.WriteMissionToDB(propertys.userid,this))
	{
		LOGERR("Write mission data Fail, account=%s,userid=%lld,macaddr=%s",
			propertys.account.to_string(), propertys.userid, propertys.macaddr.to_string());
		return MT_ERR;
	}
	//签到
	if (MT_OK != s_dbproxy.WriteSignToDB(propertys.userid,this))
	{
		LOGERR("Write sign data Fail, account=%s,userid=%lld,macaddr=%s",
			propertys.account.to_string(), propertys.userid, propertys.macaddr.to_string());
		return MT_ERR;
	}

	////初始化排序数据表
	//if (MT_OK != s_dbproxy.WriteSortTempToDB(*this))
	//{
	//	LOGERR("Write sorttemp data Fail, account=%s,userid=%lld,macaddr=%s",
	//		propertys.account.to_string(), propertys.userid, propertys.macaddr.to_string());
	//	return MT_ERR;
	//}
	//布阵
	if (MT_OK != s_dbproxy.WritePrepBattleToDB(this))
	{
		LOGERR("Write PrepBattle data Fail, account=%s,userid=%lld,macaddr=%s",
			propertys.account.to_string(), propertys.userid, propertys.macaddr.to_string());
		return MT_ERR;
	}
	//星座爬塔数据
	if (MT_OK != s_dbproxy.WriteZodiacToDB(GetUserID(),this))
	{
		LOGERR("Write Zodiac data Fail, account=%s,userid=%lld,macaddr=%s",
			propertys.account.to_string(), propertys.userid, propertys.macaddr.to_string());
		return MT_ERR;
	}

	//新手引导数据
	if (MT_OK != s_dbproxy.WriteGuideInfoToDB(GetUserID(),_guideinfo))
	{
		LOGERR("Write guide data Fail, account=%s,userid=%lld,macaddr=%s",
			propertys.account.to_string(), propertys.userid, propertys.macaddr.to_string());
		return MT_ERR;
	}
	//用户奖励数据
	if (MT_OK != s_dbproxy.WriteUserAward(GetUserID(),userAward))
	{
		LOGERR("Write guide data Fail, account=%s,userid=%lld,macaddr=%s",
			propertys.account.to_string(), propertys.userid, propertys.macaddr.to_string());
		return MT_ERR;
	}
	//请求好友信息
	req_friend_info* pReqInfo = RequestFriendMng::GetInstance()->NewAndAddReqFriendInfo(propertys.userid);
	if (NULL != pReqInfo)
	{
		if (!server.memcached_enable)
		{
			if (MT_OK != s_dbproxy.WriteFriendsReqToDB(propertys.userid,pReqInfo))
			{
				LOGERR("Write reqfriendinfo data Fail, account=%s,userid=%lld,macaddr=%s",
					propertys.account.to_string(), propertys.userid, propertys.macaddr.to_string());
				return MT_ERR;
			}
		}
	}


	LOGINFO("WRITE user data to DB ok. userid=%lld,account=%s,macaddr=%s",
		propertys.userid, propertys.account.to_string(),propertys.macaddr.to_string());
	return MT_OK;
	func_end();

}

/*注册时初始化英雄并写入数据库*/
int CUser::WriteInitHeroToDB()
{
	func_start();

	uint32_t uPos =10;
	for (uint8_t i=0;i<CGameConfigMgr::Instance()->GetConfigData().DefaultHeroCount;++i)
	{
		int16_t    template_id = CGameConfigMgr::Instance()->GetConfigData().DefaultHeros[i].template_id;
		uint8_t    lvl = CGameConfigMgr::Instance()->GetConfigData().DefaultHeros[i].lvl;		
		//uint8_t    heropos = CGameConfigMgr::Instance()->GetConfigData().DefaultHeros[i].heropos;

		int32_t iRev = addHeroAndWriteDB(template_id,lvl) ;
		if (iRev<0)
		{
			LOGERR("CUser::WriteInitHeroToDB(),add hero failed! CUser::addHeroAndWriteDB() return fail!(%d)userid=%lld,herotemplateid=%d,lvl=%u",
				iRev, GetUserID(),template_id,lvl);
			return MT_ERR ;
		}

		//初始化队长
		if (0==i)
		{
			int8_t bResult = ChangedTeamLeader(template_id);
			if (bResult<0)
			{
				LOGERR("CUser::WriteInitHeroToDB,ChangedTeamLeader() return failed ! heroid=%d",template_id);
				return MT_ERR;
			}
		}
	}
	return MT_OK;
	func_end();
}

/*注册时初始化英雄并写入数据库*/
int CUser::WriteInitItemToDB()
{
	func_start();

	uint32_t uPos =10;
	for (uint8_t i=0;i<CGameConfigMgr::Instance()->GetConfigData().DefaultItemCount;++i)
	{
		int16_t    id = CGameConfigMgr::Instance()->GetConfigData().DefaultItems[i].id;
		uint16_t    num = CGameConfigMgr::Instance()->GetConfigData().DefaultItems[i].num;		

		if (!m_Bag.AddItem(id,num))
		{
			LOGERR("CUser::WriteInitItemToDB(),add item failed! AddItem() return fail!userid=%lld,itemid=%d,num=%u", GetUserID(),id,num);
			return MT_ERR ;
		}
	}

	if (!UpdateBagInfoToDB())
	{
		LOGERR("CUser::WriteInitItemToDB(),add item failed! UpdateBagInfoToDB() return fail!userid=%lld", GetUserID());
		return MT_ERR ;
	}
	return MT_OK;
	func_end();
}

int CUser::QueryUserInfo(account_t& clientmac)
{
	func_start();
	int rst = -1;
	rst = s_dbproxy.QueryUserInfoByClientMac(clientmac, propertys);
	if (MT_OK != rst) {
		LOGERR("DBERR: query user info clientmac=%s", clientmac.to_string());
		return MT_ERR;
	}

	propertys.macaddr = clientmac;

	return QueryUserAllInfo(propertys.userid);

	func_end();
}

int CUser::QueryUserInfo(int64_t userid)
{
	func_start();
	if (userid == -1) {
		LOGERR("DBERR:%s userid=%lld", __PRETTY_FUNCTION__, userid);
		return MT_ERR;
	}
	if (MT_OK != s_dbproxy.QueryUserInfoByUserID(userid, propertys)) {
		LOGERR("DBERR: query user info userid=%lld", userid);
		return MT_ERR;
	}

	return QueryUserAllInfo(propertys.userid);

	func_end();
}

int CUser::QueryUserInfo(const char* szChannel,const char* szPlatformId,int32_t serverId)
{
	func_start();
	if (0 == strlen(szChannel))
	{
		LOGERR("CUser::QueryUserInfo,szChannel is null");
		return MT_ERR;
	}

	if (0 == strlen(szPlatformId))
	{
		LOGERR("CUser::QueryUserInfo,szPlatformId is null");
		return MT_ERR;
	}

	int rst = s_dbproxy.QueryUserInfo(szChannel,szPlatformId,serverId, propertys);
	if (MT_OK != rst)
	{
		LOGERR("DBERR: query user info szChannel=%s,szPlatformId=%s serverId=%d", szChannel,szPlatformId,serverId);
		return MT_ERR;
	}

	return QueryUserAllInfo(propertys.userid);

	func_end();
}

int CUser::QueryUserAllInfo(int64_t userid)
{
	func_start();
	if (userid == -1) {
		LOGERR("DBERR:%s userid=%lld", __PRETTY_FUNCTION__, userid);
		return MT_ERR;
	}

	if (MT_OK != s_dbproxy.QueryUserRMB(propertys.userid, userrmb)) {
		LOGERR("DBERR: query userRMB info propertys.macaddr=%s, userid=%lld", propertys.macaddr.to_string(), propertys.userid);
		return MT_ERR;
	}

	CalcVipLevel(true);

	heros.clear() ;
	if (MT_OK != s_dbproxy.QueryHeroInfoByUserId(propertys.userid, *this)) 
	{
		LOGERR("DBERR: QueryCardInfo propertys.macaddr=%s,userid=%lld", propertys.macaddr.to_string(), propertys.userid);
		heros.clear() ;
		return MT_ERR;
	}

	if (MT_OK != s_dbproxy.QueryFBInfoByUserID(propertys.userid, &FBStatus)) {
		LOGERR("DBERR: query FB info propertys.macaddr=%s", propertys.macaddr.to_string());
		FBStatus.clear() ;
		return MT_ERR;
	}

	if (MT_OK != s_dbproxy.QueryDigInfoByUserID(propertys.userid,_Dig)) {
		LOGERR("DBERR: query dig info propertys.macaddr=%s,userid=%lld", 
			propertys.macaddr.to_string(), propertys.userid);
		_Dig.clear() ;
		return MT_ERR;
	}
	//读取好友相关信息
	if (MT_OK != s_dbproxy.QueryFriendsInfoByUserID(propertys.userid,this))
	{
		LOGERR("DBERR: Friends info propertys.macaddr=%s,userid=%lld", 
			propertys.macaddr.to_string(), propertys.userid);
		return MT_ERR;
	}

	//请求好友相关读取信息
	req_friend_info reqFriends;
	uint8_t row = 0;
	if (MT_OK != s_dbproxy.QueryFriendsReqByUserID(propertys.userid, &reqFriends,row))
	{
		LOGERR("DBERR: Friends request info propertys.macaddr=%s,userid=%lld", 
			propertys.macaddr.to_string(), propertys.userid);
		return MT_ERR;
	}


	if (server.memcached_enable)
	{
		CDataMgr::Instance()->UpdateReqFriendData(userid,reqFriends);
	}
	else
	{	
		if (row == 1)
		{
			req_friend_info* pInfo = RequestFriendMng::GetInstance()->NewAndAddReqFriendInfo(propertys.userid);
			if (pInfo)
			{
				*pInfo = reqFriends;
			}
			else
			{
				LOGERR("DBERR: Friends request info pInfo is Null,propertys.macaddr=%s,userid=%lld", 
					propertys.macaddr.to_string(), propertys.userid);
				return MT_ERR;
			}
		}
	}

	//体力赠送相关
	if (MT_OK != s_dbproxy.QuerySendPhyInfoByUserId(propertys.userid, GetFriendMng().GetSendPhysicalData()))
	{
		LOGERR("DBERR: query SendPhy info propertys.macaddr=%s,userid=%lld", propertys.macaddr.to_string(), propertys.userid);
		return MT_ERR;
	}
	//背包信息
	if (MT_OK != s_dbproxy.QueryBagInfoByUserID(userid,this))
	{
		LOGERR("DBERR: query bag info propertys.macaddr=%s,userid=%lld", 
			propertys.macaddr.to_string(), propertys.userid);
		return MT_ERR;
	}
	//符石信息
	if (MT_OK != s_dbproxy.QueryUserGemsByUserID(userid,this))
	{
		LOGERR("DBERR: query user gem info propertys.macaddr=%s,userid=%lld", 
			propertys.macaddr.to_string(), propertys.userid);
		return MT_ERR;
	}
	// 检查符石数据有效性,至少要有一个主属性，发现无主属性的重新产生
	{
		bool bHasGemsUpdate = false;
		const HerosMap& heromap = GetHeros();
		for(HerosMap::const_iterator it = heromap.begin(); it != heromap.end(); ++ it)
		{
			for(int i = 0; i < MAX_GEM_STAR_POSTION_COUNT; ++ i)
			{
				hero_gem_info tempgems;
				it->second.GetHeroGemInfo((GemPos)i,tempgems);
				if(tempgems.m_tableID <= 0)
				{
					continue;
				}

				// 检测主属性有效值个数是否大于等于1
				int nCurMainAttNum = 0;
				for(int mainatti = 0; mainatti < MAX_GEM_MAIN_ATT_COUNT; ++ mainatti)
				{
					if(tempgems.m_main_attribute[mainatti] > 0)
					{
						nCurMainAttNum++;
					}
				}
				if (nCurMainAttNum <= 0)
				{
					LOGDEBUG("QueryUserAllInfo Find Hero Gems MainAttribute Count Error, Hero[%d],userid[%lld],gemid[%d]",tempgems.m_id,GetUserID(),tempgems.m_tableID);

					hero_gem_info newgemdata;
					if(false == Bag::BuildGemDataById(tempgems.m_tableID,newgemdata))
					{
						LOGERR("QueryUserAllInfo ReBuildGem Fail, Hero[%d],userid[%lld],gemid[%d]",tempgems.m_id,GetUserID(),tempgems.m_tableID);
						continue;
					}
					for(int mainatti =0;mainatti<MAX_GEM_MAIN_ATT_COUNT;mainatti++)
					{
						tempgems.m_main_attribute[mainatti] = newgemdata.m_main_attribute[mainatti];
					}
					CHero* pCurHero = this->getHeroById(it->second.GetHeroTemplateID());
					if(pCurHero == NULL)
					{
						LOGERR("QueryUserAllInfo GetHeroById Fail, Hero[%d],userid[%lld],gemid[%d]",tempgems.m_id,GetUserID(),tempgems.m_tableID);
						continue;
					}
					if(false == pCurHero->ModifyHeroGemInfo((GemPos)i,tempgems))
					{
						LOGERR("QueryUserAllInfo ModifyHeroGemInfo Fail, Hero[%d],userid[%lld],gemid[%d]",tempgems.m_id,GetUserID(),tempgems.m_tableID);
						continue;
					}
					bHasGemsUpdate = true;
				}
			}
		}
		if(bHasGemsUpdate)
		{
			SetUpdateTime(LOT_UserGems);
		}
	}


	//每日任务
	if (MT_OK != s_dbproxy.QueryDailyMissionByUserID(userid,this))
	{
		LOGERR("DBERR: query dailymission info propertys.macaddr=%s,userid=%lld", 
			propertys.macaddr.to_string(), propertys.userid);
		return MT_ERR;
	}
	//任务
	if (MT_OK != s_dbproxy.QueryMissionByUserID(userid,this))
	{
		LOGERR("DBERR: query mission info propertys.macaddr=%s,userid=%lld", 
			propertys.macaddr.to_string(), propertys.userid);
		return MT_ERR;
	}
	//签到
	if (MT_OK != s_dbproxy.QuerySignByUserID(userid,this))
	{
		LOGERR("DBERR: query sign info propertys.macaddr=%s,userid=%lld", 
			propertys.macaddr.to_string(), propertys.userid);
		return MT_ERR;
	}
	//布阵
	if (MT_OK != s_dbproxy.QueryPrepBattleByUserID(this))
	{
		LOGERR("DBERR: query PrepBattle info propertys.macaddr=%s,userid=%lld", 
			propertys.macaddr.to_string(), propertys.userid);
		return MT_ERR;
	}
	if(GetLevel() > 1)
	{
		if(GetPrepBattleMng()->GetBestCombatPower() <= 0)
		{
			GetPrepBattleMng()->init(this) ;
			GetPrepBattleMng()->RefBestCombatPower() ;
			SetUpdateTime(LOT_ArenaInfo);
		}
	}

// 	if (!server.memcached_enable)
// 	{
// 		//战力
// 		if (MT_OK != s_dbproxy.QueryComBatPowerByUserID(this))
// 		{
// 			LOGERR("DBERR: query combatpower info propertys.macaddr=%s,userid=%lld", 
// 				propertys.macaddr.to_string(), propertys.userid);
// 			return MT_ERR;
// 		}
// 
// 	}

	//帐号封停相关
	MemForbidUserInfo forbiduserinfo;

	//封停帐号信息
	if (MT_OK != s_dbproxy.QueryForbidUserByUserId(propertys.userid,forbiduserinfo))
	{
		LOGERR("DBERR: query ForbidUserInfo info propertys.macaddr=%s,userid=%lld", 
			propertys.macaddr.to_string(), propertys.userid);
		return MT_ERR;
	}

	if (MT_OK != CDataMgr::Instance()->UpdateForbidUserInfo(propertys.userid,forbiduserinfo))
	{
		LOGERR("CUser::QueryUserAllInfo UpdateForbidUserInfo()return failed. userid=%lld", propertys.userid);
		return MT_ERR;
	}

	//禁言相关
	MemForbidSpeakInfo forbidspeakinfo;

	//禁言信息
	if (MT_OK != s_dbproxy.QueryForbidSpeakByUserId(propertys.userid,forbidspeakinfo))
	{
		LOGERR("DBERR: query ForbidSpeakInfo info propertys.macaddr=%s,userid=%lld", 
			propertys.macaddr.to_string(), propertys.userid);
		return MT_ERR;
	}
	if (MT_OK != CDataMgr::Instance()->UpdateForbidSpeakInfo(propertys.userid,forbidspeakinfo))
	{
		LOGERR("CUser::QueryUserAllInfo UpdateForbidSpeakInfo()return failed. userid=%lld", propertys.userid);
		return MT_ERR;
	}

	//星座爬塔
	if (MT_OK != s_dbproxy.QueryZodiacByUserID(GetUserID(),this))
	{
		LOGERR("DBERR: query Zodiac info propertys.macaddr=%s,userid=%lld", 
			propertys.macaddr.to_string(), propertys.userid);
		return MT_ERR;
	}

	if (MT_OK != s_dbproxy.QueryUserAwardByUserID(propertys.userid, userAward))
	{
		LOGERR("DBERR: query useraward info propertys.macaddr=%s,userid=%lld", 
			propertys.macaddr.to_string(), propertys.userid);
		return MT_ERR;
	}

	if (MT_OK != s_dbproxy.QueryGuideInfoByUserID(propertys.userid, _guideinfo))
	{
		LOGERR("DBERR: query guideinfo info propertys.macaddr=%s,userid=%lld", 
			propertys.macaddr.to_string(), propertys.userid);
		return MT_ERR;
	}


	//7日登陆奖励
	if (MT_OK != s_dbproxy.QueryLoginAwardByUserID(userid,this))
	{
		LOGERR("DBERR: query login award info propertys.macaddr=%s,userid=%lld", 
			propertys.macaddr.to_string(), propertys.userid);
		return MT_ERR;
	}

	////竞技场数据
	//if (MT_OK != s_dbproxy.QueryNewArenaInfoByUserID(userid, _arena))
	//{
	//	LOGERR("DBERR: query login arena info propertys.macaddr=%s,userid=%lld", 
	//		propertys.macaddr.to_string(), propertys.userid);
	//	return MT_ERR;
	//}

	//竞技场a基础数据
	if (MT_OK != s_dbproxy.QueryArena_aInfo(userid, &_arena.arenainfo))
	{
		LOGERR("DBERR: query login arena_a info propertys.macaddr=%s,userid=%lld", 
			propertys.macaddr.to_string(), propertys.userid);
		return MT_ERR;
	}

	//竞技场a挑战纪录
	if (MT_OK != s_dbproxy.QueryArena_aRecord(userid, &_arena.records))
	{
		LOGERR("DBERR: query login arena_a records propertys.macaddr=%s,userid=%lld", 
			propertys.macaddr.to_string(), propertys.userid);
		return MT_ERR;
	}

	//设置玩家竞技场_临界数据
	{
		MemArena_aLockData _lockdata ;
		_lockdata.defenseinfo = _arena.arenainfo.defenseinfo ;
		_lockdata.records = _arena.records ;
		if(false == CArena_aSortMgr::Instance()->InitLockData(userid, _lockdata))
		{
			LOGERR("CUser::QueryUserAllInfo() CArena_aSortMgr::Instance()->InitLockData() return fail!macaddr=%s,userid=%lld,userlvl=%u", 
				propertys.macaddr.to_string(), propertys.userid, propertys.lvl);
			return MT_ERR;
		}
	}
	//玩家英雄试练数据
	if (MT_OK != s_dbproxy.QueryHeroTrialInfo(userid,_herotrial._trialCache))
	{
		LOGERR("DBERR: query login trial info propertys.macaddr=%s,userid=%lld", 
			propertys.macaddr.to_string(), propertys.userid);
		return MT_ERR;
	}

	//玩家巅峰对决数据
	MemQueryPeakVs _peakvs ;
	if (MT_OK != s_dbproxy.QueryHeroPeakVs(userid, _peakvs))
	{
		LOGERR("DBERR: query login peakvs info propertys.macaddr=%s,userid=%lld", 
			propertys.macaddr.to_string(), propertys.userid);
		return MT_ERR;
	}
	peakvs.SetData(_peakvs.m_Data) ;
	if(1 == _peakvs.m_ClearFlag)
		SetUpdateTime(LOT_PeakVs) ;

	//玩家小无尽数据
	if (MT_OK != s_dbproxy.QuerySmallEndLess(userid,smallendless))
	{
		LOGERR("DBERR: query login smallendless info propertys.macaddr=%s,userid=%lld", 
			propertys.macaddr.to_string(), propertys.userid);
		return MT_ERR;
	}

	//玩家无尽数据
	if (MT_OK != s_dbproxy.QueryEndLess(userid,endless))
	{
		LOGERR("DBERR: query login endless info propertys.macaddr=%s,userid=%lld", 
			propertys.macaddr.to_string(), propertys.userid);
		return MT_ERR;
	}

	//玩家充值信息
	MemChargeInfo &_MemChargeInfo = m_ChargeInfoMng.GetData() ;
	if (MT_OK != s_dbproxy.QueryChargeInfo(userid, _MemChargeInfo))
	{
		LOGERR("DBERR: query login charge info propertys.macaddr=%s,userid=%lld", 
			propertys.macaddr.to_string(), propertys.userid);
		return MT_ERR;
	}

	//商店
	if (MT_OK != s_dbproxy.QueryShopData(userid,m_ShopModule))
	{
		LOGERR("DBERR: query login shop data propertys.macaddr=%s,userid=%lld", 
			propertys.macaddr.to_string(), propertys.userid);
		return MT_ERR;
	}
	// 读商店中的符石随机属性
	if (MT_OK != s_dbproxy.QueryShopGemData(userid,m_ShopGemRandomData))
	{
		LOGERR("DBERR: query login shop gem data propertys.macaddr=%s,userid=%lld", 
			propertys.macaddr.to_string(), propertys.userid);
		return MT_ERR;
	}
	
	
	//
	if (MT_OK != s_dbproxy.QueryLootData(userid,m_Loot))
	{
		LOGERR("DBERR: query login loot data propertys.macaddr=%s,userid=%lld", 
			propertys.macaddr.to_string(), propertys.userid);
		return MT_ERR;
	}

	//玩家邮件
	CEmail normal_mail;
	if (MT_OK != s_dbproxy.QueryUserMailInfoByUserID(propertys.userid,normal_mail.GetUserMail()))
	{
		LOGERR("CUser::QueryUserAllInfo DBERR: query UserMail info,userid=%lld", propertys.userid);
		return MT_ERR;
	}
	if (!CNormalMailMgr::AddNormalMail(propertys.userid,normal_mail))
	{
		LOGERR("CUser::QueryUserAllInfo AddSysMail()return failed. userid=%lld", propertys.userid);
		return MT_ERR;
	} 

	MemSysMails sysmails;
	//系统邮件
	if (MT_OK != s_dbproxy.QueryPerSysEmailByUserId(propertys.userid,sysmails._sysEmailData))
	{
		LOGERR("CUser::QueryUserAllInfo DBERR: query SysEmail info userid=%lld", propertys.userid);
		return MT_ERR;
	}

	if (!CSysMailMgr::AddSysMail(propertys.userid,sysmails))
	{
		LOGERR("CUser::QueryUserAllInfo AddSysMail()return failed. userid=%lld", propertys.userid);
		return MT_ERR;
	} 

	//首冲奖励
	if (MT_OK != s_dbproxy.QueryFirstRechargeInfoByUserID(userid,_FirstRecharge))
	{
		LOGERR("DBERR: query login shop data propertys.macaddr=%s,userid=%lld", 
			propertys.macaddr.to_string(), propertys.userid);
		return MT_ERR;
	}

	//VIP 礼包
	if (MT_OK != s_dbproxy.QueryVipAwardInfoByUserID(userid,_VipAward))
	{
		LOGERR("DBERR: query vip award data propertys.macaddr=%s,userid=%lld", 
			propertys.macaddr.to_string(), propertys.userid);
		return MT_ERR;
	}
	//好友助战奖励
	FriendCombatAward CombatAward;
	if (MT_OK != s_dbproxy.QueryFriendAwardByUserID(userid,&CombatAward))
	{
		LOGERR("DBERR: query friend award data propertys error.macaddr=%s,userid=%lld", 
			propertys.macaddr.to_string(), propertys.userid);
		return MT_ERR;
	}
	//加锁，更新memcached中数据
	{
		MemLock Lock(eKey_FriendAward_Lock,userid, true);
		CDataMgr::Instance()->UpdateFriendAwardInfo(userid,CombatAward);
	}
	//活动
	if (MT_OK != s_dbproxy.QueryCommonActivityInfoByUserID(userid,this->_CommonActivity))
	{
		LOGERR("DBERR: query login common activity data propertys.macaddr=%s,userid=%lld", 
			propertys.macaddr.to_string(), propertys.userid);
		return MT_ERR;
	}
	//好友助战奖励
	if (MT_OK != s_dbproxy.QueryFriendAwardInfoByUserID(userid,this->m_FriendAwardData))
	{
		LOGERR("DBERR: query login friend activity award data propertys.macaddr=%s,userid=%lld", 
			propertys.macaddr.to_string(), propertys.userid);
		return MT_ERR;
	}	
	//贸易数据
	if (MT_OK != s_dbproxy.QueryTradeData(userid,this->m_TradeMgr))
	{
		LOGERR("DBERR: query trade data  propertys.macaddr=%s,userid=%lld", 
			propertys.macaddr.to_string(), propertys.userid);
		return MT_ERR;
	}
	//--加载节日道具兑换数据
	if (MT_OK != s_dbproxy.QueryHolidayItemExchange(userid, m_HolidayItemExchange))
	{
		LOGERR("DBERR: query trade data  propertys.macaddr=%s,userid=%lld", 
			propertys.macaddr.to_string(), propertys.userid);
		return MT_ERR;
	}

	return MT_OK;
	func_end();
}

//查询加载邮件数据
int CUser::QueryUserEmailInfo()
{
	func_start();

// 	if (MT_OK != s_dbproxy.QueryUserMailInfoByUserID(propertys.userid,emailInfo.GetUserMail()))
// 	{
// 		LOGERR("DBERR: query SendEmailInfo info,userid=%lld", propertys.userid);
// 		return MT_ERR;
// 	}
// 
// 	if (MT_OK != s_dbproxy.QueryPerSysEmailByUserId(propertys.userid,emailInfo.GetSysEmail()))
// 	{
// 		LOGERR("DBERR: query PerSysEmailInfo info userid=%lld", propertys.userid);
// 		return MT_ERR;
// 	}

	return MT_OK;
	func_end();
}

//计算0:0:0时间
uint32_t CUser::calc_time(time_t now) const
{
    func_start();
	tm *pTm = NULL;
	pTm = localtime(&now);

	uint32_t TimeInterval = pTm->tm_hour * ONE_HOUR_MIN * ONE_MIN_SEC + pTm->tm_min * ONE_MIN_SEC + pTm->tm_sec;
	return uint32_t(now - TimeInterval);
    func_end();
}

#include "encoding.h"


bool CUser::CheckUserName(const char * UserName)
{
	func_start();
	if ((NULL == UserName) || ('\0' == UserName[0]))
	{
		LOGERR("CUser::CheckUserName, UserName is null!");
		return false;
	}

	size_t len = strlen(UserName);
	if ((len > MAX_USER_NAME_LEN) || (0 == len))
	{
		LOGERR("CUser::CheckUserName,user_name_len==0 or user_name_len[%u] > MAX_USER_NAME_LEN[%d]!",
			len, MAX_USER_NAME_LEN);
		return false;
	}

	std::wstring strName = utf8tow(UserName);
	len = strName.length();

	int BlankFlag = 0;
	for (size_t i = 0; i < len; ++i)
	{
		if ((_T('\'') == strName[i]) || (_T('\"') == strName[i])
			|| (_T(',') == strName[i]) || (_T('\\') == strName[i])
			 || (_T('\a') == strName[i]) || (_T('\b') == strName[i])
			  || (_T('\f') == strName[i])  || (_T('\n') == strName[i])
			   || (_T('\r') == strName[i])  || (_T('\t') == strName[i])
			    || (_T('\v') == strName[i]))
		{
			LOGERR("CUser::CheckUserName, user name has forbidden character[%d]!UserName=%s",
				   UserName[i], UserName);
			return false;
		}

		if ((i == BlankFlag) && (_T(' ') == strName[i]))
		{
			++BlankFlag;
		}
	}

	if (BlankFlag == len)
	{
		LOGERR("CUser::CheckUserName, user name is all blank!");
		return false;
	}
	return true;
	func_end();
}

int CUser::UpdateHeroToDB(int16_t template_id)
{
	func_start();

	if (template_id <0 )
	{
		LOGWARN("CUser::UpdateHeroToDB,template_id err! uoid=%u,userid=%lld ,template_id=%d,account=%s",
			at_uoid(), propertys.userid, template_id, propertys.account.to_string());
		return MT_ERR;
	}

	CHero* pHero = getHeroById(template_id);
	if (!pHero)
	{
		LOGWARN("CUser::UpdateHeroToDB, hero not found! uoid=%u,userid=%lld ,templete_id=%d,account=%s",
			at_uoid(), propertys.userid, template_id, propertys.account.to_string());

		return MT_ERR;
	}
	if (!server.memcached_enable)
	{
		if (MT_ERR == s_dbproxy.UpDateHeroInfo(&pHero->GetData(),this))
		{
			LOGWARN("CUser::UpdateHeroToDB, update heroinfo failed! uoid=%u,userid=%lld,account=%s",
				at_uoid(), propertys.userid, propertys.account.to_string());
			return MT_ERR;
		}
	}


	//if (template_id == propertys.teamleaderid)
	{
		//s_dbproxy.UpDateSortTempMainHeroToDB(*this);
		SetUpdateTime(LOT_UserSort);
	}

	return MT_OK;
	func_end();
}

int CUser::UpdateUserDataToDB()
{
	func_start();
	//if (server.memcached_enable) return;
	// 	{
	// 		CDataMgr::Instance()->UpdateUser(*this);
	// 	}

	if (!server.memcached_enable)
	{
		if (MT_ERR == s_dbproxy.UpDateUserInfo(&propertys))
		{
			LOGWARN("CUser::UpdateUserDataToDB, UpDateUserInfo failed! uoid=%u,userid=%lld,account=%s",
				at_uoid(), propertys.userid, propertys.account.to_string());
			return MT_ERR;
		}


	}

	_update.updatetime[LOT_BaseData] = DataDirtyHelper::sDirtyStamp ++;

	return MT_OK;
	func_end();
}

int CUser::UpdateUserGemsToDB()
{
	func_start();

	if (!server.memcached_enable)
	{
		if (MT_ERR == s_dbproxy.UpDateUserGemsInfo(propertys.userid,this))
		{
			LOGWARN("CUser::UpdateUserGemsToDB, UpDateUserGemInfo failed! uoid=%u,userid=%lld,account=%s",
				at_uoid(), propertys.userid, propertys.account.to_string());
			return MT_ERR;
		}
	}

	_update.updatetime[LOT_UserGems] = DataDirtyHelper::sDirtyStamp ++;

	return MT_OK;
	func_end();
}

int CUser::UpdateTimeToDB(bool force)
{
	func_start();
	if (force || 0 != UpdateTime())
	{
		return UpdateUserDataToDB();
	}

	return MT_OK;
	func_end();
}

/****************************************************
返回值: -1, 数据错误, 修正数据, 些数据库;
         0, 当天再次操作, 不更新数据库
		 1, 非当天再次操作,写数据库
****************************************************/
int CUser::UpdateTime()
{
    func_start();
	time_t now = time(NULL);

	if (0 == propertys.last_active_time)
	{
		//第一次登录
		propertys.sustain_day_login = 1;
		propertys.last_active_time = (uint32_t)now;
		propertys.accumulated_day_login = 1;
		LOGINFO("CUser::UpdateTime, first log in! userid=%lld", propertys.userid);
		return 1;
	}
	propertys.last_active_time = (uint32_t)now;
	return 1;

// 	uint32_t hour = 0; 
// 	uint32_t minute = 0;
// 	GetServerCdTimeByIndex(eCdTimeIndex_TaskAward, hour, minute);
// 	time_t last_active_time = propertys.last_active_time;
// 	tm *pTm = localtime(&last_active_time);
// 	uint32_t CdTime = propertys.last_active_time - ((pTm->tm_hour - hour) * ONE_HOUR_MIN * ONE_MIN_SEC 
// 		                     + (pTm->tm_min - minute) * ONE_MIN_SEC + pTm->tm_sec);
// 	uint32_t last_time = 0;
// 	if (propertys.last_active_time < CdTime)
// 	{
// 		last_time = CdTime - ONE_DAY_SECOND;
// 	}
// 	else
// 	{
// 		last_time = CdTime;
// 	}
// 
// 	int32_t TimeInterval = (int32_t)(now - last_time);
// 	propertys.last_active_time = (uint32_t)now;
// 	if ((0 <= TimeInterval) && (TimeInterval < ONE_DAY_SECOND))
// 	{
// 		//LOGNOTICE("CUser::UpdateTime, no need update!");
// 		return 0;
// 	}
// 	else if ((ONE_DAY_SECOND <= TimeInterval) && (TimeInterval < (2 * ONE_DAY_SECOND)))
// 	{
// 		++propertys.sustain_day_login;
// 		++propertys.accumulated_day_login;
// 		LOGNOTICE("CUser::UpdateTime, continuous log in!sustain_day_login=%u,accumulated_day_login=%u",
// 			  propertys.sustain_day_login, propertys.accumulated_day_login);
// 		return 1;
// 	}
// 	else if ((2 * ONE_DAY_SECOND) <= TimeInterval)
// 	{
// 		propertys.sustain_day_login = 1;
// 		++propertys.accumulated_day_login;
// 		LOGNOTICE("CUser::UpdateTime, log in again!sustain_day_login=%u,accumulated_day_login=%u",
// 			propertys.sustain_day_login, propertys.accumulated_day_login);
// 		return 1;
// 	}
// 	else /*(TimeInterval < 0)*/
// 	{
// 		LOGERR("CUser::UpdateTime, now[%u] < last_time[%u]!",
// 			   now, last_time);
// 		propertys.sustain_day_login = 1;
// 		return -1;
// 	}
    func_end();
}

//根据id查询英雄对象
CHero* CUser::getHeroById(int16_t id)
{
	func_start();
	HerosMap::iterator it = heros.find(id) ;
	if(it != heros.end())
		return &it->second ;
	return NULL ;
	func_end();
}

//获得指定星级的英雄数量
uint32_t CUser::getHeroCountByStar(uint32_t nQuality)
{
	func_start();
	
	uint32_t uRev = 0 ;
	for(HerosMap::iterator it=heros.begin(); it!=heros.end(); ++it)
	{
		const HerosTab_t* pHerosTab = it->second.GetHerosTab() ;
		if(NULL == pHerosTab)
		{
			LOGERR("CUser::getHeroCountByStar(), not find herotabdata! account=%s,userid=%lld,heroid=%d", 
				propertys.account.to_string(), propertys.userid, it->second.GetHeroTemplateID());
			continue ;
		}
		if(pHerosTab->QualityLvl >= nQuality)
		{
			uRev++ ;
		}
	}
	return uRev ;
	func_end();
}

//获得指定品质(颜色)的英雄数量
uint32_t CUser::getHeroCountByQuality(uint32_t nQuality)
{
	func_start();
	
	uint32_t uRev = 0 ;
	for(HerosMap::iterator it=heros.begin(); it!=heros.end(); ++it)
	{
		if(it->second.GetQuality() >= nQuality)
		{
			uRev++ ;
		}
	}
	return uRev ;
	func_end();
}

/*  设置战斗标志 */
void CUser::CombatBegin(void)
{
	SetUserFlag(userflag_combat);
}

/*
	重新设置战斗标志
*/
void CUser::CombatEnd(void)
{
	SetUserFlag(userflag_normal);
}

int CUser::addHeroAndWriteDB(int16_t template_id,uint8_t lvl,uint8_t num)
{
	func_start(); 

	if (template_id < 0)
	{
		LOGERR("CUser:addHeroAndWriteDB, hero template_id err!userid=%lld,template_id=%d,lvl=%u,num=%u",propertys.userid, template_id, lvl,num);
		return eAddHeroStatus_LvlErr;
	}

 	if (lvl == 0)
 	{
 		LOGERR("CUser:addHeroAndWriteDB, hero lvl err!userid=%lld,template_id=%d,lvl=%u,num=%u",propertys.userid, template_id, lvl,num);
 		return eAddHeroStatus_LvlErr;
 	}

	if (num == 0)
	{
		LOGERR("CUser:addHeroAndWriteDB, hero num err!userid=%lld,template_id=%d,lvl=%u,num=%u",propertys.userid, template_id, lvl,num);
		return eAddHeroStatus_IDNumErr;
	}

	for (uint8_t i=0; i < num; ++i)
	{
		CHero *pHero = getHeroById(template_id);
		if (pHero)
		{
			int16_t itemid = pHero->GetChangeSoul();
			uint16_t itemnum = pHero->GetChangeSoulNumber()* (num - i);

			if (!m_Bag.AddItem(itemid,itemnum))
			{
				LOGERR("CUser::addHeroAndWriteDB(),add item failed! AddItem() return fail!userid=%lld,template_id=%d,lvl=%u,num=%u,itemid=%d,itemnum=%u"
					, GetUserID(),template_id, lvl,num,itemid,itemnum);
				return eAddHeroStatus_ItemErr ;
			}
			if (!UpdateBagInfoToDB())
			{
				LOGERR("CUser::addHeroAndWriteDB(),add item failed! UpdateBagInfoToDB() return fail!userid=%lld,template_id=%d,lvl=%u,num=%u,itemid=%d,itemnum=%u"
					, GetUserID(),template_id, lvl,num,itemid,itemnum);
				return eAddHeroStatus_UpdateItemErr ;
			}
			return eAddHeroStatus_OK;
		}
		else
		{
			CHero _hero ;
			_hero.clear();
			if (MT_OK != _hero.init(template_id, lvl))
			{
				LOGERR("CUser:addHeroAndWriteDB, init hero failed!userid=%lld,template_id=%d,lvl=%u,num=%u"
					,GetUserID(),template_id, lvl,num);
				return eAddHeroStatus_InitErr;
			}

			if (MT_OK != addHero(_hero))
			{
				LOGERR("CUser:addHeroAndWriteDB, add hero failed!userid=%lld,template_id=%d,lvl=%u,num=%u"
					,GetUserID(),template_id, lvl,num);
				return eAddHeroStatus_AddErr;
			}
			if (!server.memcached_enable)
			{
				if (MT_OK != s_dbproxy.WriteHeroInfoToDB(propertys.userid,&_hero.GetData()))
				{
					LOGERR("CUser:addHeroAndWriteDB. write to db failed!userid=%lld,template_id=%d,lvl=%u,num=%u"
						,GetUserID(),template_id, lvl,num);
					return eAddHeroStatus_WriteDBErr;
				}
			}
		}
	}

	return eAddHeroStatus_OK;
	func_end();
}

int CUser::addHero(const CHero& hero)
{
	func_start(); 

	if (heros.find(hero.GetHeroTemplateID())!=heros.end())
	{
		LOGERR("CUser::addHero.hero templateid repeat! account=%s,userid=%lld", propertys.account.to_string(), propertys.userid);
		return MT_ERR ;
	}

 	std::pair<HerosMap::iterator, bool> ins =  heros.insert(std::make_pair(hero.GetHeroTemplateID(), hero)) ;
 	if (!ins.second) 
 	{
 		LOGERR("CUser::addHero: ErrorHeroInfo(insert map fail) account=%s,userid=%lld", propertys.account.to_string(), propertys.userid);
 		return MT_ERR ;
 	}

	return MT_OK;
	func_end();
}

int CUser::addTower(int16_t template_id,uint8_t lvl,uint8_t num)
{
	func_start(); 

	if (template_id < 0)
	{
		LOGERR("CUser:addTower, hero template_id err!userid=%lld,template_id=%d,lvl=%u,num=%u",propertys.userid, template_id, lvl,num);
		return eAddHeroStatus_LvlErr;
	}

 	if (lvl == 0)
 	{
 		LOGERR("CUser:addTower, tower lvl err!userid=%lld,template_id=%d,lvl=%u,num=%u",propertys.userid, template_id, lvl,num);
 		return eAddHeroStatus_LvlErr;
 	}

	if (num == 0)
	{
		LOGERR("CUser:addTower, tower num err!userid=%lld,template_id=%d,lvl=%u,num=%u",propertys.userid, template_id, lvl,num);
		return eAddHeroStatus_IDNumErr;
	}

	CHero *pHero = getHeroById(template_id);
	if (pHero)
	{
		LOGERR("CUser:addTower, tower is have!userid=%lld,template_id=%d,lvl=%u,num=%u",propertys.userid, template_id, lvl,num);
		return eAddHeroStatus_AddTowerErr;
	}

	for (uint8_t i=0; i < num; ++i)
	{	
		CHero _hero ;
		_hero.clear();
		if (MT_OK != _hero.init(template_id, lvl))
		{
			LOGERR("CUser:addTower, init hero failed!userid=%lld,template_id=%d,lvl=%u,num=%u"
				,GetUserID(),template_id, lvl,num);
			return eAddHeroStatus_InitErr;
		}

		if (MT_OK != addHero(_hero))
		{
			LOGERR("CUser:addTower, add hero failed!userid=%lld,template_id=%d,lvl=%u,num=%u"
				,GetUserID(),template_id, lvl,num);
			return eAddHeroStatus_AddErr;
		}
		if (!server.memcached_enable)
		{
			if (MT_OK != s_dbproxy.WriteHeroInfoToDB(propertys.userid,&_hero.GetData()))
			{
				LOGERR("CUser:addTower. write to db failed!userid=%lld,template_id=%d,lvl=%u,num=%u"
					,GetUserID(),template_id, lvl,num);
				return eAddHeroStatus_WriteDBErr;
			}
		}
	}
	return eAddHeroStatus_OK;
	func_end();
}

uint16_t CUser::GetSkillLimited()
{
	func_start();

	const vip_tab* pTabData = CVipMng::Instance()->GetVipTabInfoByID(GetVipLevel()) ;
	if(NULL == pTabData)
	{
		LOGERR("CUser::GetMaxSkillPoint, Get vipbuyprivilege table data error!(vip_lvl=%u)",GetVipLevel());
		return 0;
	}
	return pTabData->SkillLimited ;

	func_end();
}

uint16_t CUser::SkillRevertSpeed()
{
	func_start();

	const vip_tab* pTabData = CVipMng::Instance()->GetVipTabInfoByID(GetVipLevel()) ;
	if(NULL == pTabData)
	{
		LOGERR("CUser::GetMaxSkillPoint, Get vipbuyprivilege table data error!(vip_lvl=%u)",GetVipLevel());
		return 0;
	}
	return pTabData->SkillRevertSpeed ;

	func_end();
}

uint16_t CUser::GetCurSkillPoint()
{
	func_start();

	uint32_t refTime = (uint32_t)time(NULL);
	uint16_t uCurrPhy = 0;
	uint32_t uRemainTime = 0;

	GetSkillPoint(refTime, uCurrPhy, uRemainTime);
	return uCurrPhy;

	func_end();
}

int32_t CUser::GetSkillPoint(uint32_t uRefTime, uint16_t & uCurrPhy, uint32_t & uRemainTime)
{
	uint16_t nMaxValue = GetSkillLimited() ;
	uint16_t nCurrValue = propertys.skillpoint ;
	uint32_t nScalarTime = SkillRevertSpeed() ;	// 增加一点技能点的单位时间量,比如每5分钟增加一点体力
	if(nCurrValue >= nMaxValue)
	{
		uCurrPhy = nCurrValue ;
		uRemainTime = 0 ;
		return 0 ;
	}

	if (uRefTime <= propertys.refurbish_skillpoint_time)
	{
		uCurrPhy = nCurrValue ;
		uRemainTime = nScalarTime ;
		return 0 ;
	}

	if(0 == propertys.refurbish_skillpoint_time)
	{
		propertys.refurbish_skillpoint_time = propertys.register_time ;
	}

	uint32_t nDiffTime = (uint32_t)difftime(uRefTime, propertys.refurbish_skillpoint_time);
	if (nDiffTime >= nScalarTime)
	{
		uint32_t phyNum = (nDiffTime / nScalarTime);	// 增加的体力点数
		uint32_t phyyu = (nDiffTime % nScalarTime);		
		if (nCurrValue + phyNum >= nMaxValue)
		{
			uCurrPhy = nMaxValue ;
			uRemainTime = 0 ;
		}
		else
		{
			uCurrPhy = nCurrValue + phyNum;
			uRemainTime = nScalarTime - phyyu ;
		}
	}
	else
	{
		uCurrPhy = nCurrValue ;
		uRemainTime = nScalarTime - nDiffTime ;
	}
	return 0 ;
}

int32_t CUser::SetSkillPoint(uint16_t nValue, bool bUpDB/* = false*/)
{
	func_start();
	uint32_t nScalarTime = SkillRevertSpeed() ;	// 增加一点技能点的单位时间量,比如每5分钟增加一点体力
	uint16_t nSkillLimit = GetSkillLimited() ;

	uint32_t uCurrTime = (uint32_t)time(NULL);
	uint16_t nCurSkillPoint = 0;
	uint32_t uRemainTime = 0;
	GetSkillPoint(uCurrTime, nCurSkillPoint, uRemainTime);
	/*
		1. 当满技能点由满变成不满的时候,记录当前时间为开始恢复的时间
		2. 当变化前和变化后都是不满的时候,设置刷新时间为当前时间后退 流逝的不足一点的时间
	*/
	if(nValue < nSkillLimit)
	{
		if(nCurSkillPoint >= nSkillLimit)
		{
			propertys.refurbish_skillpoint_time = uCurrTime;
		}
		else
		{
			propertys.refurbish_skillpoint_time = uCurrTime - (nScalarTime - uRemainTime) ;
		}
	}
	propertys.skillpoint = nValue;
	if (propertys.skillpoint > MAX_PLAYER_SKILLPOINT)
		propertys.skillpoint = MAX_PLAYER_SKILLPOINT;

	if(bUpDB)
	{
		if (!server.memcached_enable)
		{
		if (MT_ERR == s_dbproxy.UpDateUserInfo(&propertys))
		{
			LOGERR("SetPhy:update user info wrong, userid=%lld,skillpoint=%u,",propertys.userid, propertys.skillpoint);
		}

		}

	}
	return 0 ;
	func_end();
}

bool CUser::CheckBuySkillPointCount(void)
{
	func_start();
	if (0 != propertys.skillpoint)
	{
		return false;
	} 

	return true;
	func_end();
}

bool CUser::BuySkillPointStr(void)
{
	func_start();

	uint32_t uCurSkillPoint = GetCurSkillPoint() ;
	if(uCurSkillPoint >= MAX_PLAYER_SKILLPOINT)
	{
		LOGERR("CUser::BuySkillPointStr(), too big uCurSkillPoint! maxvalue=%d, userid=%lld",
			MAX_PLAYER_SKILLPOINT, propertys.userid);
		return false ;
	}
	if(propertys.buy_skill_point_count<=0)
		propertys.buy_skill_point_count = 1;

	int nCnt = propertys.buy_skill_point_count ;
	const BuyinCreaseData_t* info = CBuyinCreaseTabMgr::Instance()->GetData(nCnt);
	if (info == NULL)
	{
		LOGERR("CUser::BuySkillPointStr can not find BuyinCreaseData_t info with cnt = %d,userid = %lld.",nCnt,GetUserID());
		return false;
	}

	//扣除消费
	int32_t nRmb = GetRMB() ;
	int32_t nBuySkillSpend = info->BuySkillPointCost;//CGameConfigMgr::Instance()->GetConfigData().BuySkillSpend;
	int32_t nBuySkillRevertPoint = CGameConfigMgr::Instance()->GetConfigData().BuySkillRevertPoint;
	if (nRmb < nBuySkillSpend)
	{
		LOGERR("CUser::BuyPhyStr(),user_diamonds[%d] < NeedDiamond[%d]!userid=%lld",
			nRmb, nBuySkillSpend, propertys.userid);
		return false;
	}
	
	propertys.buy_skill_point_last_time=time(NULL);
	++propertys.buy_skill_point_count;	

	//申请 真金操作来源事件ID
	int64_t idOpSourceEvent = CDataMgr::Instance()->CreateOpSourceId((int32_t)RMBOS_SUB_BUY_SKILLPOINT) ;

	//组织消费接口参数
	SubRMBParam_t subRmbParam ;
	subRmbParam.source			= RMBOS_SUB_BUY_SKILLPOINT ;
	snprintf(subRmbParam.uniqueid, sizeof(subRmbParam.uniqueid), "%lld", idOpSourceEvent) ;
	//subRmbParam.uniqueid		= idOpSourceEvent ;
	subRmbParam.subvalue		= nBuySkillSpend ;
	snprintf(subRmbParam.desc, sizeof(subRmbParam.desc), "buy skillpoint cost(viplvl:%u,getskillpointstr:%d)", 
		GetVipLevel(), nBuySkillRevertPoint) ;
	//备份
	UserRMB_t oldRMB_t = GetRMB_t();
	//扣除钻石
	int iRev = SubRMB(subRmbParam) ;
	if(0 != iRev)
	{
		LOGERR("CUser::BuyPhyStr(),sub rmb failed! CUser::SubRMB() return fail!(%d)userid=%lld,costrmb=%u,userrmb=%d",
			iRev, GetUserID(), nBuySkillSpend, nRmb);
		return false ;
	}
	//日志
	const UserRMB_t& newRMB_t = GetRMB_t();
	LOG_USER_PURCHASE(this, MoneyType_Diamond, subRmbParam.source, 0, 1, oldRMB_t.m_charge-newRMB_t.m_charge, oldRMB_t.m_donate-newRMB_t.m_donate, oldRMB_t.m_bindrmb-newRMB_t.m_bindrmb);

	//给技能点
	uCurSkillPoint += nBuySkillRevertPoint ;
	if(uCurSkillPoint > MAX_PLAYER_SKILLPOINT)
	{
		LOGERR("CUser::BuySkillPointStr(), after buy truncation currskillpoint! maxvalue=%d, truevalue=%u, userid=%lld",
			MAX_PLAYER_SKILLPOINT, uCurSkillPoint, propertys.userid);
		uCurSkillPoint = MAX_PLAYER_SKILLPOINT ;
	}
	SetSkillPoint(uCurSkillPoint, true) ;

	if (!server.memcached_enable)
	{
		s_dbproxy.UpDateUserInfo(&propertys);
	}

	return true;
	func_end();
}

uint16_t CUser::GetMaxPhysicalStrength()
{
	func_start();
	
	const PlayerTab_t* pTabData = CPlayerTableManager::Instance()->GetData(propertys.lvl) ;
	if(NULL == pTabData)
	{
		LOGERR("CUser::GetMaxPhysicalStrength, Get player table data error!(lvl=%d)",propertys.lvl);
		return 0;
	}
	return pTabData->m_MaxPhysicalStrength ;

	func_end();
}

uint16_t CUser::GetCurrPhysicalStrength()
{
	func_start();
	
	uint32_t refPhyTime = (uint32_t)time(NULL);
	uint16_t uCurrPhy = 0;
	uint32_t uRemainTime = 0;

	GetPhysicalStrength(refPhyTime, uCurrPhy, uRemainTime);
	return uCurrPhy;

	func_end();
}
int32_t CUser::GetPhysicalStrength(uint32_t uRefPhyTime, uint16_t & uCurrPhy, uint32_t & uRemainTime)
{
	uint16_t nMaxValue = GetMaxPhysicalStrength() ;
	uint16_t nCurrValue = propertys.cur_physical_strength ;
	uint32_t nScalarTime = CGameConfigMgr::Instance()->GetConfigData().IncreasePhysicalTime * 60 ;	// 增加一点体力的单位时间量,比如每5分钟增加一点体力
	if(nCurrValue >= nMaxValue)
	{
		uCurrPhy = nCurrValue ;
		uRemainTime = 0 ;
		return 0 ;
	}
	if (uRefPhyTime <= propertys.refurbish_physical_time)
	{
		uCurrPhy = nCurrValue ;
		uRemainTime = nScalarTime ;
		return 0 ;
	}
	if(0 == propertys.refurbish_physical_time)
	{
		propertys.refurbish_physical_time = propertys.register_time ;
	}

	uint32_t nDiffTime = (uint32_t)difftime(uRefPhyTime, propertys.refurbish_physical_time);
	
	if (nDiffTime >= nScalarTime)
	{
		uint32_t phyNum = (nDiffTime / nScalarTime);	// 增加的体力点数
		uint32_t phyyu = (nDiffTime % nScalarTime);		
		if (nCurrValue + phyNum >= nMaxValue)
		{
			uCurrPhy = nMaxValue ;
			uRemainTime = 0 ;
		}
		else
		{
			uCurrPhy = nCurrValue + phyNum;
			uRemainTime = nScalarTime - phyyu ;
		}
	}
	else
	{
		uCurrPhy = nCurrValue ;
		uRemainTime = nScalarTime - nDiffTime ;
	}
	return 0 ;
}

int32_t CUser::SetPhysicalStrength(uint16_t nValue, bool bUpDB/* = false*/)
{
	func_start();

	uint32_t nScalarTime = CGameConfigMgr::Instance()->GetConfigData().IncreasePhysicalTime * 60 ;	// 增加一点体力的单位时间量,比如每5分钟增加一点体力
	uint16_t nMaxValue = GetMaxPhysicalStrength() ;

	uint32_t uCurrTime = (uint32_t)time(NULL);
	uint16_t uCurrPhy = 0;
	uint32_t uRemainTime = 0;
	GetPhysicalStrength(uCurrTime, uCurrPhy, uRemainTime);

	/*
		1. 当满体力由满变成不满的时候,记录当前时间为开始恢复的时间
		2. 当变化前和变化后都是不满的时候,设置刷新时间为当前时间后退 流逝的不足一点的时间
	*/

	if(nValue < nMaxValue)
	{
		if(uCurrPhy >= nMaxValue)
		{
			propertys.refurbish_physical_time = uCurrTime;
		}
		else
		{
			propertys.refurbish_physical_time = uCurrTime - (nScalarTime - uRemainTime) ;
		}
	}
	propertys.cur_physical_strength = nValue;
	if (propertys.cur_physical_strength > MAX_PLAYER_PHYSTRENGTH)
		propertys.cur_physical_strength = MAX_PLAYER_PHYSTRENGTH;

	if(bUpDB)
	{
		if (!server.memcached_enable)
		{
			if (MT_ERR == s_dbproxy.UpDateUserInfo(&propertys))
			{
				LOGERR("SetPhy:update userInfo wrong, userid=%lld,uCurrPhy=%u,",propertys.userid, propertys.cur_physical_strength);
			}
		}

	}
	return 0 ;

	func_end();
}

bool CUser::CheckBuyPhysicalCount(void)
{
	func_start();
	if (0 == propertys.buy_physical_last_time)
	{
		if (0 != propertys.buy_physical_count)
		{
			LOGERR("CUser::CheckBuyPhysicalCount, first buy physical,but buy_physical_count[%u] != 0.userid=%lld",
				   propertys.buy_physical_count, propertys.userid);
			propertys.buy_physical_count = 0;
		}
		return true;
	}

	vip_tab* pVip = CVipMng::Instance()->GetVipTabInfoByID(GetVipLevel()) ;
	if(NULL == pVip)
	{
		LOGERR("CUser::CheckBuyPhysicalCount, cannot find vipbuyprivilege data! vip_lvl=%u, userid=%lld",
				   GetVipLevel(), propertys.userid);
		return false ;
	}
	uint32_t uCurrByPhyMaxNum = pVip->BuyphyMaxTimes ;

	uint32_t hour = 0; 
	uint32_t minute = 0; 
	GetServerCdTimeByIndex(eCdTimeIndex_BuyPhyCount, hour, minute);
	if (0 != ComPareTimeWithHM(hour, minute, propertys.buy_physical_last_time, time(NULL)))
	{
		propertys.buy_physical_count = 0;
		return true;
	}

	if (propertys.buy_physical_count >= uCurrByPhyMaxNum)
	{
		LOGWARN("CUser::CheckBuyPhysicalCount, buy_physical_count[%u] >= uCurrByPhyMaxNum[%u]! userid=%lld",
			    propertys.buy_physical_count, uCurrByPhyMaxNum, propertys.userid);
		return false;
	}

	return true;
	func_end();
}

int32_t CUser::GetBuyPhyData(int32_t& nRemainNum, int32_t& nPrice)
{
	func_start();

	vip_tab* pVip = CVipMng::Instance()->GetVipTabInfoByID(GetVipLevel()) ;
	if(NULL == pVip)
	{
		LOGERR("CUser::CheckBuyPhysicalCount, cannot find vipbuyprivilege data! vip_lvl=%u, userid=%lld",
				   GetVipLevel(), propertys.userid);
		return -1 ;
	}

	if (0 == propertys.buy_physical_last_time)
	{
		if (0 != propertys.buy_physical_count)
		{
			LOGERR("CUser::CheckBuyPhysicalCount, first buy physical,but buy_physical_count[%u] != 0.userid=%lld",
				   propertys.buy_physical_count, propertys.userid);
			propertys.buy_physical_count = 0;
		}
		nRemainNum = pVip->BuyphyMaxTimes ;


		const BuyinCreaseData_t* info = CBuyinCreaseTabMgr::Instance()->GetData(1);
		if (info == NULL)
		{
			LOGERR("GetBuyPhyData: can not find BuyinCreaseData_t info with cnt = %d,userid = %lld.",1,GetUserID());
			return -1;
		}

		nPrice = info->BuyPhyDiamondCost ;
		return 0 ;
	}

	uint32_t hour = 0; 
	uint32_t minute = 0; 
	GetServerCdTimeByIndex(eCdTimeIndex_BuyPhyCount, hour, minute);
	if (0 != ComPareTimeWithHM(hour, minute, propertys.buy_physical_last_time, time(NULL)))
	{
		propertys.buy_physical_count = 0;
	}
	nRemainNum = pVip->BuyphyMaxTimes - propertys.buy_physical_count ;
	if(nRemainNum < 0)
		nRemainNum = 0 ;

	int nCnt = propertys.buy_physical_count > 0 ? propertys.buy_physical_count : 1;
	const BuyinCreaseData_t* info = CBuyinCreaseTabMgr::Instance()->GetData(nCnt);
	if (info == NULL)
	{
		LOGERR("GetBuyPhyData: can not find BuyinCreaseData_t info with cnt = %d,userid = %lld.",1,GetUserID());
		return -1;
	}

	nPrice = info->BuyPhyDiamondCost;

	return 0;
	func_end();
}

bool CUser::BuyPhyStr(void)
{
	func_start();

	vip_tab* pVip = CVipMng::Instance()->GetVipTabInfoByID(GetVipLevel()) ;
	if(NULL == pVip)
	{
		LOGERR("CUser::BuyPhyStr(), cannot find vipbuyprivilege data! vip_lvl=%u, userid=%lld",
				   GetVipLevel(), propertys.userid);
		return false ;
	}

	int nCnt = propertys.buy_physical_count + 1;
	const BuyinCreaseData_t* info = CBuyinCreaseTabMgr::Instance()->GetData(nCnt);
	if (info == NULL)
	{
		LOGERR("BuyPhyStr: can not find BuyinCreaseData_t info with cnt = %d,userid = %lld.",nCnt,GetUserID());
		return -1;
	}
	
	//当前的体力  > 最大值 - 差值，则返回失败                                                                 
	uint32_t uCurrPs = GetCurrPhysicalStrength() ;
	if(uCurrPs > MAX_PLAYER_PHYSTRENGTH - BUY_PYHSTR_INTERVAL)
	{
		LOGERR("CUser::BuyPhyStr(), too big currPhysicalStrength! maxvalue=%d, userid=%lld",
			MAX_PLAYER_PHYSTRENGTH, propertys.userid);
		return false ;
	}

	//扣除消费
	int32_t nRmb = GetRMB() ;
	if (nRmb < info->BuyPhyDiamondCost)
	{
		LOGERR("CUser::BuyPhyStr(),user_diamonds[%d] < NeedDiamond[%d]!userid=%lld",
			nRmb, info->BuyPhyDiamondCost, propertys.userid);
		return false;
	}

	//申请 真金操作来源事件ID
	int64_t idOpSourceEvent = CDataMgr::Instance()->CreateOpSourceId((int32_t)RMBOS_SUB_BUY_PHYSTR) ;

	//组织消费接口参数
	SubRMBParam_t subRmbParam ;
	subRmbParam.source			= RMBOS_SUB_BUY_PHYSTR ;
	snprintf(subRmbParam.uniqueid, sizeof(subRmbParam.uniqueid), "%lld", idOpSourceEvent) ;
	//subRmbParam.uniqueid		= idOpSourceEvent ;
	subRmbParam.subvalue		= info->BuyPhyDiamondCost ;
	snprintf(subRmbParam.desc, sizeof(subRmbParam.desc), "buy phystr cost(viplvl:%u,getphystr:%u)", 
		GetVipLevel(), CGameConfigMgr::Instance()->GetConfigData().EachBuyPhyStr) ;
	//备份
	UserRMB_t oldRMB_t = GetRMB_t();
	//扣除钻石
	int iRev = SubRMB(subRmbParam) ;
	if(0 != iRev)
	{
		LOGERR("CUser::BuyPhyStr(),sub rmb failed! CUser::SubRMB() return fail!(%d)userid=%lld,costrmb=%u,userrmb=%d",
			iRev, GetUserID(), pVip->BuyPhyneedDiamond, nRmb);
		return false ;
	}
	//日志
	const UserRMB_t& newRMB_t = GetRMB_t();
	LOG_USER_PURCHASE(this, MoneyType_Diamond, subRmbParam.source, 0, CGameConfigMgr::Instance()->GetConfigData().EachBuyPhyStr, oldRMB_t.m_charge-newRMB_t.m_charge, oldRMB_t.m_donate-newRMB_t.m_donate, oldRMB_t.m_bindrmb-newRMB_t.m_bindrmb);

	//给体力
	uCurrPs += (info->BuyPhyCount+info->PresentPhyCount);//CGameConfigMgr::Instance()->GetConfigData().EachBuyPhyStr ;
	if(uCurrPs > MAX_PLAYER_PHYSTRENGTH)
	{
		LOGINFO("CUser::BuyPhyStr(), after buy truncation currPhysicalStrength! maxvalue=%d, truevalue=%u, userid=%lld",
				   MAX_PLAYER_PHYSTRENGTH, uCurrPs, propertys.userid);
		uCurrPs = MAX_PLAYER_PHYSTRENGTH ;
	}
	SetPhysicalStrength(uCurrPs, true) ;
	++propertys.buy_physical_count;
	propertys.buy_physical_last_time = (uint32_t)time(0);

	if (!server.memcached_enable)
	{
		s_dbproxy.UpDateUserInfo(&propertys);
	}

	
	//nRemainTimes = pVip->buyphymaxtimes - propertys.buy_physical_count ;
	//if(nRemainTimes < 0)
	//	nRemainTimes = 0 ;
	return true;
	func_end();
}

int32_t CUser::TryAddExp(uint32_t nExp,uint32_t& nAfterLvl,uint32_t& nAfterExp)
{
	func_start();

	uint32_t	nPlayerExpCount = propertys.cur_exp + nExp ;
	uint32_t	nAddlvl = 0 ;
	uint8_t		nTmplvl = propertys.lvl ;
	int32_t		nNeedExps = 0 ;
	nAfterLvl = propertys.lvl ;
	nAfterExp = propertys.cur_exp ;

	while(nTmplvl + 1 <= CGameConfigMgr::Instance()->GetConfigData().Max_UserLvl)
	{
		const PlayerTab_t* pTabData = CPlayerTableManager::Instance()->GetData(nTmplvl) ;
		if(NULL == pTabData)
		{
			LOGERR("CUser::TryAddExp, Get player table data error!(userid=%lld,lvl=%u,currExp=%u,addExp=%u)",
				propertys.userid, nTmplvl, propertys.cur_exp, nExp);
			return MT_ERR;
		}

		if(nPlayerExpCount < (uint32_t)(nNeedExps + pTabData->m_NeedExps))
			break ;

		nAddlvl++ ;
		nTmplvl++ ;
		nNeedExps += pTabData->m_NeedExps ;
	}

	if(nAddlvl > 0)
	{
		nAfterLvl += nAddlvl ;
		nAfterExp = nPlayerExpCount - nNeedExps ;
	}
	else
	{
		nAfterLvl = nTmplvl ;

		//经验不足以升级
		nAfterExp += nExp ;
	}

	/*满级后 玩家经验不能超过升到下一级 的经验值*/
	if (nAfterLvl == CGameConfigMgr::Instance()->GetConfigData().Max_UserLvl)
	{
		const PlayerTab_t* pTabData = CPlayerTableManager::Instance()->GetData(nAfterLvl) ;
		if(NULL == pTabData)
		{
			LOGERR("CUser::TryAddExp, Get player table data error!(userid=%lld,lvl=%u,currExp=%u,addExp=%u)",
				propertys.userid, nAfterLvl, propertys.cur_exp, nExp);
			return MT_ERR;
		}

		if (nAfterExp > pTabData->m_NeedExps)
		{
			nAfterExp = pTabData->m_NeedExps;
		}
	}

	return MT_OK ;
	func_end();
}

int32_t CUser::AddExp(uint32_t nExp, bool bUpDB/* = false*/)
{
	func_start();

	uint32_t	nAfterLvl = 0 ;
	uint32_t	nAfterExp = 0 ; 
	uint8_t		nTmplvl = propertys.lvl ;

	if (MT_ERR == TryAddExp(nExp,nAfterLvl,nAfterExp))
	{
		return MT_ERR;
	}

	if(nAfterLvl > propertys.lvl)
	{
		ExecLevelUpActivtiy(propertys.lvl,nAfterLvl);
		ExecLevelUpActivtiy(eActivityType_newyear_levelup, propertys.lvl,nAfterLvl);
		propertys.lvl = nAfterLvl ;
		propertys.cur_exp = nAfterExp ;
		LOG_USER_LEVEL_UP(this);

		//等级改变后受影响的其他操作

		////体力值加满
		//uint16_t uMaxPhysicalStrength = GetMaxPhysicalStrength() ;
		//uint16_t uCurrPhysicalStrength = GetCurrPhysicalStrength() ;
		//if(uCurrPhysicalStrength < uMaxPhysicalStrength)
		//{
		//	SetPhysicalStrength(uMaxPhysicalStrength) ;
		//}
		//增加指定的体力值
		int32_t _addPhyValue = 0 ;
		for(int8_t _tmpLvl=nTmplvl+1; _tmpLvl<=nAfterLvl; _tmpLvl++)
		{
			const PlayerTab_t* pTabData = CPlayerTableManager::Instance()->GetData(_tmpLvl) ;
			if(NULL == pTabData)
			{
				LOGERR("CUser::AddExp, Get player table data error!(userid=%lld,lvl=%u)", propertys.userid, _tmpLvl);
				return MT_ERR;
			}
			_addPhyValue += pTabData->m_AddPhysicalStrength ;
		}
		if(_addPhyValue > 0)
		{
			int32_t CurPhy = GetCurrPhysicalStrength();
			int32_t nNewPhy = CurPhy + _addPhyValue ;
			SetPhysicalStrength(nNewPhy);
		}

		if(bUpDB)
		{
			if (!server.memcached_enable)
			{
			//数据库更新操作
			if (0 != s_dbproxy.UpDateUserInfo(&(propertys)))
			{
				LOGERR("DBERR: CUser::AddExp(), update t_user info fail!(userid=%lld,lvl=%u,currExp=%u,addExp=%u)",
					propertys.userid, nTmplvl, propertys.cur_exp, nExp);
				return MT_ERR;
			}

			}

		}

		//英雄升级
		HerosMap::const_iterator itr = heros.begin();
		for (; itr != heros.end(); ++itr )
		{
			AddHeroExp(itr->second.GetHeroTemplateID());
		}
		/*
		if (GetComBatPower()>GetMaxComBatPower())
			s_dbproxy.UpDateSortTempToDB(*this);
		else
			s_dbproxy.UpDateSortTempLvlToDB(*this);
		*/

		//竞技场a升级通知
		GetArena().OnUserLvlChange(nTmplvl, nAfterLvl) ;

		SetUpdateTime(LOT_UserSort);
	}
	else
	{
		//所加经验不足以升级的情况
		propertys.cur_exp = nAfterExp ;

		if(bUpDB)
		{
			if (!server.memcached_enable)
			{
			//数据库更新操作
			if (0 != s_dbproxy.UpDateUserInfo(&(propertys)))
			{
				LOGERR("DBERR: CUser::AddExp(), no uplvl, update t_user info fail!(userid=%lld,lvl=%u,currExp=%u,addExp=%u)",
					propertys.userid, nTmplvl, propertys.cur_exp, nExp);
				return MT_ERR;
			}

			}

		}
	}
	return MT_OK ;
	func_end();
}

int32_t CUser::AddHeroExp(int16_t heroid,uint32_t nExp)
{
	func_start();

	if (heroid < 0)
	{
		LOGERR("CUser::AddHeroExp(), heroid err!userid=%lld,heroid=%d,nexp=%u",propertys.userid, heroid, nExp);
		return eHeroAddExpResult_HeroIDErr;
	}

	CHero* pHero = getHeroById(heroid);
	if (!pHero)
	{
		LOGERR("CUser::AddHeroExp(), hero not found!userid=%lld,heroid=%d,nexp=%u",propertys.userid, heroid, nExp);
		return eHeroAddExpResult_HeroIsNotHave;
	}

	const HerosTab_t* pTab = pHero->GetHerosTab() ;
	if(NULL == pTab)
	{
		LOGERR("CUser::AddHeroExp(), HerosTab_t not found!userid=%lld,heroid=%d,nexp=%u",propertys.userid, heroid, nExp);
		return eHeroAddExpResult_NotFindTab;
	}
	if(eHeroType_Tower == pTab->HeroType)
	{
		//箭塔加经验不走此函数
		return eHeroAddExpResult_OK ;
	}

	uint8_t maxlvl = GetLevel();
	int32_t maxexp = CLevelUpTabMgr::Instance()->GetLevelUpExp(maxlvl);
	if (-1 == maxexp) 
	{
		LOGERR("CUser::AddHeroExp(), get heroleveup data err!userid=%lld,heroid=%d,nexp=%u,lvl=%u",propertys.userid, heroid, nExp,maxlvl);
		return eHeroAddExpResult_LevelUpTabErr;
	}

	uint8_t curlvl			= pHero->GetLvl();
	uint8_t oldlvl		= pHero->GetLvl();
	uint32_t experience	= pHero->GetUpExp();
	uint32_t sum_experience = nExp + pHero->GetCurExp();
	int32_t leveupexp	= 0;
	while ( sum_experience >= experience && experience > 0 && curlvl < maxlvl)
	{
		curlvl++;

		//记录英雄升级所剩的经验
		sum_experience -= experience;

		/*升级经验*/
		leveupexp = CLevelUpTabMgr::Instance()->GetLevelUpExp(curlvl);
		if (-1 == leveupexp) 
		{
			LOGERR("CUser::AddHeroExp(), get heroleveup data err!userid=%lld,heroid=%d,nexp=%u,lvl=%u",propertys.userid, heroid, nExp,curlvl);
			return eHeroAddExpResult_LevelUpTabErr;
		}

		experience = leveupexp;
	}

	/*等级不能超过玩家等级 英雄经验不能超过 英雄升级到下一级的经验  超出不计 */
	if (curlvl > maxlvl || (curlvl == maxlvl && sum_experience >= maxexp))
	{
		curlvl = maxlvl;
		sum_experience = maxexp;
	}

	pHero->SetLvl(curlvl);
	pHero->SetCurExp(sum_experience);
	/*英雄升级 装备升级*/
	if (curlvl > oldlvl)
	{
		for (uint8_t i = eEquipPos_Weapon;i<eEquipPos_Max;++i)
		{
			if (pHero->IsInvalidEquipPosInfo((EquipPos)i))
			{
				pHero->AddEquipExp((EquipPos)i,0);
			}
		}

		/*Bag数据更新到DB*/
		if (!UpdateBagInfoToDB())
		{
			LOGERR("CUser::AddHeroExp(), update baginfo failed!userid=%lld,heroid=%d,nexp=%u",propertys.userid, heroid, nExp);
			return eHeroAddExpResult_DBErr;
		}
	}

	/*Hero数据更新到DB*/
	if ( MT_OK != UpdateHeroToDB(heroid))
	{
		LOGERR("CUser::AddHeroExp(), update heroinfo failed!userid=%lld,heroid=%d,nexp=%u",propertys.userid, heroid, nExp);
		return eHeroAddExpResult_DBErr;
	}

	return eHeroAddExpResult_OK;
	func_end();
}

int32_t CUser::AddTowerExp(int16_t heroid,uint32_t nExp)
{
	func_start();

	if (heroid < 0)
	{
		LOGERR("CUser::AddTowerExp(), heroid err!userid=%lld,heroid=%d,nexp=%u",propertys.userid, heroid, nExp);
		return eHeroAddExpResult_HeroIDErr;
	}

	CHero* pHero = getHeroById(heroid);
	if (!pHero)
	{
		LOGERR("CUser::AddTowerExp(), tower not found!userid=%lld,heroid=%d,nexp=%u",propertys.userid, heroid, nExp);
		return eHeroAddExpResult_HeroIsNotHave;
	}

	const HerosTab_t* pTab = pHero->GetHerosTab() ;
	if(NULL == pTab)
	{
		LOGERR("CUser::AddTowerExp(), tower HerosTab_t not found!userid=%lld,heroid=%d,nexp=%u",propertys.userid, heroid, nExp);
		return eHeroAddExpResult_NotFindTab;
	}
	if(eHeroType_Tower != pTab->HeroType)
	{
		LOGERR("CUser::AddTowerExp(), not is tower!userid=%lld,heroid=%d,nexp=%u",propertys.userid, heroid, nExp);
		return eHeroAddExpResult_IsNotTower ;
	}

	uint8_t maxlvl = GetLevel();
	int32_t maxexp = CLevelUpTabMgr::Instance()->GetLevelUpExp(maxlvl);
	if (-1 == maxexp) 
	{
		LOGERR("CUser::AddTowerExp(), get heroleveup data err!userid=%lld,heroid=%d,nexp=%u,lvl=%u",propertys.userid, heroid, nExp,maxlvl);
		return eHeroAddExpResult_LevelUpTabErr;
	}

	uint8_t curlvl			= pHero->GetLvl();
	uint8_t oldlvl			= pHero->GetLvl();
	uint32_t experience		= pHero->GetTowerUpExp();
	uint32_t sum_experience = nExp + pHero->GetCurExp();
	int32_t leveupexp	= 0;
	while ( sum_experience >= experience && experience > 0 && curlvl < maxlvl)
	{
		curlvl++;

		//记录英雄升级所剩的经验
		sum_experience -= experience;

		/*升级经验*/
		leveupexp = CLevelUpTabMgr::Instance()->GetTowerLevelUpExp(curlvl);
		if (-1 == leveupexp) 
		{
			LOGERR("CUser::AddTowerExp(), get towerleveup data err!userid=%lld,heroid=%d,nexp=%u,lvl=%u",propertys.userid, heroid, nExp,curlvl);
			return eHeroAddExpResult_LevelUpTabErr;
		}
		experience = leveupexp;
	}

	/*等级不能超过玩家等级 英雄经验不能超过 英雄升级到下一级的经验  超出不计 */
	if (curlvl > maxlvl || (curlvl == maxlvl && sum_experience >= maxexp))
	{
		curlvl = maxlvl;
		sum_experience = maxexp;
	}

	pHero->SetLvl(curlvl);
	pHero->SetCurExp(sum_experience);

	/*Hero数据更新到DB*/
	if ( MT_OK != UpdateHeroToDB(heroid))
	{
		LOGERR("CUser::AddTowerExp(), update heroinfo failed!userid=%lld,heroid=%d,nexp=%u",propertys.userid, heroid, nExp);
		return eHeroAddExpResult_DBErr;
	}

	return eHeroAddExpResult_OK;
	func_end();
}

//设置用户标识
void CUser::SetUserFlag( user_flag flag )
{
	if (flag == userflag_combat || flag == userflag_normal||flag == userflag_killoff)
	{
		m_UserFlag = flag;
	}
}

bool CUser::IsUpBoundFriends()
{
	if (m_FriendMgr.GetCurFriendsCount() >= GetMaxFriendsCount())
	{
		return true;
	}
	return false;
}

int CUser::conUserFriendExist( CUser* friendInfo)
{
	func_start();
	if (NULL == friendInfo)
	{
		return -1;
	}
	if (friendInfo->GetFriendMng().IsFriend(GetUserID()))
	{
		//玩家好友数据的更新
		if (!m_FriendMgr.AddFriend(friendInfo->GetUserID()))
		{
			LOGWARN("UpdateFriend: User=%lld account=%s information your friend is full"
				,propertys.userid,propertys.account.to_string());
			return -1;
		}

		if (!server.memcached_enable)
		{
		//数据库好友信息表数据的更新
		if (MT_ERR == s_dbproxy.UpDateFriendsInfo(propertys.userid,this))
		{
			LOGERR("UpdateFriend: User=%lld account=%s information failed! update friendInfo db is wrong",
				propertys.userid,propertys.account.to_string());
			return -1;
		}
		}


//		SetUpdateTime(LOT_Friend);
		
		return 0;
	}
	return 1;
	func_end();
}

void CUser::delUnlineFriend( int64_t userid )
{
	func_start();
	if (ISINVALIDGUID(userid))
	{
		return;
	}
	GetFriendMng().ClearFriend(userid);
	//体力交互相关的处理
	uint32_t uCount = 0;
	for (int i = 0; i < MAX_SEND_PHY_EACH_OTHER_NUM; ++i)
	{
		SendPhysicalData* pData = GetFriendMng().GetSendPhysicalDataByIdx(i);
		if (NULL == pData)
		{
			continue;
		}
		pData->delFriendInfo(userid);
		uCount = pData->currCount;
		if (uCount == 0)
		{
			if (!server.memcached_enable)
			{
		//更新数据库数据
			if (MT_ERR == s_dbproxy.DeleteSendPhyInfo(propertys.userid,i))
			{
				LOGERR("Delete friend SendPhyInfo:delete sendPhy table failed,userid=%lld,Status=%u",
					propertys.userid,i);
			}
			}
	
		}
		else
		{
			if (!server.memcached_enable)
			{
		//更新数据库数据
			if (MT_ERR == s_dbproxy.UpDateSendPhyInfo(propertys.userid,pData,i))
			{
				LOGERR("Delete friend SendPhyInfo:update sendPhy table failed,userid=%lld,Status=%u",
					propertys.userid,i);
			}
			}
	
		}
	}
	func_end();
}

//获取队长的英雄
CHero* CUser::GetTeamLeaderHero()
{
	CHero* pHero = getHeroById(propertys.teamleaderid);
	if (NULL != pHero)
	{
		return pHero;
	}
	return NULL;
}
//玩家真金(钻石)产出唯一接口,
//内部阻塞更新数据库数据成功后更改内存数据
//返回0表示操作成功
int CUser::AddRMB(AddRMBParam_t & param)
{
/*
产出规则如下：
1，凡是跟充值相关的获取值，且该笔充值尚未完全确认收入的，都记录成真金，购买真金＝充值数x兑换比例，赠送真金＝获取值－购买真金。
2，凡是跟充值无关的获取值，或者虽然跟充值有关系，但是该笔充值已经完全确认收入以后的，都记录成绑金，绑金值＝获取值。
3，累计充值和充值未完全确认期间的充值获取值（比如月卡第二天除当前之外的其它天获取），记录成赠送真金。
*/
	func_start();

	//参数有效性检查...
	int iRev = param.check() ;
	if(0 != iRev)
	{
		LOGERR("CUser::AddRMB(),param check fail!(rev=%d,UserID=%lld,source=%u,uniqueid=%s,charge=%d,donate=%d,bind=%d,desc=%s,selfdata=%s)", 
			iRev, propertys.userid, param.source, param.uniqueid, param.charge, param.donate, param.bind, param.desc, param.selfdata);
		return -1 ;
	}

	//组织变化后数据
	UserRMB_t _afterData ;
	_afterData.m_charge		= userrmb.m_charge + param.charge ;
	_afterData.m_donate		= userrmb.m_donate + param.donate ;
	_afterData.m_bindrmb	= userrmb.m_bindrmb + param.bind  ;

	//"充值累计总额"暂时累加"购买真金"值
	_afterData.m_chargecount = userrmb.m_chargecount + param.charge ;
	if (!server.memcached_enable)
	{
		//更新数据库
		if(MT_OK != s_dbproxy.UpDateUserRMB(propertys.userid, _afterData))
		{
			LOGERR("DBERR: CUser::AddRMB(),s_dbproxy.UpDateUserRMB() return fail!(UserID=%lld,source=%u,uniqueid=%s,charge=%d,donate=%d,bind=%d,desc=%s,selfdata=%s)", 
				propertys.userid, param.source, param.uniqueid, param.charge, param.donate, param.bind, param.desc, param.selfdata);
			return -2 ;
		}
	}

	//更改内存
	userrmb = _afterData ;
	

	//组织日志数据
	memset(param.time, 0, sizeof(param.time)) ;
	time_t timep;
	struct tm *ptm;
	time(&timep);
	ptm = localtime(&timep);      //取得当地时间
	int rst = snprintf(param.time, sizeof(param.time), "%d-%02d-%02d %02d:%02d:%02d", 
                    1900 + ptm->tm_year, 1 + ptm->tm_mon, 
                    ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);

	//写日志
	log_money_changed(USER_RMB_LOGFILE_VER, 
						param.time, 
						propertys.userid, 
						(unsigned int)RMBOT_Add, 
						param.source, 
						param.uniqueid,
						param.charge,
						param.donate,
						param.bind,
						userrmb.m_charge,
						userrmb.m_donate,
						userrmb.m_bindrmb,
						param.desc,
						param.selfdata) ;
	//日志统计
	LOG_USER_REWARD(this, MoneyType_Diamond, param.charge, param.donate, param.bind, param.source);

	SetUpdateTime(LOT_UserRMB);

	return 0 ;
	func_end();
}

//玩家真金(钻石)消耗唯一接口,
//内部阻塞更新数据库数据成功后更改内存数据
//返回0表示操作成功
int CUser::SubRMB(SubRMBParam_t & param)
{
/*
消耗规则如下：
1，优先消耗绑金，其次消耗赠送真金，最后消耗购买真金。
2，对于已上线项目，除了上述三个类型产出外，还有一个剩余金，此时，消耗顺序是：剩余金，绑金，赠送真金，购买真金。
3，消耗时，如果当前购买真金＋当前赠送真金＋当前绑金<0时，消耗失败。
4，当前绑金值<=0时，消耗赠送真金；当前赠送真金值<=0时，消耗购买真金；当前购买真金值<=0时，消耗失败。
5，玩家利用bug刷产出时，额外获取的是什么类型的，扣除什么类型的产出。
     比如：玩家利用充值bug刷产出时，额外获取的是购买真金和赠送真金，那么扣除的也是购买真金和赠送真金。且扣除跟额外获取的比例一致。可以扣除为负值。
                 玩家利用赠送bug刷产出时，额外获取的是绑金，那么扣除的也是绑金。且扣除跟额外获取的比例一致。可以扣除为负值。
*/
	func_start();

	//参数有效性检查...
	int iRev = param.check() ;
	if(0 != iRev)
	{
		LOGERR("CUser::SubRMB(),param check fail!(rev=%d,UserID=%lld,source=%u,uniqueid=%s,subvalue=%d,desc=%s,selfdata=%s)", 
			iRev, propertys.userid, param.source, param.uniqueid, param.subvalue, param.desc, param.selfdata);
		return -1 ;
	}

	//检查本次消耗能否成功
	if(param.subvalue > userrmb.GetCount())
	{
		LOGERR("CUser::SubRMB(),no enough rmb!(rmbcount=%d,UserID=%lld,source=%u,uniqueid=%s,subvalue=%d,desc=%s,selfdata=%s)", 
			userrmb.GetCount(), propertys.userid, param.source, param.uniqueid, param.subvalue, param.desc, param.selfdata);
		return -2 ;
	}
	//钻石消耗活动
	if(param.subvalue>0)
	{
		ExecActivity(eActivityType_usediamond, param.subvalue);
	}

	//组织变化后数据
	int _iSubBind	= 0 ;
	int _iSubDonate	= 0 ;
	int _iSubCharge	= 0 ;
	int _iSubValue = param.subvalue ;
	UserRMB_t _afterData = userrmb ;
	if(_afterData.m_bindrmb - _iSubValue >= 0)
	{
		_afterData.m_bindrmb = _afterData.m_bindrmb - _iSubValue ;
		_iSubBind = _iSubValue ;
	}
	else
	{
		_iSubValue = _iSubValue - _afterData.m_bindrmb ;
		_iSubBind = _afterData.m_bindrmb ;
		_afterData.m_bindrmb = 0 ;

		if(_afterData.m_donate - _iSubValue >= 0)
		{
			_afterData.m_donate = _afterData.m_donate - _iSubValue ;
			_iSubDonate = _iSubValue ;
		}
		else
		{
			_iSubValue = _iSubValue - _afterData.m_donate ;
			_iSubDonate = _afterData.m_donate ;
			_iSubCharge = _iSubValue ;
			_afterData.m_donate = 0 ;
			_afterData.m_charge = _afterData.m_charge - _iSubValue ;
		}
	}
	if (!server.memcached_enable)
	{
		//更新数据库
		if(MT_OK != s_dbproxy.UpDateUserRMB(propertys.userid, _afterData))
		{
			LOGERR("DBERR: CUser::SubRMB(),s_dbproxy.UpDateUserRMB() return fail!(UserID=%lld,source=%u,uniqueid=%s,subvalue=%d,desc=%s,selfdata=%s)", 
				propertys.userid, param.source, param.uniqueid, param.subvalue, param.desc, param.selfdata);
			return -3 ;
		}
	}

	//VIP相关处理
	static float sgRatio = CGameConfigMgr::Instance()->GetConfigData().ConsumeVipExp * 1.f / CGameConfigMgr::Instance()->GetConfigData().ConsumeDiamond;
	SetVipExp(GetVipExp() + param.subvalue * sgRatio, CGameConfigMgr::Instance()->GetConfigData().AddVipExpAnything);

	//更改内存
	userrmb = _afterData ;

	//组织日志数据
	memset(param.time, 0, sizeof(param.time)) ;
	time_t timep;
	struct tm *ptm;
	time(&timep);
	ptm = localtime(&timep);      //取得当地时间
	int rst = snprintf(param.time, sizeof(param.time), "%d-%02d-%02d %02d:%02d:%02d", 
                    1900 + ptm->tm_year, 1 + ptm->tm_mon, 
                    ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);

	//写日志
	log_money_changed(USER_RMB_LOGFILE_VER, 
						param.time, 
						propertys.userid, 
						(unsigned int)RMBOT_Sub, 
						param.source, 
						param.uniqueid,
						_iSubCharge,
						_iSubDonate,
						_iSubBind,
						userrmb.m_charge,
						userrmb.m_donate,
						userrmb.m_bindrmb,
						param.desc,
						param.selfdata) ;
	
	SetUpdateTime(LOT_UserRMB);
	return 0 ;
	func_end();
}

void CUser::CalcVipLevel(bool bLoadData/* = false*/)
{
	func_start();
	uint32_t uVipLevel = 0;

	CVipMng::Instance()->CalcVipLevel(propertys.vip_exp, uVipLevel, GetUserID());
	ExecVipLevelUpActivtiy(eActivityType_newyear_viplevelup, propertys.vip_lvl, uVipLevel);	//刷新VIP冲级活动状态
	bool bChanged = false ;
	if(uVipLevel != propertys.vip_lvl)
	{
		if (!bLoadData)//只有在线升级才发邮件，重新加载数据不发邮件了
		{
			int temp = propertys.vip_lvl + 1;
			for (; temp <= uVipLevel; ++temp)
			{
				if (_VipAward.Status == eVipAward_NoGet)
				{
					VipAwardTabItem* pVipAwardTabItem = CVipAwardTabMng::Instance()->GetVipAwardInfoByID(temp);
					if (pVipAwardTabItem)
					{
						//发送奖励(邮件)
						MemSystemEmailData _MailData ;
						CSysMailMgr::CreateSysEmailId(_MailData.sysid);
						snprintf(_MailData.msgData, sizeof(_MailData.msgData)-1, "~!%d|0_%d", 4, temp) ;
						_MailData.msg_len  = (strlen(_MailData.msgData) > sizeof(_MailData.msgData)-1)?sizeof(_MailData.msgData)-1:strlen(_MailData.msgData) ;
						_MailData.affixStat = eAffixStatus_UnDone;
						_MailData.affixdata.count = 1 ;
						_MailData.affixdata.item[0].affixType = eOutPutAwardType_Item ;
						_MailData.affixdata.item[0].affixNum = 1;
						_MailData.affixdata.item[0].affixPara = pVipAwardTabItem->bagid;
						_MailData.emailReadStatus = eEmailReadStatus_UnDone;
						_MailData.sendTime = (uint32_t)time(NULL) ;
						if(!CSysMailMgr::AddSysMailItem(GetUserID(), _MailData, true))
						{
							LOGERR("CalcVipLevel, CSysMailMgr::AddSysMailItem return fail! bagid=%d, userid=%lld!", pVipAwardTabItem->bagid, GetUserID());
						}

						_VipAward.LastTime = time(NULL);
						SetUpdateTime(LOT_VipAward);
					}
				}
				else
				{
					_VipAward.Status = eVipAward_NoGet;
				}
			}
		}

		bChanged = true;
	}

	const uint32_t uPreVipLevel = propertys.vip_lvl; 

	propertys.vip_lvl = uVipLevel;
	//vip升级后重置特权数据
	if(bChanged && !bLoadData)
	{
		GetSmallEndLess().OnVipUp();
		ClearDataForVipLevelUp();
		m_ZodiacMng.OnUserVipChange( uPreVipLevel, uVipLevel);
	}
	func_end();
}

uint32_t CUser::GetVipLevel()
{
	//2014.11.20封测临时实现
	//正式上线需要调整!!!
	int32_t nMaxVipLvl = CVipMng::Instance()->GetMaxVipLvl();
	//uint32_t uVipLevel = propertys.vip_lvl + 1;
	uint32_t uVipLevel = propertys.vip_lvl ;
	if(uVipLevel > nMaxVipLvl)
		uVipLevel = nMaxVipLvl ;
	return uVipLevel ;
}

void CUser::ClearDataForVipLevelUp()
{
	////购买体力的次数清除
	//propertys.buy_physical_count=0;
	////购买金币的次数清除
	//propertys.buy_glod_count = 0;
	////接受体力反馈的次数 HERO-4981【建议】VIP升级时，获取好好友体力赠送会重置，建议优化，避免玩家获取体力赠送收益过大
	//propertys.byAcceptPhyCount = 0;
	////接受体力赠送的次数
	//propertys.byPresentPhyCount=0;
	////清除十二宫数据,设置刷新次数和时间
	//m_ZodiacMng.ResetRefurbishCount();   //-- 这里不能直接按当前级别重置，，先注掉， 会导致一个问题：当前数量是0时，升级时就算次数不增加也会恢复每日次数 2015-11-17 by fx
	///重置关卡购买次数
	//这里不能清理，精英本的扫荡次数,用尽后，vip升级后，任然不能买 
	//FBStatus.ResetBuyTimes();
	_VipAward.ReSetStatus();

	UpdateUserDataToDB();

	//扫荡券数量清除(已经在日常任务系统完成)

	//购买关卡的次数清除(内部实现存储数据库逻辑)
	//FBStatus.ClearFBStatus4VipLevelUp(GetUserID());
}

/*
	@param1:好友id
	@param2:交互状态,赠送或者接受赠送
*/
int CUser::HandlerSendPhyEachOther( int64_t uFriendId,uint8_t uEachOtherStatus )
{
	func_start();
	//不是好友不能进行赠送、接受赠送操作
	if (!GetFriendMng().IsFriend(uFriendId))
	{
		return eEachOtherPhyResult_NoFriend;
	}
	//好友是否在线
	CUser*  userFriend = CDataMgr::Instance()->FindTempUserByUserID(uFriendId);
	if ((NULL == userFriend)||
		(NULL!=userFriend && userFriend->GetLoadFlag()==eUserLoadFlag_TmpLoad)
		)
	{
		return eEachOtherPhyResult_UnOnline;
	}
	if (MT_ERR == CDataMgr::Instance()->FindFriends(userFriend->GetUserID(), userFriend->GetFriendMng()) )
	{
		return eEachOtherPhyResult_Uninit;
	}
	uint32_t uHandlerTime = (uint32_t)time(NULL);
	uint32_t uAcceptNum   = 1;
	uint32_t uPresentNum  = 1;
	//获取接受赠送和赠送的次数限制
	CalcSendPhyEachOtherInfo(uAcceptNum,uPresentNum);
	//赠送体力的刷新时间
	if (propertys.sendPhyRefTime == 0)
	{
		propertys.byAcceptPhyCount  = 0;
		propertys.byPresentPhyCount = 0;
		propertys.sendPhyRefTime    = uHandlerTime;
	}
	else 
	{
		uint32_t hour = 0; 
		uint32_t minute = 0;
		GetServerCdTimeByIndex(eCdTimeIndex_SendPhyRefurbish, hour, minute);
		if (0 != ComPareTimeWithHM(hour, minute, propertys.sendPhyRefTime, uHandlerTime))
		{
			propertys.byAcceptPhyCount  = 0;
			propertys.byPresentPhyCount = 0;
			propertys.sendPhyRefTime    = uHandlerTime;
		}
	}
	//判断玩家每天接受体力的次数
	if (uEachOtherStatus == eCurrSendPhyHandlerStatus_AcceptPhy)
	{
		if (propertys.byAcceptPhyCount >= uAcceptNum)
		{
			return eEachOtherPhyResult_MaxAcceptNum;
		}
	}
	//判断玩家每天赠送体力的次数
// 	else if (uEachOtherStatus == eCurrSendPhyHandlerStatus_SendPhy)
// 	{	
// 		if (propertys.byPresentPhyCount >= uPresentNum)
// 		{
// 			return eEachOtherPhyResult_MaxPrizeNum;
// 		}
// 	}
	//赠送体力逻辑处理
	if (uEachOtherStatus == eCurrSendPhyHandlerStatus_SendPhy)
	{
		//先判断今天我有没有给好友赠送过体力
		uint32_t uEachOtherTime = 0;
		SendPhysicalData* pFriendSPData = userFriend->GetFriendMng().GetSendPhysicalDataByIdx(eCurrSendPhyHandlerStatus_SendPhy);
		if (NULL == pFriendSPData)
		{
			return eEachOtherPhyResult_ErrInfo;
		}
		uint8_t count = pFriendSPData->currCount;
		PhyFriendInfo* pPhyFriendInfo = pFriendSPData->GetPhyFriendInfoByUserid(propertys.userid);
		if (NULL != pPhyFriendInfo)
		{
			//每天只能赠送体力一次
			uEachOtherTime = pPhyFriendInfo->handlerTime;
			uint32_t hour = 0; 
			uint32_t minute = 0;
			GetServerCdTimeByIndex(eCdTimeIndex_SendPhyRefurbish, hour, minute);
			if (ComPareTimeWithHM(hour,minute,GetFriendMng().GetSendPhyTimeData(),uEachOtherTime) == 0)
			{
				return eEachOtherPhyResult_DoneSendPhy;
			}
			pPhyFriendInfo->handlerTime = uHandlerTime;
		}
		else
		{
			pFriendSPData->AddFriend(propertys.userid,uHandlerTime);
		}
		//不存在的话，数据库创建一条记录
		if (count == 0)
		{
			if (!server.memcached_enable)
			{
			//更新数据库数据
			if (MT_ERR == s_dbproxy.WriteSendPhyInfoToDB(userFriend->GetUserID(),pFriendSPData,uEachOtherStatus))
			{
				LOGERR("SendPhyEachOther:write sendPhy table failed,userid=%lld,Status=%u,friendId=%lld",
					GetUserID(),uEachOtherStatus,userFriend->GetUserID());
				return eEachOtherPhyResult_ErrInfo;
			}
			}

		}
		else
		{
			if (!server.memcached_enable)
			{
			//更新数据库数据
			if (MT_ERR == s_dbproxy.UpDateSendPhyInfo(userFriend->GetUserID(),pFriendSPData,uEachOtherStatus))
			{
				LOGERR("SendPhyEachOther:update sendPhy table failed,userid=%lld,Status=%u,friendId=%lld",
					GetUserID(),uEachOtherStatus,userFriend->GetUserID());
				return eEachOtherPhyResult_ErrInfo;
			}
			}

		}
		//赠送体力次数增加
		propertys.byPresentPhyCount++;
		//记录日志
		log_send_phy_eachother(propertys.userid,uEachOtherStatus,uFriendId,CGameConfigMgr::Instance()->GetConfigData().AcceptSendPhyNum/*server.accept_sendphy_num*/,GetCurrPhysicalStrength(),"SendPhyEachOther");
	
		if (!server.memcached_enable)
		{
		//更新玩家的数据
		if (MT_ERR == s_dbproxy.UpDateUserInfo(&propertys))
		{
			LOGERR("SendPhyEachOther:update user table failed,userid=%lld,Status=%u,friendId=%lld",
				GetUserID(),uEachOtherStatus,uFriendId);
			return eEachOtherPhyResult_ErrInfo;
		}

		}
		UPDATE_FRIENDS_DATA(userFriend, userFriend->GetFriendMng());

		return eEachOtherPhyResult_SendOk;
	}

	if (uEachOtherStatus == eCurrSendPhyHandlerStatus_AcceptPhy)
	{
		//判断体力是否已满
// 		if (GetCurrPhysicalStrength() > GetMaxPhysicalStrength())
// 		{
// 			return eEachOtherPhyResult_MaxPSFull;
// 		}
		//玩家的赠送体力队列(好友对玩家的赠送)
		SendPhysicalData* pSendPhy = GetFriendMng().GetSendPhysicalDataByIdx(eCurrSendPhyHandlerStatus_SendPhy);
		if (NULL == pSendPhy)
		{
			return eEachOtherPhyResult_ErrInfo;
		}
		//在接受体力之前，判断好友有没有给玩家发送过赠送体力请求
		PhyFriendInfo* pPhyInfoData = pSendPhy->GetPhyFriendInfoByUserid(uFriendId);
		if (NULL == pPhyInfoData)
		{
			LOGERR("friend[%lld] no send phy for [%lld]",uFriendId,GetUserID());
			return eEachOtherPhyResult_ErrInfo;
		}
		uint32_t uSendPhyTime   = 0;	//好友上次赠送体力的时间
		uint32_t uAcceptPhyTime = 0;	//玩家上次接受体力的时间
		//好友的接受赠送体力队列
		SendPhysicalData* pFriendSPData = userFriend->GetFriendMng().GetSendPhysicalDataByIdx(eCurrSendPhyHandlerStatus_AcceptPhy);
		if (NULL == pFriendSPData)
		{
			return eEachOtherPhyResult_ErrInfo;
		}
		uint32_t uCount         = pFriendSPData->currCount;
		//判断玩家是否接受过好友的赠送体力
		PhyFriendInfo* pFriendPhyInfoData = pFriendSPData->GetPhyFriendInfoByUserid(propertys.userid);
		if (NULL != pFriendPhyInfoData)
		{
			uAcceptPhyTime = pFriendPhyInfoData->handlerTime;
			uSendPhyTime   = pPhyInfoData->handlerTime;
			//判断今天有没有接受过
			if (uAcceptPhyTime >= uSendPhyTime)
			{
				return eEachOtherPhyResult_DoneAccPhy;
			}
			pFriendPhyInfoData->handlerTime = uHandlerTime;
		}
		else//处理玩家从来没有接受过这个好友的赠送体力的情况
		{
			pFriendSPData->AddFriend(propertys.userid,uHandlerTime);
		}
		if (uCount == 0)
		{
			if (!server.memcached_enable)
			{
				//更新数据库数据
				if (MT_ERR == s_dbproxy.WriteSendPhyInfoToDB(userFriend->GetUserID(),pFriendSPData,uEachOtherStatus))
				{
					LOGERR("SendPhyEachOther:write sendPhy table failed,userid=%lld,Status=%u,friendId=%lld",
						propertys.userid,uEachOtherStatus,userFriend->GetUserID());
					return eEachOtherPhyResult_ErrInfo;
				}
			}

		}
		else
		{
			if (!server.memcached_enable)
			{
				//更新数据库数据
				if (MT_ERR == s_dbproxy.UpDateSendPhyInfo(userFriend->GetUserID(),pFriendSPData,uEachOtherStatus))
				{
					LOGERR("SendPhyEachOther:update sendPhy table failed,userid=%lld,Status=%u,friendId=%lld",
						propertys.userid,uEachOtherStatus,userFriend->GetUserID());
					return eEachOtherPhyResult_ErrInfo;
				}
			}

		}
		//判断玩家体力的增减
		SetPhysicalStrength(GetCurrPhysicalStrength() + CGameConfigMgr::Instance()->GetConfigData().AcceptSendPhyNum/*server.accept_sendphy_num*/);
		//接受体力的次数++
		propertys.byAcceptPhyCount++;
		//记录日志
		log_send_phy_eachother(GetUserID(),uEachOtherStatus,uFriendId,CGameConfigMgr::Instance()->GetConfigData().AcceptSendPhyNum/*server.accept_sendphy_num*/,GetCurrPhysicalStrength(),"SendPhyEachOther");
		
		if (!server.memcached_enable)
		{

		//更新玩家的数据
		if (MT_ERR == s_dbproxy.UpDateUserInfo(&propertys))
		{
			LOGERR("SendPhyEachOther:update user table failed,userid=%lld,Status=%u,friendId=%lld",
				GetUserID(),uEachOtherStatus,uFriendId);
			return eEachOtherPhyResult_ErrInfo;
		}
		}
		


		UPDATE_USER_DATA(userFriend);
		UPDATE_FRIENDS_DATA(userFriend, userFriend->GetFriendMng());

		return eEachOtherPhyResult_AcceptOk;
	}
	return eEachOtherPhyResult_ErrInfo;
	func_end();
}

void CUser::CalcSendPhyEachOtherInfo( uint32_t& uMaxAcceptPhyNum,uint32_t& uMaxPrizePhyNum )
{
	func_start();
	uMaxAcceptPhyNum = 1;
	uMaxPrizePhyNum  = 1;
	vip_tab* pTab = CVipMng::Instance()->GetVipTabInfoByID(GetVipLevel());
	if (NULL == pTab)
	{
		LOGERR("CalcSendPhyEachOtherInfo pTab is Null,ID[%d]",GetVipLevel());
		return;
	}
	uMaxAcceptPhyNum = pTab->AcceptPhyMaxTimes;
	uMaxPrizePhyNum = pTab->SendPhyMaxTimes;
	func_end();
}

bool CUser::GetServerCdTimeByIndex( uint32_t index, uint32_t& hour, uint32_t& minute )
{
	func_start();
	if (index >= eCdTimeIndex_Max)
	{
		LOGERR("GetServerCdTimeByIndex index[%d] is error",index);
		return false;
	}
	cdtime_tab* pTab = CCdTimeTabMng::Instance()->GetCdTimeTabInfoByID(index);
	if (NULL == pTab)
	{
		LOGERR("GetServerCdTimeByIndex pTab is null,Index[%d]",index);
		return false;
	}
	hour = pTab->refurbishHour;
	minute = pTab->refurbishMinute;
	return true;
	func_end();
}

bool CUser::SetSendPhyEachOtherExecuteStatus( int64_t uFriendId,SendPhyEachOtherData* uSendPhyData )
{
	func_start();
	uint8_t count = 0;
	const time_t nowTime = time(NULL);

	/*查找这个好友在内存池中有没有存在，就是好友在没在线的处理*/
	CUser*  userFriend = CDataMgr::Instance()->FindTempUserByUserID(uFriendId);
	GetFriendMng().SetSendPhyTimeData((uint32_t)nowTime);
	//好友不在线的情况
	if ((NULL == userFriend)
		|| (NULL!=userFriend && userFriend->GetLoadFlag()==eUserLoadFlag_TmpLoad)
		|| MT_OK != CDataMgr::Instance()->FindFriends(userFriend->GetUserID(), userFriend->GetFriendMng())
		)
	{
		for (int i = 0; i < MAX_SEND_PHY_EACH_OTHER_NUM; ++i)
		{
			SendPhysicalData* pData = GetFriendMng().GetSendPhysicalDataByIdx(i);
			if (NULL == pData)
			{
				continue;
			}
			/*玩家与这个好友之间之前从没有做过交互的情况*/
			uSendPhyData[count].executeStatus = eEachDayPhyStatusHandlerStatus_NoExecute;
			PhyFriendInfo* pPhyFriendInfoData = pData->GetPhyFriendInfoByUserid(uFriendId);
			if (NULL == pPhyFriendInfoData)
			{
				uSendPhyData[count].eachOtherTime = 0;
			}
			else
			{
				uSendPhyData[count].eachOtherTime = pPhyFriendInfoData->handlerTime;
			}
			uSendPhyData[count].eachOtherStatus = i;
			count++;
		}
		return true;
	}
	uint32_t hour = 0; 
	uint32_t minute = 0;
	GetServerCdTimeByIndex(eCdTimeIndex_SendPhyRefurbish, hour, minute);
		
	/*玩家在线的情况*/
	for (int i = 0; i < MAX_SEND_PHY_EACH_OTHER_NUM; ++i)
	{
		//赠送和接受赠送体力
		SendPhysicalData* pData = GetFriendMng().GetSendPhysicalDataByIdx(i);
		if (NULL == pData)
		{
			continue;
		}
		//玩家与这个好友之间之前从没有做过交互的情况
		PhyFriendInfo* pPhyFriendInfo = pData->GetPhyFriendInfoByUserid(uFriendId);
		if (NULL == pPhyFriendInfo)
		{
			uSendPhyData[count].eachOtherTime = 0;
		}
		else
		{
			uSendPhyData[count].eachOtherTime = pPhyFriendInfo->handlerTime;
		}
		//好友相关的赠送和接受赠送体力
		SendPhysicalData* pDataFriend =userFriend->GetFriendMng().GetSendPhysicalDataByIdx(i);
		if (NULL == pDataFriend)
		{
			continue;
		}
		if (eCurrSendPhyHandlerStatus_SendPhy == i)
		{
			/*下面是判断玩家今天可不可以发送赠送体力的消息*/
			uint32_t uEachOtherTime = 0;
			PhyFriendInfo* pFriendPhyInfo = pDataFriend->GetPhyFriendInfoByUserid(propertys.userid);
			if (NULL == pFriendPhyInfo)
			{
				uSendPhyData[count].executeStatus = eEachDayPhyStatusHandlerStatus_CanExecute;
			}
			else
			{
				/*判断玩家赠送体力有没有在同一天请求过*/
				if (ComPareTimeWithHM(hour,minute,nowTime,pFriendPhyInfo->handlerTime) == 0)
				{
					uSendPhyData[count].executeStatus = eEachDayPhyStatusHandlerStatus_NoExecute;
				}
				else
				{
					uSendPhyData[count].executeStatus = eEachDayPhyStatusHandlerStatus_CanExecute;
				}
			}
		}
		else if (eCurrSendPhyHandlerStatus_AcceptPhy == i)
		{
			//下面是判断玩家是否可以接受赠送体力的消息
			uint32_t uSendPhyTime   = 0;		//上一次好友对玩家赠送体力的时间
			uint32_t uAcceptPhyTime = 0;		//上一次玩家接受赠送体力的时间
			uSendPhyTime = uSendPhyData[0].eachOtherTime;
			if (0 == uSendPhyTime)
			{
				uSendPhyData[count].executeStatus = eEachDayPhyStatusHandlerStatus_NoExecute;
			}
			else
			{
				PhyFriendInfo* pFriendPhyInfo = pDataFriend->GetPhyFriendInfoByUserid(propertys.userid);
				if (NULL == pFriendPhyInfo)
				{
					uSendPhyData[count].executeStatus = eEachDayPhyStatusHandlerStatus_CanExecute;
				}
				else
				{
					uAcceptPhyTime = pFriendPhyInfo->handlerTime;
					if (uAcceptPhyTime <= uSendPhyTime)
					{
						uSendPhyData[count].executeStatus = eEachDayPhyStatusHandlerStatus_CanExecute;
					}
					else if(ComPareTimeWithHM(hour,minute,nowTime,uSendPhyTime) != 0)
					{
						uSendPhyData[count].executeStatus = eEachDayPhyStatusHandlerStatus_NoExecute;
					}
					else
					{
						uSendPhyData[count].executeStatus = eEachDayPhyStatusHandlerStatus_CannotAccp;
					}
				}
			}
		}
		uSendPhyData[count].eachOtherStatus = i;
		count++;
	}
	return true;
	func_end();
}
//删除好友时对玩家之间的赠送体力的请求信息的处理
void CUser::HandlerSendPhyInfo( const int64_t uFriendId)
{
	func_start();
	/*查找这个好友在内存池中有没有存在，就是好友在没在线的处理*/
	CUser*  userFriend = CDataMgr::Instance()->FindTempUserByUserID(uFriendId);
	if (MT_OK != CDataMgr::Instance()->FindFriends(uFriendId, userFriend->GetFriendMng()))
	{
		LOGERR("CUser::HandlerSendPhyInfo, FindFriends failed ,userid=%lld, friendId=%lld",
			propertys.userid, uFriendId);
		return;
	}

	uint8_t  uCount       = 0;
	for (int i = 0; i < MAX_SEND_PHY_EACH_OTHER_NUM; ++i)
	{
		SendPhysicalData* pData = GetFriendMng().GetSendPhysicalDataByIdx(i);
		if (NULL == pData)
		{
			continue;
		}
		pData->delFriendInfo(uFriendId);
		uCount = pData->currCount;
		if (uCount == 0)
		{

			if (!server.memcached_enable)
			{
			/*更新数据库数据*/
			if (MT_ERR == s_dbproxy.DeleteSendPhyInfo(propertys.userid,i))
			{
				LOGERR("Delete friend SendPhyInfo:delete sendPhy table failed,userid=%lld,Status=%u,friendId=%lld",
					propertys.userid,i,uFriendId);
				return;
			}
			}

		}
		else
		{
			if (!server.memcached_enable)
			{
		/*更新数据库数据*/
			if (MT_ERR == s_dbproxy.UpDateSendPhyInfo(propertys.userid,pData,i))
			{
				LOGERR("Delete friend SendPhyInfo:update sendPhy table failed,userid=%lld,Status=%u,friendId=%lld",
					propertys.userid,i,uFriendId);
				return;
			}
			}
	
		}
		/*好友在线时相应的把玩家的赠送信息删掉*/
		if (userFriend && userFriend->GetLoadFlag()==eUserLoadFlag_Login)
		{
			SendPhysicalData* pFriendData =userFriend->GetFriendMng().GetSendPhysicalDataByIdx(i);
			if (NULL == pFriendData)
			{
				continue;
			}
			pFriendData->delFriendInfo(propertys.userid);
			UPDATE_FRIENDS_DATA(userFriend, userFriend->GetFriendMng());
			uCount = pFriendData->currCount;
			if (uCount == 0)
			{
				if (!server.memcached_enable)
				{
			/*更新数据库数据*/
				if (MT_ERR == s_dbproxy.DeleteSendPhyInfo(userFriend->GetUserID(),i))
				{
					LOGERR("Delete friend SendPhyInfo:delete sendPhy table failed,userid=%lld,Status=%u,friendId=%lld",
						userFriend->GetUserID(),i,propertys.userid);
					return;
				}
				}
	
			}
			else
			{
				if (!server.memcached_enable)
				{
		/*更新数据库数据*/
				if (MT_ERR == s_dbproxy.UpDateSendPhyInfo(userFriend->GetUserID(),pFriendData,i))
				{
					LOGERR("Delete user SendPhyInfo:update sendPhy table failed,userid=%lld,Status=%u,friendId=%lld",
						userFriend->GetUserID(),i,propertys.userid);
					return;
				}
				}
		
			}
		}
	}
	func_end();
}

int CUser::BuyFriendByDiamond()
{
	func_start();
	vip_tab*  pTab = CVipMng::Instance()->GetVipTabInfoByID(GetVipLevel());
	if (NULL == pTab)
	{
		LOGERR("CUser::BuyFriendByDiamond NULL == vip_tab,userid[%lld],viplevel[%d]",GetUserID(),GetVipLevel());
		return -1;
	}
	if ((int32_t)propertys.buy_friend_count >= pTab->BuyFriendMaxTimes)
	{
		LOGERR("CUser::BuyFriendByDiamond, already achieve MaxBuyFriendCount! userid=%lld,buy_friend_count=%u",
			propertys.userid, propertys.buy_friend_count);
		return eBuyFriendUplimitResult_MaxCount;
	}
	buyfriend_tab* pBuyFriendTab = CBuyFriendTabMng::Instance()->GetBuyFriendTabByID(propertys.buy_friend_count);
	if (NULL == pBuyFriendTab)
	{
		LOGERR("CUser::BuyFriendByDiamond NULL == BuyFriendTab,userid[%lld],BuyFriendTabID[%d]",GetUserID(),propertys.buy_friend_count);
		return -1;
	}
	int32_t NeedDiamonds = pBuyFriendTab->needdiamond;
	if (GetRMB() < NeedDiamonds)
	{
		LOGERR("CUser::BuyFriendByDiamond, diamonds not enough! userid=%lld,buy_friend_count=%u,userDiamonds=%d",
			propertys.userid, propertys.buy_friend_count, GetRMB());
		return eBuyFriendUplimitResult_DiamondNotEnough;
	}
	uint32_t  add_friend_num = pBuyFriendTab->addfriendcount;
	if ((GetMaxFriendsCount() + add_friend_num) > MAX_FRIENDS_NUM)
	{
		LOGERR("CUser::BuyFriendByDiamond, achieve max friend Count! userid=%lld,buy_friend_num=%u",
			propertys.userid, add_friend_num);
		return -1;
	}
	//申请 真金操作来源事件ID
	int64_t idOpSourceEvent = CDataMgr::Instance()->CreateOpSourceId((int32_t)RMBOS_SUB_BUYFRIEND) ;
	SubRMBParam_t subRmbParam ;
	//组织消费接口参数
	subRmbParam.source			= RMBOS_SUB_BUYFRIEND ;
	snprintf(subRmbParam.uniqueid, sizeof(subRmbParam.uniqueid), "%lld", idOpSourceEvent) ;
	//subRmbParam.uniqueid		= idOpSourceEvent ;
	subRmbParam.subvalue		= NeedDiamonds;
	snprintf(subRmbParam.desc, sizeof(subRmbParam.desc), "buy friend cost(%d),viplv(%u)",NeedDiamonds,GetVipLevel()) ;
	//备份
	UserRMB_t oldRMB_t = GetRMB_t();
	//扣除钻石
	int iRev = SubRMB(subRmbParam) ;
	if(0 != iRev)
	{
		LOGERR("request buyfriend information failed! CUser::SubRMB() return fail!(%d)userid=%lld,costrmb=%u,userrmb=%d",
			iRev,GetUserID(),NeedDiamonds,GetRMB());
		return -1;
	}
	//日志
	const UserRMB_t& newRMB_t = GetRMB_t();
	LOG_USER_PURCHASE(this, MoneyType_Diamond, subRmbParam.source, 0, add_friend_num, oldRMB_t.m_charge-newRMB_t.m_charge, oldRMB_t.m_donate-newRMB_t.m_donate, oldRMB_t.m_bindrmb-newRMB_t.m_bindrmb);

	propertys.buy_friend_Num += add_friend_num;
	++propertys.buy_friend_count;
	LOGINFO("BuyFriendByDiamond Succeed,userid[%lld],AddFriendNum[%d],CurFriendNum[%d]",
		GetUserID(),add_friend_num,propertys.buy_friend_count);

	if (!server.memcached_enable)
	{
	s_dbproxy.UpDateUserInfo(&propertys);

	}

	return eBuyFriendUplimitResult_Ok;
	func_end();
}

void CUser::RefOnlineFriendId()
{
	func_start();
	ReStartFriendInfo.clear();
	int FriendCount = GetFriendMng().GetCurFriendsCount();
	for(uint8_t i=0; i<FriendCount && i<MAX_FRIENDS_NUM; i++)
	{
		Friend* pFriend = GetFriendMng().GetFriendIdByIdx(i);
		if (NULL == pFriend)
		{
			continue;
		}
		CUser*  user = CDataMgr::Instance()->FindTempUserByUserID(pFriend->friendid);
		if (user == NULL)
		{
			continue;
		}
		ReStartFriendInfo.AddFriend(*pFriend);
	}
	func_end();
}

uint32_t CUser::GetInviteHead( void ) const
{
	return invite_head;
}

uint32_t CUser::GetInviteEnd( void ) const
{
	return invite_end;
}

void CUser::UpdateCurrInviteStatus( Invite_Friends& friendStatus,Invite_Friends& strangerStatus )
{
	func_start();
	/*设置玩家自身好友的当天邀请状态码*/
	time_t now = time(NULL);
	uint32_t hour = 0; 
	uint32_t minute = 0;
	GetServerCdTimeByIndex(eCdTimeIndex_EqCalc, hour, minute);
	if (0 != ComPareTimeWithHM(hour, minute, userEQLastRefTime, now))
	{
		GetInviteCombatMng().ClearInviteUserId();
		userEQLastRefTime = (uint32_t)now;
	}
	uint16_t index_count=0;
	for (uint8_t i=0; i<friendStatus.CurFriedsCount&&i<MAX_FRIENDS_NUM; i++)
	{
		bool bDone = GetInviteCombatMng().FindInviteUserId(friendStatus.friends[i].userid);
		if (bDone)
		{
			friendStatus.friends[i].cur_invite_status = eInviteFriendResultStatus_Done;
		}
		else
		{
			friendStatus.friends[i].cur_invite_status = eInviteFriendResultStatus_UnDone;
		}
		GetInviteCombatMng().AddConfirmUserId(friendStatus.friends[i].userid);
	}
	/*设置玩家当天邀请过陌生人的添加友情点的状态码*/
	for (uint8_t i=0; i<strangerStatus.CurFriedsCount&&i<INVITE_STRANGER_NUM; i++)
	{
		bool bDone = GetInviteCombatMng().FindInviteUserId(strangerStatus.friends[i].userid);
		if (bDone)
		{
			strangerStatus.friends[i].cur_invite_status = eInviteFriendResultStatus_Done;
		}
		else
		{
			strangerStatus.friends[i].cur_invite_status = eInviteFriendResultStatus_UnDone;
		}
		GetInviteCombatMng().AddConfirmUserId(strangerStatus.friends[i].userid);
	}
	func_end();
}

void CUser::AddEQ(uint32_t val)
{
	propertys.EQ += val;
	if(propertys.EQ > (int)CGameConfigMgr::Instance()->GetConfigData().MaxUserGetEq/*server.max_user_get_eq*/)
	{
		propertys.EQ =(int)CGameConfigMgr::Instance()->GetConfigData().MaxUserGetEq/*server.max_user_get_eq*/;
	}
}

void CUser::AddEQ( uint8_t friendStatus,int64_t friendId )
{
	func_start();
	//如果今天已经邀请过助战，不添加友情点
// 	if (0 != GetInviteCombatMng().SetInviteUserId(friendId))
// 	{
// 		LOGINFO("AddEQ, already!userid=%lld,account=%s",propertys.userid,propertys.account.to_string());
// 		return;
// 	}
	uint16_t AddOtherEQ = 0;
	uint16_t AddSelfEQ=0;
	//好友添加友情点
	if (FriendStatue_Friend == friendStatus)
	{
		AddOtherEQ = CGameConfigMgr::Instance()->GetConfigData().ByInvitedFriend;
		AddSelfEQ = CGameConfigMgr::Instance()->GetConfigData().InviteFriend;
	}
	else if (FriendStatue_Stranger == friendStatus || FriendStatue_Robot == friendStatus)//陌生人添加友情点
	{
		AddOtherEQ = CGameConfigMgr::Instance()->GetConfigData().ByInvitedStranger;
		AddSelfEQ = CGameConfigMgr::Instance()->GetConfigData().InviteStranger;
	}
	CUser*  pUser = NULL;
	if (FriendStatue_Stranger == friendStatus || FriendStatue_Friend == friendStatus)
	{
		pUser =  CDataMgr::Instance()->FindTempUserByUserID(friendId);
		if (NULL == pUser)
		{
			LOGERR("MSGERR:AddEQ, InviteFriend object not find! friendId=%lld", friendId);
		}
	}
	//加好友或者陌生人的友情点
	if (pUser)
	{ 
		pUser->AddEQ(AddOtherEQ);
		//判断是否是好友
		if (FriendStatue_Friend == friendStatus/*GetFriendMng().IsFriend(friendId)*/)
		{
			//先加锁，再修改数据
			FriendCombatAward	Award;
			MemLock Lock(eKey_FriendAward_Lock, friendId, true);
			int ret = CDataMgr::Instance()->GetFriendAwardInfo(friendId,Award);
			if (ret == MT_OK)
			{
				Award.CombatAwardCount++;
				if (Award.CombatAwardCount <= CGameConfigMgr::Instance()->GetConfigData().AwardDiamondTimes)
				{
					CDataMgr::Instance()->UpdateFriendAwardInfo(friendId,Award);
				}
			}
		}
		//
		if (MT_ERR == pUser->UpdateUserDataToDB())
		{
			LOGERR("AddInviteFriendEq:update userInfo wrong userid=%lld",pUser->GetUserID());
			return;
		}
		LOGINFO("AddUserEQ:OK! userid=%lld,account=%s,currentEQ=%d,AddEQ=%d",
			pUser->GetUserID(),pUser->GetUserAccountStr(),pUser->GetEQ(),AddOtherEQ);
	}
	//加自己的友情点
	{
		AddEQ(AddSelfEQ);
		if (MT_ERR == UpdateUserDataToDB())
		{
			LOGERR("AddUserFriendEq:update userInfo wrong userid=%lld,account=%s",
				propertys.userid,propertys.account.to_string());
			return;
		}
		LOGINFO("AddUserEQ:OK! userid=%lld,account=%s,currentEQ=%d,AddEQ=%d",
			GetUserID(),GetUserAccountStr(),GetEQ(),AddSelfEQ);
	}
	func_end();
}

void CUser::RefUserPoolIndex( void )
{
	func_start();
	//const uint32_t nUserNumber = s_user_pool.size()
	uint32_t RobotNumber = CArenaRobotTabMgr::Instance()->GetArenaRobotCount();
	uint32_t nUserNumber = CDataMgr::Instance()->GetUserCount() - RobotNumber;
	if (nUserNumber < invite_end + 1)
	{
		invite_head = 0;
		invite_end  = nUserNumber - 1;
	}
	else if (invite_end + 1 == nUserNumber)
	{
		invite_head = 0;
		invite_end  = EACH_REFURBISH_NUM - 1;
	}
	else if (nUserNumber - invite_end - 1>= EACH_REFURBISH_NUM)
	{
		invite_head = invite_head + EACH_REFURBISH_NUM ;
		invite_end  = invite_end + EACH_REFURBISH_NUM ;
	}
	/*下面的规则是当内存池中的用户数量不怎么增加的时候，在一定的条件下，把最后
		没有刷到的用户给补充上，而下次就会再把刷新的下标值从头开始
	*/
	if (nUserNumber > EACH_REFURBISH_NUM && 
		nUserNumber - invite_end - 1 < EACH_REFURBISH_NUM &&
		nUserNumber - EACH_REFURBISH_NUM >= nUserNumber - invite_end - 1)
	{
		if (refurbishNum >= LAST_REFURBISH_NUM)
		{
			if (refPoolStatus == eRefUserPoolIndexStatus_UnDone)
			{
				invite_head = invite_head + nUserNumber - invite_end;
				invite_end  = nUserNumber - 1;
				refPoolStatus = eRefUserPoolIndexStatus_Done ;
			}
			else
			{
				invite_head = 0;
				invite_end  = EACH_REFURBISH_NUM - 1;
				refurbishNum = 0;
				refPoolStatus = eRefUserPoolIndexStatus_UnDone;
			}
		}
		else
		{
			refurbishNum++;
		}
	}
	func_end();
}

void CUser::UpdateInviteCombatTime( int64_t& friendUserId, int8_t& friendStatus)
{
	func_start();
	if (FriendStatue_Robot == friendStatus)
	{
		return;
	}
	if (ISINVALIDGUID(friendUserId))
	{
		return;
	}
	bool bFind = GetInviteCombatMng().FindConfirmUserId(friendUserId);
	if (!bFind)
	{
		//LOGERR("UpdateCombatTime friendId double request! userid=%lld,friendUserId=%lld",propertys.userid,friendUserId);
		friendUserId = -1;
		friendStatus = -1;
		return;
	}
	//
	GetInviteCombatMng().DelConfirmUserId(friendUserId);
	CUser* user = CDataMgr::Instance()->FindTempUserByUserID(friendUserId);
	if (user == NULL)
	{
		LOGERR("UpdateCombatTime friend is not exist! userid=%lld,friendUserId=%lld",propertys.userid,friendUserId);
		friendUserId = -1;
		friendStatus = -1;
		return;
	}
	if (!GetInviteCombatMng().AddInviteCombatTime(friendUserId))
	{
		LOGERR("UpdateCombatTime add combat time fail! userid=%lld,friendUserId=%lld",propertys.userid,friendUserId);
		return;
	}
	func_end();
}

void CUser::RandInviteIndex( void )
{
	uint32_t RobotCounts	= CArenaRobotTabMgr::Instance()->GetArenaRobotCount();
	uint32_t uUserCounts	= CDataMgr::Instance()->GetUserCount() - RobotCounts;
	if (uUserCounts <= 0)	return;
	if (uUserCounts <= EACH_REFURBISH_NUM)
	{
		invite_head = 0;
		invite_end  = uUserCounts - 1;
	}
	else
	{
		// 随机定位区间[n, n + EACH_REFURBISH_NUM - 1]
		//invite_head = rand()%uUserCounts;
		uint32_t uRandMax = uUserCounts;
		uint32_t uRandMin = 0;
		invite_head = (double)rand() / RAND_MAX * ( uRandMax - uRandMin ) + uRandMin;

		if (invite_head + EACH_REFURBISH_NUM > uUserCounts)
		{
			// 一切为了头尾机会公平
			if (invite_head % 2)
			{
				invite_head = uUserCounts - EACH_REFURBISH_NUM;
				invite_end  = uUserCounts - 1;
			}
			else
			{
				invite_head = 0;
				invite_end  = MIN(invite_head + EACH_REFURBISH_NUM - 1, uUserCounts - 1);
			}
		} 
		else
		{
			invite_end  = MIN(invite_head + EACH_REFURBISH_NUM - 1, uUserCounts - 1);
		}
	}
	LOGDEBUG("CUser::RandInviteIndex->uUserCounts = %d :: [invite_head = %d, invite_end = %d]", uUserCounts, invite_head, invite_end);
}

uint16_t CUser::GetMaxFriendsCount()
{
	uint16_t nCnt = CGameConfigMgr::Instance()->GetConfigData().DefaultFriendNum;
	const PlayerTab_t* pPlayerTab = CPlayerTableManager::Instance()->GetData(GetLevel());
	if (pPlayerTab)
	{
		nCnt += (uint16_t)(pPlayerTab->m_FriendMaxNum);
	}

	return nCnt;
}

void CUser::GetAwardState(uint32_t uRefPhyTime, int8_t& freeGoldAwardTimes, uint32_t& freeGoldAwardNextTime, uint32_t& freeDiamondsAwardNextTime, uint32_t& freeGemAwardNextTime)
{
	func_start();
	uint32_t hour = 0; 
	uint32_t minute = 0; 
	GetServerCdTimeByIndex(eCdTimeIndex_GoldAward, hour, minute);
	if(0 == propertys.freegoldrewardtime)
	{
		propertys.byfreegoldrewardcount = 0;
	}
	else if (0 != ComPareTimeWithHM(hour, minute, propertys.freegoldrewardtime, uRefPhyTime))
	{
		propertys.byfreegoldrewardcount = 0;
	}
	freeGoldAwardTimes = propertys.byfreegoldrewardcount ;

	if(0 == freeGoldAwardTimes)
	{
		freeGoldAwardNextTime = 0 ;
	}
	else if((freeGoldAwardTimes > 0) && (freeGoldAwardTimes < CGameConfigMgr::Instance()->GetConfigData().gold_award_freetimes))
	{
		uint32_t nScalarTime = CGameConfigMgr::Instance()->GetConfigData().gold_award_freecd * 60 ;	// 免费金币抽奖冷却时间(单位:分钟)
		uint32_t nDiffTime = (uint32_t)difftime(uRefPhyTime, propertys.freegoldrewardtime);
		if(nDiffTime >= nScalarTime)
		{
			freeGoldAwardNextTime = 0 ;
		}
		else
		{
			freeGoldAwardNextTime = nScalarTime - nDiffTime ;
		}
	}
	if(0 == propertys.freediamondrewardtime)
	{
		freeDiamondsAwardNextTime = 0;
	}
	else
	{
		uint32_t nScalarTime = GetDiamondAwardCd();
		uint32_t nDiffTime = (uint32_t)difftime(uRefPhyTime, propertys.freediamondrewardtime);
		if(nDiffTime >= nScalarTime)
		{
			freeDiamondsAwardNextTime = 0 ;
		}
		else
		{
			freeDiamondsAwardNextTime = nScalarTime - nDiffTime ;
		}
	}
	if(0 == propertys.freegemrewardtime)
	{
		freeGemAwardNextTime = 0;
	}
	else
	{
		uint32_t nScalarTime = GetGemAwardCd();
		uint32_t nDiffTime = (uint32_t)difftime(uRefPhyTime, propertys.freegemrewardtime);
		if(nDiffTime >= nScalarTime)
		{
			freeGemAwardNextTime = 0 ;
		}
		else
		{
			freeGemAwardNextTime = nScalarTime - nDiffTime ;
		}
	}
	func_end();
}

uint32_t CUser::GetDiamondAwardCd()
{
	func_start();
	
	const static size_t nFreeCdCnt = CGameConfigMgr::Instance()->GetConfigData().LotteryDiamondFreeCds.size();	// 免费钻石抽奖冷却时间(单位:分钟)

	int nCnt = GetFreeDiamondLottery();
	if (nCnt > nFreeCdCnt)
	{
		nCnt = nFreeCdCnt; 
	}

	int idx = 0;
	if (nCnt > 0)
	{
		idx = nCnt - 1;
	}

	return CGameConfigMgr::Instance()->GetConfigData().LotteryDiamondFreeCds[idx] * 60;
	func_end();
}

uint32_t CUser::GetGemAwardCd()
{
	func_start();

	const static size_t nFreeCdCnt = CGameConfigMgr::Instance()->GetConfigData().LotteryGemFreeCds.size();	// 免费符石抽奖冷却时间(单位:分钟)

	int nCnt = GetFreeGemLottery();
	if (nCnt > nFreeCdCnt)
	{
		nCnt = nFreeCdCnt; 
	}

	int idx = 0;
	if (nCnt > 0)
	{
		idx = nCnt - 1;
	}

	return CGameConfigMgr::Instance()->GetConfigData().LotteryGemFreeCds[idx] * 60;
	func_end();
}

int CUser::GetAward(int8_t flag, int8_t mode, uint8_t& GoodsCount, AwardGood* info, int8_t& freeGoldAwardTimes, uint32_t& freeGoldAwardNextTime, uint32_t& freeDiamondsAwardNextTime, uint32_t& freeGemAwardNextTime)
{
	func_start();
	if (NULL == info)
	{
		LOGERR("CUser::GetAward(), param error! AwardGood==NULL! userid=%lld,account=%s",propertys.userid, propertys.account.to_string());
		return eGetAwardResult_DateErr;
	}

	if (AwardMode_One == mode)
	{
		GoodsCount = 1;
	}
	else if (AwardMode_Ten == mode)
	{
		GoodsCount = MAX_AWARD_GOODS_NUM;
	}
	else
	{
		LOGERR("CUser::GetAward(), param error! mode==%d,userid=%lld,account=%s", mode, propertys.userid, propertys.account.to_string());
		return eGetAwardResult_DateErr;
	}

	int32_t Cost = 0;
	//获得抽奖状态
	uint32_t uCurrTime = (uint32_t)time(NULL);
	GetAwardState(uCurrTime, freeGoldAwardTimes, freeGoldAwardNextTime, freeDiamondsAwardNextTime, freeGemAwardNextTime) ;

	//当前抽奖次数
	int32_t RewardCount = 0; 
	int32_t uPurpleCardNum = 0;
	bool isFree = false ;
	if (AwardType_Gold == flag) 
	{
		//金币抽奖
		//Cost = GoodsCount * CGameConfigMgr::Instance()->GetConfigData().gold_per_get_award;
		Cost = ((AwardMode_One == mode) ? CGameConfigMgr::Instance()->GetConfigData().gold_per_get_award : CGameConfigMgr::Instance()->GetConfigData().gold_per_ten_award);
		if((AwardMode_One == mode)
			&& (freeGoldAwardTimes < CGameConfigMgr::Instance()->GetConfigData().gold_award_freetimes)
			&& (0 == freeGoldAwardNextTime))
		{
			Cost = 0 ;
			isFree = true ;
		}
		int32_t nGolds = GetGolds() ;
		if (nGolds < Cost)
		{
			LOGERR("CUser::GetAward,user_gold[%u] < NeedGold[%d]!userid=%lld,mode=%d",
				nGolds, Cost, propertys.userid, mode);
			return eGetAwardResult_GoldNotEnough;
		}
		RewardCount = propertys.eq_reward_count;
	}
	else if (AwardType_Diamond == flag)
	{
		//钻石抽奖
		Cost = ((AwardMode_One == mode) ? CGameConfigMgr::Instance()->GetConfigData().diamond_per_get_award : CGameConfigMgr::Instance()->GetConfigData().diamond_per_ten_award);
		if((AwardMode_One == mode) && (0 == freeDiamondsAwardNextTime))
		{
			Cost = 0 ;
			isFree = true ;
		}
	
		////判断是否有免费十连抽的资格
		//if (AwardMode_Ten == mode && eNowCanFreeLottery == GetDiamondFreeTenLottery())
		//{
		//	GetUserData().buy_diamonds = 1;
		//	Cost = 0 ;
		//}

		int32_t nRmb = GetRMB() ;
		if (nRmb < Cost)
		{
			LOGERR("CUser::GetAward,user_diamonds[%d] < NeedDiamond[%d]!userid=%lld,mode=%d",
				nRmb, Cost, propertys.userid, mode);
			return eGetAwardResult_DiamondNotEnough;
		}
		RewardCount = propertys.reward_count;
	}
	else if (AwardType_Gems == flag)
	{		
		// 符石抽奖
		Cost = ((AwardMode_One == mode) ? CGameConfigMgr::Instance()->GetConfigData().gem_per_get_award : CGameConfigMgr::Instance()->GetConfigData().gem_per_ten_award);
		if((AwardMode_One == mode) && (0 == freeGemAwardNextTime))
		{
			Cost = 0 ;
			isFree = true ;
		}

		int32_t nRmb = GetRMB() ;
		if (nRmb < Cost)
		{
			LOGERR("CUser::GetAward,user_diamonds[%d] < NeedDiamond[%d]!userid=%lld,mode=%d",
				nRmb, Cost, propertys.userid, mode);
			return eGetAwardResult_DiamondNotEnough;
		}
		RewardCount = propertys.gem_reward_count;
	}
	else
	{
		LOGERR("CUser::GetAward(), param error! flag==%d,userid=%lld,account=%s", flag, propertys.userid, propertys.account.to_string());
		return eGetAwardResult_DateErr;
	}

	std::list<award_item> _awarditems ;
	for (int i = 0; i < GoodsCount && i < MAX_AWARD_GOODS_NUM; ++i)
	{
		int tempRewardCount = 0;
		if (0 == RewardCount)
			tempRewardCount = FIRST_AWARD_VALUE;
		else
			tempRewardCount = RewardCount;

		if (RewardCount>99)
			tempRewardCount =RewardCount%100;

		int32_t DropGroupID = CAwardTable::Instance()->GetAwardDropGroup(flag, tempRewardCount);
		if (DropGroupID < 0)
		{
			LOGERR("CUser::GetAward(),CAwardTable::GetAward() return fail!userid=%lld,mode=%d,flag=%d", propertys.userid,mode, flag);
			return eGetAwardResult_DateErr;
		}
		
		LOGINFO("===========    before check DropGroupID = %d",DropGroupID);
		if (AwardType_Diamond == flag)
		{
			if (Cost >= 0)
			{
				if (AwardMode_One == mode)
				{
					if(!CheckUserMask(eMask_LotteryFirstDiamonds))
					{
						//首次付费钻石单抽奖励，替换掉落组ID
						DropGroupID = (Cost > 0)?CGameConfigMgr::Instance()->GetConfigData().LotteryFirstDiamondsDrop:DropGroupID;
						(Cost > 0)?SetUserMask(eMask_LotteryFirstDiamonds):void();
					}
					if(Cost == 0)
					{
						//如果是免费钻石单次抽奖,更新下计数
						SetFreeDiamondLottery(GetFreeDiamondLottery() + 1);
					}
				}
				else if (AwardMode_Ten == mode && !CheckUserMask(eMask_LotteryFirstMuchDiamonds) && i == 1)
				{
					// 首次付费钻石10连抽奖励,替换第二次抽卡掉落组ID
					DropGroupID = (Cost > 0)?CGameConfigMgr::Instance()->GetConfigData().LotteryFirstMuchDiamondsDrop:DropGroupID;
					(Cost > 0)?SetUserMask(eMask_LotteryFirstMuchDiamonds):void();
				}
			}
#if 0		//注销掉前6次免费抽走单独掉落ID的逻辑
			else if (Cost == 0)
			{
				if (AwardMode_One == mode)
				{
					//如果是免费钻石单次抽奖,更新下计数
					SetFreeDiamondLottery(GetFreeDiamondLottery() + 1);

					if (GetFreeDiamondLottery() > 0 && GetFreeDiamondLottery() <= CGameConfigMgr::Instance()->GetConfigData().LotteryDiamondFreeDrops.size())
					{
						DropGroupID = CGameConfigMgr::Instance()->GetConfigData().LotteryDiamondFreeDrops[GetFreeDiamondLottery() - 1];
					}
				}
			}
#endif
		}
		else if (AwardType_Gems == flag)
		{
			if (Cost >= 0)
			{
				if (AwardMode_One == mode)
				{					
					if(!CheckUserMask(eMask_LotteryFirstGem))
					{
						//首次付费符石单抽奖励，替换掉落组ID
						DropGroupID = (Cost > 0)?CGameConfigMgr::Instance()->GetConfigData().LotteryFirstGemsDrop:DropGroupID;

						(Cost > 0)?SetUserMask(eMask_LotteryFirstGem):void();
					}
					if(Cost == 0)
					{
						//如果是免费符石单次抽奖,更新下计数
						SetFreeGemLottery(GetFreeGemLottery() + 1);
					}
				}
				else if (AwardMode_Ten == mode && !CheckUserMask(eMask_LotteryFirstMuchGem) && i == 1)
				{
					// 首次付费符石10连抽奖励,替换第二次抽卡掉落组ID
					DropGroupID = (Cost > 0)?CGameConfigMgr::Instance()->GetConfigData().LotteryFirstMuchGemsDrop:DropGroupID;
					(Cost > 0)?SetUserMask(eMask_LotteryFirstMuchGem):void();
				}
			}
		}

		LOGINFO("===========    after check DropGroupID = %d",DropGroupID);

		std::list<award_item> _tempawarditems ;
		int iRev = COutPutAward::OutPutCalculate(DropGroupID, propertys.userid, eOutPutModule_Reward, _tempawarditems) ;
		if(eOutPutCalculateResult_Ok != iRev)
		{
			LOGERR("CUser::GetAward(),COutPutAward::OutPutCalculate() return fail!userid=%lld,mode=%d,flag=%d,DropGroupID=%d", propertys.userid,mode, flag,DropGroupID);
			return eGetAwardResult_DateErr;
		}

		if (_tempawarditems.empty())
		{
			LOGERR("CUser::GetAward(),treasure info not found!userid=%lld,mode=%d,flag=%d,DropGroupID=%d", propertys.userid,mode, flag,DropGroupID);
			return eGetAwardResult_DateErr;
		}

		const award_item& awarditem = _tempawarditems.front();
		info[i].type = awarditem.type;
		if (eOutPutAwardType_Hero == awarditem.type )
		{
			info[i].template_id = awarditem.itemid;

			if (getHeroById(info[i].template_id))
			{
				info[i].Param = 1;
			}
			else
			{
				info[i].Param = 0;
				//检查是不是随机到2个相同的英雄
				for(int j=0; j<i ; ++j)
				{
					if( info[i].template_id==info[j].template_id && 
						info[i].type==info[j].type )
					{
						info[i].Param = 1;
					}
				}
			}			
	 
		}
		else if (eOutPutAwardType_Item == awarditem.type )
		{
			info[i].template_id = awarditem.itemid;
			info[i].Param = awarditem.param;
		}
		else if (eOutPutAwardType_Gem == awarditem.type)
		{
			info[i].template_id = awarditem.itemid;
			info[i].Param = awarditem.param;
			memcpy(info[i].gemattid,awarditem.gematt,sizeof(int32_t)*(MAX_GEM_MAIN_ATT_COUNT+MAX_GEM_ADDITIONAL_ATT_COUNT));			
		}
		else if (eOutPutAwardType_EvilDiamond == awarditem.type)
		{
			info[i].template_id = awarditem.itemid;
			info[i].Param = awarditem.param;
		}
		else
		{
			LOGERR("CUser::GetAward(),award type err!userid=%lld,mode=%d,flag=%d,DropGroupID=%d", propertys.userid,mode, flag,DropGroupID);
			return eGetAwardResult_DateErr;
		}
		
		_awarditems.push_back(awarditem);

#if		1	//抽卡奖励经验药水 HERO-4587  【抽卡】抽卡需求奖励经验药水 
		//额外奖励
		int32_t DropExtendID = CAwardTable::Instance()->GetAwardDropExtend(flag, tempRewardCount);
		if (DropExtendID >= 0)
		{
			std::list<award_item> _extawarditems ;
			COutPutAward::OutPutCalculate(DropExtendID, propertys.userid, eOutPutModule_Reward, _extawarditems) ;
			if(_extawarditems.size() > 0)
			{
				iRev = COutPutAward::UserAddAwards(*this, eOutPutModule_Reward, _extawarditems) ;
				if(eUserAddAwardsResult_Ok != iRev)
				{
					LOGERR("CUser::GetAward(),extend COutPutAward::UserAddAward()return failed(%d)!userid=%lld,mode=%d,flag=%d",iRev, propertys.userid,mode, flag);
				}
			}
		}
#endif

		RewardCount++;
	}

	//先 扣除消耗
	int64_t idOpSourceEvent = -1 ;
	char _uniqueid[MRMBP_UNIQUEID_SIZE] = {0} ; 
	SubRMBParam_t subRmbParam ;
	int iRev = -1 ;
	if (AwardType_Gold == flag)
	{
		SubGolds(Cost) ;
		propertys.eq_reward_count += GoodsCount;
		log_get_award(false, propertys.userid, flag, mode, -1, -1, 0, -1, 0, Cost, GetGolds(), propertys.eq_reward_count, 0);
		LOG_USER_PURCHASE(this, MoneyType_Gold, GOLDOS_SUB_GET_AWARD, //抽卡消耗
				0, 1, Cost, 0, 0);
	}
	else if (AwardType_Diamond == flag)
	{

		//修改属性数据
		propertys.reward_count += GoodsCount;

		if(Cost > 0)
		{
			//申请 真金操作来源事件ID
			idOpSourceEvent = CDataMgr::Instance()->CreateOpSourceId((int32_t)RMBOS_SUB_BUY_HERO) ;
			snprintf(_uniqueid, sizeof(_uniqueid), "%lld", idOpSourceEvent) ;

			//组织消费接口参数
			subRmbParam.source			= RMBOS_SUB_BUY_HERO ;
			snprintf(subRmbParam.uniqueid, sizeof(subRmbParam.uniqueid), "%lld", idOpSourceEvent) ;
			//subRmbParam.uniqueid		= idOpSourceEvent ;
			subRmbParam.subvalue		= Cost ;
			snprintf(subRmbParam.desc, sizeof(subRmbParam.desc), "diamond reward system cost") ;
			//备份
			UserRMB_t oldRMB_t = GetRMB_t();
			//扣除钻石
			iRev = SubRMB(subRmbParam) ;
			if(0 != iRev)
			{
				LOGERR("CUser::GetAward(),reward sub rmb failed! CUser::SubRMB() return fail!(%d)userid=%lld,costrmb=%u,userrmb=%d,rewardmode=%d,rewardflag=%d",
					iRev, GetUserID(), Cost, GetRMB(), mode, flag);
				return eGetAwardResult_DateErr ;
			}
			const UserRMB_t& newRMB_t = GetRMB_t();
			LOG_USER_PURCHASE(this, MoneyType_Diamond, subRmbParam.source, 0, 1, oldRMB_t.m_charge-newRMB_t.m_charge, oldRMB_t.m_donate-newRMB_t.m_donate, oldRMB_t.m_bindrmb-newRMB_t.m_bindrmb);
		}
	}
	else if (AwardType_Gems == flag)
	{
		if((GetBag()->GetGemCount()+GoodsCount) > MAX_GEM_IN_BAG_COUNT)
		{
			LOGERR("CUser::GetAward(),Failed For Too Many Gems In Bag!userid=%lld,mode=%d,flag=%d",iRev, propertys.userid,mode, flag);
			return eGetAwardResult_TooManyGems ;
		}
		//修改属性数据
		propertys.gem_reward_count += GoodsCount;

		if(Cost > 0)
		{
			//申请 真金操作来源事件ID
			idOpSourceEvent = CDataMgr::Instance()->CreateOpSourceId((int32_t)RMBOS_SUB_BUY_HERO) ;
			snprintf(_uniqueid, sizeof(_uniqueid), "%lld", idOpSourceEvent) ;

			//组织消费接口参数
			subRmbParam.source			= RMBOS_SUB_BUY_HERO ;
			snprintf(subRmbParam.uniqueid, sizeof(subRmbParam.uniqueid), "%lld", idOpSourceEvent) ;
			//subRmbParam.uniqueid		= idOpSourceEvent ;
			subRmbParam.subvalue		= Cost ;
			snprintf(subRmbParam.desc, sizeof(subRmbParam.desc), "gem reward system cost") ;
			//备份
			UserRMB_t oldRMB_t = GetRMB_t();
			//扣除钻石
			iRev = SubRMB(subRmbParam) ;
			if(0 != iRev)
			{
				LOGERR("CUser::GetAward(),reward sub rmb failed! CUser::SubRMB() return fail!(%d)userid=%lld,costrmb=%u,userrmb=%d,rewardmode=%d,rewardflag=%d",
					iRev, GetUserID(), Cost, GetRMB(), mode, flag);
				return eGetAwardResult_DateErr ;
			}
			const UserRMB_t& newRMB_t = GetRMB_t();
			LOG_USER_PURCHASE(this, MoneyType_Diamond, subRmbParam.source, 0, 1, oldRMB_t.m_charge-newRMB_t.m_charge, oldRMB_t.m_donate-newRMB_t.m_donate, oldRMB_t.m_bindrmb-newRMB_t.m_bindrmb);
		}
	}

	iRev = COutPutAward::UserAddAwards(*this, eOutPutModule_Reward, _awarditems) ;
	if(eUserAddAwardsResult_Ok != iRev)
	{
		LOGERR("CUser::GetAward(),COutPutAward::UserAddAward()return failed(%d)!userid=%lld,mode=%d,flag=%d",iRev, propertys.userid,mode, flag);
 		return eGetAwardResult_DateErr ;
	}
	
	//更新抽奖状态数据
	if(isFree)
	{
		if (AwardType_Gold == flag)
		{
			propertys.byfreegoldrewardcount++ ;
			propertys.freegoldrewardtime = uCurrTime ;

			freeGoldAwardTimes = propertys.byfreegoldrewardcount ;
			freeGoldAwardNextTime = CGameConfigMgr::Instance()->GetConfigData().gold_award_freecd * 60 ;
		}
		else if(AwardType_Diamond == flag)
		{
			propertys.freediamondrewardtime = uCurrTime;
			freeDiamondsAwardNextTime = GetDiamondAwardCd();
		}
		else if(AwardType_Gems == flag)
		{
			propertys.freegemrewardtime = uCurrTime;
			freeGemAwardNextTime = GetGemAwardCd();
		}
	}
	if (!server.memcached_enable)
	{
	s_dbproxy.UpDateUserInfo(&propertys);

	}

	return eGetAwardResult_Ok;
	func_end();
}

void CUser::ClearPayment(void)
{
	func_start();
#if defined(PAY_PLATFORM) && (PAY_PLATFORM==BUSINESS_PLATFORM_APPLE)
	m_PaymentOrderInfoAppStore.clear();	// iOS支付订单信息
#endif
	func_end();
}
//出售道具
ItemOperateResult CUser::SellItem( int16_t id,int16_t num )
{
	if (num <= 0)
	{
		LOGERR("CUser::SellItem Fail,userid[%lld] id[%d],num[%d] <= 0,",GetUserID(),id,num);
		return ItemOperateResult_Fail; 
	}
	//道具是否存在
	Item* pItem = GetBag()->FindItem(id);
	if (NULL == pItem)
	{
		LOGERR("CUser::SellItem Fail,not find item id[%d],userid[%lld]",id,GetUserID());
		return ItemOperateResult_NotFindItem;
	}
	//是否可以出售
	if (!pItem->IsCanSell())
	{
		LOGERR("CUser::SellItem Fail,not can sell id[%d],userid[%lld]",id,GetUserID());
		return ItemOperateResult_NotSell;
	}
	//所卖道具数量是否够
	if (pItem->GetNum() < num)
	{
		LOGERR("CUser::SellItem Fail,curnum[%d] is low num[%d]id[%d],userid[%lld]",pItem->GetNum(),num,id,GetUserID());
		return ItemOperateResult_NumLow;
	}

	int32_t price = pItem->GetSellPrice();
	int32_t totalprice = pItem->GetSellPrice()*num;

	//
	if (!GetBag()->SubItem(id,num))
	{
		LOGERR("CUser::SellItem Fail,Itemid[%d],Num[%d],userid[%lld]",id,num,GetUserID());
		return ItemOperateResult_Fail;
	}
	//
	AddGolds(totalprice);
	LOG_USER_REWARD(this, MoneyType_Gold, totalprice, 0, 0, GOLDOS_ADD_Sell_Item);

	UpdateUserDataToDB();
	//更新背包数据
	UpdateBagInfoToDB();
	LOGINFO("CUser::SellItem Ok,userid[%lld] id[%d],GetGolds[%d],num[%d],price[%d]",GetUserID(),id,totalprice,num,price);
	return ItemOperateResult_Ok;
}

//购买道具
ItemOperateResult CUser::BuyItem(int16_t itemid,int16_t itemnum)
{
	//检查购买数量
	if (itemnum <= 0)
	{
		LOGERR("CUser::BuyItem Fail,userid[%lld] id[%d],num[%d] <= 0,",GetUserID(),itemid,itemnum);
		return ItemOperateResult_Fail; 
	}

	//检查道具购买表
	itembuy_tab*  tabItemBuy = CItemBuyTabMng::Instance()->GetData(itemid);
	if (!tabItemBuy)
	{
		LOGERR("CUser::BuyItem not find itemid in itembuytab ItemId:%d", itemid);
		return ItemOperateResult_Fail;
	}

	//判断金币类型
	int moneytype = tabItemBuy->moneytype;
	if (moneytype <= eOutPutAwardType_Untype || moneytype >= eOutPutAwardType_Max)
	{
		LOGERR("CUser::BuyItem, invalid moneytype moneytype:%d", moneytype);
		return ItemOperateResult_Fail;
	}

	//获得当前货币
	int CurMoney = -1;
	switch(moneytype)
	{
	case eOutPutAwardType_Diamond:		//钻石
		CurMoney = GetRMB();
		break;

	case eOutPutAwardType_Golds:		//金币 
		CurMoney = GetGolds();
		break;
	case eOutPutAwardType_ArenaPoint:	//勋章
	case eOutPutAwardType_StarPoint:	//星魂
	case eOutPutAwardType_EndLessPoint:	//晶石
		break;

	default:
		break;
	}

	//判断金币是否足够
	int totalCost = itemnum*tabItemBuy->price;	//总消耗
	if (CurMoney < totalCost)
	{
		LOGERR("CUser::BuyItem, money not enough CurMoney:%d CostMoney:%d", CurMoney, totalCost);
		return ItemOperateResult_GoldsLow;
	}

	switch(moneytype)
	{
	case eOutPutAwardType_Diamond:		//钻石
		{
			//申请 真金操作来源事件ID
			int64_t idOpSourceEvent = CDataMgr::Instance()->CreateOpSourceId((int32_t)RMBOS_SUB_ITEMBUY) ;

			//组织消费接口参数
			SubRMBParam_t subRmbParam ;
			subRmbParam.source			= RMBOS_SUB_ITEMBUY ;
			snprintf(subRmbParam.uniqueid, sizeof(subRmbParam.uniqueid), "%lld", idOpSourceEvent) ;
			subRmbParam.subvalue		= totalCost ;
			snprintf(subRmbParam.desc, sizeof(subRmbParam.desc), "user buy item cost") ;

			//备份
			UserRMB_t oldRMB_t = GetRMB_t();

			//扣除钻石
			int iRev = SubRMB(subRmbParam) ;
			if(0 != iRev)
			{
				LOGERR("CUser::BuyItem, sub rmb failed! CUser::SubRMB() return fail!(%d)userid=%lld,costrmb=%u,userrmb=%d",
					iRev, this->GetUserID(), totalCost, this->GetRMB());
				return ItemOperateResult_GoldsLow ;
			}
			//日志
			const UserRMB_t& newRMB_t = GetRMB_t();
			LOG_USER_PURCHASE(this, MoneyType_Diamond, subRmbParam.source, 0, 1, oldRMB_t.m_charge-newRMB_t.m_charge, oldRMB_t.m_donate-newRMB_t.m_donate, oldRMB_t.m_bindrmb-newRMB_t.m_bindrmb);
		}
		break;
	case eOutPutAwardType_Golds:		//金币 
		{
			SetGolds(GetGolds() - totalCost);
			LOG_USER_PURCHASE(this, MoneyType_Gold, GOLDOS_SUB_ITEMBUY, 0, 1, totalCost, 0, 0);
		}
		break;

	case eOutPutAwardType_ArenaPoint:		//勋章
	case eOutPutAwardType_StarPoint:		//星魂
	case eOutPutAwardType_EndLessPoint:		//晶石
		break;
	default:
		break;
	}

	//判断传入的道具ID
	if (!GetBag()->AddItem(itemid,itemnum))
	{
		LOGERR("CUser::BuyItem, add item failed!userid=%lld, itemid=%d,itemnum=%d",GetUserID(),itemid,itemnum);
		return ItemOperateResult_Fail ;
	}
	if (!UpdateBagInfoToDB())
	{
		LOGERR("CUser::BuyItem, UpdateBagInfoToDB failed!userid=%lld, itemid=%d,itemnum=%d",GetUserID(),itemid,itemnum);
		return ItemOperateResult_Fail ;
	}

	UpdateUserDataToDB();
	//更新背包数据
	UpdateBagInfoToDB();
	LOGINFO("CUser::BuyItem Ok,userid[%lld] id[%d], num[%d]",GetUserID(),itemid, itemnum);
	return ItemOperateResult_Ok;
}

/*
	 使用道具
*/
ItemOperateResult CUser::UseItem( int16_t id ,int16_t heroid,AckUseItem* pPacket)
{
	if (NULL == pPacket)
	{
		return ItemOperateResult_Fail;
	}
	//装备不走这个接口
	if (GetBag()->IsEquipment(id))
	{
		LOGERR("CUser::UseItem Fail,this item[%d] is equip,userid[%lld]",id,GetUserID());
		return ItemOperateResult_Fail;
	}
	//道具是否存在
	Item* pItem = GetBag()->FindItem(id);
	if (NULL == pItem)
	{
		LOGERR("CUser::UseItem Fail,not find item id[%d],userid[%lld]",id,GetUserID());
		return ItemOperateResult_NotFindItem;
	}
	//能不能使用
	if (!pItem->IsCanUse())
	{
		LOGERR("CUser::UseItem Fail,can not use item id[%d],userid[%lld]",id,GetUserID());
		return ItemOperateResult_NotUse;
	}
	item_tab* pItemTab = pItem->GetItemTab();
	if (NULL == pItemTab)
	{
		return ItemOperateResult_Fail;
	}
	switch(pItemTab->type)
	{
	case ItemType_GetPhysicalStrength:
		{
			uint16_t CurPS = GetCurrPhysicalStrength();
			SetPhysicalStrength(CurPS + pItemTab->param1);
			UpdateUserDataToDB();
			uint32_t refPhyTime = (uint32_t)time(NULL);
			uint16_t uCurrPhy = 0;
			uint32_t uRemainTime = 0;
			GetPhysicalStrength(refPhyTime, uCurrPhy, uRemainTime);
			pPacket->param1 = uCurrPhy;
			pPacket->param2 = uRemainTime;
			break;
		}
	case ItemType_GemBax:
		{
			std::list<award_item> _awarditems;
			int iRev = COutPutAward::OutPutCalculate(pItemTab->param1, GetUserID(), eOutPutModule_GemBox, _awarditems);
			if(eOutPutCalculateResult_Ok != iRev)
			{
				LOGERR("CUser::UseItem, COutPutAward::OutPutCalculate() return fail! rev=%d, userId=%lld, itemIdx=%d",iRev, GetUserID(), id);
				return ItemOperateResult_Fail;
			}

			iRev = COutPutAward::UserAddAwards(*this, eOutPutModule_GemBox, _awarditems);
			if(eUserAddAwardsResult_Ok != iRev)
			{
				LOGERR("CUser::UseItem, COutPutAward::UserAddAwards() return fail! rev=%d, userId=%lld, idx = %d",iRev, GetUserID(), id);
				return ItemOperateResult_Fail;
			}

			if (_awarditems.size() >= MAX_BOX_ITEMS)
			{
				LOGERR("CUser::UseItem, drop item num[%u] >= MAX_BOX_ITEMS[%d] return fail! userId=%lld, idx = %d",
					_awarditems.size(),MAX_BOX_ITEMS, GetUserID(), id);
				return ItemOperateResult_Fail;
			}

			pPacket->cnt = _awarditems.size();
			std::list<award_item>::iterator it = _awarditems.begin();
			for (int idx = 0; idx < MAX_BOX_ITEMS && it != _awarditems.end(); ++it, ++idx)
			{
				pPacket->boxitems[idx] = *it;
			}

			break;
		}

	default: 
		{
			LOGERR("CUser::UseItem Fail,Item[%d] Type Error,userid[%lld]",id,GetUserID());
			return ItemOperateResult_Fail;
		}
		break;
	}
	//减去使用的道具
	GetBag()->SubItem(id);
	UpdateBagInfoToDB();
	return ItemOperateResult_Ok;
}
/*
	装备合成
*/
ItemOperateResult CUser::EquipCompose( int16_t equipid ,vector<int32_t>& materialids)
{
	//对合成的装备进行判断
	if (!GetBag()->IsEquipment(equipid))
	{
		LOGERR("CUser::EquipCompose Fail,equip[%d] is not Equip,userid[%lld]",equipid,GetUserID());
		return ItemOperateResult_NotEquip;
	}
	//合成的装备信息
	equipment_tab* pEquipTab = CEquipmentTabMng::Instance()->GetEquipmentTabInfoByID(equipid);
	if(NULL==pEquipTab)
	{
		LOGERR("CUser::EquipCompose Fail,equipid[%d] not in CEquipmentTabMng,userid[%lld]",equipid,GetUserID());
		return ItemOperateResult_NotInTab;
	}
	//碎片合成装备
	/*
	if (pEquipTab->chipIDForCompose != 0)
	{
		//对装备碎片的判断
		Item* pEquipChip = GetBag()->FindItem(pEquipTab->chipIDForCompose);
		if (NULL == pEquipChip)
		{
			LOGERR("CUser::EquipCompose Fail,not find item chipid[%d],userid[%lld]",pEquipTab->chipIDForCompose,GetUserID());
			return ItemOperateResult_NotFindItem;
		}
		item_tab* pItemTab = pEquipChip->GetItemTab();
		if (NULL == pItemTab)
		{
			LOGERR("CUser::EquipCompose Fail,pItemTab is null chipid[%d],userid[%lld]",pEquipTab->chipIDForCompose,GetUserID());
			return ItemOperateResult_Fail;
		}
		if (pItemTab->type != ItemType_EquipChip)
		{
			LOGERR("CUser::EquipCompose Fail,type is not equip chaip,chipid[%d],userid[%lld]",pEquipTab->chipIDForCompose,GetUserID());
			return ItemOperateResult_EquipChip;
		}
		//碎片的数量是否足够
		if (pEquipChip->GetNum() < pEquipTab->composeneednum)
		{
			LOGERR("CUser::EquipCompose Fail,chipnum[%d] is low num[%d],userid[%lld],Chipid[%d],EquipID[%d]",
				pEquipChip->GetNum(),pEquipTab->composeneednum,GetUserID(),pEquipTab->chipIDForCompose,equipid);
			return ItemOperateResult_NumLow;
		}
		//对金币进行判断
		int32_t curGolds = GetGolds();
		if (curGolds < pEquipTab->composeneedglod)
		{
			LOGERR("CUser::EquipCompose Fail,curgolds[%d] is low needgolds[%d],userid[%lld],EquipID[%d],chipid[%d]",
				curGolds,pEquipTab->composeneedglod,GetUserID(),equipid,pEquipTab->chipIDForCompose);
			return ItemOperateResult_GoldsLow;
		}
		//删除消耗碎片
		GetBag()->SubItem(pEquipTab->chipIDForCompose,pEquipTab->composeneednum);
		materialids.push_back(pEquipTab->chipIDForCompose);
	}
	else*/
	{
		//装备合成装备
		if (pEquipTab->equipIDForComposeNum == 0)
		{
			LOGERR("CUser::EquipCompose Fail,compose equip num is 0,userid[%lld]",GetUserID());
			return ItemOperateResult_Fail;
		}
		for (int idx =0; idx<MaxEquipNumForCompose;idx++)
		{
			if (pEquipTab->equipIDForCompose[idx].itemID > 0)
			{
				//对装备的判断
				Item* pEquipChip = GetBag()->FindItem(pEquipTab->equipIDForCompose[idx].itemID);
				if (NULL == pEquipChip)
				{
					LOGERR("CUser::EquipCompose Fail,not find equip id[%d],userid[%lld]",pEquipTab->equipIDForCompose[idx].itemID,GetUserID());
					return ItemOperateResult_NotFindItem;
				}
			}
		}
		//对金币进行判断
		int32_t curGolds = GetGolds();
		if (curGolds < pEquipTab->composeneedglod)
		{
			LOGERR("CUser::EquipCompose Fail,curgolds[%d] is low needgolds[%d],userid[%lld],EquipID[%d]",
				curGolds,pEquipTab->composeneedglod,GetUserID(),equipid);
			return ItemOperateResult_GoldsLow;
		}
		for (int idx =0; idx<MaxEquipNumForCompose;idx++)
		{
			if (pEquipTab->equipIDForCompose[idx].itemID > 0)
			{
				//合成装备后，删除道具
				GetBag()->SubItem((int16_t)pEquipTab->equipIDForCompose[idx].itemID, pEquipTab->equipIDForCompose[idx].itemNum);
				materialids.push_back(pEquipTab->equipIDForCompose[idx].itemID);
			}
		}
	}
	//成功后
	SubGolds(pEquipTab->composeneedglod);
	UpdateUserDataToDB();
	//添加合成的装备
	GetBag()->AddItem(equipid);
	UpdateBagInfoToDB();
	LOGINFO("CUser::EquipCompose Ok,userid[%lld],NewEquip[%d],NeedGolds[%d],NeedChipsNum[%d]",
		GetUserID(),equipid,pEquipTab->composeneedglod,pEquipTab->composeneednum);
	LOG_USER_PURCHASE(this, MoneyType_Gold, GOLDOS_SUB_EQUIP_COMPOSE, //装备合成消耗
				0, 1, pEquipTab->composeneedglod, 0, 0);
	return ItemOperateResult_Ok;
}

bool CUser::UpdateBagInfoToDB()
{
	func_start();
	
	if (!server.memcached_enable)
	{
		if (MT_ERR == s_dbproxy.UpDateBagItemInfo(propertys.userid,this))
		{
			LOGWARN("CUser::UpdateUserDataToDB, UpDateBagItemInfo failed! uoid=%u,userid=%lld,account=%s",
				at_uoid(), propertys.userid, propertys.account.to_string());
			return false;
		}
		if (MT_ERR == s_dbproxy.UpDateBagEquipInfo(propertys.userid,this))
		{
			LOGWARN("CUser::UpdateUserDataToDB, UpDateBagEquipInfo failed! uoid=%u,userid=%lld,account=%s",
				at_uoid(), propertys.userid, propertys.account.to_string());
			return false;
		}
		if (MT_ERR == s_dbproxy.UpDateUserGemsInfo(propertys.userid,this))
		{
			LOGWARN("CUser::UpdateBagInfoToDB, UpDateUserGemsInfo failed! uoid=%u,userid=%lld,account=%s",
				at_uoid(), propertys.userid, propertys.account.to_string());
			return false;
		}
	}
	else
	{
		if (GetBag()->IsChangedItem())
			SetUpdateTime( LOT_UserItems );			
		if (GetBag()->IsChangedGem())
			SetUpdateTime(LOT_UserGems);
		if (GetBag()->IsChangedEquip())
			SetUpdateTime( LOT_UserEquip );
	}
	return true;
	func_end();
}
/*
	装备强化
*/
ItemOperateResult CUser::EquipSharpen( ReqEquipSharpen* pPacket, int32_t & nOldLvl )
{
	func_start();
	if (NULL == pPacket)
	{
		LOGERR("CUser::EquipSharpen Fail,NULL == pPacket,userid[%lld]",GetUserID());
		return ItemOperateResult_Fail;
	}
	CHero* pHero = getHeroById(pPacket->heroid);
	if (NULL == pHero)
	{
		LOGERR("CUser::EquipSharpen Fail,Not Hero[%d],userid[%lld]",pPacket->heroid,GetUserID());
		return ItemOperateResult_NotHero;
	}
	EquipPos eEquipPos = (EquipPos)pPacket->equippos;
	hero_equippos_info* pEquipPosInfo = pHero->GetHeroEquipPosInfo(eEquipPos);
	if (NULL == pEquipPosInfo)
	{
		LOGERR("CUser::EquipSharpen Fail,Equip Pos[%d] Error,userid[%lld]",eEquipPos,GetUserID());
		return ItemOperateResult_Fail;
	}
	//服务器和客户端的强化装备id不一致
	if (pEquipPosInfo->equipid != pPacket->itemid)
	{
		LOGERR("CUser::EquipSharpen Fail,equipid[%d] != itemid[%d],userid[%lld]",pEquipPosInfo->equipid,pPacket->itemid,GetUserID());
		return ItemOperateResult_Fail;
	}
	equipment_tab* pHeroEquipTab = CEquipmentTabMng::Instance()->GetEquipmentTabInfoByID(pEquipPosInfo->equipid);
	if (NULL == pHeroEquipTab)
	{
		LOGERR("CUser::EquipSharpen Fail,Sharpen EquipID[%d] Not Find,userid[%lld]",pEquipPosInfo->equipid,GetUserID());
		return ItemOperateResult_Fail;
	}
	if (ItemOperateResult_Fail == CheckEquipPos(pHeroEquipTab,eEquipPos))
	{
		LOGERR("CUser::EquipSharpen CheckEquipPos Fail,EquipType[%d] Error,Client Type[%d],userid[%lld]",
			pHeroEquipTab->type,eEquipPos,GetUserID());
		return ItemOperateResult_Fail;
	}
	nOldLvl = pEquipPosInfo->sharpenlevel ;
	int32_t totalexp = 0;		//强化材料产生的总经验
	int16_t itemid = -1;
	//对强化装备的材料进行判断
	for (int idx = 0;idx < pPacket->materialcount && idx < MaxBagNum; idx++)
	{
		itemid = pPacket->material[idx].materialid;
		Item* pItem = GetBag()->FindItem(itemid);
		if (NULL == pItem)
		{
			LOGERR("CUser::EquipSharpen Fail,not find materialID[%d],equipid[%d],userid[%lld]",
				itemid,pPacket->itemid,GetUserID());
			return ItemOperateResult_NotFindItem;
		}
		//判断数量是否够
		int16_t materialnum = pPacket->material[idx].materialnum;
		if (pItem->GetNum()<materialnum)
		{
			LOGERR("CUser::EquipSharpen Fail,materialID[%d] num[%d] is lower then servernum[%d],userid[%lld]",
				itemid,materialnum,pItem->GetNum(),GetUserID());
			return ItemOperateResult_NumLow;
		}
		if (!pItem->IsEquip())
		{	//装备强化特殊材料的判断
			item_tab* ptab = pItem->GetItemTab();
			if (NULL == ptab)
			{
				LOGERR("CUser::EquipSharpen Fail,NULL == ptab,userid[%lld]",GetUserID());
				return ItemOperateResult_Fail;
			}
			if (ptab->type != ItemType_EquipSharpen)
			{
				LOGERR("CUser::EquipSharpen Fail,type[%d] error materialID[%d],userid[%lld]",ptab->type,itemid,GetUserID());
				return ItemOperateResult_NotPowder;
			}
			totalexp += (ptab->param1 * materialnum);
		}
		else
		{
			equipment_tab* ptab = pItem->GetEquipTab();
			if (NULL == ptab)
			{
				LOGERR("CUser::EquipSharpen Fail,NULL == ptab22,userid[%lld]",GetUserID());
				return ItemOperateResult_Fail;
			}
			//只能消耗当前装备同品质或低品质的装备
			if(ptab->quality > pHeroEquipTab->quality)
			{
				LOGERR("CUser::EquipSharpen Fail,quality[%d] high, EquipQuality[%d] materialID[%d],userid[%lld]",
					ptab->quality,pHeroEquipTab->quality,itemid,GetUserID());
				return ItemOperateResult_QualityHigh;
			}
			/*
			if (ptab->type != pHeroEquipTab->type)//只能消耗同类型的装备
			{
				LOGERR("CUser::EquipSharpen Fail,Type[%d] diff EquipType[%d] materialID[%d],userid[%lld]",
					ptab->type,pHeroEquipTab->type,itemid,GetUserID());
				return ItemOperateResult_DiffType;
			}
			*/
			totalexp += (ptab->fuseaddexp * materialnum);
		}
	}
	if (totalexp <= 0)
	{
		LOGERR("CUser::EquipSharpen Fail,totalexp[%d] <= 0,userid[%lld]",totalexp,GetUserID());
		return ItemOperateResult_Fail;
	}
	//删除强化材料
	for (int idx = 0;idx < pPacket->materialcount && idx < MaxBagNum; idx++)
	{
		GetBag()->SubItem(pPacket->material[idx].materialid,pPacket->material[idx].materialnum);
	}
	//添加英雄装备经验
	pHero->AddHeroEquipInfo(eEquipPos,pPacket->itemid,totalexp);
	UpdateHeroToDB(pPacket->heroid);
	//
	UpdateBagInfoToDB();
	return ItemOperateResult_Ok;
	func_end();
}

int32_t* CUser::GetShopGemData(int nShop,int nItemIndex,int nAttribute)
{
	if(nShop >= eShop_max || nShop < 0) return NULL; 
	if(nItemIndex >= TOTAL_ITEM_CNT || nItemIndex < 0) return NULL; 
	if(nAttribute >= MAX_GEM_MAIN_ATT_COUNT+MAX_GEM_ADDITIONAL_ATT_COUNT || nAttribute < 0) return NULL; 
	
	
	return &m_ShopGemRandomData[nShop][nItemIndex][nAttribute];
}

int32_t* CUser::GetShopGemData(int nShop,int nItemIndex)
{
	if(nShop >= eShop_max || nShop < 0) return NULL; 
	if(nItemIndex >= TOTAL_ITEM_CNT || nItemIndex < 0) return NULL;


	return m_ShopGemRandomData[nShop][nItemIndex];
}

void CUser::CopyShopGemData(int nShop,int32_t* inoutGemData)
{
	memcpy(inoutGemData,m_ShopGemRandomData[nShop],sizeof(m_ShopGemRandomData[nShop]));
}

emGemHandleResult CUser::HeroInlayGem(int32_t pHeroid,int32_t suit,int32_t index)
{
	func_start();

	hero_gem_info pPacket ;
	//獲取背包符文
	if (!GetBag()->GetOneGemInfo(pPacket,suit,index))
	{
		LOGERR("CUser::HeroInlayGem Fail,NULL == pPacket,userid[%lld]",GetUserID());
		return emGemNotExist;
	}
	CHero* pHero = getHeroById(pHeroid);
	if (NULL == pHero)
	{
		LOGERR("CUser::HeroInlayGem Fail,Not Hero[%d],userid[%lld]",pHeroid,GetUserID());
		return emGemHeroNotExist;
	}


	gem_tab* pgeData =   CGemTabMng::Instance()->GetGemTabInfoByID(pPacket.m_tableID);
	if (pgeData == NULL)
	{
		LOGERR("CHero::HeroInlayGem. no gem found in gem.tab [%d] ",pPacket.m_tableID);
		return emGemNotExist;
	}

	//鑲嵌
	if (!pHero->AddHeroGemInfo((GemPos)(pgeData->m_position),pPacket))
	{
		LOGERR("CUser::HeroInlayGem Fail, Hero[%d],userid[%lld]",pPacket.m_id,GetUserID());
		return emGemHandleResult_error;
	}
	//刪除背包中的符文
	if (!GetBag()->DeleteGem(pgeData->m_suit,pPacket.m_id))
	{
		LOGERR("CUser::HeroInlayGem Fail, Hero[%d],userid[%lld]",pPacket.m_id,GetUserID());
		return emGemDelete_error;
	}

	//更新背包数据
	UpdateBagInfoToDB();
	return emGemHandleResult_ok;
	func_end();
}


emGemHandleResult CUser::HeroOutlayGem(GemPos position , int heroid)
{
	func_start();
	CHero* pHero = getHeroById(heroid);
	if (NULL == pHero)
	{
		LOGERR("CUser::HeroOutlayGem Fail,Not Hero[%d],userid[%lld]",heroid,GetUserID());
		return emGemHeroNotExist;
	}

	hero_gem_info tmpInfo;
	if (!pHero->GetHeroGemInfo(GemPos(position),tmpInfo))
	{
		LOGERR("CUser::HeroOutlayGem Fail,No gem like this on Hero[%d],position[%d]",heroid,position);
		return emGemNotExist;
	}
	 
	if ( false == GetBag()->AddGem(&tmpInfo) ||  false == pHero->DeleteHeroGemInfo(GemPos(position))  )
	{
		LOGERR("CUser::HeroOutlayGem Fail, operate failed Hero[%d],userid[%lld]",heroid,GetUserID());
		return emGemHandleResult_error;
	}

	//更新背包数据
	UpdateBagInfoToDB();
	return emGemHandleResult_ok;
	func_end();
}


emGemHandleResult CUser::HeroDecomposeGemOnHero(GemPos position , int32_t heroid)
{
	func_start();
	CHero* pHero = getHeroById(heroid);
	if (NULL == pHero)
	{
		LOGERR("CUser::HeroDecomposeGemOnHero Fail,Not Hero[%d],userid[%lld]",heroid,GetUserID());
		return emGemHeroNotExist;
	}

	hero_gem_info tempGem;
	if (!pHero->GetHeroGemInfo(GemPos(position),tempGem))
	{
		LOGERR("CUser::HeroDecomposeGemOnHero Fail,No gem like this on Hero[%d],position[%d]",heroid,position);
		return emGemNotExist;
	}
	gem_tab* pgeData =   CGemTabMng::Instance()->GetGemTabInfoByID(tempGem.m_tableID);
	if (pgeData == NULL)
	{
		LOGERR("CHero::HeroDecomposeGemOnHero. no gem found in gem.tab [%d] ",tempGem.m_tableID);
		return emGemNotExist;
	}

	gemenhance_tab*  enhencetab =   CGemTabMng::Instance()->GetGemEnhanceTabInfoByID(tempGem.m_level);

	if(enhencetab == NULL)
	{
		LOGERR("CUser::HeroDecomposeGemOnHero Fail No Such Gem in Table <gemenhence> [level error] , heroid[%d],position[%d]",heroid,position);
		return emGemReadTable_error ;
	}

	gemstar_tab*  startab =   CGemTabMng::Instance()->GetGemStarTabInfoByID(pgeData->m_starcount);

	if(startab == NULL)
	{
		LOGERR("CUser::HeroDecomposeGemOnHero Fail No Such Gem in Table <gemstar_tab> [level error] , heroid[%d],position[%d]",heroid,position);
		return emGemReadTable_error ;
	}
	/*分解获得魔晶公式变为：
		分解获得魔晶=强化等级對應的分解所得數目×星级分解获得魔晶比例+星级额外获得魔晶*/
	double rateLevel =( startab->m_split_get_rate )/10000;

	int evilsGet = enhencetab->m_split_get_num* rateLevel + startab->m_split_get_additional; 
	
	if (false == pHero->DeleteHeroGemInfo(GemPos(position) ))
	{
		LOGERR("CUser::HeroDecomposeGemOnHero delete Fail, Hero[%d],userid[%lld]",heroid,GetUserID());
		return emGemDelete_error;
	}
	
	AddEvilDiamonds(evilsGet);

	//更新背包数据
	UpdateBagInfoToDB();

	return emGemHandleResult_ok;
	func_end();
}

emGemHandleResult CUser::HeroDecomposeGemInBag(int16_t suit , int32_t index)
{
	func_start();
	if (suit < GemSuit_None || suit>GemSuit_Max)
	{
		LOGERR("CUser::HeroDecomposeGemInBag  Fail,[ suit error ]suit[%d],userid[%lld]",suit,GetUserID());
		return emGemNotExist;
	}
	if (index < 0 || index >= MAX_GEM_IN_BAG_COUNT)
	{
		LOGERR("CUser::HeroDecomposeGemInBag Fail [ index error ] , index[%d],userid[%lld]",index,GetUserID());
		return emGemNotExist;
	}



	hero_gem_info tempGem;
	if (!GetBag()->GetOneGemInfo(tempGem,suit,index))
	{
		LOGERR("CUser::HeroDecomposeGemInBag Fail No Such Gem in Bag, suit[%d],index[%d]",suit,index);
		return emGemNotExist;
	}

	if (tempGem.m_tableID == -1)
	{
		LOGERR("CUser::HeroDecomposeGemInBag  Fail No Such Gem in Bag m_tableID == -1,, suit[%d],index[%d]",suit,index);
		return emGemNotExist;
	}

	gem_tab* pgeData =   CGemTabMng::Instance()->GetGemTabInfoByID(tempGem.m_tableID);
	if (pgeData == NULL)
	{
		LOGERR("CHero::HeroDecomposeGemInBag. no gem found in gem.tab [%d] ",tempGem.m_tableID);
		return emGemReadTable_error;
	}
	
	gemenhance_tab*  enhencetab =   CGemTabMng::Instance()->GetGemEnhanceTabInfoByID(tempGem.m_level);
	if(enhencetab == NULL)
	{
		LOGERR("CUser::HeroDecomposeGemInBag Fail No Such Gem in Table <gemenhence>, suit[%d],index[%d]",suit,index);
		return emGemReadTable_error ;
	}

	gemstar_tab*  startab =   CGemTabMng::Instance()->GetGemStarTabInfoByID(pgeData->m_starcount);

	if(startab == NULL)
	{
		LOGERR("CUser::HeroDecomposeGemInBag Fail No Such Gem in Table <gemenhence>, suit[%d],index[%d]",suit,index);
		return emGemReadTable_error ;
	}
	/*分解获得魔晶公式变为：
	分解获得魔晶=强化等级對應的分解所得數目×星级分解获得魔晶比例+星级额外获得魔晶*/
	float rateLevel = ( startab->m_split_get_rate )/10000.f;
	int evilsGet = enhencetab->m_split_get_num * rateLevel + startab->m_split_get_additional;

	if (!GetBag()->DeleteGem(suit,index))
	{
		LOGERR("CUser::HeroDecomposeGemInBag delete Fail, index[%d],userid[%lld]",index,GetUserID());
		return emGemDelete_error;
	}

	AddEvilDiamonds(evilsGet);

	//更新背包数据
	UpdateBagInfoToDB();

	return emGemHandleResult_ok;
	func_end();
}

emGemHandleResult CUser::HeroDecomposeGemInBatch(int16_t star,int16_t suit)
{
	func_start();

	vector<int> starstate;//選定的星級二進製錶示
	int s = 1;

	for (int idx =MAX_GEM_STAR_COUNT; idx > 0;--idx)
	{
		starstate.push_back((star>>(idx-1))&1);
	}
	
	vector<hero_gem_info> vecGemInfo;

	if (!GetBag()->GetGemInfos(vecGemInfo))
	{
		LOGERR("CUser::HeroDecomposeGemInBatch Fail get gem infos error");
		return emGemNotExist;
	}

	if (vecGemInfo.size() == 0 )
	{
		LOGERR("CUser::HeroDecomposeGemInBatch Fail no gems in bag");
		return emGemBagEmpty;
	}

	int evilsGet = 0;
	for (int j = 0; j<starstate.size();++j)
	{
		if (starstate[j] == 1)
		{
			int8_t tempstar = j+1;
			gemstar_tab*  startab =   CGemTabMng::Instance()->GetGemStarTabInfoByID(tempstar);
			if(startab == NULL)
			{
				LOGERR("CUser::HeroDecomposeGemInBatch Fail No Such star in Table <gemstar>, star[%d]",tempstar);
				return emGemReadTable_error ;
			}
			float rateLevel = ( startab->m_split_get_rate )/10000.f;
			
			for (int i = 0;i<vecGemInfo.size();++i)
			{ 
				gem_tab* pgeData =   CGemTabMng::Instance()->GetGemTabInfoByID(vecGemInfo[i].m_tableID);
				if (pgeData == NULL)
				{
					LOGERR("CHero::HeroDecomposeGemInBatch. no gem found in gem.tab [%d] ",vecGemInfo[i].m_tableID);
					return emGemReadTable_error;
				}

				if (pgeData->m_starcount != tempstar  || pgeData->m_suit != suit )//星級不符合 或者套裝不符合
				{
					continue;
				}
				gemenhance_tab*  enhencetab =   CGemTabMng::Instance()->GetGemEnhanceTabInfoByID(vecGemInfo[i].m_level);
				if(enhencetab == NULL)
				{
					LOGERR("CUser::HeroDecomposeGemInBatch Fail No Such Gem in Table <gemenhence>, level [%d],",vecGemInfo[i].m_level);
					return emGemReadTable_error ;
				}
				/*分解获得魔晶公式变为：
				分解获得魔晶=强化等级對應的分解所得數目×星级分解获得魔晶比例+星级额外获得魔晶*/
				evilsGet += enhencetab->m_split_get_num * rateLevel + startab->m_split_get_additional;
				if (!GetBag()->DeleteGem(pgeData->m_suit,vecGemInfo[i].m_id))
				{
					LOGERR("CUser::HeroDecomposeGemInBatch delete Fail, suit[%d],index[%d]",pgeData->m_suit,vecGemInfo[i].m_id);
					return emGemDelete_error;
				}
			}
		}
	}

	
	AddEvilDiamonds(evilsGet);

	//更新背包数据
	UpdateBagInfoToDB();

	return emGemHandleResult_ok;
	func_end();
}

//強化
emGemHandleResult CUser::HeroEnhenceGemOnHero(GemPos position , int32_t heroid,int isusedianmond)
{
	func_start();

	CHero* pHero = getHeroById(heroid);
	if (NULL == pHero)
	{
		LOGERR("CUser::HeroEnhenceGemOnHero Fail,Not Hero[%d],userid[%lld]",heroid,GetUserID());
		return emGemHeroNotExist;
	}

	hero_gem_info tempGem;
	if (!pHero->GetHeroGemInfo(GemPos(position),tempGem))
	{
		LOGERR("CUser::HeroEnhenceGemOnHero Fail,No gem like this on Hero[%d],position[%d]",heroid,position);
		return emGemNotExist;
	}

	if (tempGem.m_tableID == -1)
	{
		LOGERR("CUser::HeroEnhenceGemOnHero Fail No Such Gem on hero, heroid[%d],position[%d]",heroid,position);
		return emGemNotExist;
	}

	gem_tab* pgeData =   CGemTabMng::Instance()->GetGemTabInfoByID(tempGem.m_tableID);
	if (pgeData == NULL)
	{
		LOGERR("CHero::HeroEnhenceGemOnHero. no gem found in gem.tab [%d] ",tempGem.m_tableID);
		return emGemReadTable_error;
	}

	//判斷是否是最高等級
	gemenhance_tab*  enhencetab =   CGemTabMng::Instance()->GetGemEnhanceTabInfoByID(tempGem.m_level + 1);
	if(enhencetab == NULL)
	{
		LOGERR("CUser::HeroEnhenceGemOnHero Fail No Such Gem in Table <gemenhence> [level is max] , heroid[%d],position[%d]",heroid,position);
		return emGemMaxLevel ;
	}

	gemstar_tab*  startab =   CGemTabMng::Instance()->GetGemStarTabInfoByID(pgeData->m_starcount);
	if(startab == NULL)
	{
		LOGERR("CUser::HeroEnhenceGemOnHero Fail No Such star in Table <gemstar>, star[%d]",pgeData->m_starcount);
		return emGemReadTable_error ;
	}
	// 魔晶 钻石强化不同星级的符文 要乘以星级表里的强化消耗比例，
	float enhenceRate =  startab->m_enhance_cost_rate/ 10000.f;
	int32_t cost = Double2Int( (FLOAT_MT)(enhencetab->m_enhance_cost_num * enhenceRate));
	int32_t costdiamond = Double2Int( (FLOAT_MT)(enhencetab->m_split_cost_diamond * enhenceRate));
	if (isusedianmond == 1)
	{
		UserRMB_t oldRMB_t = GetRMB_t();

		//扣除消费
		int32_t nRmb = GetRMB() ;
		if (nRmb < costdiamond)
		{
			LOGERR("CUser::HeroEnhenceGemOnHero(),user_diamonds[%d] < NeedDiamond[%d]!userid=%lld",
				nRmb, costdiamond, propertys.userid);
			return emGemDiamondNotEnough;
		}
		//申请 真金操作来源事件ID
		int64_t idOpSourceEvent = CDataMgr::Instance()->CreateOpSourceId((int32_t)RMBOS_SUB_GEM_ENHENCE) ;
		//组织消费接口参数
		SubRMBParam_t subRmbParam ;
		subRmbParam.source			= RMBOS_SUB_GEM_ENHENCE ;
		snprintf(subRmbParam.uniqueid, sizeof(subRmbParam.uniqueid), "%lld", idOpSourceEvent) ;
		subRmbParam.subvalue		= costdiamond ;
		snprintf(subRmbParam.desc, sizeof(subRmbParam.desc), "enhenece gem  cost(viplvl:%u)",GetVipLevel()) ;

		//扣除钻石
		int iRev = SubRMB(subRmbParam) ;
		if(0 != iRev)
		{
			LOGERR("CUser::HeroEnhenceGemOnHero(),sub rmb failed! CUser::SubRMB() return fail!(%d)userid=%lld,costrmb=%u,userrmb=%d",
				iRev, GetUserID(), costdiamond, nRmb);
			return emGemHandleResult_error ;
		}

		//日志
		const UserRMB_t& newRMB_t = GetRMB_t();
		LOG_USER_PURCHASE(this, MoneyType_Diamond, subRmbParam.source, 0, 1, oldRMB_t.m_charge-newRMB_t.m_charge, oldRMB_t.m_donate-newRMB_t.m_donate, oldRMB_t.m_bindrmb-newRMB_t.m_bindrmb);

	}else
	{
		if (cost  > propertys.evilDiamond)
		{
			LOGERR("CUser::HeroEnhenceGemOnHero Fail for EvilDiamond not enough, heroid[%d],position[%d]",heroid,position);
			return emGemEvilDiamondNotEnough ;
		}
		SubEvilDiamonds(cost);
	}
	//先扣除材料 在修改數據
	bool canEnhence = rand() % 10000 < enhencetab->m_enhance_success_rate;
	//使用魔晶強化 判定失敗  钻石强化100%成功
	if ((!canEnhence) && isusedianmond != 1)
	{
		return emGemEnhence_Not_Lucky ;
	}

	tempGem.m_level ++;

	//獲取當前強化等級對應的quality表
	gemquality_tab* gemquality = CGemTabMng::Instance()->GetGemQualityTabInfoByEnhenceLevel(tempGem.m_level);
	int8_t sumAdditional = 0;
	for (int i = 0;i<MAX_GEM_ADDITIONAL_ATT_COUNT;i++)
	{
		gemattribute_tab* onegemAddAtt = CGemTabMng::Instance()->GetGemAttributeTabInfoByID(tempGem.m_add_attribute[i]);
		if (onegemAddAtt !=NULL)
		{
			sumAdditional++;
		}
	}
	gem_tab* gemtab = CGemTabMng::Instance()->GetGemTabInfoByID(tempGem.m_tableID);

	//強化等級成為x 沒有附加屬性 添加一條 符文的副屬性條數和品質對應
	//1.符合條件的增加符文屬性條目
	if (gemquality != NULL  &&  sumAdditional < gemquality->m_add_num)
	{
		int addtype = JGetRand(gemtab->m_add_floor,gemtab->m_add_proof);
		gemattribute_tab* onegemAddAtt = CGemTabMng::Instance()->GetGemAttributeTabInfoByID(addtype);
		int16_t attValue = onegemAddAtt->m_att_value;
		tempGem.m_add_attribute[gemquality->m_add_num-1] = addtype;
		tempGem.m_quality ++;//添加副屬性 提升品質
	}

	if (!pHero->ModifyHeroGemInfo(position,tempGem))
	{
		LOGERR("CUser::HeroEnhenceGemOnHero Fail for update level failed, heroid[%d],position[%d]",heroid,position);
		return emGemHandleResult_error ;
	}
	
	//更新背包数据
	UpdateBagInfoToDB();
	
	return emGemHandleResult_ok;
	func_end();
}

emGemHandleResult CUser::HeroEnhenceGemInBag(int16_t suit , int32_t index,int isusedianmond)
{
	func_start();
	if (suit < GemSuit_None || suit>GemSuit_Max)
	{
		LOGERR("CUser::HeroEnhenceGemInBag Fail, suit[%d],userid[%lld]",suit,GetUserID());
		return emGemNotExist;
	}
	if (index < 0 || index >= MAX_GEM_IN_BAG_COUNT)
	{
		LOGERR("CUser::HeroEnhenceGemInBag Fail, index[%d],userid[%lld]",index,GetUserID());
		return emGemNotExist;
	}
	hero_gem_info tempGem;
	if (!GetBag()->GetOneGemInfo(tempGem,suit,index))
	{
		LOGERR("CUser::HeroEnhenceGemInBag Fail No Such Gem in Bag, suit[%d],index[%d]",suit,index);
		return emGemNotExist;
	}
	gem_tab* pgeData =   CGemTabMng::Instance()->GetGemTabInfoByID(tempGem.m_tableID);
	
	if (pgeData == NULL)
	{
		LOGERR("CHero::HeroEnhenceGemInBag. no gem found in gem.tab [%d] ",tempGem.m_tableID);
		return emGemReadTable_error;
	}
	
	gemenhance_tab*  enhencetab =   CGemTabMng::Instance()->GetGemEnhanceTabInfoByID(tempGem.m_level +1);

	if(enhencetab == NULL)
	{
		LOGERR("CUser::HeroEnhenceGemInBag Fail No Such Gem in Table <gemenhence>, suit[%d],index[%d]",suit,index);
		return emGemMaxLevel ;
	}



	//計算魔晶 和鑽石消耗
	gemstar_tab*  startab =   CGemTabMng::Instance()->GetGemStarTabInfoByID(pgeData->m_starcount);
	if(startab == NULL)
	{
		LOGERR("CUser::HeroEnhenceGemInBag Fail No Such star in Table <gemstar>, star[%d]",pgeData->m_starcount);
		return emGemReadTable_error ;
	}

	float enhenceRate =  startab->m_enhance_cost_rate/ 10000.f;
	// 魔晶 钻石强化不同星级的符文 要乘以星级表里的强化消耗比例，
	int32_t cost = Double2Int( (FLOAT_MT)(enhencetab->m_enhance_cost_num * enhenceRate));
	int32_t costdiamond = Double2Int( (FLOAT_MT)(enhencetab->m_split_cost_diamond * enhenceRate));
	//扣除貨幣
	if (isusedianmond == 1)
	{
		//扣除消费
		int32_t nRmb = GetRMB() ;
		if (nRmb < costdiamond)
		{
			LOGERR("CUser::HeroEnhenceGemInBag(),user_diamonds[%d] < NeedDiamond[%d]!userid=%lld",
				nRmb, costdiamond, propertys.userid);
			return emGemDiamondNotEnough;
		}
		//申请 真金操作来源事件ID
		int64_t idOpSourceEvent = CDataMgr::Instance()->CreateOpSourceId((int32_t)RMBOS_SUB_GEM_ENHENCE) ;

		//组织消费接口参数
		SubRMBParam_t subRmbParam ;
		subRmbParam.source			= RMBOS_SUB_GEM_ENHENCE ;
		snprintf(subRmbParam.uniqueid, sizeof(subRmbParam.uniqueid), "%lld", idOpSourceEvent) ;
		subRmbParam.subvalue		= costdiamond ;
		snprintf(subRmbParam.desc, sizeof(subRmbParam.desc), "enhenece gem  cost(viplvl:%u)",GetVipLevel()) ;

		//备份
		UserRMB_t oldRMB_t = GetRMB_t();

		//扣除钻石
		int iRev = SubRMB(subRmbParam) ;
		if(0 != iRev)
		{
			LOGERR("CUser::HeroEnhenceGemInBag(),sub rmb failed! CUser::SubRMB() return fail!(%d)userid=%lld,costrmb=%u,userrmb=%d",
				iRev, GetUserID(), costdiamond, nRmb);
			return emGemHandleResult_error ;
		}

		//日志
		const UserRMB_t& newRMB_t = GetRMB_t();
		LOG_USER_PURCHASE(this, MoneyType_Diamond, subRmbParam.source, 0, 1, oldRMB_t.m_charge-newRMB_t.m_charge, oldRMB_t.m_donate-newRMB_t.m_donate, oldRMB_t.m_bindrmb-newRMB_t.m_bindrmb);

	}else{
		if (cost  > propertys.evilDiamond)
		{
			LOGERR("CUser::HeroEnhenceGemInBag Fail for EvilDiamond not enough, suit[%d],index[%d]",suit,index);
			return emGemEvilDiamondNotEnough ;
		}
		SubEvilDiamonds(cost);
	}

	bool canEnhence = rand() % 10000 < enhencetab->m_enhance_success_rate;
	//使用魔晶強化 判定失敗 
	if ((!canEnhence) && isusedianmond != 1)
	{
		return emGemEnhence_Not_Lucky ;
	}

	//開始修改數據
	//1.強化等級
	tempGem.m_level ++;
	//2.屬性添加
	//獲取當前強化等級對應的quality表
	gemquality_tab* gemquality = CGemTabMng::Instance()->GetGemQualityTabInfoByEnhenceLevel(tempGem.m_level);
	int8_t sumAdditional = 0;
	for (int i = 0;i<MAX_GEM_ADDITIONAL_ATT_COUNT;i++)
	{
		gemattribute_tab* onegemAddAtt = CGemTabMng::Instance()->GetGemAttributeTabInfoByID(tempGem.m_add_attribute[i]);
		if (onegemAddAtt !=NULL)
		{
			sumAdditional++;
		}
	}
	gem_tab* gemtab = CGemTabMng::Instance()->GetGemTabInfoByID(tempGem.m_tableID);
	//強化等級成為x 沒有附加屬性 添加一條 符文的副屬性條數和品質對應
	//1.符合條件的增加符文屬性條目
	if (gemquality != NULL  &&  sumAdditional < gemquality->m_add_num)
	{
		int addtype = JGetRand(gemtab->m_add_floor,gemtab->m_add_proof);
		gemattribute_tab* onegemAddAtt = CGemTabMng::Instance()->GetGemAttributeTabInfoByID(addtype);
		int16_t attValue = onegemAddAtt->m_att_value;
		tempGem.m_add_attribute[gemquality->m_add_num - 1] = addtype;
		tempGem.m_quality ++;//添加副屬性 提升品質
	}

	//修改符文信息
	if (!GetBag()->ModifyGem(&tempGem))
	{
		LOGERR("CUser::HeroEnhenceGemInBag Fail for update level failed, suit[%d],index[%d]",suit,index);
		return emGemHandleResult_error ;
	}
	
	//更新背包数据
	UpdateBagInfoToDB();

	return emGemHandleResult_ok;
	func_end();
}

emGemHandleResult CUser::GetUserGeminfos(vector<hero_gem_info>& resultlist)
{
	if (!GetBag()->GetGemInfos(resultlist))
	{
		LOGERR("CUser::GetUserGeminfos Fail,userid[%lld]",GetUserID());
		return emGemHandleResult_fail;
	}
	return emGemHandleResult_ok;
}

emGemHandleResult CUser::GetUserOneGeminfo(int8_t suit,int32_t index,hero_gem_info& outGem)
{
	vector<hero_gem_info> resultlist;

	if (!GetBag()->GetGemInfos(resultlist))
	{
		LOGERR("CUser::GetUserOneGeminfo Fail,userid[%lld]",GetUserID());
		return emGemHandleResult_fail;
	}

	for (int i = 0;i<resultlist.size();i++)
	{
		if (index ==resultlist[i].m_id)//id相同
		{
			gem_tab* pgemData =   CGemTabMng::Instance()->GetGemTabInfoByID(resultlist[i].m_tableID);
			if (pgemData == NULL)
			{
				LOGERR("CHero::GetUserOneGeminfo. no gem found in gem.tab [%d] ",resultlist[i].m_tableID);
				return emGemHandleResult_fail;
			}
			if ( suit ==pgemData->m_suit )//套裝相同
			{
				outGem.copy(resultlist[i]);
				return emGemHandleResult_ok;
			}
		}
	}
	LOGERR("CHero::GetUserOneGeminfo. no gem found in bag ");
	return emGemHandleResult_fail;
}


emGemHandleResult CUser::GetHeroOneGeminfo(int32_t heroid,int32_t pos,hero_gem_info& outGem)
{
	CHero* pHero = getHeroById(heroid);
	if (NULL == pHero)
	{
		LOGERR("CUser::GetHeroOneGeminfo Fail,Not Hero[%d],userid[%lld]",heroid,GetUserID());
		return emGemHeroNotExist;
	}
	hero_gem_info tempGem;
	if (!pHero->GetHeroGemInfo(GemPos(pos),tempGem))
	{
		LOGERR("CUser::GetHeroOneGeminfo Fail,No gem like this on Hero[%d],position[%d]",heroid,pos);
		return emGemNotExist;
	}
	if (tempGem.m_tableID == -1)
	{
		LOGERR("CUser::GetHeroOneGeminfo Fail No Such Gem on hero, heroid[%d],position[%d]",heroid,pos);
		return emGemNotExist;
	}
	outGem.copy(tempGem);
	return emGemHandleResult_ok;
}


ItemOperateResult CUser::EquipHero(ReqEquipReplace* pPacket)
{
	func_start();
	if (NULL == pPacket)
	{
		LOGERR("CUser::EquipHero Fail,NULL == pPacket,userid[%lld]",GetUserID());
		return ItemOperateResult_Fail;
	}
	CHero* pHero = getHeroById(pPacket->herotabid);
	if (NULL == pHero)
	{
		LOGERR("CUser::EquipHero Fail,Not Hero[%d],userid[%lld]",pPacket->herotabid,GetUserID());
		return ItemOperateResult_NotHero;
	}
	//只有装备才能替换
	if (!GetBag()->IsEquipment(pPacket->newitemid))
	{
		LOGERR("CUser::EquipHero Fail,equip[%d] is not Equip,userid[%lld]",pPacket->newitemid,GetUserID());
		return ItemOperateResult_NotEquip;
	}
	EquipPos eEquipPos = (EquipPos)pPacket->equippos;
	hero_equippos_info* pEquipPosInfo = pHero->GetHeroEquipPosInfo(eEquipPos);
	if (NULL == pEquipPosInfo)
	{
		LOGERR("CUser::EquipHero Fail,Equip Pos[%d] Error,userid[%lld]",eEquipPos,GetUserID());
		return ItemOperateResult_Fail;
	}
	//已经装备道具,不能再装备
	if (pEquipPosInfo->equipid != -1)
	{
		LOGERR("CUser::EquipHero Fail,Equip Pos[%d] is Have Equip,userid[%lld]",eEquipPos,GetUserID());
		return ItemOperateResult_Fail;
	}
	//进阶等级
	uint8_t UpgradeTimes = pHero->GetUpgradeTimes();
	//
	const HerosTab_t* pHeroTab = CHerosTabMgr::Instance()->GetHeroData(pHero->GetHeroTemplateID());
	if (NULL == pHeroTab)
	{
		LOGERR("CUser::EquipHero Fail,HeroTabID[%d] is Not Find,userid[%lld]",pHero->GetHeroTemplateID(),GetUserID());
		return ItemOperateResult_Fail;
	}
	int32_t ServerNewItemID = pHeroTab->NeedEquipIDForEvolution[UpgradeTimes][eEquipPos];
	if (pPacket->newitemid != ServerNewItemID)
	{
		LOGERR("CUser::EquipHero Fail,ClientItemID[%d] != ServerItemID[%d],userid[%lld]",pPacket->newitemid ,ServerNewItemID,GetUserID());
		return ItemOperateResult_Fail;
	}
	equipment_tab* pNewHeroEquipTab = CEquipmentTabMng::Instance()->GetEquipmentTabInfoByID(pPacket->newitemid);
	if (NULL == pNewHeroEquipTab)
	{
		LOGERR("CUser::EquipHero Fail, NewEquipID[%d] Not Find,userid[%lld]",pPacket->newitemid,GetUserID());
		return ItemOperateResult_Fail;
	}
	//装备等级不能高过英雄等级
	if (pNewHeroEquipTab->equiplv > pHero->GetLvl())
	{
		LOGERR("CUser::EquipHero Fail,EquipLv[%d] > HeroLv[%d],userid[%lld]",
			pNewHeroEquipTab->equiplv,pHero->GetLvl(),GetUserID());
		return ItemOperateResult_HeroLvLow;
	}
	//添加装备以及装备exp到英雄身上
	pHero->AddHeroEquipInfo(eEquipPos,pPacket->newitemid,0);
	UpdateHeroToDB(pPacket->herotabid);
	//从背包删除道具
	GetBag()->SubItem(pPacket->newitemid);
	UpdateBagInfoToDB();
	return ItemOperateResult_Ok;
	func_end();
}

ItemOperateResult CUser::CheckEquipPos( equipment_tab* pTab,EquipPos pos )
{
	func_start();
	if (pos<= eEquipPos_None || pos >= eEquipPos_Max)
	{
		return ItemOperateResult_Fail;
	}
	if (NULL == pTab)
	{
		return ItemOperateResult_Fail;
	}
	switch(pTab->type)
	{
	case EquipmentType_Weapon:
		{
			if (eEquipPos_Weapon != pos)
			{
				return ItemOperateResult_Fail;
			}
		}
		break;
	case EquipmentType_Amulet:
		{
			if (eEquipPos_Amulet != pos)
			{
				return ItemOperateResult_Fail;
			}
		}
		break;
	case EquipmentType_Armour:
		{
			if (eEquipPos_Armour != pos)
			{
				return ItemOperateResult_Fail;
			}
		}
		break;
	case EquipmentType_Cloak:
		{
			if (eEquipPos_Mantle != pos)
			{
				return ItemOperateResult_Fail;
			}
		}
		break;
	case EquipmentType_SpecialCimelia:
	case EquipmentType_CommonCimelia:
		{
			if (eEquipPos_Treasure != pos)
			{
				return ItemOperateResult_Fail;
			}
		}
		break;
	case EquipmentType_SpecialHoly:
	case EquipmentType_CommonHoly:
		{
			if (eEquipPos_Holy != pos)
			{
				return ItemOperateResult_Fail;
			}
		}
		break;
	default: return ItemOperateResult_Fail;
	}
	return ItemOperateResult_Ok;
	func_end();
}

int CUser::BuyGoldByDiamond(int32_t& nBuyGoldRate)
{
	func_start();
	const PlayerTab_t* pTabData = CPlayerTableManager::Instance()->GetData(GetLevel());
	if (NULL == pTabData)
	{
		return eCommConsumeResult_Fail;
	}
	
	if (0 == propertys.buy_glod_last_time)
	{
		if (0 != propertys.buy_glod_count)
		{
			LOGERR("CUser::CheckBuyPhysicalCount, first buy physical,but buy_physical_count[%u] != 0.userid=%lld",
				propertys.buy_glod_count, propertys.userid);
			propertys.buy_glod_count = 0;
		}
	}
	vip_tab* pVip = CVipMng::Instance()->GetVipTabInfoByID(GetVipLevel()) ;
	if(NULL == pVip)
	{
		LOGERR("CUser::BuyGoldByDiamond, cannot find vip data! vip_lvl=%u, userid=%lld",
			GetVipLevel(), propertys.userid);
		return eCommConsumeResult_Fail ;
	}
	//
	RefBuyGoldCount();
	//达到购买次数不能再购买
	if (propertys.buy_glod_count >= pVip->BuyGoldMaxTimes)
	{
		LOGWARN("CUser::BuyGoldByDiamond, buy_glod_count[%u] >= BuyGoldMaxTimes[%u]! userid=%lld",
			propertys.buy_glod_count, pVip->BuyGoldMaxTimes, propertys.userid);
		return eCommConsumeResult_Fail;
	}
	const BuyinCreaseData_t* pinCreaseData=CBuyinCreaseTabMgr::Instance()->GetData(propertys.buy_glod_count + 1);
	if (NULL == pinCreaseData)
	{
		LOGERR("CUser::BuyGoldByDiamond(),pinCreaseData is NULL!userid=%lld,buygoldcount[%d],tableid[%d]",
			propertys.buy_glod_count,propertys.buy_glod_count + 1);
		return eCommConsumeResult_Fail;
	}
	/////////////////
	//计算暴击率
	int32_t nRate = 1;
	int32_t nRand = rand()%1000;
	
	for (int i=0; i<pinCreaseData->BuyGlodRateCount;i++)
	{
		if (nRand < pinCreaseData->BuyGlodRateInfo[i].CrticalRate)
		{
			nRate = pinCreaseData->BuyGlodRateInfo[i].GoldTimes;
			break;
		}
	}
	uint32_t addgolds = pTabData->m_BaseGold * nRate;
	//超过上限不能再购买金币
	uint32_t totalgold = GetGolds() + addgolds;
	LOGINFO("BuyGoldByDiamond:: nRand = %d,nRate = %d,addgolds = %d,totalgold = %d,userid = %lld",nRand,nRate,addgolds,totalgold,GetUserID());
	if (totalgold > MAX_PLAYER_GLOD_NUM)
	{
		LOGINFO("CUser::BuyGoldByDiamond userid[%lld] level[%d] buyglod[%d] + curgold[%d] > Golds Max[%d]",
			GetUserID(),GetLevel(),addgolds,GetGolds(),MAX_PLAYER_GLOD_NUM);
		return CommConsumeResult_GoldFull;
	}
	/////////////////
	//扣除消费
	int32_t nRmb = GetRMB() ;
	if (nRmb < pinCreaseData->Diamond)
	{
		LOGERR("CUser::BuyGoldByDiamond(),user_diamonds[%d] < NeedDiamond[%d]!userid=%lld",
			nRmb, pinCreaseData->Diamond, propertys.userid);
		return eCommConsumeResult_Fail;
	}
	//申请 真金操作来源事件ID
	int64_t idOpSourceEvent = CDataMgr::Instance()->CreateOpSourceId((int32_t)RMBOS_SUB_BUY_GOLD) ;

	//组织消费接口参数
	SubRMBParam_t subRmbParam ;
	subRmbParam.source			= RMBOS_SUB_BUY_GOLD ;
	snprintf(subRmbParam.uniqueid, sizeof(subRmbParam.uniqueid), "%lld", idOpSourceEvent) ;
	subRmbParam.subvalue		= pinCreaseData->Diamond ;
	snprintf(subRmbParam.desc, sizeof(subRmbParam.desc), "buy golds cost(viplvl:%u,getphystr:%u)", 
		GetVipLevel(), addgolds) ;

	//备份
	UserRMB_t oldRMB_t = GetRMB_t();

	//扣除钻石
	int iRev = SubRMB(subRmbParam) ;
	if(0 != iRev)
	{
		LOGERR("CUser::BuyGoldByDiamond(),sub rmb failed! CUser::SubRMB() return fail!(%d)userid=%lld,costrmb=%u,userrmb=%d",
			iRev, GetUserID(), pinCreaseData->Diamond, nRmb);
		return eCommConsumeResult_Fail ;
	}

	//日志
	const UserRMB_t& newRMB_t = GetRMB_t();
	LOG_USER_PURCHASE(this, MoneyType_Diamond, subRmbParam.source, 0, 1, oldRMB_t.m_charge-newRMB_t.m_charge, oldRMB_t.m_donate-newRMB_t.m_donate, oldRMB_t.m_bindrmb-newRMB_t.m_bindrmb);

	nBuyGoldRate = nRate;
	AddGolds(addgolds);
	LOG_USER_REWARD(this, MoneyType_Gold, addgolds, 0, 0, GOLDOS_ADD_BuyGoldByDiamond);

	++propertys.buy_glod_count;
	time_t now = 0;
	time(&now);
	propertys.buy_glod_last_time = (uint32_t)now;
	if (!server.memcached_enable)
	{
		s_dbproxy.UpDateUserInfo(&propertys);

	}

	return eCommConsumeResult_Successe;
	func_end();
}

void CUser::RefBuyGoldCount()
{
	uint32_t hour = 0; 
	uint32_t minute = 0; 
	GetServerCdTimeByIndex(eCdTimeIndex_BuyGolds, hour, minute);
	if (0 != ComPareTimeWithHM(hour, minute, propertys.buy_glod_last_time, time(NULL)))
	{
		propertys.buy_glod_count = 0;
	}
}

void CUser::RefBuySkillPointCount()
{
	uint32_t hour = 0; 
	uint32_t minute = 0; 
	GetServerCdTimeByIndex(eCdTimeIndex_BuySkillPoint, hour, minute);
	if (0 != ComPareTimeWithHM(hour, minute, propertys.buy_skill_point_last_time, time(NULL)))
	{
		propertys.buy_skill_point_count = 0;
	}
}

int8_t CUser::ChangedTeamLeader( int16_t heroid )
{
	func_start();
	if (propertys.teamleaderid == heroid)
	{
		LOGINFO("CUser::ChangedTeamLeader same heroid[%d],userid[%lld]",heroid,GetUserID());
		return 1;
	}
	CHero* pHero = getHeroById(heroid);
	if (NULL == pHero)
	{
		LOGERR("CUser::ChangedTeamLeader NULL == pHero,userid[%lld],heroid[%d]",GetUserID(),heroid);
		return -1;
	}
	propertys.teamleaderid = heroid;
// 	//存储数据库
// 	UpdateUserDataToDB();
	//s_dbproxy.UpDateSortTempMainHeroToDB(*this);
	SetUpdateTime(LOT_UserSort);
	return 0;
	func_end();
}

int CUser::GetComBatPower(Prep_Function pre)
{
	uint8_t herocount = 0;
	int combatpower = 0;
	const int16_t* prepBattleArea = GetHerosFromPrepBattleArea(pre);
	if (NULL == prepBattleArea)
	{
		return 0;
	}
	for (int i=0; i<MAX_PREP_HERO_NUM;i++)
	{
		if (prepBattleArea[i]<0) continue;
		CHero* pHero = getHeroById(prepBattleArea[i]);
		if (!pHero)
		{
			continue;
		}
		if (herocount>=MAX_HERO_TEAM_NUM)
			break; 
		combatpower+=pHero->GetCombatPower();
		herocount++;
	}
	return combatpower;
}
bool CUser::IsHaveHero(Prep_Function pre, int32_t heroId)
{
	const int16_t* prepBattleArea = GetHerosFromPrepBattleArea(pre);
	if (NULL == prepBattleArea)
	{
		return false;
	}
	for (int i=0; i<MAX_PREP_HERO_NUM;i++)
	{
		if (prepBattleArea[i]<0) continue;
		if(prepBattleArea[i]==heroId)
		{
			return true;
		}
	}
	return false ;
}

int CUser::GetCampHeroCount(Prep_Function pre, emElement e)
{
	const int16_t* prepBattleArea = GetHerosFromPrepBattleArea(pre);
	if (NULL == prepBattleArea)
	{
		return 0;
	}
	int herocount(0);
	for (int i=0; i<MAX_PREP_HERO_NUM;i++)
	{
		if (prepBattleArea[i]<0) continue;
		CHero* pHero = getHeroById(prepBattleArea[i]);
		if (!pHero)
		{
			continue;
		}
		if (herocount>=MAX_HERO_TEAM_NUM)
		{break; }
		if ( pHero->GetHerosTab()->ElementType==e )
		{
			herocount++;
		}
	}
	return herocount ;
}
// 
// bool CUser::CheckForbid()
// {
// 	if (eForbid_Reason_Not != _forbiduserinfo.reason )
// 	{
// 		if (_forbiduserinfo.forbidTime == -1) return true;
// 		uint32_t nowTime = (uint32_t)time(NULL);
// 		uint32_t firbidEndTime = _forbiduserinfo.forbidStartTime + _forbiduserinfo.forbidTime;
// 
// 		//封停时间还没到
// 		if (nowTime<firbidEndTime) return true; 
// 
// 		_forbiduserinfo.clear();
// 		//删除表中数据 未完成
// 		s_dbproxy.DeleteForbidUserInfo(propertys.userid);
// 		return false;
// 	}
// 	return false;
// }
// 
// void CUser::SetForbidUserInfo( ForbidUserInfo* data )
// {
// 	if (NULL == data)
// 	{
// 		LOGERR("CUser::SetForbidUserInfo NULL == data");
// 		return;
// 	}
// 	memcpy(&_forbiduserinfo,data,sizeof(_forbiduserinfo));
// }

//检测玩家是否可领取奖励
void CUser::CheckServerAward()
{
	func_start();
	CServerAwardMgr::Instance()->initdata();
	CServerAwardMgr::Instance()->updateServerAward();

	//清理过期无效数据
	{
		std::set<int64_t>::iterator itruseraward= userAward.begin();
		for (; itruseraward!=userAward.end();)
		{
			if (!CServerAwardMgr::Instance()->FindAwardInfo(*itruseraward))
			{
				userAward.erase(itruseraward++);
			}
			else
				++itruseraward;
		}
	}

	uint32_t nowTime = (uint32_t)time(NULL);
	VecAwardInfo& awards = CServerAwardMgr::Instance()->GetAwards();
	bool update=false;
	for (VecAwardInfo::iterator itr = awards.begin(); itr!=awards.end(); ++itr)
	{
		if(1==itr->delflg) //已经是删除标识的，就不用往下走了
			continue;

 		const AwardInfo& pAwardInfo = *itr; 
		
 		//已经领取过奖励
 		if (userAward.count(itr->awardId.toInt64())>0) continue;
 
 		//如果玩家符合领奖条件
 		if (!pAwardInfo.fitCondition(nowTime, propertys.lvl,propertys.register_time)) 
 			continue;

		//重置首充标识
		if(pAwardInfo.languageFlag==0xff &&  strcmp(pAwardInfo.msgData, "GMFirstChargeResetGM")==0)
		{
			userAward.insert(pAwardInfo.awardId.toInt64());
			//ResetFristChargeActivity(); //这句打开也不一定好用
			CChargeInfoMng& mng = GetChargeInfoMng();
			mng.ResetChargeCount();
			update = true ;
			continue;
		}

		//如果不是当前语言的奖励则设置为已领取，修改切换语言能重复领奖的bug
		// languageFlag == -1(0xff)时表示全部语言都发。
		if(pAwardInfo.languageFlag != 0xff && pAwardInfo.languageFlag != this->m_LanguageFlag)
		{
			userAward.insert(pAwardInfo.awardId.toInt64());
			update = true ;
			continue;
		}

		AffixData affixdata;
		email_id_t sysid;
		if (pAwardInfo.affixType != NULL)
		{
			affixdata.count = (std::min)((int32_t)pAwardInfo.affixCount, (int32_t)MAX_AFFIX_NUM);
			for(int16_t i=0; i<affixdata.count && i<MAX_AFFIX_NUM; ++i)
			{
				affixdata.item[i].affixType = pAwardInfo.affixType[i];
				affixdata.item[i].affixPara = pAwardInfo.affixPara[i];
				affixdata.item[i].affixNum  = pAwardInfo.affixNum[i];
			}
		}

		CSysMailMgr::CreateSysEmailId(sysid);
		CSysMailMgr::AddSysMailItem(propertys.userid,sysid,pAwardInfo.msgData,affixdata);

 		userAward.insert(pAwardInfo.awardId.toInt64());
		update = true ;
	}

	if(update)
		_update.updatetime[LOT_UserAward] = DataDirtyHelper::sDirtyStamp ++;

	func_end();
}

uint32_t CUser::memrcv(mem_buffer& rcvbuf)
{
	uint32_t rbytes = 0;

	rbytes += rcvbuf>>uoid;
	rbytes += rcvbuf>>key;
	rbytes += rcvbuf>>m_nBindAccountPlatState;
	rbytes += rcvbuf>>AccountPlatErrKey;
	rbytes += rcvbuf>>BindPlatCount;
	rbytes += rcvbuf>>propertys; 
	rbytes += rcvbuf>>m_LoadFlag;

	uint8_t herocount = 0;
	rbytes += rcvbuf>>herocount;

	CHero hero;
	MemHeroItem herodata;
	for (uint8_t i = 0;i<herocount;++i)
	{
		herodata.clear();
		hero.clear();

		rbytes += herodata.memrcv(rcvbuf);
  
		hero.init(herodata.hero);
		for(int j = 0; j < MAX_GEM_STAR_POSTION_COUNT; ++ j)
		{
			if(herodata.gem_info[j].m_tableID > 0)
				hero.AddHeroGemInfo((GemPos)j,herodata.gem_info[j]);
				
		}		
		hero.SetUpdatetime(herodata.updatetime);

		std::pair<HerosMap::iterator, bool> ins =  heros.insert(std::make_pair(hero.GetHeroTemplateID(), hero)) ;
		if (!ins.second) 
		{
			//LOGERR("CUser::addHero: ErrorHeroInfo(insert map fail) account=%s,userid=%lld", propertys.account.to_string(), propertys.userid);
			//return MT_ERR ;
		}
	}

	MemUserItems memuseritem;
	rbytes += memuseritem.memrcv(rcvbuf);

	vector<BagItemInfo>::iterator itr = memuseritem.VecBaseItemInfo.begin();
	for (;itr!=memuseritem.VecBaseItemInfo.end();++itr)
	{
		m_Bag.AddItem(itr->itemid,itr->itemnum);
	}

	MemUserGems memgem;
	rbytes+=memgem.memrcv(rcvbuf);
	vector<hero_gem_info>::iterator itrge = memgem.VecGemInfo.begin();
	m_Bag.CleanGemsInfo();
	for (;itrge!=memgem.VecGemInfo.end();++itrge)
	{
		m_Bag.AddGem(&*itrge);
	}

 	rbytes += rcvbuf>>userrmb; 
// 	rbytes += emailInfo.memrcv(rcvbuf);
 
 	rbytes += FBStatus.memrcv(rcvbuf);
 	rbytes += DailyMission.memrcv(rcvbuf);
 	rbytes += rcvbuf>>MissionData.GetData();
 	rbytes+= rcvbuf>>m_UserFlag;
	rbytes+= rcvbuf>>m_LanguageFlag;

//	rbytes += m_FriendMgr.memrcv(rcvbuf);
 	rbytes += _Dig.memrcv(rcvbuf);
 	rbytes += m_InviteComabtMng.memrcv(rcvbuf);
 	rbytes += ReStartFriendInfo.memrcv(rcvbuf);

 	rbytes += rcvbuf>>invite_head;
 	rbytes += rcvbuf>>invite_end;
 	rbytes += rcvbuf>>userEQLastRefTime;
 	rbytes += rcvbuf>>refurbishNum;
 	rbytes += rcvbuf>>refPoolStatus;

 	rbytes += m_SignMng.memrcv(rcvbuf);
 	rbytes += rcvbuf>>m_prepBattleMng.GetPrepBattleAreaData();

 	rbytes += rcvbuf>>m_ZodiacMng.GetZodiacInfoData();
	rbytes += rcvbuf>>m_ZodiacMng.GetZodiacEnemys();
	rbytes += rcvbuf>>m_ZodiacMng.m_VerifyNum ;

	uint16_t userawardcount = 0;
	rbytes += rcvbuf>>userawardcount;
 
	int64_t userawardid = -1;
	for (uint16_t i = 0;i<userawardcount;++i)
	{
		rbytes += rcvbuf>>userawardid;
		userAward.insert(userawardid);
	}

	rbytes += rcvbuf>>_guideinfo;
	rbytes += m_CombatFlow.memrcv(rcvbuf);
	rbytes += _arena.memrcv(rcvbuf);
	rbytes += _herotrial.memrcv(rcvbuf);
	rbytes += peakvs.memrcv(rcvbuf);
	rbytes += smallendless.memrcv(rcvbuf);
	rbytes += endless.memrcv(rcvbuf);
	rbytes += m_LastPvpAttackRecord.memrcv(rcvbuf);
	rbytes += rcvbuf>>_update;
	rbytes += rcvbuf>>m_LoginAward;
	rbytes += m_ChargeInfoMng.GetData().memrcv(rcvbuf);
	rbytes += m_ShopModule.memrcv(rcvbuf);
	for(int i = 0; i < eShop_max; ++ i)
	{
		for(int j = 0; j < TOTAL_ITEM_CNT; ++ j)
		{
			for(int k = 0; k < MAX_GEM_MAIN_ATT_COUNT+MAX_GEM_ADDITIONAL_ATT_COUNT; ++ k)
			{
				uint8_t nHasValue = 0;
				rbytes += rcvbuf >> nHasValue;
				if(nHasValue != 0)
				{
					rbytes += rcvbuf >> m_ShopGemRandomData[i][j][k];
				}
			}
		}
	}
	rbytes += m_TradeMgr.memrcv(rcvbuf);
	rbytes += m_Loot.memrcv(rcvbuf);

	//公告状态数据
	rbytes += rcvbuf >> m_noticelastIndex;
	rbytes += rcvbuf >> m_noticelastId;
	rbytes += rcvbuf.encapsulat(m_ChatNoticeCheckMask, MD5_MASK_LEN);
	
	//聊天
	rbytes += rcvbuf >> m_ChatMsgId;
	rbytes += rcvbuf >> m_LastChatTime;
	rbytes += rcvbuf >> m_LastChatTimeLaba;
	rbytes += _FirstRecharge.memrcv(rcvbuf);
	rbytes += _VipAward.memrcv(rcvbuf);
	rbytes += _CommonActivity.memrcv(rcvbuf);
#if defined(PAY_PLATFORM) && (PAY_PLATFORM==BUSINESS_PLATFORM_APPLE)
	rbytes += m_PaymentOrderInfoAppStore.memrcv(rcvbuf);
#endif

	rbytes += m_FriendAwardData.memrcv(rcvbuf);
	rbytes += m_HolidayItemExchange.memrcv(rcvbuf);

	rbytes += rcvbuf >> m_JYCombatCnt;
	rbytes += rcvbuf >> m_JYDropHunShiCnt;
	return rbytes;
}

uint32_t CUser::memsnd(mem_buffer& sndbuf)
{
	uint32_t sbytes = 0;
	sbytes += sndbuf<<uoid;
	sbytes += sndbuf<<key;
	sbytes += sndbuf<<m_nBindAccountPlatState;
	sbytes += sndbuf<<AccountPlatErrKey;
	sbytes += sndbuf<<BindPlatCount;
	sbytes += sndbuf<<propertys;
	sbytes += sndbuf<<m_LoadFlag;

	uint8_t herocount = (uint8_t)heros.size();
	sbytes += sndbuf<<herocount;

	MemHeroItem* hero;
	HerosMap::iterator itr = heros.begin();
	for (;itr!=heros.end();++itr)
	{
		itr->second.CalcCombatPower();
		hero = &itr->second; 
		if (hero)
			sbytes += hero->memsnd(sndbuf); 
	}

	MemUserItems memuseritem;
	m_Bag.GetItemInfos(memuseritem.VecBaseItemInfo); 
	sbytes +=memuseritem.memsnd(sndbuf);

	MemUserGems memgem;
	m_Bag.GetGemInfos(memgem.VecGemInfo);
	sbytes+=memgem.memsnd(sndbuf);

 	sbytes += sndbuf<<userrmb;
// 	sbytes += emailInfo.memsnd(sndbuf);
 	sbytes += FBStatus.memsnd(sndbuf);
 	sbytes += DailyMission.memsnd(sndbuf);

 	sbytes += sndbuf<<MissionData.GetData();
	sbytes += sndbuf<<m_UserFlag;
	sbytes += sndbuf<<m_LanguageFlag;

//	sbytes += m_FriendMgr.memsnd(sndbuf);
	sbytes += _Dig.memsnd(sndbuf);

 	sbytes += m_InviteComabtMng.memsnd(sndbuf);
 	sbytes += ReStartFriendInfo.memsnd(sndbuf);


	sbytes += sndbuf<<invite_head;
	sbytes += sndbuf<<invite_end;
	sbytes += sndbuf<<userEQLastRefTime;
	sbytes += sndbuf<<refurbishNum;
 	sbytes += sndbuf<<refPoolStatus;


 	sbytes += m_SignMng.memsnd(sndbuf);

 	sbytes += sndbuf<<m_prepBattleMng.GetPrepBattleAreaData();

// 	sbytes += sndbuf<<_maxcombatpower;
// 
// 	sbytes += sndbuf<<_forbiduserinfo;
 	sbytes += sndbuf<<m_ZodiacMng.GetZodiacInfoData();
	sbytes += sndbuf<<m_ZodiacMng.GetZodiacEnemys() ;
	sbytes += sndbuf<<m_ZodiacMng.m_VerifyNum ;

	uint16_t userawardcount = (uint16_t)userAward.size();
	sbytes += sndbuf<<userawardcount;

	std::set<int64_t>::iterator itruseraward = userAward.begin();
	for (;itruseraward!=userAward.end();++itruseraward)
	{
		sbytes += sndbuf<<*itruseraward;
	}

	sbytes += sndbuf<<_guideinfo;
	sbytes += m_CombatFlow.memsnd(sndbuf);
 	sbytes += _arena.memsnd(sndbuf);
	sbytes += _herotrial.memsnd(sndbuf);
	sbytes += peakvs.memsnd(sndbuf);
	sbytes += smallendless.memsnd(sndbuf);
	sbytes += endless.memsnd(sndbuf);
	sbytes += m_LastPvpAttackRecord.memsnd(sndbuf);

	sbytes += sndbuf<<_update;
	sbytes += sndbuf <<m_LoginAward;
	sbytes += m_ChargeInfoMng.GetData().memsnd(sndbuf);
	sbytes += m_ShopModule.memsnd(sndbuf);
	for(int i = 0; i < eShop_max; ++ i)
	{
		for(int j = 0; j < TOTAL_ITEM_CNT; ++ j)
		{
			for(int k = 0; k < MAX_GEM_MAIN_ATT_COUNT+MAX_GEM_ADDITIONAL_ATT_COUNT; ++ k)
			{
				uint8_t nHasValue = 0;
				if(m_ShopGemRandomData[i][j][k] != -1)
				{
					nHasValue = 1;
					sbytes += sndbuf << nHasValue;
					sbytes += sndbuf << m_ShopGemRandomData[i][j][k];
				}
				else
				{
					nHasValue = 0;
					sbytes += sndbuf << nHasValue;
				}
			}
		}
	}
	sbytes += m_TradeMgr.memsnd(sndbuf);
	sbytes += m_Loot.memsnd(sndbuf);
	//公告状态数据
	sbytes += sndbuf << m_noticelastIndex;
	sbytes += sndbuf << m_noticelastId;
	sbytes += sndbuf.commit(m_ChatNoticeCheckMask, MD5_MASK_LEN);	

	// 聊天
	sbytes += sndbuf << m_ChatMsgId;
	sbytes += sndbuf << m_LastChatTime;
	sbytes += sndbuf << m_LastChatTimeLaba;

	sbytes += _FirstRecharge.memsnd(sndbuf); 
	sbytes += _VipAward.memsnd(sndbuf); 
	sbytes += _CommonActivity.memsnd(sndbuf); 		
#if defined(PAY_PLATFORM) && (PAY_PLATFORM==BUSINESS_PLATFORM_APPLE)
	sbytes += m_PaymentOrderInfoAppStore.memsnd(sndbuf);
#endif
	
	sbytes += m_FriendAwardData.memsnd(sndbuf);
	sbytes += m_HolidayItemExchange.memsnd(sndbuf);
	
	sbytes += sndbuf << m_JYCombatCnt;
	sbytes += sndbuf << m_JYDropHunShiCnt;
	return sbytes;
}

size_t CUser::get_memsize()
{
	size_t ze = 0;

	ze += sizeof(uoid);
	ze += sizeof(key);
	ze += sizeof(m_nBindAccountPlatState);
	ze += sizeof(AccountPlatErrKey);
	ze += sizeof(BindPlatCount);
	ze += sizeof(propertys);
	ze += sizeof(m_LoadFlag);

	uint8_t herocount = (uint8_t)heros.size();
	ze += sizeof(herocount);

	HerosMap::iterator itr = heros.begin();
	for (;itr!=heros.end();++itr)
	{
	 ze +=itr->second.get_memsize();  
	}

	//背包
	MemUserItems memuseritem;
	m_Bag.GetItemInfos(memuseritem.VecBaseItemInfo); 
 	ze += memuseritem.get_memsize();

	MemUserGems memgem;
	m_Bag.GetGemInfos(memgem.VecGemInfo);
	ze += memgem.get_memsize();

 	ze += sizeof(userrmb);

//	ze += emailInfo.get_memsize();
	ze += FBStatus.get_memsize();
	ze += DailyMission.get_memsize();

	ze += sizeof(MissionData.GetData());
	ze += sizeof(m_UserFlag);
	ze += sizeof(m_LanguageFlag);

//	ze += m_FriendMgr.get_memsize();
	ze += _Dig.get_memsize();

	ze += m_InviteComabtMng.get_memsize();
	ze += ReStartFriendInfo.get_memsize();

	ze += sizeof(invite_head);
	ze += sizeof(invite_end);
	ze += sizeof(userEQLastRefTime);
	ze += sizeof(refurbishNum);
	ze += sizeof(refPoolStatus);

	ze += m_SignMng.get_memsize();

	ze += sizeof(m_prepBattleMng.GetPrepBattleAreaData());

// 	ze += sizeof(_maxcombatpower);
// 	ze += sizeof(_forbiduserinfo);
	ze += sizeof(m_ZodiacMng.GetZodiacInfoData());
	ze += sizeof(m_ZodiacMng.GetZodiacEnemys()) ;
	ze += sizeof(m_ZodiacMng.m_VerifyNum) ;

	uint16_t userawardcount = (uint16_t)userAward.size();
	ze += sizeof(userawardcount);
	ze += userawardcount*sizeof(int64_t);

 	ze += sizeof(_guideinfo);
	ze += m_CombatFlow.get_memsize();
	ze += _arena.get_memsize();
	ze += _herotrial.get_memsize();
	ze += peakvs.get_memsize();
	ze += smallendless.get_memsize();
	ze += endless.get_memsize();
	ze += m_LastPvpAttackRecord.get_memsize() ;
 	ze += sizeof(_update);
	ze += sizeof(m_LoginAward);
	ze += m_ChargeInfoMng.GetData().get_memsize() ;
	ze += m_ShopModule.get_memsize() ;
	for(int i = 0; i < eShop_max; ++ i)
	{
		for(int j = 0; j < TOTAL_ITEM_CNT; ++ j)
		{
			for(int k = 0; k < MAX_GEM_MAIN_ATT_COUNT+MAX_GEM_ADDITIONAL_ATT_COUNT; ++ k)
			{
				ze += sizeof(uint8_t);
				if(m_ShopGemRandomData[i][j][k] != -1)
				{
					ze += sizeof(m_ShopGemRandomData[i][j][k]);
				}
			}
		}
	}
	ze += m_TradeMgr.get_memsize() ;
	ze += m_Loot.get_memsize();
	//公告状态数据
	ze += sizeof(m_noticelastIndex);
	ze += sizeof(m_noticelastId);
	ze += sizeof(char)*MD5_MASK_LEN;
	ze += sizeof(m_ChatMsgId);
	ze += sizeof(m_LastChatTime);
	ze += sizeof(m_LastChatTimeLaba);
	ze += _FirstRecharge.get_memsize() ;
	ze += _VipAward.get_memsize() ;
	ze += _CommonActivity.get_memsize();
#if defined(PAY_PLATFORM) && (PAY_PLATFORM==BUSINESS_PLATFORM_APPLE)
	ze += m_PaymentOrderInfoAppStore.get_memsize() ;
#endif
	//// 			LOGDEBUG("CUser::memsnd: snd data account=%s,mac=%s,userid=%lld,username=%s,key=%u,uoid=%u,herocount=%u"
	//// 				, propertys.account.to_string(),propertys.macaddr.to_string(), propertys.userid,propertys.username,key,uoid,herocount);
	ze += m_FriendAwardData.get_memsize();
	ze += m_HolidayItemExchange.get_memsize();

	ze += sizeof(m_JYCombatCnt);
	ze += sizeof(m_JYDropHunShiCnt);
	return ze;
}

void CUser::SetUpdateTime(uint8_t model)
{
	if (model>=LOT_Max) return;
	_update.updatetime[model] = DataDirtyHelper::sDirtyStamp ++;
}

void CUser::SetLoginAwardInfo( LoginAwardInfo& Info )
{
	m_LoginAward = Info;
}

bool CUser::GetArenaInfo(ArenaUser& arena,int8_t type)
{
	//判断是否是攻击或防守阵容
	if (type!=PrepFunc_AreanDefend &&type!=PrepFunc_AreanAttack )
	{
		LOGERR("CUser::GetArenaInfo: type err!type=%d account=%s,userid=%lld",type, propertys.account.to_string(), propertys.userid);
		return false ;
	}

	prepBattleMng* prepbattle = GetPrepBattleMng();
	if (!prepbattle)
	{
		LOGERR("CUser::GetArenaInfo: prepbattle is null! account=%s,userid=%lld",propertys.account.to_string(), propertys.userid);
		return false ;
	}

	arena.clear();
	arena.userid = propertys.userid ;
	arena.teamleaderid = propertys.teamleaderid;
	strncpy(arena.username, propertys.username, sizeof(arena.username));
	arena.lvl = propertys.lvl;
	//arena.rank = CArenaSortMgr::Instance()->GetRankForUser(this);
	uint8_t uIndex = 0;
	for (int j=0; j<MAX_PREP_HERO_NUM;j++)
	{
		if (prepbattle->GetPrepBattleAreaData().m_prepBattle[type][j]<0) continue;
		CHero* pHero = getHeroById(prepbattle->GetPrepBattleAreaData().m_prepBattle[type][j]);
		if (!pHero) continue;
		if (uIndex >= MAX_HERO_PVPTEAM_NUM)
		{
			break;
		}
		arena.lineup.heros[uIndex].herodata = pHero->GetData();
		arena.lineup.heros[uIndex].pos = j;
		++uIndex;
	}
	arena.lineup.heronum = uIndex;
	return true;
}

bool CUser::GetShareData(UserShareData& data)
{
	//判断是否是攻击或防守阵容
	int8_t type = PrepFunc_AreanDefend ;
	prepBattleMng* prepbattle = GetPrepBattleMng();
	if (!prepbattle)
	{
		LOGERR("CUser::GetArenaInfo: prepbattle is null! account=%s,userid=%lld",propertys.account.to_string(), propertys.userid);
		return false ;
	}

	data.clear();
	data.userid = propertys.userid ;
	data.teamleaderid = propertys.teamleaderid;
	strncpy(data.username, propertys.username, sizeof(data.username));
	data.lvl = propertys.lvl;
	data.rank = 0 ;
	uint8_t uIndex = 0;
	for (int j=0; j<MAX_PREP_HERO_NUM;j++)
	{
		if (prepbattle->GetPrepBattleAreaData().m_prepBattle[type][j]<0) continue;
		CHero* pHero = getHeroById(prepbattle->GetPrepBattleAreaData().m_prepBattle[type][j]);
		if (!pHero) continue;
		if (uIndex >= MAX_HERO_PVPTEAM_NUM)
		{
			break;
		}
		data.lineup.heros[uIndex].herodata = pHero->GetData();
		data.lineup.heros[uIndex].pos = j;
		++uIndex;
	}
	data.lineup.heronum = uIndex;

	if(data.lineup.heronum <= 0)
	{
		type = PrepFunc_ComCombat ;
		uIndex = 0;
		for (int j=0; j<MAX_PREP_HERO_NUM;j++)
		{
			if (prepbattle->GetPrepBattleAreaData().m_prepBattle[type][j]<0) continue;
			CHero* pHero = getHeroById(prepbattle->GetPrepBattleAreaData().m_prepBattle[type][j]);
			if (!pHero) continue;
			if (uIndex >= MAX_HERO_PVPTEAM_NUM)
			{
				break;
			}
			data.lineup.heros[uIndex].herodata = pHero->GetData();
			data.lineup.heros[uIndex].pos = j;
			++uIndex;
		}
		data.lineup.heronum = uIndex;
	}
	return true;
}

bool CUser::GetGuildComatInfo(GuildComabtUser& data,int8_t type)
{
	//判断是否是攻击或防守阵容
	if (type <= PrepFunc_None || type >= PrepFunc_Max)
	{
		LOGERR("CUser::GetGuildComatInfo: type err!type=%d account=%s,userid=%lld",type, propertys.account.to_string(), propertys.userid);
		return false ;
	}

	prepBattleMng* prepbattle = GetPrepBattleMng();
	if (!prepbattle)
	{
		LOGERR("CUser::GetGuildComatInfo: prepbattle is null! account=%s,userid=%lld",propertys.account.to_string(), propertys.userid);
		return false ;
	}

	data.clear();
	data.userid = propertys.userid ;
	data.guildid = GetGuildID64() ;
	data.teamleaderid = propertys.teamleaderid;
	strncpy(data.username, propertys.username, sizeof(data.username));
	data.lvl = propertys.lvl;
	data.combatpower = 0 ;
	uint8_t uIndex = 0;
	for (int j=0; j<MAX_PREP_HERO_NUM;j++)
	{
		if (prepbattle->GetPrepBattleAreaData().m_prepBattle[type][j]<0) continue;
		CHero* pHero = getHeroById(prepbattle->GetPrepBattleAreaData().m_prepBattle[type][j]);
		if (!pHero) continue;
		if (uIndex >= MAX_HERO_PVPTEAM_NUM)
		{
			break;
		}
		data.lineup.heros[uIndex].herodata = pHero->GetData();
		data.lineup.heros[uIndex].pos = j;
		for(int i = 0;i < MAX_GEM_STAR_POSTION_COUNT; ++ i)
		{
			hero_gem_info tempgem;
			pHero->GetHeroGemInfo((GemPos)i,tempgem);
			data.gems[uIndex][i].copy(tempgem);
		}
		
		data.combatpower += CPropertyProxy::GetCombatPower(data.lineup.heros[uIndex].herodata,data.gems[uIndex]) ;
		++uIndex;
	}
	data.lineup.heronum = uIndex;
	if(data.lineup.heronum <= 0)
	{
		LOGERR("CUser::GetGuildComatInfo: no hero! account=%s,userid=%lld,type=%d",propertys.account.to_string(), propertys.userid, type);
		return false ;
	}
	return true;
}


bool CUser::SetDefaultArenaPrepBattle()
{
	const int16_t* pPba = GetPrepBattleMng()->GetHerosIDByFunc(PrepFunc_ComCombat);
	if (NULL ==pPba)
	{
		LOGERR("CUser::SetDefaultArenaPrepBattle:USER PBA err!account=%s,userid=%lld", propertys.account.to_string(), propertys.userid);
		return false ;
	}

	int16_t	 prepBattle[MAX_PREP_HERO_NUM] ;	//布阵区(不包含好友)
	memset(prepBattle,-1,sizeof(prepBattle));
	for (uint8_t i=0;i<MAX_PREP_HERO_NUM;++i)
	{
		prepBattle[i] = pPba[i];
	}

	//初始化默认阵容
	if(false == GetPrepBattleMng()->changePrepBattleAreaHeros(PrepFunc_AreanDefend,prepBattle,-1,-1,-1))
	{
		//目前初始化 玩家初始化没有阵容 调用 上面接口 会返回错误
		LOGERR("CUser::SetDefaultArenaPrepBattle,changePrepBattleAreaHeros() return failed !account=%s,userid=%lld", propertys.account.to_string(), propertys.userid);
		return false;
	}

	return true;
}

int8_t CUser::GetDiamondFreeTenLottery()
{
	time_t nCurrTime = time(0);
	struct tm nTmNow;
	ljlocaltime(&nCurrTime,&nTmNow);

	struct tm nTmRegDay;

	time_t nRegisterTime = GetUserData().register_time;

	ljlocaltime(&nRegisterTime,&nTmRegDay);

	if (nTmNow.tm_year == nTmRegDay.tm_year && nTmNow.tm_mon == nTmRegDay.tm_mon && nTmNow.tm_mday == nTmRegDay.tm_mday)
	{
		// 如果当前时间跟注册时间是同一天,提示客户端，第二天可以进行免费十连抽
		return eNextDayCanFreeLottery;
	}

	if (GetUserData().buy_diamonds == 0)
	{
		return eNowCanFreeLottery;
	}

	return eCanNotFreeLottery;
}

int8_t CUser::GetGemFreeTenLottery()
{
	time_t nCurrTime = time(0);
	struct tm nTmNow;
	ljlocaltime(&nCurrTime,&nTmNow);

	struct tm nTmRegDay;

	time_t nRegisterTime = GetUserData().register_time;

	ljlocaltime(&nRegisterTime,&nTmRegDay);

	if (nTmNow.tm_year == nTmRegDay.tm_year && nTmNow.tm_mon == nTmRegDay.tm_mon && nTmNow.tm_mday == nTmRegDay.tm_mday)
	{
		// 如果当前时间跟注册时间是同一天,提示客户端，第二天可以进行免费十连抽
		return eNextDayCanFreeLottery;
	}

	if (GetUserData().gem_buy_diamonds == 0)
	{
		return eNowCanFreeLottery;
	}

	return eCanNotFreeLottery;
}

int CUser::BuyLootCardByDiamond()
{
	func_start();
	Item* pItem = GetBag()->FindItem(LootCardID);
	if (pItem)
	{
		//当前拥有掠夺令数量已经达到存储上限
		if (pItem->GetNum() >= MaxItemNum)
		{
			return eBuyLootCardResult_MaxCount;
		}
	}
	//
	vip_tab* pVip = CVipMng::Instance()->GetVipTabInfoByID(GetVipLevel()) ;
	if(NULL == pVip)
	{
		return eBuyLootCardResult_Fail ;
	}
	//刷新购买掠夺卡次数
	RefBuyLootCardCount();
	//今日掠夺令购买次数已达上限
	if (m_Loot.buycardcount >= pVip->BuyLootCartTimes)
	{
		return eBuyLootCardResult_NoTimes ;
	}
	//
	if (0 == m_Loot.buycardlasttime)
	{
		if (0 != m_Loot.buycardcount)
		{
			LOGERR("CUser::BuyLootCardByDiamond, first buy physical,but buy_lootcard_count[%u] != 0.userid=%lld",
				m_Loot.buycardcount, propertys.userid);
			m_Loot.buycardcount = 0;
		}
	}
	//
	const BuyinCreaseData_t* pinCreaseData=CBuyinCreaseTabMgr::Instance()->GetData(m_Loot.buycardcount + 1);
	if (NULL == pinCreaseData)
	{
		return eBuyLootCardResult_Fail;
	}
	//扣除消费
	int32_t nRmb = GetRMB() ;
	if (nRmb < pinCreaseData->Diamond)
	{
		LOGERR("CUser::BuyLootCardByDiamond(),user_diamonds[%d] < NeedDiamond[%d]!userid=%lld",
			nRmb, pinCreaseData->Diamond, propertys.userid);
		return eBuyLootCardResult_DiamondNotEnough;
	}
	//申请 真金操作来源事件ID
	int64_t idOpSourceEvent = CDataMgr::Instance()->CreateOpSourceId((int32_t)RMBOS_SUB_BUY_LootCard) ;
	//组织消费接口参数
	SubRMBParam_t subRmbParam ;
	subRmbParam.source			= RMBOS_SUB_BUY_LootCard ;
	snprintf(subRmbParam.uniqueid, sizeof(subRmbParam.uniqueid), "%lld", idOpSourceEvent) ;
	subRmbParam.subvalue		= pinCreaseData->Diamond ;
	snprintf(subRmbParam.desc, sizeof(subRmbParam.desc), "buy lootcard cost(viplvl:%u)",GetVipLevel()) ;
	
	//备份
	UserRMB_t oldRMB_t = GetRMB_t();

	//扣除钻石
	int iRev = SubRMB(subRmbParam) ;
	if(0 != iRev)
	{
		LOGERR("CUser::BuyLootCardByDiamond(),sub rmb failed! CUser::SubRMB() return fail!(%d)userid=%lld,costrmb=%u,userrmb=%d",
			iRev, GetUserID(), pinCreaseData->Diamond, nRmb);
		return eBuyLootCardResult_Fail ;
	}
	
	//日志
	const UserRMB_t& newRMB_t = GetRMB_t();
	LOG_USER_PURCHASE(this, MoneyType_Diamond, subRmbParam.source, 0, 1, oldRMB_t.m_charge-newRMB_t.m_charge, oldRMB_t.m_donate-newRMB_t.m_donate, oldRMB_t.m_bindrmb-newRMB_t.m_bindrmb);

	//添加道具数量
	GetBag()->AddItem(LootCardID);
	//
	++m_Loot.buycardcount;
	time_t now = 0;
	time(&now);
	m_Loot.buycardlasttime = (uint32_t)now;
	return eBuyLootCardResult_Ok;
	func_end();
}

void CUser::RefBuyLootCardCount()
{
	uint32_t hour = 0; 
	uint32_t minute = 0; 
	if (!GetServerCdTimeByIndex(eCdTimeIndex_Loot, hour, minute))
	{
		LOGERR("CUser::RefBuyLootCardCount Fail,GetServerCdTimeByIndex is false.userid[%lld]",GetUserID());
		return;
	}
	if (0 != ComPareTimeWithHM(hour, minute, m_Loot.buycardlasttime, time(NULL)))
	{
		m_Loot.buycardcount = 0;
	}
}

int CUser::LootNoCombatByDiamond( bool useItem )
{
	func_start();
	uint32_t lootendtime =  m_Loot.nocombatendtime;
	uint32_t curtime = time(NULL);
	const ServerGameConfigData& _GameConfigData = CGameConfigMgr::Instance()->GetConfigData();
	if ((lootendtime != 0) && (lootendtime - curtime > _GameConfigData.AviodWarLimit))
	{
		//免战时间已达上限，无法继续免战
		return eLootNoCombatResult_MaxCount;
	}
	RefLootNoCombatCount();
	//
	if (0 == m_Loot.nocombatlasttime)
	{
		if (0 != m_Loot.nocombatcount)
		{
			LOGERR("CUser::LootNoCombatByDiamond, first buy physical,but nocombatcount[%u] != 0.userid=%lld",
				m_Loot.nocombatcount, propertys.userid);
			m_Loot.nocombatcount = 0;
		}
	}
	int32_t SubDiamond = 0;
	int32_t nRmb =0;
	if (useItem)
	{
		Item* NoCombatItem = GetBag()->FindItem(NoCombatCardID);
		if (NoCombatItem)
		{
			if (NoCombatItem->GetNum() < 1)
			{
				//道具数量不足失败
				LOGERR("CUser::LootNoCombatByDiamond, item[%d] Num[%d] < 1.userid=%lld",
					NoCombatCardID, NoCombatItem->GetNum(),propertys.userid);
				return eLootNoCombatResult_NoItem;
			}
		}
		else
		{
			//道具不存在失败
			LOGERR("CUser::LootNoCombatByDiamond, item[%d] is null.userid=%lld",NoCombatCardID,propertys.userid);
			return eLootNoCombatResult_NoItem;
		}
	}
	else
	{
		vip_tab* pVip = CVipMng::Instance()->GetVipTabInfoByID(GetVipLevel()) ;
		if(NULL == pVip)
		{
			return eLootNoCombatResult_Fail ;
		}
		//今日掠夺免战购买次数已达上限(只限制为花钻石购买，不限制使用免战令)
		if (m_Loot.nocombatcount >= pVip->LootNoCombatTimes)
		{
			LOGERR("CUser::LootNoCombatByDiamond(),nocombatcount[%d] > MaxTimes[%d]!userid=%lld",
				m_Loot.nocombatcount, pVip->LootNoCombatTimes, propertys.userid);
			return eLootNoCombatResult_NoTimes ;
		}
		//判断钻石数是否充足
		const BuyinCreaseData_t* pinCreaseData=CBuyinCreaseTabMgr::Instance()->GetData(m_Loot.nocombatcount + 1);
		if (NULL == pinCreaseData)
		{
			return eLootNoCombatResult_Fail;
		}
		//扣除消费
		nRmb = GetRMB() ;
		if (nRmb < pinCreaseData->LootNoCombat)
		{
			LOGERR("CUser::LootNoCombatByDiamond(),user_diamonds[%d] < NeedDiamond[%d]!userid=%lld",
				nRmb, pinCreaseData->LootNoCombat, propertys.userid);
			return eLootNoCombatResult_DiamondNotEnough;
		}
		SubDiamond = pinCreaseData->LootNoCombat;
	}
	//条件成立
	if (useItem)
	{
		//扣除道具
		GetBag()->SubItem(NoCombatCardID);
		UpdateBagInfoToDB() ;
	}
	else
	{
		//申请 真金操作来源事件ID
		int64_t idOpSourceEvent = CDataMgr::Instance()->CreateOpSourceId((int32_t)RMBOS_SUB_BUY_LootNoCombat) ;
		//组织消费接口参数
		SubRMBParam_t subRmbParam ;
		subRmbParam.source			= RMBOS_SUB_BUY_LootCard ;
		snprintf(subRmbParam.uniqueid, sizeof(subRmbParam.uniqueid), "%lld", idOpSourceEvent) ;
		subRmbParam.subvalue		= SubDiamond ;
		snprintf(subRmbParam.desc, sizeof(subRmbParam.desc), "buy loot nocombat cost(viplvl:%u)",GetVipLevel()) ;
		
		//备份
		UserRMB_t oldRMB_t = GetRMB_t();

		//扣除钻石
		int iRev = SubRMB(subRmbParam) ;
		if(0 != iRev)
		{
			LOGERR("CUser::BuyLootCardByDiamond(),sub rmb failed! CUser::SubRMB() return fail!(%d)userid=%lld,costrmb=%u,userrmb=%d",
				iRev, GetUserID(), SubDiamond, nRmb);
			return eLootNoCombatResult_Fail ;
		}
		
		//日志
		const UserRMB_t& newRMB_t = GetRMB_t();
		LOG_USER_PURCHASE(this, MoneyType_Diamond, subRmbParam.source, 0, 1, oldRMB_t.m_charge-newRMB_t.m_charge, oldRMB_t.m_donate-newRMB_t.m_donate, oldRMB_t.m_bindrmb-newRMB_t.m_bindrmb);
	}
	//添加免战时间
	if (0 == m_Loot.nocombatcount)
	{
		m_Loot.nocombatendtime = time(NULL) + _GameConfigData.AvoidWarTime;
	}
	else
	{
		m_Loot.nocombatendtime += _GameConfigData.AvoidWarTime;
	}
	//免战次数++
	m_Loot.nocombatlasttime = time(NULL);
	m_Loot.nocombatcount++;
	//
	if (useItem == 1)
	{
		SetUpdateTime(LOT_UserItems);
		SetUpdateTime(LOT_UserEquip);
	}
	SetUpdateTime(LOT_Loot);
	UPDATE_USER_DATA(this);
	return eLootNoCombatResult_Ok;
	func_end();
}

void CUser::RefLootNoCombatCount()
{
	uint32_t hour = 0; 
	uint32_t minute = 0; 
	if (!GetServerCdTimeByIndex(eCdTimeIndex_Loot, hour, minute))
	{
		LOGERR("CUser::RefLootNoCombatCount Fail,GetServerCdTimeByIndex is false.userid[%lld]",GetUserID());
		return;
	}
	if (0 != ComPareTimeWithHM(hour, minute, m_Loot.nocombatlasttime, time(NULL)))
	{
		m_Loot.nocombatcount = 0;
	}
}
void CUser::FillLootUserInfo(LootUser & pvpuserinfo)
{
	pvpuserinfo.userid = GetUserID();
	pvpuserinfo.teamleaderid = GetTeamLeader();
	strncpy(pvpuserinfo.username, GetUserData().username, sizeof(pvpuserinfo.username));
	pvpuserinfo.lvl = GetLevel();
	prepBattleArea& prepbattlearea = GetPrepBattleMng()->GetPrepBattleAreaData();
	uint8_t uIndex = 0;
	for (int j=0; j<MAX_PREP_HERO_NUM;j++)
	{
		if (prepbattlearea.m_prepBattle[PrepFunc_ComCombat][j]<0) 
		{
			continue;
		}
		CHero* pHero = getHeroById(prepbattlearea.m_prepBattle[PrepFunc_ComCombat][j]);
		if (NULL == pHero)
		{
			continue;
		}
		if (uIndex >= MAX_HERO_PVPTEAM_NUM)
		{
			break;
		}
		pvpuserinfo.lineup.heros[uIndex].herodata = pHero->GetData();
		pvpuserinfo.lineup.heros[uIndex].pos = j;
		++uIndex;
	}
	pvpuserinfo.lineup.heronum = uIndex;
	pvpuserinfo.type = LootUserType_Player ;
}
/*
	一键装备
*/
int32_t CUser::EquipItemOneKey( int32_t heroid ,int8_t* equipstatue,int32_t needtotalglods, BagItemSync& bgsync)
{
	func_start();
	if (NULL == equipstatue)
	{
		LOGERR("EquipItemOneKey Fail,NULL == equipstatue,userid[%lld]",GetUserID());
		return HeroEquipReturn_Fail;
	}
	//英雄是否存在
	CHero* pHero = getHeroById(heroid);
	if (NULL == pHero)
	{
		LOGERR("EquipItemOneKey Fail,NULL == pHero,heroid[%d],userid[%lld]",heroid,GetUserID());
		return HeroEquipReturn_Fail;
	}
	int32_t	nheroLevel = pHero->GetLvl();
	const HerosTab_t* pHeroTab = CHerosTabMgr::Instance()->GetHeroData(pHero->GetHeroTemplateID());
	if (NULL == pHeroTab)
	{
		LOGERR("EquipItemOneKey Fail,NULL == pHeroTab,heroid[%d],HeroTemplateID[%d],userid[%lld]",heroid,pHero->GetHeroTemplateID(),GetUserID());
		return HeroEquipReturn_Fail;
	}
	Bag		NeedItems;
	int32_t	NeedGolds = 0;
	//进化次数
	int32_t nUpgradeTimes = pHero->GetUpgradeTimes();
	//判断装备道具的有效性
	for (int idx =0;idx<MAX_HERO_EQUIPPOS;idx++)
	{
		//判断状态有效性
		if (equipstatue[idx] < HeroEquipStatue_None || equipstatue[idx] >= HeroEquipStatue_Max)
		{
			LOGERR("EquipItemOneKey Fail,equipstatue[%d] error,userid[%lld]",equipstatue[idx],GetUserID());
			return HeroEquipReturn_StatueError;
		}
		if (equipstatue[idx] == HeroEquipStatue_None)
		{
			continue;
		}
		//已经装备道具
		hero_equippos_info* pEquipPosInfo = pHero->GetHeroEquipPosInfo((EquipPos)idx);
		if (pEquipPosInfo->equipid != -1)
		{
			LOGERR("EquipItemOneKey Fail,already equip item,userid[%lld]",GetUserID());
			return HeroEquipReturn_AlreadyEquipItem;
		}
		//需要装备的道具id
		int32_t equipid = pHeroTab->NeedEquipIDForEvolution[nUpgradeTimes][idx];
		//
		equipment_tab* pEquiptab = CEquipmentTabMng::Instance()->GetEquipmentTabInfoByID(equipid);
		if (NULL == pEquiptab)
		{
			LOGERR("EquipItemOneKey Fail,NULL == pEquiptab,equipid[%d],userid[%lld]",equipid,GetUserID());
			return HeroEquipReturn_Fail;
		}
		//装备等级不足
		if (pEquiptab->equiplv > nheroLevel )
		{
			LOGERR("EquipItemOneKey Fail,pEquiptab->equiplv[%d] > nheroLevel[%d],userid[%lld]",pEquiptab->equiplv,nheroLevel,GetUserID());
			return HeroEquipReturn_LevelNotEnough;
		}
		//装备道具
		if (equipstatue[idx] == HeroEquipStatue_Equip) 
		{
			Item* pItem = m_Bag.FindItem(equipid);
			if (NULL == pItem)
			{
				LOGERR("EquipItemOneKey Fail,can not find item[%d] in bag for Equip,userid[%lld]",equipid,GetUserID());
				return HeroEquipReturn_NotFindItem;
			}
			NeedItems.AddItem(equipid);
		}
		else if (equipstatue[idx] == HeroEquipStatue_Compose) 
		{
			//合成道具
			//已经存在道具失败
			Item* pItem = m_Bag.FindItem(equipid);
			if (NULL != pItem)
			{
				Item* pTemp = NeedItems.FindItem(equipid);
				if (NULL != pTemp)
				{
					if (pTemp->GetNum() < pItem->GetNum())
					{
						LOGERR("EquipItemOneKey Fail,already find item[%d] in bag for Compose,num[%d],neednum[%d],userid[%lld]",equipid, pItem->GetNum(),pTemp->GetNum(),GetUserID());
						return HeroEquipReturn_Fail;
					}
				}
				else
				{
					LOGERR("EquipItemOneKey Fail,already find item[%d] in bag for Compose,userid[%lld]",equipid,GetUserID());
					return HeroEquipReturn_Fail;
				}
			}

			if (!FindItemForEquip(equipid,1,NeedItems,NeedGolds))
			{
				LOGERR("EquipItemOneKey Fail,FindItemForEquip fail,equipid[%d],userid[%lld]",equipid,GetUserID());
				return HeroEquipReturn_Fail;
			}
		}
	}
	//判断金币是否充足
	if (GetGolds() < NeedGolds)
	{
		LOGERR("EquipItemOneKey Fail,gold not enough,golds[%d],needgolds[%d],userid[%lld]",GetGolds(),NeedGolds,GetUserID());
		return HeroEquipReturn_GoldsNotEnough;
	}
	//客户端和服务器数据不匹配
	if (NeedGolds != needtotalglods)
	{
		LOGERR("EquipItemOneKey Fail,server needgolds[%d],client needgolds[%d],userid[%lld]",NeedGolds,needtotalglods,GetUserID());
		return HeroEquipReturn_Fail;
	}
	//判断道具是否充足
	vector<BagItemInfo> vecItems;
	NeedItems.GetItemInfos(vecItems);
	vector<BagItemInfo>::iterator it = vecItems.begin();
	for (; it!= vecItems.end();it++)
	{
		Item* pItem = m_Bag.FindItem(it->itemid);
		if (NULL == pItem)
		{
			LOGERR("EquipItemOneKey Fail, ------item[%d] not in bag,userid[%lld]",it->itemid,GetUserID());
			return HeroEquipReturn_NotFindItem;
		}
		if (pItem->GetNum() < it->itemnum)
		{
			LOGERR("EquipItemOneKey Fail,item[%d] num[%d] not enough,neednum[%d],userid[%lld]",it->itemid,pItem->GetNum(),it->itemnum,GetUserID());
			return HeroEquipReturn_ItemNotEnough;
		}
	}
	//条件成立,扣除金币
	SubGolds(NeedGolds);
	LOG_USER_PURCHASE(this, MoneyType_Gold, GOLDOS_SUB_EQUIP_ONE_KEY, //一键穿戴消耗
				0, 1, NeedGolds, 0, 0);
	//条件成立,删除道具
	it = vecItems.begin();
	for (; it!= vecItems.end();it++)
	{
		m_Bag.SubItem(it->itemid,it->itemnum);
		FixBagSynInfoBuySub(this, it->itemid, bgsync);
		UpdateBagInfoToDB() ;
	}
	//装备道具
	for (int idx =0;idx<MAX_HERO_EQUIPPOS;idx++)
	{
		if ((equipstatue[idx]==HeroEquipStatue_Equip) ||(equipstatue[idx]==HeroEquipStatue_Compose))
		{
			int32_t addequipid = pHeroTab->NeedEquipIDForEvolution[nUpgradeTimes][idx];
			pHero->AddHeroEquipInfo((EquipPos)idx,addequipid,0);
		}
	}
	UPDATE_USER_DATA(this);
	return HeroEquipReturn_Ok;
	func_end();
}

bool CUser::FindItemForEquip( int32_t equipid,int16_t num,Bag& nNeedBag, int32_t& NeedGolds)
{
	func_start();

	///////////////////
	equipment_tab* pEquiptab = CEquipmentTabMng::Instance()->GetEquipmentTabInfoByID(equipid);
	if (NULL == pEquiptab)
	{
		//条件不成立
		return false;
	}
	//此物品不能合成
	if ( pEquiptab->equipIDForComposeNum==0 )
	{
		return false ;
	}
	for(int8_t i=0; i<pEquiptab->equipIDForComposeNum; ++i)
	{
		Item* pItem = m_Bag.FindItem(pEquiptab->equipIDForCompose[i].itemID);
		if(pItem==NULL)
		{
			return false ;
		}
		//物品数量不够
		if(pItem->GetNum() < pEquiptab->equipIDForCompose[i].itemNum )
		{
			return false ;
		}
		nNeedBag.AddItem(pEquiptab->equipIDForCompose[i].itemID, pEquiptab->equipIDForCompose[i].itemNum);
	}
	//需要的金币
	NeedGolds += pEquiptab->composeneedglod;	
	return true ;
	///////////////////
	/*
	bool bAddItem = false;
	Item* pItem = m_Bag.FindItem(equipid);
	if (NULL != pItem)
	{
		Item* pTempItem = nNeedBag.FindItem(equipid);
		if (NULL != pTempItem)
		{
			if (pTempItem->GetNum() < pItem->GetNum())
			{
				bAddItem = true;
			}
		}
		else
		{
			bAddItem = true;
		}
	}
	if (!bAddItem)
	{
		equipment_tab* pEquiptab = CEquipmentTabMng::Instance()->GetEquipmentTabInfoByID(equipid);
		if (NULL == pEquiptab)
		{
			//条件不成立
			return false;
		}
		else
		{
			//装备合成装备
			if (0 != pEquiptab->equipIDForComposeNum)
			{
				for (int ii=0;ii<MaxEquipNumForCompose;ii++)
				{
					if (0 == pEquiptab->equipIDForCompose[ii].itemID)
					{
						continue;
					}
					if (!FindItemForEquip(pEquiptab->equipIDForCompose[ii].itemID,1,nNeedBag,NeedGolds))
					{
						return false;
					}
				}
			}
			else
			{
				//碎片合成装备
				if (0 != pEquiptab->chipIDForCompose)
				{
					if (!FindItemForEquip(pEquiptab->chipIDForCompose,pEquiptab->composeneednum,nNeedBag,NeedGolds))
					{
						return false;
					}
				}
			}
			//计算合成装备需要的金币数量
			NeedGolds += pEquiptab->composeneedglod;
			return true;
		}
	}
	else
	{
		nNeedBag.AddItem(equipid,num);
		return true;
	}
	return false;*/
	func_end();
	return false ;	
}

bool CUser::_FindItemForEquip( int32_t equipid,int16_t num,Bag& nNeedBag, int32_t& NeedGolds)
{
	func_start();
	int16_t _MyBagNum = 0 ;
	int16_t _NeedNum = num ;
	Item* pItem = m_Bag.FindItem(equipid);
	if (NULL != pItem)
	{
		Item* pTempItem = nNeedBag.FindItem(equipid);
		if (NULL != pTempItem)
		{
			if ((pTempItem->GetNum() + num) > pItem->GetNum())
			{
				_MyBagNum = pItem->GetNum() - pTempItem->GetNum() ;
				_NeedNum = pTempItem->GetNum() + num - pItem->GetNum() ;
			}
			else
			{
				_MyBagNum = num ; 
				_NeedNum = 0 ;
			}
		}
		else
		{
			if (pItem->GetNum() >= num)
			{
				_MyBagNum = num;
				_NeedNum = 0 ;
			}
			else
			{
				_MyBagNum = pItem->GetNum() ;
				_NeedNum = num - _MyBagNum ;
			}
		}
	}

	if(_MyBagNum > 0)
	{
		nNeedBag.AddItem(equipid, _MyBagNum);
	}
	if(_NeedNum <= 0)
	{
		return true ;
	}

	equipment_tab* pEquiptab = CEquipmentTabMng::Instance()->GetEquipmentTabInfoByID(equipid);
	if (NULL == pEquiptab)
	{
		//条件不成立
		return false;
	}

	//装备合成装备
	if (0 != pEquiptab->equipIDForComposeNum)
	{
		//计算合成装备需要的金币数量
		NeedGolds += pEquiptab->composeneedglod;
		for (int ii=0;ii<MaxEquipNumForCompose;ii++)
		{
			if (0 == pEquiptab->equipIDForCompose[ii].itemID)
			{
				continue;
			}
			if (!FindItemForEquip(pEquiptab->equipIDForCompose[ii].itemID,1,nNeedBag,NeedGolds))
			{
				return false;
			}
		}
	}
	else if (0 != pEquiptab->chipIDForCompose)
	{
		//计算合成装备需要的金币数量
		NeedGolds += pEquiptab->composeneedglod;
		//碎片合成装备
		if (!FindItemForEquip(pEquiptab->chipIDForCompose,pEquiptab->composeneednum,nNeedBag,NeedGolds))
		{
			return false;
		}
	}
	return false;
	func_end();
}


int8_t CUser::GetMonthCardState(time_t now)
{
    func_start();

	

	if(0 == propertys.monthcardbuytime)
		return 0 ;
	uint32_t _uNow = (uint32_t)now ;
	if(_uNow < propertys.monthcardbuytime)
		return 1 ;

	uint32_t _hour = 0; 
	uint32_t _minute = 0; 
	GetServerCdTimeByIndex(eCdTimeIndex_DaliyQuest, _hour, _minute);
	if (0 == ComPareTimeWithHM(_hour, _minute, propertys.monthcardbuytime, now, 30))
		return 1 ;
	return 0 ;
	func_end();
}

int8_t CUser::GetYearCardState(time_t now)
{
    func_start();
	if(0 == propertys.yearcardbuytime)
		return 0 ;
	uint32_t _uNow = (uint32_t)now ;
	if(_uNow < propertys.yearcardbuytime)
		return 0 ;

	uint32_t _hour = 0; 
	uint32_t _minute = 0; 
	GetServerCdTimeByIndex(eCdTimeIndex_DaliyQuest, _hour, _minute);
	if (0 == ComPareTimeWithHM(_hour, _minute, propertys.yearcardbuytime, now, ONE_YEAR_DAY))
		return 1 ;
	return 0 ;
	func_end();
}

int8_t CUser::GetNewMonthCardState(time_t now)
{
    func_start();

	if(0 == propertys.newmonthcardbuytime)
		return 0 ;
	uint32_t _uNow = (uint32_t)now ;
	if(_uNow < propertys.newmonthcardbuytime)
		return 1 ;

	uint32_t _hour = 0; 
	uint32_t _minute = 0; 
	GetServerCdTimeByIndex(eCdTimeIndex_DaliyQuest, _hour, _minute);
	if (0 == ComPareTimeWithHM(_hour, _minute, propertys.newmonthcardbuytime, now, 30))
		return 1 ;
	return 0 ;
	func_end();
}

int32_t CUser::GetMonthCardRemaind(time_t now)
{
	if(1 != GetMonthCardState(now))
		return 0 ;

	uint32_t _hour = 0; 
	uint32_t _minute = 0; 
	GetServerCdTimeByIndex(eCdTimeIndex_DaliyQuest, _hour, _minute);
	uint32_t _uNow = (uint32_t)now ;

	if(1 == GetYearCardState(now))
	{
		_uNow = propertys.yearcardbuytime + ONE_YEAR_SECOND ;
	}
	return _GetTimeRemaind(_uNow, _hour, _minute, propertys.monthcardbuytime, propertys.monthcarddrawtime, 30) ;
}

int32_t CUser::GetYearCardRemaind(time_t now)
{
 	if(1 != GetYearCardState(now))
		return 0 ;
	uint32_t _hour = 0; 
	uint32_t _minute = 0; 
	GetServerCdTimeByIndex(eCdTimeIndex_DaliyQuest, _hour, _minute);
	return _GetTimeRemaind(now, _hour, _minute, propertys.yearcardbuytime, propertys.yearcarddrawtime, ONE_YEAR_DAY) ;
}

int32_t CUser::GetNewMonthCardRemaind(time_t now)
{
 	if(1 != GetNewMonthCardState(now))
		return 0 ;
	uint32_t _hour = 0; 
	uint32_t _minute = 0; 
	GetServerCdTimeByIndex(eCdTimeIndex_DaliyQuest, _hour, _minute);
	return _GetTimeRemaind(now, _hour, _minute, propertys.newmonthcardbuytime, propertys.newmonthcarddrawtime, 30) ;
}

void CUser::DoBuyYearCard(time_t now)
{
	uint32_t _hour = 0; 
	uint32_t _minute = 0; 
	GetServerCdTimeByIndex(eCdTimeIndex_DaliyQuest, _hour, _minute);

	if(1 == GetMonthCardState(now))
	{
		uint32_t _addtime = ONE_YEAR_SECOND ;
		if(0 != propertys.monthcarddrawtime)
		{
			if (0 == ComPareTimeWithHM(_hour, _minute, propertys.monthcarddrawtime, now))
			{
				_addtime -= ONE_DAY_SECOND ;
			}
		}
		//if((0 != propertys.monthcarddrawtime) && (propertys.monthcarddrawtime > propertys.monthcardbuytime))
		if(0 != propertys.monthcarddrawtime)
			propertys.monthcarddrawtime += _addtime ;	
		propertys.monthcardbuytime += _addtime ;
	}
	propertys.yearcardbuytime = (uint32_t)now ;
}

//uint32_t CUser::GetMonthCardBuyTime(time_t now)
//{
//	return 0 ;
//}
//uint32_t CUser::GetMonthCardDrawTime(time_t now)
//{
//	if(0 != propertys.monthcardbuytime)
//	{
//		uint32_t _hour = 0; 
//		uint32_t _minute = 0; 
//		GetServerCdTimeByIndex(eCdTimeIndex_DaliyQuest, _hour, _minute);
//		if(0 != propertys.yearcardbuytime)
//		{
//			if (0 == ComPareTimeWithHM(_hour, _minute, propertys.monthcardbuytime, propertys.yearcardbuytime, 30))
//			{
//				if(1 != GetYearCardState(now))
//				{
//					if((0 != propertys.monthcarddrawtime) && (propertys.monthcarddrawtime < (propertys.monthcardbuytime + ONE_YEAR_SECOND)))
//					{
//						//在自己的有效期内 有 购买年卡, 并且年卡用完
//						//将月卡的最后领取时间 推后 一年
//						return propertys.monthcarddrawtime + ONE_YEAR_SECOND;
//					}
//				}
//			}
//		}
//	}
//	return propertys.monthcarddrawtime ;
//}

int32_t CUser::_GetTimeRemaind(time_t now, uint32_t hour, uint32_t minute, uint32_t buytime, uint32_t drawtime, uint32_t days)
{
	uint32_t _hour = hour; 
	uint32_t _minute = minute; 
	uint32_t _cd_tmp = _hour * ONE_HOUR_SEC + _minute * ONE_MIN_SEC ;

	tm *_pTm_now = localtime(&now);
	uint32_t _now_tmp = _pTm_now->tm_hour * ONE_HOUR_SEC + _pTm_now->tm_min * ONE_MIN_SEC + _pTm_now->tm_sec ;

	time_t _t_buyTime = (time_t)buytime ;
	tm *_pTm_buyTime = localtime(&_t_buyTime);
	uint32_t _buyTime_tmp = _pTm_buyTime->tm_hour * ONE_HOUR_SEC + _pTm_buyTime->tm_min * ONE_MIN_SEC + _pTm_buyTime->tm_sec ;

	time_t _t_drawTime = (time_t)drawtime ;
	tm *_pTm_drawTime = localtime(&_t_drawTime);
	uint32_t _drawTime_tmp = _pTm_drawTime->tm_hour * ONE_HOUR_SEC + _pTm_drawTime->tm_min * ONE_MIN_SEC + _pTm_drawTime->tm_sec ;

	uint32_t _endAddDays = days ;
	if(_buyTime_tmp < _cd_tmp)
		_endAddDays = _endAddDays - 1 ;
	uint32_t _uEndTime = buytime - _buyTime_tmp + _cd_tmp + (_endAddDays * ONE_DAY_SECOND) ;

	uint32_t _uStart_nowTime = ((uint32_t)now) - _now_tmp + _cd_tmp ;
	if(_now_tmp < _cd_tmp)
		_uStart_nowTime = _uStart_nowTime - ONE_DAY_SECOND ;
	uint32_t _uStart_Time = _uStart_nowTime ;

	if(0 != drawtime)
	{
		uint32_t _uStart_drawTime = drawtime - _drawTime_tmp + _cd_tmp ;
		if(_drawTime_tmp >= _cd_tmp)
			_uStart_drawTime = _uStart_drawTime + ONE_DAY_SECOND ;

		if(_uStart_drawTime > _uStart_Time)
			_uStart_Time = _uStart_drawTime ;
	}
	//time_t _t_pTms = (time_t)_uEndTime ;
	//tm *_pTm_s = localtime(&_t_pTms);

	//time_t _t_pTme = (time_t)_uStart_Time ;
	//tm *_pTm_e = localtime(&_t_pTme);
	//int64_t tmptime = (int64_t)difftime(_uStart_Time, _uEndTime) ;
	int32_t _days = (_uEndTime - _uStart_Time) / ONE_DAY_SECOND ;
	int32_t _secs = (_uEndTime - _uStart_Time) % ONE_DAY_SECOND ;
	return _days ;
}

void CUser::ExecFristChargeActivity()
{
	if (!CheckUserMask(eMask_FirstChargeActivityFinish))
	{
		SetUserMask(eMask_FirstChargeActivityFinish);
	}
}
void CUser::ResetFristChargeActivity()
{
	if (CheckUserMask(eMask_FirstChargeActivityFinish)==true)
	{
		UnSetUserMask(eMask_FirstChargeActivityFinish);
	}

	if (CheckUserMask(eMask_FirstChargeRewarded)==true)
	{
		UnSetUserMask(eMask_FirstChargeRewarded);
	}
}
void CUser::ExecLoginTimeActivity(bool newuser)
{
	{//应策划要求，暂时关闭，上线送钻石的邮件功能 liugang
		return ;
	}
	//登录成功，执行每日登录活动
	ActivityData* pActivityData = CActivityDataTabMgr::Instance()->GetData(eActivityType_login);
	if (!pActivityData)
	{
		return;
	}

	if (true != CActivityDataTabMgr::Instance()->CheckOpen(eActivityType_login))
	{
		LOGINFO("====ExecLoginTimeActivity,but not in activity time!");
		return;
	}

	uint32_t hour = 0,minute = 0;
	CCdTimeTabMng::Instance()->GetServerCdTimeByIndex(eCdTimeIndex_LoginReward,hour,minute);
	if (newuser || 0 != ComPareTimeWithHM(hour,minute,propertys.last_login_time,time(0)))
	{
		SetLoginTimes(GetLoginTimes() + 1);

		bool bIsFind = false;
		size_t idx = 0;
		for (; idx < pActivityData->mCnt; ++idx)
		{
			if (pActivityData->mFinishNum[idx] >= GetLoginTimes())
			{
				bIsFind = true;
				break;
			}
		}
		
		if (bIsFind)
		{
			int nDiamondNum = pActivityData->mDropId[idx];

			//发送奖励(邮件)
			MemSystemEmailData _MailData ;
			CSysMailMgr::CreateSysEmailId(_MailData.sysid);
			snprintf(_MailData.msgData, sizeof(_MailData.msgData)-1, "~!%d|0_%d", 3, nDiamondNum) ;
			_MailData.msg_len  = (strlen(_MailData.msgData) > sizeof(_MailData.msgData)-1)?sizeof(_MailData.msgData)-1:strlen(_MailData.msgData) ;
			_MailData.affixStat = eAffixStatus_UnDone;
			_MailData.affixdata.count = 1 ;
			_MailData.affixdata.item[0].affixType = eOutPutAwardType_Diamond ;
			_MailData.affixdata.item[0].affixNum = nDiamondNum;
			_MailData.emailReadStatus = eEmailReadStatus_UnDone;
			_MailData.sendTime = (uint32_t)time(NULL) ;
			if(!CSysMailMgr::AddSysMailItem(GetUserID(), _MailData, true))
			{
				LOGERR("PktCS_ReqLogin, CSysMailMgr::AddSysMailItem return fail! diamon=%d, userid=%lld!", nDiamondNum, GetUserID());
			}
		}
	}
}

void CUser::ExecLevelUpActivtiy(uint8_t oldlv, uint8_t newlv)
{
	//执行冲级送道具活动
	const ActivityData* pActivityData = CActivityDataTabMgr::Instance()->GetData(eActivityType_levelup);
	if (!pActivityData)
	{
		return;
	}

	if (true != CActivityDataTabMgr::Instance()->CheckOpen(eActivityType_levelup))
	{
		LOGINFO("====ExecLevelUpActivtiy,but not in activity time!");
		return;
	}

	if (pActivityData->mCnt > MAX_LEVELUP_STAGE)
	{
		LOGERR("现在还不支持等级段格数[%u]超过[%u]",pActivityData->mCnt,MAX_LEVELUP_STAGE);
		return;
	}

	if(_FirstRecharge.DateRecord==0 )
	{
		_FirstRecharge.DateRecord = time(NULL);

		for(uint32_t i=2; i<=GetLevel(); ++i)
		{
			ExecLevelUpActivtiy(i-1, i);
		}
	}
	_FirstRecharge.Cnt = pActivityData->mCnt;
	for (size_t idx = 0; idx < pActivityData->mCnt; ++idx)
	{
		if (oldlv < pActivityData->mFinishNum[idx] && newlv >= pActivityData->mFinishNum[idx])
		{
			_FirstRecharge.Status[idx] = eLevelUpActivityStatus_NoGet;
		}
	}

	SetUpdateTime(LOT_FirstRecharge);
}

void CUser::ExecLevelUpActivtiy(uint8_t activityType, uint8_t oldlv, uint8_t newlv)
{
	//冲级送礼类型判断
	if ( eActivityType_newyear_levelup != activityType
		&&eActivityType_levelup != activityType)
		return;

	//执行冲级送道具活动
	const ActivityData* ptabActivityData = CActivityDataTabMgr::Instance()->GetData(activityType);
	if (!ptabActivityData)
	{
		return;
	}

	//活动未开放
	if (true != CActivityDataTabMgr::Instance()->CheckOpen(activityType))
		return;

	if (ptabActivityData->mCnt > MAX_ACTIVITY_STAGE)
	{
		LOGERR("现在还不支持等级段格数[%u]超过[%u]",ptabActivityData->mCnt,MAX_LEVELUP_STAGE);
		return;
	}

	ActivityAwardData* pActivityData = & _CommonActivity.szActivity[activityType];
	if ( pActivityData==NULL)
	{
		mtAssert(false) ;
		return ;
	}

	if(pActivityData->Param[0]==0 )
	{
		pActivityData->Param[0] = time(NULL);

		for(uint32_t i=2; i<=GetLevel(); ++i)
		{
			ExecLevelUpActivtiy(activityType, i-1, i);
		}
	}

	pActivityData->Cnt = ptabActivityData->mCnt;
	for (size_t idx = 0; idx < pActivityData->Cnt; ++idx)
	{
		if (oldlv < ptabActivityData->mFinishNum[idx] && newlv >= ptabActivityData->mFinishNum[idx] && pActivityData->Status[idx]==eAtivityAwardStatus_NoReach)
		{
			pActivityData->Status[idx] = eLevelUpActivityStatus_NoGet;
		}
	}

	SetUpdateTime(LOT_Activity);
}

//更新VIP冲级送礼奖励状态
void CUser::ExecVipLevelUpActivtiy(uint8_t activityType, uint8_t oldlv, uint8_t newlv)
{
	//VIP冲级送礼类型判断
	if ( eActivityType_newyear_viplevelup != activityType)
		return;

	//执行冲级送道具活动
	const ActivityData* ptabActivityData = CActivityDataTabMgr::Instance()->GetData(activityType);
	if (!ptabActivityData)
	{
		return;
	}

	if (true != CActivityDataTabMgr::Instance()->CheckOpen(activityType))
	{
		return;
	}

	if (ptabActivityData->mCnt > MAX_ACTIVITY_STAGE)
	{
		LOGERR("现在还不支持等级段格数[%u]超过[%u]",ptabActivityData->mCnt,MAX_LEVELUP_STAGE);
		return;
	}

	ActivityAwardData* pActivityData = & _CommonActivity.szActivity[activityType];
	if ( pActivityData==NULL)
	{
		mtAssert(false) ;
		return ;
	}

	if(pActivityData->Param[0]==0 )
	{
		pActivityData->Param[0] = time(NULL);

		for(uint32_t i=2; i<=GetVipLevel(); ++i)
		{
			ExecVipLevelUpActivtiy(activityType, i-1, i);
		}
	}

	pActivityData->Cnt = ptabActivityData->mCnt;
	for (size_t idx = 0; idx < pActivityData->Cnt; ++idx)
	{
		if (oldlv < ptabActivityData->mFinishNum[idx] && newlv >= ptabActivityData->mFinishNum[idx] && pActivityData->Status[idx]==eAtivityAwardStatus_NoReach)
		{
			pActivityData->Status[idx] = eLevelUpActivityStatus_NoGet;
		}
	}

	SetUpdateTime(LOT_Activity);
}


void CUser::CheckLevelUpActivityDate()
{
	const ActivityData* pTab = CActivityDataTabMgr::Instance()->GetData(eActivityType_levelup);
	if (!pTab)
	{
		return;
	}

	if(_FirstRecharge.DateRecord!=0)
	{
		time_t now =  time(NULL);
		//每天一次的玩法
		if ( pTab->mTimeRule==eActivityTimeRule_1 /*&& CompareDateWithYMD(now, _FirstRecharge.DateRecord)!=0*/ )
		{
			uint32_t nPreGetAwardTime = _FirstRecharge.DateRecord;// 上一次得奖的时间。
			uint32_t nActivityStartTime = pTab->mStartTime;// 活动的开始时间
			uint8_t nPreGetAwardDayIndex = (nPreGetAwardTime - nActivityStartTime) / ONE_DAY_SECOND;// 得到上次得奖是在第几天
			uint8_t nNowDayIndex = (now - nActivityStartTime) / ONE_DAY_SECOND;// 当前是在第几天

			if(nPreGetAwardDayIndex != nNowDayIndex)
			{
				_FirstRecharge.clear();	
			}
					
		}

		//多天一次的玩法
		if(pTab->mTimeRule==eActivityTimeRule_2 )
		{
			if(CActivityDataTabMgr::Instance()->CheckOpen(eActivityType_levelup)!=true) //没有在活动时间内
			{
				_FirstRecharge.clear();	
			}
			else //活动时间内
			{
				uint32_t startTime = server.open_server_time + pTab->mOpenServerBeginSec ;
				uint32_t endTime = server.open_server_time + pTab->mOpenServerEndSec ;

				if (_FirstRecharge.DateRecord >= startTime && _FirstRecharge.DateRecord <= endTime)
				{
				}
				else
				{
					_FirstRecharge.clear();
				}
			}
		}
	}
	else
	{
		_FirstRecharge.clear();
	}
	ExecLevelUpActivtiy(0, 0);
	ExecLevelUpActivtiy(eActivityType_newyear_levelup, 0, 0);
}

void CUser::CheckActivityDate(eActivityType type)
{
	if(eActivityType_login==type || eActivityType_levelup==type)
	{
		return ;
	}
	
	ActivityAwardData* pAwardData = NULL;
	
	if(type>=eActivityType_zhaoyunitem && type<eActivityType_Num)
	{
		pAwardData = & _CommonActivity.szActivity[type];
	}
	
	if(pAwardData==NULL)
	{
		return ;
	}

	ActivityData* pTab = CActivityDataTabMgr::Instance()->GetData(type);
	if (pTab==NULL)
	{
		return ;
	}
	pAwardData->Cnt = pTab->mCnt;	
	
	if(pAwardData->Param[0]!=0)
	{
		time_t now =  time(NULL);
		//每天一次的玩法
		if ((pTab->mTimeRule==eActivityTimeRule_1  || pTab->mTimeRule==eActivityTimeRule_3) /*&& CompareDateWithYMD(now, pAwardData->Param[0])!=0*/ )
		{
			uint32_t nPreGetAwardTime = pAwardData->Param[0];// 上一次得奖的时间。
			uint32_t nActivityStartTime = pTab->mStartTime;// 活动的开始时间
			uint8_t nPreGetAwardDayIndex = (nPreGetAwardTime - nActivityStartTime) / ONE_DAY_SECOND;// 得到上次得奖是在第几天
			uint8_t nNowDayIndex = (now - nActivityStartTime) / ONE_DAY_SECOND;// 当前是在第几天

			if(nPreGetAwardDayIndex != nNowDayIndex)
			{
				pAwardData->clearStatus();
				pAwardData->clearParam();	
			}

					
		}
		//多天一次的玩法
		if(pTab->mTimeRule==eActivityTimeRule_2 )
		{
			if(CActivityDataTabMgr::Instance()->CheckOpen(type)!=true) //没有在活动时间内
			{
				pAwardData->clearStatus();
				pAwardData->clearParam();
			}
			else //活动时间内
			{
				uint32_t startTime = server.open_server_time + pTab->mOpenServerBeginSec ;
				uint32_t endTime = server.open_server_time + pTab->mOpenServerEndSec ;

				if (pAwardData->Param[0] >= startTime && pAwardData->Param[0] <= endTime)
				{
				}
				else
				{
					pAwardData->clearStatus();
					pAwardData->clearParam();
				}
			}
		}
		//固定天数
		if(pTab->mTimeRule==eActivityTimeRule_0 )
		{
			if (pAwardData->Param[0] < pTab->mStartTime ||  
				pAwardData->Param[0] > pTab->mEndTime ||
				CActivityDataTabMgr::Instance()->CheckOpen(type)!=true)
			{
				pAwardData->clearStatus();
				pAwardData->clearParam();
			}
		}
	}
	else
	{
		pAwardData->clearStatus();
		pAwardData->clearParam();
	}
}

void CUser::ExecActivity(eActivityType activityType, int16_t addcnt,  int8_t activityParam)
{
	if(eActivityType_zhaoyunitem== activityType
		//eActivityType_dailycharge == activityType || 
		//eActivityType_usediamond == activityType ||
		//eActivityType_buygold == activityType
		)
	{
		return ;
	}

	const ActivityData* pTab = CActivityDataTabMgr::Instance()->GetData(activityType);
	if (!pTab)
	{
		return;
	}
	if(activityType>=eActivityType_zhaoyunitem && activityType<eActivityType_Num)
	{}
	else
	{
		return ;
	}

	char* szActiviyTitle[]={"上线送豪礼","冲级送道具", "赵云碎片收集", "每日充值", "钻石消耗", "炼金活动","抽卡", "血战竞技场","6元送钻石","累计登录天数发奖品", "普通战斗产出翻倍", "竞技场产出翻倍", "星座爬塔产出翻倍", "大无尽产出翻倍", "节日送礼", "新年连续登陆", "新年冲级", "新年vip冲级"};


	if ( sizeof(szActiviyTitle)/sizeof(szActiviyTitle[0]) !=eActivityType_Num )		 
	{
		mtAssert(false) ;
		return ;
	}
	
	ActivityAwardData* pActivityData = & _CommonActivity.szActivity[activityType];
	if ( pActivityData==NULL)
	{
		mtAssert(false) ;
		return ;
	}

	if (true != CActivityDataTabMgr::Instance()->CheckOpen(activityType))
	{
		LOGDEBUG("====%s,but not in activity time!", szActiviyTitle[activityType]);
		return;
	}

	if (pTab->mCnt > MAX_ACTIVITY_STAGE)
	{
		LOGERR("现在还不支持 %s 收集格数[%u]超过[%u]",szActiviyTitle[activityType], pTab->mCnt,MAX_ACTIVITY_STAGE);
		return;
	}
	
	switch(activityType)
	{
	case eActivityType_zhaoyunitem://赵云碎片
	case eActivityType_dailycharge://每日充值
	case eActivityType_usediamond://钻石消耗
	case eActivityType_buygold://炼金活动
	case eActivityType_pvp:
	case eActivityType_6rmb:
		{
			//数量
			int32_t oldcnt = pActivityData->Param[1];
			int32_t newcnt = oldcnt + addcnt;
			pActivityData->Param[1] = newcnt;

			if(pActivityData->Param[0]==0 )
			{
				pActivityData->Param[0] = time(NULL);
			}

			pActivityData->Cnt = pTab->mCnt;
			for (size_t idx = 0; idx < pTab->mCnt; ++idx)
			{
				if (oldcnt < pTab->mFinishNum[idx] && newcnt >= pTab->mFinishNum[idx] && pActivityData->Status[idx]==eAtivityAwardStatus_NoReach )
				{
					pActivityData->Status[idx] = eAtivityAwardStatus_NoGet;
				}
			}

			SetUpdateTime(LOT_Activity);
			break ;
		}
	case eActivityType_addlogin7dayaward:
	case eActivityType_festivalgift:	//节日送礼活动
		{
			//时间
			uint32_t now = time(NULL);
			if(pActivityData->Param[2]>0 && 
				CompareDateWithYMD(now, pActivityData->Param[2])==0 )
			{
				break ;
			}
			pActivityData->Param[2] = now;

			//数量
			int32_t oldcnt = pActivityData->Param[1];
			int32_t newcnt = oldcnt + addcnt;
			pActivityData->Param[1] = newcnt;
			
			if(pActivityData->Param[0]==0 )
			{
				pActivityData->Param[0] = time(NULL);
			}

			pActivityData->Cnt = pTab->mCnt;
			for (size_t idx = 0; idx < pTab->mCnt; ++idx)
			{
				if (oldcnt < pTab->mFinishNum[idx] && newcnt >= pTab->mFinishNum[idx] && pActivityData->Status[idx]==eAtivityAwardStatus_NoReach )
				{
					pActivityData->Status[idx] = eAtivityAwardStatus_NoGet;
				}
			}

			SetUpdateTime(LOT_Activity);
			break ;
		}
	case eActivityType_herocard:
		{
			//数量
			if(activityParam==AwardType_Gold)
			{
				int32_t oldcnt = pActivityData->Param[1];
				int32_t newcnt = oldcnt + addcnt;
				pActivityData->Param[1] = newcnt;

				if(pActivityData->Param[0]==0 )
				{
					pActivityData->Param[0] = time(NULL);
				}

				pActivityData->Cnt = pTab->mCnt;
				for (size_t idx = 0; idx < pTab->mCnt && idx<2; ++idx)
				{
					if (oldcnt < pTab->mFinishNum[idx] && newcnt >= pTab->mFinishNum[idx] && pActivityData->Status[idx]==eAtivityAwardStatus_NoReach )
					{
						pActivityData->Status[idx] = eAtivityAwardStatus_NoGet;
					}
				}
			}
			else if (activityParam==AwardType_Diamond)
			{
				int32_t oldcnt = pActivityData->Param[2];
				int32_t newcnt = oldcnt + addcnt;
				pActivityData->Param[2] = newcnt;

				if(pActivityData->Param[0]==0 )
				{
					pActivityData->Param[0] = time(NULL);
				}

				pActivityData->Cnt = pTab->mCnt;
				for (size_t idx = 2; idx < pTab->mCnt; ++idx)
				{
					if (oldcnt < pTab->mFinishNum[idx] && newcnt >= pTab->mFinishNum[idx] && pActivityData->Status[idx]==eAtivityAwardStatus_NoReach )
					{
						pActivityData->Status[idx] = eAtivityAwardStatus_NoGet;
					}
				}
			}
			
			SetUpdateTime(LOT_Activity);
			break ;
		}
	case eActivityType_newyear_login:
		{
			uint32_t  timenow = time(NULL);	//现在的时间

			if(pActivityData->Param[0]==0 )
			{
				pActivityData->Param[0] = timenow;
			}

			//判断时间
			if (timenow < pTab->mStartTime || timenow > pTab->mEndTime)
				break;
			
			//判断奖励奖励索引
			uint8_t awardindex = (timenow - pTab->mStartTime)/ONE_DAY_SECOND;
			if (awardindex >= pTab->mCnt)
				break;

			pActivityData->Cnt = pTab->mCnt;

			if(pActivityData->Status[awardindex]==eAtivityAwardStatus_NoReach)
			{
				pActivityData->Status[awardindex] = eAtivityAwardStatus_NoGet;
			}

			SetUpdateTime(LOT_Activity);
			break ;
		}
		break;
	}

}

//更新新年活动信息
void CUser::UpdateNewYearActivity()
{
	func_start();
	CheckActivityDate(eActivityType_newyear_login);
	CheckActivityDate(eActivityType_newyear_levelup);
	CheckActivityDate(eActivityType_newyear_viplevelup);

	ExecActivity(eActivityType_newyear_login, 1 );
	ExecLevelUpActivtiy(eActivityType_newyear_levelup, 0,GetLevel());
	ExecVipLevelUpActivtiy(eActivityType_newyear_viplevelup, 0, GetVipLevel());

	UPDATE_USER_DATA(this);
	func_end();
}


uint8_t CUser::Rename(const char* username)
{
	func_start();
	// 先看他是第几次改名
	int times = GetRenameTimes();
	if (times > 0)
	{
		// 扣钱判断，如果钱不够返回false
		int32_t nRmb = GetRMB() ;
		int32_t nRenameDiamond = CGameConfigMgr::Instance()->GetConfigData().RenameDiamond;
		if (nRmb < nRenameDiamond)
		{
			LOGERR("CUser::Rename(),user_diamonds[%d] < NeedDiamond[%d]!userid=%lld",
				nRmb, nRenameDiamond, propertys.userid);
			return eModifyUserNameResult_DiamondNotEnough;
		}

		//申请 真金操作来源事件ID
		int64_t idOpSourceEvent = CDataMgr::Instance()->CreateOpSourceId((int32_t)RMBOS_SUB_RENAME) ;

		//组织消费接口参数
		SubRMBParam_t subRmbParam ;
		subRmbParam.source			= RMBOS_SUB_RENAME ;
		snprintf(subRmbParam.uniqueid, sizeof(subRmbParam.uniqueid), "%lld", idOpSourceEvent) ;
		//subRmbParam.uniqueid		= idOpSourceEvent ;
		subRmbParam.subvalue		= nRenameDiamond ;

		//备份
		UserRMB_t oldRMB_t = GetRMB_t();

		//扣除钻石
		int iRev = SubRMB(subRmbParam) ;
		if(0 != iRev)
		{
			return eModifyUserNameResult_DiamondNotEnough ;
		}
		
		//日志
		const UserRMB_t& newRMB_t = GetRMB_t();
		LOG_USER_PURCHASE(this, MoneyType_Diamond, subRmbParam.source, 0, 1, oldRMB_t.m_charge-newRMB_t.m_charge, oldRMB_t.m_donate-newRMB_t.m_donate, oldRMB_t.m_bindrmb-newRMB_t.m_bindrmb);
	}
	// 执行改名操作
	SetUserName(username);
	SetRenameTimes(times + 1);
	if (!server.memcached_enable)
	{
		s_dbproxy.UpDateUserInfo(&propertys);
	}

	SetUpdateTime(LOT_UserSort);
	return eModifyUserNameResult_Ok;
	func_end();
}

void CUser::SetVipExp(float vipexp, bool ischarge) 
{ 
	if (ischarge || GetVipLevel() > 0)
	{
		propertys.vip_exp = vipexp;
		CalcVipLevel();
		static int32_t sgMaxVipLv = CVipMng::Instance()->GetMaxVipLvl();
		if (propertys.vip_lvl >= sgMaxVipLv)
		{
			vip_tab* ptab = CVipMng::Instance()->GetVipTabInfoByID(sgMaxVipLv);
			if (ptab)
			{
				propertys.vip_exp = 1.f * ptab->NeedVipExp;
			}
		}
	}
}

void CUser::LoginAddVipExp()
{
	time_t nTimeNow = time(0);
	uint32_t hour = 0; 
	uint32_t minute = 0; 
	GetServerCdTimeByIndex(eCdTimeIndex_VipReward, hour, minute);
	if (0 != ComPareTimeWithHM(hour, minute, propertys.last_login_time, nTimeNow))
	{
		if (GetVipLevel() > 0)
		{
			SetVipExp(GetVipExp() + CGameConfigMgr::Instance()->GetConfigData().LoginVipExp, CGameConfigMgr::Instance()->GetConfigData().AddVipExpAnything);
		}
	}
}

//guild
int32_t CUser::GetGuildCon()
{//仅供显示
	if (GetGuildID().toInt64() != 0)
	{//有公会
		GuildMemberList* pMemberList = CDataMgr::Instance()->GetGuildMemberList(GetGuildID().toInt64());
		if (pMemberList)
		{
			GuildMember* pMember = pMemberList->GetMemberByUserId(GetUserID());
			if (pMember)
			{
				return pMember->m_Contribute;
			}
		}
	}
	return 0;
}


