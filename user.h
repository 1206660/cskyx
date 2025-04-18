/**********************************************************
   user.h
   mt-svr

   Created by cui on 13-11-27.
   Copyright (c) 2013年 Locojoy. All rights reserved.

   说明:
       玩家定义
************************************************************/

#ifndef mt_svr_user_h
#define mt_svr_user_h

#include <stdint.h>
#include <map>
#include <set>
#include <vector>
#include <string>
//#include "mt_debug.h"
#include "memlock.h"
#include "payplatform.h"
#include "hash_map_wapper.h"
#include "userdef.h"
#include "utilpool.h"
#include "msgdef.h"
#include "email.h"
#include "shop.h"
#include "hero.h"
#include "combatdef.h"
#include "fbstate.h"
#include "friendmng.h"
#include "userrmbdef.h"
#include "dig.h"
#include "awarddef.h"
#include "paymentdef.h"
#include "invitecombatmng.h"
#include "usermgr.h"
#include "bag.h"
#include "dailymission.h"
#include "mission.h"
#include "signmng.h"
#include "prepbattlemng.h"
#include "combatflow.h"
#include "zodiacmng.h"
#include "serveraward.h"
#include "guidedef.h"
#include "datamgr.h"
#include "globalconfig.h"
#include "arenadef.h"
#include "arena.h"
#include "arena_a.h"
#include "loginawarddef.h"
#include "herotrialmng.h"
#include "chargeinfomng.h"
#include "firstrecharge.h"
#include "vipaward.h"
#include "leveuptab.h"
#include "peakvs.h"
#include "smallendless.h"
#include "endless.h"
#include "logsdkdef.h"
#include "activityaward.h"
#include "activitytab.h"
#include "payordermgr.h"
#include "friendaward.h"
#include "trademgr.h"
#include "sortdef.h"
#include "holidayitemexchangedef.h"
#include "memmgr.h"

/*后面的uint64_t本来应该是uint32_t类型,因为现在只支持64bit的数值,所以先改成64位的数了*/
typedef std::map<uint16_t, CHero> HerosMap;
typedef std::set<uint16_t> HeroIdSet;
typedef std::vector<CHero *> HeroObjVector;

//将int8_t数组转换为int32
int32_t g_Int8ArrayToInt32(int8_t* ids, int8_t nsize) ;
//将int32数组转换为int8数组
int	g_Int32ToInt8Array(int32_t nValue, int8_t* ids, int8_t nsize) ;
//只用于机器人生成guid
int64_t	GenerateRobotGuid();
int64_t	GeneratePeakVsRobotGuid();
int64_t	GetFirstRobotGuid();
bool	IsRobot(int64_t userid);
class mem_buffer;
class CombatFlow;
class CShopMng;
class CUser
{
public:
    DECLARE_UOID(CUser);
	/////////////////////////////////////////////////////////////
    CUser(void);
    ~CUser(void);
    /////////////////////////////////////////////////////////////
    bool match_key(uint32_t _key) const {
        return (_key == key);
    }

	bool IsValid()
	{
		if (uoid<0)
			return false;
		return true;
	}

	std::string getDebugString(void) ;

	void set_uoid(uint32_t _uoid){uoid=_uoid;}
	uint32_t get_uoid(){return uoid;}
	
	uint32_t key;
    /////////////////////////////////////////////////////////////
    /*创建角色时初始化函数*/
//	int init(const account_t& account,const account_t& clientuuid, const char* username,uint8_t constellation,int64_t userid,int64_t plataccountid,const char* plataccountname);
	int init(const char* szDeviceID,const char* username,uint8_t constellation,int64_t userid,const char* szChannel,const char* szPlatformId,int32_t serverId);

	/*服务器重启后, 用户首次登录后初始化*/
	int init();
	/*设置玩家数据载入标识*/
	void SetLoadFlag(eUserLoadFlag flag);
	eUserLoadFlag GetLoadFlag();

	/*初始化基础信息*/
	int InitUserBaseInfoToMem();

	/*注册时将新生成的用户信息写入数据库*/
	int InitUserInfoToMem();

	/*注册时将新生成的用户附带信息写入数据库*/
	int WriteAdditionalDataToDB(void);

	/*注册时初始化英雄并写入数据库*/
	int WriteInitHeroToDB(void);

	/*注册时初始化道具并写入数据库*/
	int WriteInitItemToDB(void);

	/*登录时到数据库查询用户信息*/
	int QueryUserInfo(account_t& clientmac);
	int QueryUserInfo(int64_t userid);
	int QueryUserInfo(const char* szChannel,const char* szPlatformId , int32_t serverId);

	/*查询加载除user表之外的表*/
	int QueryUserAllInfo(int64_t userid);

	//查询加载邮件数据
    int QueryUserEmailInfo();

	int64_t GetUserID(void) const { return propertys.userid; }
	int64_t GetPlatAccountID(void) const { return propertys.PlatAccountID; }
	void SetConstellation(uint8_t constellation) 
	{ 
		propertys.constellation = constellation;
	}
	void SetLvl(uint8_t lvl) 
	{ 
		if(propertys.lvl != lvl)
		{
			propertys.lvl = lvl;
			LOG_USER_LEVEL_UP(this);
		}
		
	}
	//account_t& GetUserAccount() { return propertys.account;}
	account_t& GetMac() { return propertys.macaddr;}
	const char* GetUserAccountStr()  { return propertys.account.to_string();}
	const char* GetUserName() const { return propertys.username;}
	const char* GetPlatAccountName() const { return propertys.PlatAccountName;}
	const char* GetPlatformId()const { return propertys.PlatformId;}
	void SetUserid(int64_t userid) { propertys.userid = userid;}
	void SetPlatAccountID(int64_t PlatAccountID) { propertys.PlatAccountID = PlatAccountID;}

	void SetChannelId(int32_t channelId) { propertys.ChannelId = channelId; }
	const int32_t GetChannelId() { return propertys.ChannelId; }

	void SetServerId(int32_t serverId) {propertys.ServerId = serverId;}
	const int32_t GetServerId() { return propertys.ServerId;}
	
	void SetPlatAccountName(char* PlatAccountName) 
	{ 
		if (PlatAccountName)
		{
			strncpy(propertys.PlatAccountName, PlatAccountName, sizeof(propertys.PlatAccountName));
		}
	}

	uint32_t GetBindPlatCount()
	{
		return BindPlatCount;
	}

	void SetBindPlatCount(uint32_t count)
	{
		if (count > MAX_BINDPLAT_COUNT)
		{
			BindPlatCount = 0;
		}
		else
		{
			BindPlatCount = count;
		}
	}
	uint32_t GetAccountPlatErrKey()
	{
		return AccountPlatErrKey;
	}

	void SetAccountPlatErrKey(int32_t key)
	{
		AccountPlatErrKey = key;
	}

  	bool IsBindPlat() 
  	{ 
  		return propertys.PlatAccountID > 0;
  	}

	user_data_t& GetUserData(void) { return propertys; }
	void SubGolds(int32_t golds)
	{
		if ( golds > propertys.golds )
			propertys.golds = 0;
		else
			propertys.golds -= golds;
	}

	void AddGolds(int32_t golds)
	{
		if ( golds > 0 )
		{
			propertys.golds += golds;
			if (propertys.golds > MAX_PLAYER_GLOD_NUM)
			{
				propertys.golds = MAX_PLAYER_GLOD_NUM;
			}
		}
	}
	void SetGolds(int32_t golds) 
	{
		propertys.golds = golds; 
		if (propertys.golds > MAX_PLAYER_GLOD_NUM)
		{
			propertys.golds = MAX_PLAYER_GLOD_NUM;
		}
		if (propertys.golds < 0)
		{
			propertys.golds = 0;
		}
	}


	void SubEvilDiamonds(int32_t golds)
	{
		if ( golds > propertys.evilDiamond )
			propertys.evilDiamond = 0;
		else
			propertys.evilDiamond -= golds;
	}

	void AddEvilDiamonds(int32_t golds)
	{
		if ( golds > 0 )
		{
			propertys.evilDiamond += golds;
			if (propertys.evilDiamond > MAX_PLAYER_EVIL_DIAMOND_NUM)
			{
				propertys.evilDiamond = MAX_PLAYER_EVIL_DIAMOND_NUM;
			}
		}
	}
	void SetEvilDiamonds(int32_t golds) 
	{
		propertys.evilDiamond = golds; 
		if (propertys.evilDiamond > MAX_PLAYER_EVIL_DIAMOND_NUM)
		{
			propertys.evilDiamond = MAX_PLAYER_EVIL_DIAMOND_NUM;
		}
		if (propertys.evilDiamond < 0)
		{
			propertys.evilDiamond = 0;
		}
	}


	void SetUserId(int64_t id) { propertys.userid = id; }

	void AddEQ(uint32_t val);

	//用于助战
	void AddEQ(uint8_t friendStatus,int64_t friendId);
	void SubEQ(uint32_t val)
	{
		propertys.EQ -= val;
		if(propertys.EQ < 0)
		{
			propertys.EQ = 0;
		}
	}
	int32_t GetEQ()
	{
		return propertys.EQ;
	}

	void SetAccount(const char* pAccount)
	{	
		uuid_string_t strAccount;
		memset(strAccount, 0, sizeof(uuid_string_t));
		strncpy((char *)&strAccount, pAccount, sizeof(uuid_string_t));
		propertys.account = strAccount;
	}

	void SetLastLoginTime(uint32_t time)
	{
		propertys.last_login_time = time;
	}

	int32_t  GetGolds() const
	{
		return propertys.golds;
	}

	eBindAccountPlatState  GetBindAccountPlatState(void) const {
		return (eBindAccountPlatState)m_nBindAccountPlatState;
	}
	void SetBindAccountPlatState(eBindAccountPlatState _us) {
		m_nBindAccountPlatState = _us;
	}

	//公会 guild begin
	void		SetGuildId(int32_t serverid,int32_t guildid){ propertys.guild_server_id = serverid;propertys.guild_id = guildid;};
	void		SetGuildId(int64_t guild){ guild_id_t guildid(guild); propertys.guild_server_id = guildid.getSvrId();propertys.guild_id = guildid.getGuildId(); }
	int32_t		GetGuildCon();		//获得公会贡献值
	guild_id_t	GetGuildID(){ return guild_id_t(propertys.guild_server_id,propertys.guild_id);}
	int64_t		GetGuildID64(){ return GetGuildID().toInt64(); }
	bool		IsInGuild(){ return propertys.guild_id > 0;}
	void		ExitGuild(){ propertys.guild_server_id = 0;propertys.guild_id = 0; }
	//guild end
	//////////////////////////////////////////////////////////////////////////
	/*清除用户所有信息*/
	void clear();

	/*更新英雄数据*/
	int UpdateHeroToDB(int16_t id);

	int UpdateTimeToDB(bool force = false);

	//更新玩家数据到DB
	int UpdateUserDataToDB();

	int UpdateUserGemsToDB();

	int UpdateTime();

	/* 邮件系统 已经单独分出来处理*/

	const HerosMap& GetHeros() const { return heros; }
	CHero* getHeroById(int16_t id);
	uint32_t getHeroCount() { return (uint32_t)heros.size();}
	uint32_t getHeroCountByStar(uint32_t nStar) ;			//获得指定星级的英雄数量
	uint32_t getHeroCountByQuality(uint32_t nQuality) ;		//获得指定品质(颜色)的英雄数量

	/*添加单个英雄到背包并且添加到DB中,返回值 eAddHeroStatus */
	int addHeroAndWriteDB(int16_t template_id,uint8_t lvl = 1,uint8_t num = 1);

	/*添加英雄 不更新DB*/
	int addHero(const CHero& hero);

	/*添加英雄 不更新DB*/
	int addTower(int16_t template_id,uint8_t lvl = 1,uint8_t num = 1);

	//战斗数据相关接口
	CombatFlow& GetCombatFlow(void) { return m_CombatFlow; }
	CFirstRecharge& GetFirstRecharge(void) { return _FirstRecharge; }
	CVipAward& GetVipAward(void) { return _VipAward; }
	void CombatBegin(void);
	void CombatEnd(void);

	uint8_t GetLevel() const { return propertys.lvl;}
	uint32_t GetExp() const { return propertys.cur_exp;}

	/*体力相关*/
	uint16_t	GetMaxPhysicalStrength(void) ;
	uint16_t	GetCurrPhysicalStrength(void) ;
	int32_t		GetPhysicalStrength(uint32_t uRefPhyTime, uint16_t & uCurrPhy, uint32_t & uRemainTime) ;
	int32_t		SetPhysicalStrength(uint16_t nValue, bool bUpDB = false);
	bool		CheckBuyPhysicalCount(void);								//判断是否可以购买体力
	bool		BuyPhyStr(void);											//玩家购买体力的接口
	int32_t		GetBuyPhyData(int32_t& nRemainNum, int32_t& nPrice) ;		//获得当天购买体力的剩余次数和购买价格
  
  	/*技能点相关*/
	uint16_t	GetSkillLimited(void) ;												//技能点上限
  	uint16_t	SkillRevertSpeed(void) ;											//技能恢复速度
  	uint16_t	GetCurSkillPoint(void) ;	
  	int32_t		GetSkillPoint(uint32_t uRefPhyTime, uint16_t & uCurrPhy, uint32_t & uRemainTime) ;
  	int32_t		SetSkillPoint(uint16_t nValue, bool bUpDB = false);
	bool		CheckBuySkillPointCount(void);
  	bool		BuySkillPointStr(void);												//玩家购买技能点的接口
  
	/*经验升级*/
	int32_t		AddExp(uint32_t nExp, bool bUpDB = false);

	/*计算 玩家 添加经验后的等级 和 剩余经验 不更新内存*/
	int32_t		TryAddExp(uint32_t nExp,uint32_t& nAfterLvl,uint32_t& nAfterExp);


	/*	英雄经验升级并更新DB  英雄升级后 装备升级并更新DB 
		等级不能超过玩家等级 英雄经验不能超过 英雄升级到下一级的经验  超出不计
		玩家达到等级上限时 策划通过配表来实现  例如玩家最大等级60级 会配英雄升到61级的经验值
		玩家升级时 调用此接口 nExp传0
		返回结果 eHeroAddExpResult
	*/
	int32_t		AddHeroExp(int16_t heroid,uint32_t nExp = 0);
	int32_t		AddTowerExp(int16_t heroid,uint32_t nExp);
	void check_tab()
	{
		bool bt = false;
		uint32_t idx = 0;
		int maxexp = -1;
		do 
		{
			maxexp = CLevelUpTabMgr::Instance()->GetLevelUpExp(idx++);
			if (0 != maxexp) 
			{
				bt = true;
				break;
			}

			maxexp = CLevelUpTabMgr::Instance()->GetLevelUpExp(idx++);
			if (25 != maxexp) 
			{
				bt = true;
				break;
			}

			maxexp = CLevelUpTabMgr::Instance()->GetLevelUpExp(idx++);
			if (40 != maxexp) 
			{
				bt = true;
				break;
			}

			maxexp = CLevelUpTabMgr::Instance()->GetLevelUpExp(idx++);
			if (55 != maxexp) 
			{
				bt = true;
				break;
			}

			maxexp = CLevelUpTabMgr::Instance()->GetLevelUpExp(idx++);
			if (70 != maxexp) 
			{
				bt = true;
				break;
			}

			maxexp = CLevelUpTabMgr::Instance()->GetLevelUpExp(idx++);
			if (85 != maxexp) 
			{
				bt = true;
				break;
			}

			maxexp = CLevelUpTabMgr::Instance()->GetLevelUpExp(idx++);
			if (100 != maxexp) 
			{
				bt = true;
				break;
			}

			maxexp = CLevelUpTabMgr::Instance()->GetLevelUpExp(idx++);
			if (115 != maxexp) 
			{
				bt = true;
				break;
			}
		} while (0);
		
		if (bt)
		{
			LOGERR("CUser::check_tab failed,maxexp = %d,idx = %d",maxexp,idx-1);
		}
	}
	/*获得关卡状态*/
	UserFbStatus& GetFbStatus() { return FBStatus;}
	const int16_t* GetHerosFromPrepBattleArea(Prep_Function idx){ return m_prepBattleMng.GetHerosIDByFunc(idx);}
	//获取队长的英雄
	CHero*	GetTeamLeaderHero();

	void SetUserFlag(user_flag flag);				//设置用户标识
	user_flag GetUserFlag() { return m_UserFlag;}	//获取用户标识
	void SetLanguageFlag(uint8_t flag){m_LanguageFlag=flag;}
	uint8_t GetLanguageFlag(){ return m_LanguageFlag; }
	bool	IsUpBoundFriends();				//是否超出好友上限
	int	BuyFriendByDiamond();			//通过钻石购买好友
	/*
		获取好友信息时,处理体力相关状态
	*/
	bool SetSendPhyEachOtherExecuteStatus(int64_t uFriendId,SendPhyEachOtherData* uSendPhyData);
	/*对赠送体力中各种消息状态的处理*/
	int  HandlerSendPhyEachOther(int64_t uFriendId,uint8_t uEachOtherStatus);
	/*当前赠送体力接受赠送与接受奖励的最大次数*/
	void CalcSendPhyEachOtherInfo(uint32_t& uMaxAcceptPhyNum,uint32_t& uMaxPrizePhyNum);
	/*删除好友时对玩家之间的赠送体力的请求信息的处理*/
	void HandlerSendPhyInfo(const int64_t uFriendId/*好友ID*/);
	//通过索引获取功能的刷新时间
	static bool GetServerCdTimeByIndex(uint32_t index, uint32_t& hour, uint32_t& minute);
	/*
		函数说明:添加好友时判定对方玩家身上有没有自己
		@return 0 表示有  1 表示没有
	*/
	int conUserFriendExist(CUser* friendInfo);
	/*这个方法只是在重启服务器时，给玩家加载好友信息是，删除好友用
	   删除条件如下: 在重启服务器时，对方玩家删除自己时，等自己上线时
	   相应的，把对方也给删除掉，
	   上述的情况，在以下情况是，无法实现的，就是对方玩家删除自己时，而
	   【这时服务器不给力又重启了】，当自己登录时，是不能删除对方的，这时它会在客户
	   端只显示对方玩家的userid，这时就变成了，单方面了，但是这对后续的操作
	   没有什么影响，(这个问题，先这样 看看后面会不会用存库的操作)
	*/
	void delUnlineFriend(int64_t userid);
	//==========玩家真金(钻石)相关接口===============
	//获得真金(钻石)总量
	int32_t GetRMB() { return userrmb.GetCount(); }
	const UserRMB_t& GetRMB_t()const {return userrmb;}
	//获得充值累计总额
	int32_t GetChargeCount() { return userrmb.m_chargecount; }
	//玩家真金(钻石)产出唯一接口,
	//内部阻塞更新数据库数据成功后更改内存数据
	//返回0表示操作成功
	int AddRMB(AddRMBParam_t & param) ;
	//玩家真金(钻石)消耗唯一接口,
	//内部阻塞更新数据库数据成功后更改内存数据
	//返回0表示操作成功
	int SubRMB(SubRMBParam_t & param) ;
	//==========玩家真金(钻石)相关接口===============

	void ClearDataForVipLevelUp(); //vip升级，清除玩家数据
	/*刷新用户的vip等级*/
	void CalcVipLevel(bool bLoadData = false);
	//获得玩家的vip等级
	uint32_t GetVipLevel() ;
	float GetVipExp() { return propertys.vip_exp;}
	void SetVipExp(float vipexp,bool ischarge = false) ;
	CDig& GetDigData(){return _Dig;}
	
	//获得每日任务数据
	CDailyMission& GetDailyMissionData(){return DailyMission;}
	//获得任务数据
	CMission& GetMissionData(){return MissionData;}

	//助战相关
	void  RefOnlineFriendId();
	uint32_t GetInviteHead(void) const;
	uint32_t GetInviteEnd(void) const;
	//更新当天玩家邀请过好友的状态码
	void UpdateCurrInviteStatus(Invite_Friends& friendStatus,Invite_Friends& strangerStatus);
	// 优化算法，尝试解决好友系统的刷新bug
	void RandInviteIndex(void);
	//在刷新助战好友时刷选规则下标值变更的接口
	void RefUserPoolIndex(void);
	MemFriends& GetReStartFriendInfo()
	{
		return ReStartFriendInfo;
	}
	//添加好友或者陌生人的最后一次邀请的战斗时间
	void	 UpdateInviteCombatTime(int64_t& friendUserId,int8_t& friendStatus);
	//获取最大好友数量
	uint16_t	GetMaxFriendsCount();	
	//抽卡操作接口
	void GetAwardState(uint32_t uRefPhyTime, int8_t& freeGoldAwardTimes, uint32_t& freeGoldAwardNextTime, uint32_t& freeDiamondsAwardNextTime, uint32_t& freeGemAwardNextTime);
	int GetAward(int8_t flag, int8_t mode, uint8_t& GoodsCount, AwardGood* info, int8_t& freeGoldAwardTimes, uint32_t& freeGoldAwardNextTime, uint32_t& freeDiamondsAwardNextTime, uint32_t& freeGemAwardNextTime);
	uint32_t GetDiamondAwardCd();
	uint32_t GetGemAwardCd();

	InviteCombatMng& GetInviteCombatMng() { return m_InviteComabtMng;}
//	ChallengeBoss& GetChallengeBoss() { return _ChallengeBoss;}
	//背包
	Bag*	GetBag() { return &m_Bag;}
	bool	UpdateBagInfoToDB();										//更新背包数据到DB
	ItemOperateResult	SellItem(int16_t id,int16_t num);				//出售道具
	ItemOperateResult	BuyItem(int16_t itemid,int16_t itemnum);				//购买道具
	ItemOperateResult	UseItem(int16_t id,int16_t heroid,AckUseItem* pPacket);//使用道具
	ItemOperateResult	EquipCompose(int16_t equipid,vector<int32_t>& materialids);				//装备合成
	ItemOperateResult	EquipSharpen(ReqEquipSharpen* pPacket, int32_t & nOldLvl);	//装备强化
	ItemOperateResult	EquipHero(ReqEquipReplace* pPacket);	//装备替换
	ItemOperateResult	CheckEquipPos(equipment_tab* pTab,EquipPos pos);//检查装备位置
	 
	emGemHandleResult HeroInlayGem(int32_t pHeroid,int32_t suit,int32_t index);//鑲嵌
	emGemHandleResult HeroOutlayGem(GemPos position , int32_t heroid);//拆卸
	emGemHandleResult GetUserGeminfos(vector<hero_gem_info>& resultlist);//獲得符文信息
	emGemHandleResult GetUserOneGeminfo(int8_t suit,int32_t index,hero_gem_info& outGem);
	emGemHandleResult GetHeroOneGeminfo(int32_t heroid,int32_t pos,hero_gem_info& outGem);
	emGemHandleResult HeroDecomposeGemOnHero(GemPos position , int32_t heroid);	//分解英雄的符文
	emGemHandleResult HeroDecomposeGemInBag(int16_t suit , int32_t index);//分解背包的符文
	emGemHandleResult HeroDecomposeGemInBatch(int16_t star,int16_t suit);//批量分解背包符文
	emGemHandleResult HeroEnhenceGemOnHero(GemPos position , int32_t heroid,int isusedianmond);//強化英雄符文
	emGemHandleResult HeroEnhenceGemInBag(int16_t suit , int32_t index,int isusedianmond);//強化背包符文


	int	BuyGoldByDiamond(int32_t& nBuyGoldRate);			//通过钻石购买金币
	void	RefBuyGoldCount();								//刷新购买金币次数

	void	RefBuySkillPointCount();						//刷新购买技能点次数

	SignMng*	GetSignMng() { return &m_SignMng;}
	int8_t	ChangedTeamLeader(int16_t heroid);				//改变队长
	int16_t	GetTeamLeader(){ return propertys.teamleaderid;}				//队长
	void SetDefaultHeroId(int16_t id){propertys.defaultheroid = id;}//设置默认英雄id
	int16_t GetDefaultHeroId(){return propertys.defaultheroid;}//获取默认英雄id
	inline bool CheckUserMask(eUserMask mask)
	{
		return (propertys.bitmask & mask) != 0;
	}
	inline void SetUserMask(eUserMask mask)
	{
		propertys.bitmask |= mask;
	}
	inline void UnSetUserMask(eUserMask mask)
	{
		uint32_t n = mask ;
		propertys.bitmask &= ~n;
	}
	inline uint32_t GetFreeDiamondLottery(){return propertys.freeDiamondLottery;}
	inline void SetFreeDiamondLottery(uint32_t t){propertys.freeDiamondLottery = t;}
	inline uint32_t GetFreeGemLottery(){return propertys.freeGemLottery;}
	inline void SetFreeGemLottery(uint32_t t){propertys.freeGemLottery = t;}
	//战斗力 不包括 好友 替补
	int GetComBatPower(Prep_Function pre=PrepFunc_ComCombat);
	bool IsHaveHero(Prep_Function pre, int32_t heroId);
	//得到某个阵营，英雄的数量
	int GetCampHeroCount(Prep_Function pre, emElement e); 
	prepBattleMng* GetPrepBattleMng() { return &m_prepBattleMng;}

	void CheckServerAward();

	//
	ZodiacMng* GetZodiacMng() { return &m_ZodiacMng;}
	//获取新手引导数据
	guide_data_t& GetGuideInfo(){return _guideinfo;}

	//pvp相关
	CArena_a& GetArena(){ return _arena; }
	ArenaChallengeRecord& GetLastPvpAttackRecord() { return m_LastPvpAttackRecord; }	//获得最近一次pvp挑战纪录(数据)
	
	uint32_t memrcv(mem_buffer& rcvbuf);
	uint32_t memsnd(mem_buffer& sndbuf);
	size_t get_memsize();

	void SetUpdateTime(uint8_t model);
	//
	void SetLoginAwardInfo(LoginAwardInfo& Info);
	int32_t	GetLoginAwardTime(){return m_LoginAward.m_loginawardtime;}
	int32_t GetLoginAwardCount() { return m_LoginAward.m_loginawardcount;}

	// 英雄试练
	HeroTrialMng& GetHeroTrial() {return _herotrial;}
	// 巅峰对决
	CPeakVs& GetPeakVs() {return peakvs;}
	// 小无尽
	CSmallEndLess& GetSmallEndLess() {return smallendless;}
	// 小无尽
	CEndLess& GetEndLess() {return endless;}
	//组织竞技场玩家数据
	bool GetArenaInfo(ArenaUser& arena,int8_t type);
	//组织玩家共享数据
	bool GetShareData(UserShareData& data);
	//组织竞技场玩家数据
	bool GetGuildComatInfo(GuildComabtUser& data,int8_t type);


	//设置竞技场默认阵容
	bool SetDefaultArenaPrepBattle();

	//充值信息管理器
	CChargeInfoMng& GetChargeInfoMng() {return m_ChargeInfoMng ;}

	//获取免费十连抽资格状态
	int8_t GetDiamondFreeTenLottery();
	int8_t GetGemFreeTenLottery();

	//公告状态数据
	int32_t & GetNoticeLastIndex(void) { return m_noticelastIndex; }
	int32_t & GetNoticeLastId(void) { return m_noticelastId; }
	char* GetChatNoticeCheckMask(){return m_ChatNoticeCheckMask;}
	void SetChatNoticeCheckMask(const char* strMask){strncpy(m_ChatNoticeCheckMask, strMask, MD5_MASK_LEN);}

	//商店模块
	CShopModule& GetShopModule(){ return m_ShopModule; }
	int32_t* GetShopGemData(int nShop,int nItemIndex,int nAttribute);
	int32_t* GetShopGemData(int nShop,int nItemIndex);
	void CopyShopGemData(int nShop,int32_t* inoutGemData);

	//贸易
	CTradeMgr& GetTradeMgr(){ return m_TradeMgr; }

	LootInfo*	GetLootInfo() { return &m_Loot;}
	int		BuyLootCardByDiamond();		//通过钻石购买掠夺令
	void	RefBuyLootCardCount();		//刷新购买掠夺令次数
	int		LootNoCombatByDiamond(bool useItem);	//通过钻石/免战令购买掠夺令
	void	RefLootNoCombatCount();		//刷新掠夺免战次数
	void	FillLootUserInfo(LootUser & pvpuserinfo) ;
	//一键装备
	int32_t	EquipItemOneKey(int32_t heroid,int8_t* equipstatue,int32_t needtotalglods,  BagItemSync& bgsync);
	/*
		递归实现道具的查找
	*/
	bool	FindItemForEquip(int32_t equipid,int16_t num,Bag& nNeedBag, int32_t& NeedGolds);
	bool	_FindItemForEquip(int32_t equipid,int16_t num,Bag& nNeedBag, int32_t& NeedGolds);

	uint64_t GetChatMsgId() {return m_ChatMsgId;}
	void SetChatMsgId(uint64_t id) {m_ChatMsgId = id;}
	uint32_t GetChatTime() {return m_LastChatTime;}
	uint32_t GetChatTimeLaba() {return m_LastChatTimeLaba;}
	void SetChatTime(uint32_t t) {m_LastChatTime = t;}
	void SetChatTimeLaba(uint32_t t) {m_LastChatTimeLaba = t;}
	
	uint32_t calc_time(time_t now) const;
	bool CheckUserName(const char * UserName);
	void SetUserName(const char* name)
	{
		strncpy(propertys.username, name, sizeof(propertys.username));
		propertys.username[MAX_USER_NAME_LEN]='\0';
	}
	//月卡相关
	int8_t GetMonthCardState(time_t now) ;
	int32_t GetMonthCardRemaind(time_t now) ;
	//uint32_t GetMonthCardBuyTime(time_t now) ;
	//uint32_t GetMonthCardDrawTime(time_t now) ;

	//年卡相关
	int8_t GetYearCardState(time_t now) ;
	int32_t GetYearCardRemaind(time_t now) ;
	void DoBuyYearCard(time_t now) ;
	int32_t _GetTimeRemaind(time_t now, uint32_t hour, uint32_t minute, uint32_t buytime, uint32_t drawtime, uint32_t days) ;

	//新月卡相关
	int8_t GetNewMonthCardState(time_t now) ;
	int32_t GetNewMonthCardRemaind(time_t now) ;

	void ExecFristChargeActivity();
	void ResetFristChargeActivity();
	uint32_t GetLoginTimes(){return propertys.loginTimes;}
	void SetLoginTimes(uint32_t t){propertys.loginTimes = t;}
	uint32_t GetlvUpRewardLv(){return propertys.lvUpRewardLv;}
	void SetlvUpRewardLv(uint32_t t){propertys.lvUpRewardLv = t;}
	
	void ExecLoginTimeActivity(bool newuser = false);
	void ExecLevelUpActivtiy(uint8_t oldlv, uint8_t newlv);
	void ExecLevelUpActivtiy(uint8_t activityType, uint8_t oldlv, uint8_t newlv);
	void ExecVipLevelUpActivtiy(uint8_t activityType, uint8_t oldlv, uint8_t newlv);
	void CheckLevelUpActivityDate();
	void CheckActivityDate(eActivityType type);	
	void ExecActivity(eActivityType type, int16_t addcnt, int8_t activityParam=0);
	void UpdateNewYearActivity();	//更新新年活动
	CActivityAward& GetActivityInfo(){return _CommonActivity;}
	CFriendAward& 	GetFriendAward(){return m_FriendAwardData;}
	// 修改玩家名字相关
	uint8_t GetRenameTimes() { return propertys.renameTimes; }
	void		SetRenameTimes(uint8_t times) { propertys.renameTimes = times; }
	uint8_t		Rename(const char* username);
	void LoginAddVipExp();
	S_HolidayItemExchangeData& GetHolidayItemExchange(){ return m_HolidayItemExchange;}
	FriendMgr& GetFriendMng() { return m_FriendMgr;}


protected:
	eUserLoadFlag	m_LoadFlag;
//	uint8_t     RegisterCount;        //一次注册过程中，注册消息的请求次数。0-5次。 与客户端保持一致    
	int8_t		m_nBindAccountPlatState;
	int32_t     AccountPlatErrKey;	//缓存平台错误码
	uint8_t		BindPlatCount;        //一次绑定平台帐号过程中，消息的请求次数。与客户端保持一致

    user_data_t propertys;				//玩家基础属性数据
	UserRMB_t	userrmb ;				//玩家真金(钻石)数据
	
	/*玩家邮件的信息*/
	// CEmail emailInfo;
	//CEmail normal_mail_info;  // 普通邮件变量改名字，防止修改代码造成遗漏xiangzhaosong 2016/1/21

	/*英雄*/
	HerosMap	heros ;				//全部英雄

	/*普通关卡状态*/
	UserFbStatus	FBStatus;
	//每日任务
	CDailyMission	DailyMission ;
	//任务
	CMission		MissionData ;
	
	user_flag	m_UserFlag;	//用户标记
	uint8_t		m_LanguageFlag;		//语言标识
	//好友管理类
	FriendMgr	m_FriendMgr;
	CDig _Dig;
	InviteCombatMng	m_InviteComabtMng;
	//----------------------------------------------------------------------//
	//助战相关
	/*
		下面的变量主要是为了，重启服务器时,给
		邀请助战好友时用的，这个主要是防止出现错误日志的
		问题，和重启服务器后,自己加好友时,那个助战列表里
		有时出现好友个数不正确的问题
	*/
	MemFriends		ReStartFriendInfo;
	//下面这两个变量是为邀请助战好友刷新的下标值
	uint32_t	invite_head;
	uint32_t	invite_end;

	/*下面声明的变量是主要控制玩家本身当天友情点刷新的问题*/
	uint32_t userEQLastRefTime;
	/*
		下面的变量主要是控制当内存下标值达到内存池索引最后的时候
		相应的下标值在一定条件下改变的规则
	*/
	uint8_t refurbishNum;
	uint8_t refPoolStatus; /*0表示不刷新下标值，1表示刷新下标值*/

	//背包
	Bag		m_Bag;
	//签到系统
	SignMng	m_SignMng;
	//布阵
	prepBattleMng	m_prepBattleMng;

// 	//队伍最大战力
// 	int _maxcombatpower;
	//战斗
	CombatFlow		m_CombatFlow ;

// 	//帐号封停相关
// 	ForbidUserInfo _forbiduserinfo;

	//星座爬塔
	ZodiacMng	m_ZodiacMng;

	/*玩家领取奖励Id*/
	std::set<int64_t> userAward;

	guide_data_t _guideinfo;  //玩家引导信息

	MemModelUpdate _update;//各模块更新时间

	//pvp相关
	//CArena _arena; //竞技场信息
	CArena_a _arena; //竞技场信息
	ArenaChallengeRecord m_LastPvpAttackRecord ;	//最近一次pvp挑战纪录(数据)

	LoginAwardInfo	m_LoginAward;	//7日登录奖励
	HeroTrialMng _herotrial; //英雄试练
	CPeakVs peakvs;//巅峰对决
	CSmallEndLess smallendless;//小无尽
	CEndLess endless;//无尽
	CChargeInfoMng m_ChargeInfoMng ;	//充值信息管理器

	//公告状态数据
	int32_t m_noticelastIndex ;
	int32_t m_noticelastId ;
	char	m_ChatNoticeCheckMask[MD5_MASK_LEN+1];					//聊天的md5吗

	//聊天
	uint64_t m_ChatMsgId;
	uint32_t m_LastChatTime;
	uint32_t m_LastChatTimeLaba;
	// 商店模块
	CShopModule m_ShopModule;
	// 商店中的符石随机数据
	int32_t m_ShopGemRandomData[eShop_max][TOTAL_ITEM_CNT][MAX_GEM_MAIN_ATT_COUNT+MAX_GEM_ADDITIONAL_ATT_COUNT];

	//贸易
	CTradeMgr	m_TradeMgr;

	LootInfo	m_Loot;			//掠夺相关

	CFirstRecharge _FirstRecharge;	//首冲奖励数据
	CVipAward _VipAward;			//VIP奖励数据

	CActivityAward	_CommonActivity;	//赵云碎片等等
	CFriendAward	m_FriendAwardData;	//助战好友奖励 
	//--节日道具兑换
	S_HolidayItemExchangeData m_HolidayItemExchange; 
	//精英本的魂石掉落数据计数器

public:
	int32_t		m_JYCombatCnt;
	int32_t		m_JYDropHunShiCnt;

// =====================Payments===============================
#if defined(PAY_PLATFORM) && (PAY_PLATFORM==BUSINESS_PLATFORM_APPLE)
private:
	AppStoreOrderInfo m_PaymentOrderInfoAppStore;	// iOS支付订单信息
public:
	AppStoreOrderInfo& RefPaymentOrderInfoAppStore(void)
	{
		return m_PaymentOrderInfoAppStore;
	}
#endif
public:
	void ClearPayment(void);
};

extern CUser* g_CurrUser ;

#define HANDLER_VERIFY_IDENTIFY_UNLOCK(ptrU,ptrPkt,ptrCon)    \
    ptrU =  CDataMgr::Instance()->FindUserByUoid(ptrPkt->uoid); \
    if (NULL == ptrU)   {       \
        char szIP[32] = {0}; int  nPort = 0;    \
        ptrCon->getPeerToString(szIP,&nPort);     \
        ptrCon->addReplyOpResult(eOpResult_ReLogin);\
        LOGERR("MERR: (%s,%d,%s)Not find user! Client uoid=%u is wrong,Client key=%u,IP=%s,Port=%d", __FILE__, __LINE__, __FUNCTION__, ptrPkt->uoid, ptrPkt->key,szIP, nPort);\
        return -1;  \
    }               \
    if (ptrPkt->key != ptrU->key)  {   \
        char szIP[32] = {0}; int  nPort = 0;    \
        ptrCon->getPeerToString(szIP,&nPort);     \
        ptrCon->addReplyOpResult(eOpResult_ReLogin);\
        LOGERR("MERR! (%s,%d,%s)Session key not match! userid=%lld,Client Key=%u,Server key=%u,IP=%s,Port=%d,",__FILE__, __LINE__, __FUNCTION__, ptrU->GetUserData().userid, ptrPkt->key, ptrU->key,szIP, nPort);\
        return -1;  \
    }  \
	if (ptrU->GetUserFlag() == userflag_killoff)  {   \
		char szIP[32] = {0}; int  nPort = 0;    \
		ptrCon->getPeerToString(szIP,&nPort);     \
		ptrCon->addReplyOpResult(eOpResult_KillOff);\
		ptrU->SetUserFlag(userflag_normal);\
		ptrU->key=-1;\
		LOGERR("MERR! (%s,%d,%s)user killoff! userid=%lld,Client Key=%u,Server key=%u,IP=%s,Port=%d,",__FILE__, __LINE__, __FUNCTION__, ptrU->GetUserData().userid, ptrPkt->key, ptrU->key,szIP, nPort);\
		return -1;  \
	} \
	g_CurrUser = ptrU; \


#define HANDLER_VERIFY_IDENTIFY(ptrU,ptrPkt,ptrCon)    \
	int64_t __userid_loc =-1;  \
    IMemKey<uint32_t> key(ptrPkt->uoid,eKey_UoidAndUserId); \
    int verify_result = CMemMgr::Instance()->GetData(key, __userid_loc); \
    if (eMemResult_OK != verify_result) \
    { \
	    char szIP[32] = {0}; int  nPort = 0;    \
	    ptrCon->getPeerToString(szIP,&nPort);     \
	    ptrCon->addReplyOpResult(eOpResult_ReLogin);\
		LOGERR("MERR: (%s,%d,%s)find userid by uoid fail! Client uoid=%u is wrong,Client key=%u,IP=%s,Port=%d,pktId=%d", __FILE__, __LINE__, __FUNCTION__, ptrPkt->uoid, ptrPkt->key,szIP, nPort, ptrPkt->get_id());\
    	return -1; \
    } \
	MemLock __memtrylock_loc(eKey_UserIdAndUser_Lock, __userid_loc, false);	\
	if(false == __memtrylock_loc.Lock()) {	\
		char szIP[32] = {0}; int  nPort = 0;    \
        ptrCon->getPeerToString(szIP,&nPort);     \
        LOGERR("MERR: (%s,%d,%s)user trylock fail! Client uoid=%u is wrong,Client key=%u,IP=%s,Port=%d,pktId=%d,UserId=%lld", __FILE__, __LINE__, __FUNCTION__, ptrPkt->uoid, ptrPkt->key,szIP, nPort, ptrPkt->get_id(), __userid_loc);\
        return -1;  \
    }               \
	LOGDEBUG("CDataMgr::Instance()->FindUserByUserIDD----------start %f",__userid_loc); \
    ptrU =  CDataMgr::Instance()->FindUserByUserID(__userid_loc); \
	LOGDEBUG("CDataMgr::Instance()->FindUserByUserIDD----------end "); \
    if (NULL == ptrU)   {       \
        char szIP[32] = {0}; int  nPort = 0;    \
        ptrCon->getPeerToString(szIP,&nPort);     \
        ptrCon->addReplyOpResult(eOpResult_ReLogin);\
        LOGERR("MERR: (%s,%d,%s)Not find user! Client uoid=%u is wrong,Client key=%u,IP=%s,Port=%d,UserId=%lld", __FILE__, __LINE__, __FUNCTION__, ptrPkt->uoid, ptrPkt->key,szIP, nPort, __userid_loc);\
        return -1;  \
    }               \
	 if (ptrPkt->key != ptrU->key)  {   \
        char szIP[32] = {0}; int  nPort = 0;    \
        ptrCon->getPeerToString(szIP,&nPort);     \
        ptrCon->addReplyOpResult(eOpResult_ReLogin);\
        LOGERR("MERR! (%s,%d,%s)Session key not match! userid=%lld,Client Key=%u,Server key=%u,IP=%s,Port=%d",__FILE__, __LINE__, __FUNCTION__, ptrU->GetUserData().userid, ptrPkt->key, ptrU->key,szIP, nPort);\
        return -1;  \
	}  \
	if (ptrU->GetUserFlag() == userflag_killoff)  {   \
		char szIP[32] = {0}; int  nPort = 0;    \
		ptrCon->getPeerToString(szIP,&nPort);     \
		ptrCon->addReplyOpResult(eOpResult_KillOff);\
		ptrU->SetUserFlag(userflag_normal);\
		ptrU->key=-1;\
		LOGERR("MERR! (%s,%d,%s)user killoff! userid=%lld,Client Key=%u,Server key=%u,IP=%s,Port=%d,",__FILE__, __LINE__, __FUNCTION__, ptrU->GetUserData().userid, ptrPkt->key, ptrU->key,szIP, nPort);\
		return -1;  \
	} \
	g_CurrUser = ptrU; \
	DataDirtyHelper __dataHelper;\

//验证玩家的标识是否是战斗
#define HANDLER_VERIFY_USERFLAG(ptrU,ptrPkt,ptrCon) \
	if(NULL != ptrU) \
	{ \
		if(ptrU->GetUserFlag() == userflag_combat) \
		{\
			char szIP[32] = {0}; int  nPort = 0;    \
			ptrCon->getPeerToString(szIP,&nPort);     \
			LOGERR("MERR! (%s,%d,%s)user flag[%d] is  error! userid=%lld,packetid[%d],IP=%s,Port=%d,",__FILE__, __LINE__, __FUNCTION__,ptrU->GetUserFlag(),ptrU->GetUserData().userid, ptrPkt->get_id(),szIP, nPort);\
			ptrU->SetUserFlag(userflag_normal);\
		}\
	} 

// 1）建议把这个宏，扩展为两个参数，第二个参数为LOT类型
// 2）LOT类型做成64位或者128位，每位表示一种类型，可以按位或，
//    这样就不会发生调用UPDATE_USER_DATA,忘记调用或者调用顺序错误SetUpdateTime导致存储异常
#define UPDATE_USER_DATA(ptrU)    \
	if (server.memcached_enable&&NULL != ptrU)   {       \
	ptrU->SetUpdateTime(LOT_BaseData);\
 	CDataMgr::Instance()->UpdateUser(*ptrU);\
	CDataMgr::Instance()->SetUserDataDirty( ptrU->GetUserID() );\
	} 

#define UPDATE_FRIENDS_DATA(ptrUSER, DATA)    \
	if (server.memcached_enable)   {       \
	CDataMgr::Instance()->UpdateFriends(ptrUSER->GetUserID(), DATA);\
	CDataMgr::Instance()->SetUserDataDirty( ptrUSER->GetUserID() );\
	} 

#define LOCK_USER_BY_UOID(__uoid)  \
	MemLock __memlock_loc(eKey_UoidAndUser_Lock, __uoid, true) 

struct DataDirtyHelper
{
	~DataDirtyHelper()
	{
		CDataMgr::Instance()->FlushDirtyUsers( );
	}

	static uint32_t sDirtyStamp;
};
#endif
