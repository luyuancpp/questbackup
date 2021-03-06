#include <gtest/gtest.h>


#include "GenCode/Config/ConfigModule.h"
#include "GenCode/Config/ActivityCfg.h"

#include "Quest/QuestList.h"
#include "Quest/ActivityQuestAcceptor.h"
#include "GenCode/Config/LevelCfg.h"

#include "CommonLogic/Quest/QuestDefine.h"
#include "Quest/DailyQuest.h"
#include "Quest/ActivityQuestRelation.h"
#include "GameDefine_Result.h"
#include "Quest/QuestFactory.h"
#include "Quest/AchievementFactory.h"
#include "CommonLogic/Quest/QuestDefine.h"
#include "GenCode/Config/AchievementDataCfg.h"
#include "Quest/AchievementFactory.h"

muduo::net::EventLoop * pLoop = NULL;



class QustAcceptCalaclatorMock
{
public:

    QustAcceptCalaclatorMock(AvatarQuest::QuestList * pQuestManager)
        :m_pQuestManager(pQuestManager)
    {
        auto cb = std::bind(&QustAcceptCalaclatorMock::OnAcceptQuest, this, std::placeholders::_1);
        m_pQuestManager->SetQuestAcceptCalculator(cb);
    }

    void OnAcceptQuest(int32_t quest_id)
    {
        const QuestElement * pQuestElement = QuestTable::Instance().GetElement(quest_id);
        if (nullptr == pQuestElement)
        {
            return;
        }
        TriggerType(quest_id, pQuestElement->target_1_type);
        TriggerType(quest_id, pQuestElement->target_2_type);
        TriggerType(quest_id, pQuestElement->target_3_type);
        TriggerType(quest_id, pQuestElement->target_4_type);
        TriggerType(quest_id, pQuestElement->target_5_type);
    }

    void   TriggerType(int32_t quest_id, int32_t nType)
    {
        switch (nType)
        {
        case AvatarQuest::E_QUEST_LEVELUP:
        {
            m_pQuestManager->TriggerEvent(
				quest_id,
                AvatarQuest::E_QUEST_LEVELUP,
                0,
                m_pQuestManager->GetLevel());
        }
        break;
        }
    }

    AvatarQuest::QuestList * m_pQuestManager{ nullptr };
};


std::size_t GetRecordCompleteSize(AvatarQuest::QuestList& oQuestManager )
{
    std::size_t i = 0;
    for (auto it : QuestTable::Instance().GetAllID())
    {
        if (oQuestManager.CheckRecordToCompleteQuest(it))
        {
            ++i;
        }
    }
    return i;
}

TEST(QuestTest, QuestGmCommand)
{
 
    AvatarQuest::QuestList oQuestManager;
    oQuestManager.SetCheckUniqueQuest(false);
    QustAcceptCalaclatorMock m_oqac(&oQuestManager);
    oQuestManager.LevelUp(60);
    EXPECT_EQ(0, oQuestManager.GetAcceptQuestSize());

    int32_t firstId = QuestTable::Instance().GetAllID()[11];
    int32_t secondId = QuestTable::Instance().GetAllID()[12];

    oQuestManager.CompleteAllQuest();
    //EXPECT_EQ(4, oQuestManager.GetAcceptQuestSize());
    EXPECT_TRUE(GetRecordCompleteSize(oQuestManager) >= oQuestManager.GetCompleteQuestSize());


    oQuestManager.Clear();
    oQuestManager.AcceptQuest(firstId);
    oQuestManager.TriggerEvent(QuestTable::Instance().GetElement(firstId)->target_1_type, QuestTable::Instance().GetElement(firstId)->target_1_id[0], QuestTable::Instance().GetElement(firstId)->target_1_amount);
    EXPECT_EQ(1, oQuestManager.GetAcceptQuestSize());
  
    oQuestManager.OnKillMonster(QuestTable::Instance().GetElement(secondId)->target_1_id[0], QuestTable::Instance().GetElement(secondId)->target_1_amount);
    EXPECT_EQ(1, oQuestManager.GetAcceptQuestSize());
    EXPECT_EQ(1, oQuestManager.GetEventObserverSize());
}

TEST(QuestTest, QuestGiveUpQuest)
{
    AvatarQuest::QuestList oQuestManager;
    int32_t questId = 11025;
    EXPECT_EQ(OR_QUEST_HASNT_QUEST, oQuestManager.GiveUpQuest(questId));
    oQuestManager.AcceptQuest(questId);
    EXPECT_EQ(AvatarQuest::E_QUEST_STATUS_ACCEPTED, oQuestManager.GetQuestStatus(questId));
    EXPECT_EQ(OR_OK, oQuestManager.GiveUpQuest(questId));
    EXPECT_EQ(AvatarQuest::E_QUEST_STATUS_UN_ACCEPT, oQuestManager.GetQuestStatus(questId));
    EXPECT_EQ(0, oQuestManager.GetAcceptQuestSize());
    EXPECT_EQ(0, oQuestManager.GetCompleteQuestSize());
    oQuestManager.AcceptQuest(questId);
    oQuestManager.DirectToCompleteQuest(questId);
    EXPECT_EQ(OR_QUEST_HASNT_QUEST, oQuestManager.CompleteQuest(questId));
    EXPECT_EQ(OR_QUEST_HASNT_QUEST, oQuestManager.GiveUpQuest(questId));
    EXPECT_EQ(0, oQuestManager.GetEventObserverSize());
}

TEST(QuestTest, QuestGiveUpQuest1)
{
    AvatarQuest::QuestList oQuestManager;
    int32_t questId = 11001;
    EXPECT_EQ(OR_QUEST_HASNT_QUEST, oQuestManager.GiveUpQuest(questId));
    oQuestManager.AcceptQuest(questId);
    EXPECT_EQ(AvatarQuest::E_QUEST_STATUS_ACCEPTED, oQuestManager.GetQuestStatus(questId));
    EXPECT_EQ(OR_QUEST_GIVE_UP_QUEST, oQuestManager.GiveUpQuest(questId));
    EXPECT_EQ(AvatarQuest::E_QUEST_STATUS_ACCEPTED, oQuestManager.GetQuestStatus(questId));
    EXPECT_EQ(1, oQuestManager.GetAcceptQuestSize());
    EXPECT_EQ(0, oQuestManager.GetCompleteQuestSize());
    oQuestManager.AcceptQuest(questId);
    oQuestManager.DirectToCompleteQuest(questId);
    EXPECT_EQ(OR_QUEST_COMPLETED, oQuestManager.CompleteQuest(questId));
    EXPECT_EQ(OR_QUEST_COMPLETED, oQuestManager.GiveUpQuest(questId));
    //auto accept next quest
    EXPECT_EQ(1, oQuestManager.GetEventObserverSize());
}

TEST(QuestTest, QuestCompleteQuest)
{
    int32_t firstId = QuestTable::Instance().GetAllID()[11];
    AvatarQuest::QuestList oQuestManager;
    oQuestManager.SetCheckUniqueQuest(false);
    EXPECT_EQ(AvatarQuest::E_QUEST_STATUS_UN_ACCEPT, oQuestManager.GetQuestStatus(firstId));
    oQuestManager.AcceptQuest(firstId);
    EXPECT_EQ(AvatarQuest::E_QUEST_STATUS_ACCEPTED, oQuestManager.GetQuestStatus(firstId));

    EXPECT_TRUE(oQuestManager.FindAcceptedQuest(firstId));
    EXPECT_FALSE(oQuestManager.FindCompletedQuest(firstId));


    oQuestManager.DirectToCompleteQuest(firstId);

    EXPECT_EQ(OR_QUEST_COMPLETED, oQuestManager.AcceptQuest(firstId));
    EXPECT_FALSE(oQuestManager.FindAcceptedQuest(firstId));

    EXPECT_TRUE(oQuestManager.FindCompletedQuest(firstId));
    EXPECT_EQ(AvatarQuest::E_QUEST_STATUS_COMPLETED, oQuestManager.GetQuestStatus(firstId));
    EXPECT_EQ(1, oQuestManager.GetEventObserverSize());
}


TEST(QuestTest, QuestEventDriven)
{
    // id error

    AvatarQuest::QuestList oQuestManager;

    int32_t firstId = 11002;

    oQuestManager.AcceptQuest(firstId);

    oQuestManager.TriggerEvent(AvatarQuest::E_QUEST_TALKING_WITH_NPC, 1, 1);
    oQuestManager.CompleteQuest(firstId);
    EXPECT_EQ(AvatarQuest::E_QUEST_STATUS_ACCEPTED, oQuestManager.GetQuestStatus(firstId));

    // size error
    oQuestManager.TriggerEvent(AvatarQuest::E_QUEST_TALKING_WITH_NPC, QuestTable::Instance().GetElement(firstId)->target_1_id[0], 4);
    oQuestManager.CompleteQuest(firstId);
    EXPECT_EQ(AvatarQuest::E_QUEST_STATUS_ACCEPTED, oQuestManager.GetQuestStatus(firstId));

    oQuestManager.TriggerEvent(AvatarQuest::E_QUEST_KILL_MONSTER, QuestTable::Instance().GetElement(firstId)->target_1_id[0], QuestTable::Instance().GetElement(firstId)->target_1_amount);
    oQuestManager.CompleteQuest(firstId);
    EXPECT_EQ(AvatarQuest::E_QUEST_STATUS_COMPLETED, oQuestManager.GetQuestStatus(firstId));
    EXPECT_EQ(1, oQuestManager.GetEventObserverSize());
}


TEST(QuestTest, QuestStep)
{
    QuestElement oQuestElement;
    oQuestElement.target_1_type = AvatarQuest::E_QUEST_KILL_MONSTER;
    oQuestElement.target_1_id.push_back(1);
    oQuestElement.target_1_amount = 5;
    oQuestElement.target_2_id.push_back(2);
    oQuestElement.target_2_type = AvatarQuest::E_QUEST_KILL_MONSTER;
    oQuestElement.target_2_amount = 5;
    oQuestElement.is_auto_awarded = 1;
    AvatarQuest::CreateQuestParam p;
    
    p.quest_factory_ptr_.reset(new AvatarQuest::QuestFactory());
    AvatarQuest::Quest oQuest(&oQuestElement, p);
 
    EXPECT_EQ(2, oQuest.GetQuestStepSize());


    EXPECT_EQ(AvatarQuest::E_QUEST_STATUS_ACCEPTED, oQuest.GetQuestStatus());

    AvatarQuest::QuestEvent oQuestEvent(AvatarQuest::E_QUEST_KILL_MONSTER, 1, 5);
    oQuest.TriggerEvent(oQuestEvent);

    EXPECT_EQ(AvatarQuest::E_QUEST_STATUS_ACCEPTED, oQuest.GetQuestStatus());
    AvatarQuest::QuestEvent oQuestEvent1(AvatarQuest::E_QUEST_KILL_MONSTER, 2, 5);
    EXPECT_EQ(OR_QUEST_ACHIVE_ERROR, oQuest.CompleteQuest());
    oQuest.TriggerEvent(oQuestEvent1);
    //EXPECT_EQ(OR_OK, oQuest.CompleteQuestStep(0));

    EXPECT_EQ(AvatarQuest::E_QUEST_STATUS_ACHIEVED, oQuest.GetQuestStatus());
    EXPECT_EQ(OR_OK, oQuest.CompleteQuest());
    EXPECT_EQ(AvatarQuest::E_QUEST_STATUS_COMPLETED, oQuest.GetQuestStatus());
 
}

TEST(QuestTest, LevelUp40)
{
    AvatarQuest::QuestList oQuestManager;
    oQuestManager.LevelUp(40);
    QustAcceptCalaclatorMock m_qac(&oQuestManager);
    oQuestManager.AcceptQuest(11001);
    oQuestManager.DirectToCompleteQuest(11001);
    oQuestManager.DirectToCompleteQuest(11002);
    EXPECT_EQ(AvatarQuest::E_QUEST_STATUS_COMPLETED, oQuestManager.GetQuestStatus(11003));
    EXPECT_EQ(AvatarQuest::E_QUEST_STATUS_UN_ACCEPT, oQuestManager.GetQuestStatus(11005));
    oQuestManager.DirectToCompleteQuest(11004);
    EXPECT_EQ(AvatarQuest::E_QUEST_STATUS_COMPLETED, oQuestManager.GetQuestStatus(11005));
    oQuestManager.DirectToCompleteQuest(11006);
    EXPECT_EQ(AvatarQuest::E_QUEST_STATUS_COMPLETED, oQuestManager.GetQuestStatus(11007));
    EXPECT_EQ(1, oQuestManager.GetEventObserverSize());
}

TEST(QuestTest, LevelUp10)
{
    AvatarQuest::QuestList oQuestManager;
    QustAcceptCalaclatorMock m_qac(&oQuestManager);
    oQuestManager.LevelUp(1);

    oQuestManager.AcceptQuest(11001);
    oQuestManager.DirectToCompleteQuest(11001);
    oQuestManager.DirectToCompleteQuest(11002);
    EXPECT_EQ(AvatarQuest::E_QUEST_STATUS_ACCEPTED, oQuestManager.GetQuestStatus(11003));
    oQuestManager.LevelUp(2);
    EXPECT_EQ(AvatarQuest::E_QUEST_STATUS_ACCEPTED, oQuestManager.GetQuestStatus(11003));
    oQuestManager.LevelUp(10);
    EXPECT_EQ(AvatarQuest::E_QUEST_STATUS_COMPLETED, oQuestManager.GetQuestStatus(11003));
    EXPECT_EQ(1, oQuestManager.GetEventObserverSize());
    oQuestManager.AcceptQuest(90290);
    oQuestManager.LevelUp(43);
    oQuestManager.LevelUp(44);
    oQuestManager.LevelUp(45);
    EXPECT_EQ(AvatarQuest::E_QUEST_STATUS_COMPLETED, oQuestManager.GetQuestStatus(11003));
}

TEST(QuestTest, CompleteQuestStep)
{
    QuestElement oQuestElement;
    oQuestElement.target_1_type = AvatarQuest::E_QUEST_TALKING_WITH_NPC;
    oQuestElement.target_1_id.push_back(1);
    oQuestElement.target_1_amount = 5;
    oQuestElement.target_2_id.push_back(2);
    oQuestElement.target_2_type = AvatarQuest::E_QUEST_TALKING_WITH_NPC;
    oQuestElement.target_2_amount = 5;
    oQuestElement.is_auto_awarded = 1;
    AvatarQuest::CreateQuestParam p;
    p.quest_factory_ptr_.reset(new AvatarQuest::QuestFactory());
    AvatarQuest::Quest oQuest(&oQuestElement, p);

    AvatarQuest::QuestEvent oQuestEvent(AvatarQuest::E_QUEST_TALKING_WITH_NPC, 1, 5);
    oQuest.TriggerEvent(oQuestEvent);
    EXPECT_EQ(OR_QUEST_ACHIVE_ERROR, oQuest.CompleteQuest());
    AvatarQuest::QuestEvent oQuestEvent2(AvatarQuest::E_QUEST_TALKING_WITH_NPC, 2, 5);
    oQuest.TriggerEvent(oQuestEvent2);
    EXPECT_EQ(OR_OK, oQuest.CompleteQuest());

}


TEST(QuestTest, CompleteQuestFiveStep)
{
    AvatarQuest::QuestList oQuestManager;

    int32_t nConfigId = 11015;
    

    const QuestElement * pQuestElement = QuestTable::Instance().GetElement(nConfigId);
    QuestElement * pChageQuestElement = const_cast<QuestElement *>(pQuestElement);
    pChageQuestElement->target_2_id.push_back(0);
    pChageQuestElement->target_3_id.push_back(0);
    pChageQuestElement->target_4_id.push_back(0);
    pChageQuestElement->target_5_id.push_back(0);

    oQuestManager.AcceptQuest(nConfigId);
    oQuestManager.TriggerEvent(pQuestElement->target_1_type, pQuestElement->target_1_id[0], pQuestElement->target_1_amount);
    EXPECT_EQ(OR_QUEST_ACHIVE_ERROR, oQuestManager.CompleteQuest(nConfigId));
    
    oQuestManager.TriggerEvent(pQuestElement->target_2_type, pQuestElement->target_2_id[0], pQuestElement->target_2_amount);
    EXPECT_EQ(OR_QUEST_ACHIVE_ERROR, oQuestManager.CompleteQuest(nConfigId));
    oQuestManager.TriggerEvent(pQuestElement->target_3_type, pQuestElement->target_3_id[0], pQuestElement->target_3_amount);
    EXPECT_EQ(OR_QUEST_ACHIVE_ERROR, oQuestManager.CompleteQuest(nConfigId));
    
    oQuestManager.TriggerEvent(pQuestElement->target_4_type, pQuestElement->target_4_id[0], pQuestElement->target_4_amount);
    EXPECT_EQ(OR_QUEST_ACHIVE_ERROR, oQuestManager.CompleteQuest(nConfigId));
    oQuestManager.TriggerEvent(pQuestElement->target_5_type, pQuestElement->target_5_id[0], pQuestElement->target_5_amount);
    EXPECT_EQ(OR_QUEST_COMPLETED, oQuestManager.CompleteQuest(nConfigId));
    EXPECT_EQ(0, oQuestManager.GetEventObserverSize());
}

TEST(QuestTest, AutoCompleteQuest)
{
    AvatarQuest::QuestList oQuestManager;
    oQuestManager.AcceptQuest(11009);
    oQuestManager.TriggerEvent(AvatarQuest::E_QUEST_TALKING_WITH_NPC, 11002, 1);
    EXPECT_EQ(0, oQuestManager.GetAcceptQuestSize());

}



TEST(QuestTest, GetReward)
{
    AvatarQuest::QuestList oQuestManager;
    int32_t config_id = 11008;
    EXPECT_EQ(OR_QUEST_HASNT_QUEST, oQuestManager.GetReward(config_id));
    oQuestManager.AcceptQuest(config_id);
    EXPECT_EQ(OR_QUEST_ACHIVE_ERROR, oQuestManager.GetReward(config_id));
    oQuestManager.OnKillMonster(QuestTable::Instance().GetElement(config_id)->target_1_id[0], QuestTable::Instance().GetElement(config_id)->target_1_amount);
    EXPECT_EQ(OR_QUEST_COMPLETED, oQuestManager.GetReward(config_id));
    EXPECT_EQ(AvatarQuest::E_QUEST_STATUS_COMPLETED, oQuestManager.GetQuestStatus(config_id));
    
}

TEST(QuestTest, QuestTypeUnique)
{
    AvatarQuest::QuestList oQuestManager;
    oQuestManager.SetCheckUniqueQuest(true);
    oQuestManager.LevelUp(60);
    int32_t questId = 10000;
    for (int32_t i = 0; i < 5; ++i)
    {
        int32_t j = i * 2;
        EXPECT_EQ(OR_OK, oQuestManager.AcceptQuest(questId + j));
        EXPECT_EQ(OR_QUEST_TYPE_UNIQUE, oQuestManager.AcceptQuest(questId + j + 1));
        EXPECT_EQ(i + 1, oQuestManager.GetSubQuestTypeListSize());
    }

    questId = 11001;
    EXPECT_EQ(OR_OK, oQuestManager.AcceptQuest(questId));
    for (int32_t i = 0; i < 10; ++i)
    {
        EXPECT_EQ(OR_QUEST_TYPE_UNIQUE, oQuestManager.AcceptQuest(questId + i));

    }

}

TEST(QuestTest, AcceptQuestZhanxing)
{

    const ActivityElement* pEle = ActivityTable::Instance().GetElement(eActivityID::E_ACTIVITY_ZHANXING);

    std::size_t c = pEle->rounds_max * pEle->times_max;

    AvatarQuest::QuestList oQuestManager;
    oQuestManager.LevelUp(60);
    ActivityQuestManger o(&oQuestManager);
    o.OnDBComplete();
    i32_v_type v;
    for (std::size_t i = 0; i < c; ++i)
    {
        
        EXPECT_EQ(OR_OK, o.ParticipateActivity(eActivityID::E_ACTIVITY_ZHANXING));
        EXPECT_EQ(OR_QUEST_TYPE_UNIQUE, o.ParticipateActivity(eActivityID::E_ACTIVITY_ZHANXING));

        int32_t questid = oQuestManager.GetQuestFromType(AvatarQuest::E_ZHANXING);
        oQuestManager.DirectToCompleteQuest(questid);

    }
    
}

TEST(QuestTest, GMClearQuest)
{
    AvatarQuest::QuestList oQuestManager;
    oQuestManager.AcceptAllQuest();
    oQuestManager.OnGmClear();
    EXPECT_EQ(0, oQuestManager.GetAcceptQuestSize());
    EXPECT_EQ(0, oQuestManager.GetCompleteQuestSize());
    
}

void Zhanxing(AvatarQuest::QuestList & oQuestManager)
{
    oQuestManager.Clear();
    oQuestManager.LevelUp(60);
    ActivityQuestManger o(&oQuestManager);
    o.OnDBComplete();
    o.OnNewDayCome();
    EXPECT_EQ(1, o.GetRewardTimes(E_ACTIVITY_ZHANXING));
    for (int32_t j = 0; j < ActivityTable::Instance().GetElement(E_ACTIVITY_ZHANXING)->rounds_max; ++j)
    {
        for (int32_t i = 0; i < ActivityTable::Instance().GetElement(E_ACTIVITY_ZHANXING)->times_max; ++i)
        {
            EXPECT_EQ(OR_OK, o.ParticipateActivity(eActivityID::E_ACTIVITY_ZHANXING));
            EXPECT_EQ(OR_QUEST_TYPE_UNIQUE, o.ParticipateActivity(eActivityID::E_ACTIVITY_ZHANXING));
            int32_t questid = oQuestManager.GetQuestFromType(AvatarQuest::E_ZHANXING);
            EXPECT_EQ(OR_OK, oQuestManager.DirectToCompleteQuest(questid));
        }
    }
    EXPECT_EQ(1, o.GetRewardTimes(E_ACTIVITY_ZHANXING));
}

void NotRepetitionActivity(AvatarQuest::QuestList & oQuestManager, int32_t activity_id)
{
    oQuestManager.LevelUp(60);
    ActivityQuestManger o(&oQuestManager);
    o.OnDBComplete();
    o.OnNewDayCome();
    EXPECT_EQ(1, o.GetRewardTimes(activity_id));
    int32_t s = ActivityTable::Instance().GetElement(activity_id)->times_max * ActivityTable::Instance().GetElement(activity_id)->rounds_max;
    for (int32_t j = 0; j < s; ++j)
    {
        EXPECT_EQ(OR_OK, o.ParticipateActivity(activity_id));
        EXPECT_EQ(OR_QUEST_TYPE_UNIQUE, o.ParticipateActivity(activity_id));
        EXPECT_EQ(OR_OK, o.DirectToCompleteQuest(activity_id));
    }
    EXPECT_EQ(1, o.GetRewardTimes(activity_id));
}

TEST(QuestTest, Zhanxing)
{
    AvatarQuest::QuestList oQuestManager;
    ActivityQuestManger o(&oQuestManager);
    for (size_t i = 0; i < 10; i++)
    {
        Zhanxing(oQuestManager);
    }
    
}

void Wuzhengqin(AvatarQuest::QuestList & oQuestManager, ActivityQuestManger &o)
{
    oQuestManager.Clear();
    oQuestManager.LevelUp(60);
    EXPECT_EQ(1, o.GetRewardTimes(eActivityID::E_ACTIVITY_HUNT));
    int32_t times = ActivityTable::Instance().GetElement(eActivityID::E_ACTIVITY_HUNT)->rounds_max
        * ActivityTable::Instance().GetElement(eActivityID::E_ACTIVITY_HUNT)->times_max;
    for (int32_t j = 0; j < times; ++j)
    {
        EXPECT_EQ(OR_OK, o.ParticipateActivity(eActivityID::E_ACTIVITY_HUNT));
       
        EXPECT_EQ(OR_QUEST_TYPE_UNIQUE, o.ParticipateActivity(eActivityID::E_ACTIVITY_HUNT));
        int32_t questid = oQuestManager.GetQuestFromType(AvatarQuest::E_LIEMO);
        oQuestManager.DirectToCompleteQuest(questid);

    }
    EXPECT_EQ(1, o.GetRewardTimes(eActivityID::E_ACTIVITY_HUNT));
}

TEST(QuestTest, Wuzhengqin)
{
    AvatarQuest::QuestList oQuestManager;
    ActivityQuestManger o(&oQuestManager);
    for (size_t i = 0; i < 2; i++)
    {
        o.OnNewDayCome();
        Wuzhengqin(oQuestManager, o);
    }
}

TEST(QuestTest, QuestTimeOutFailed)
{
    AvatarQuest::QuestList oQuestManager;
    oQuestManager.AcceptQuest(11018);
    EXPECT_EQ(AvatarQuest::E_QUEST_STATUS_ACCEPTED, oQuestManager.GetQuestStatus(11018));
    sleep(1);
    pLoop->loop();
    EXPECT_EQ(AvatarQuest::E_QUEST_STATUS_TIEMOUT, oQuestManager.GetQuestStatus(11018));

    QuestData db;
    db.set_questbegintime(1);
    db.set_configid(11018); 
    AvatarQuest::CreateQuestParam p;
    p.quest_factory_ptr_.reset(new AvatarQuest::QuestFactory());
    AvatarQuest::QuestList::quest_ptr pt(new AvatarQuest::Quest(db, p));
    oQuestManager.AddTestQuest(pt);
    EXPECT_TRUE(pt->IsTimeOut());

    db.set_questbegintime(0);
    db.set_configid(11018);
    AvatarQuest::QuestList::quest_ptr pt1(new AvatarQuest::Quest(db, p));
    pt1->m_bTestTimeOutComplete = true;
    oQuestManager.AddTestQuest(pt1);
    sleep(1);
    pLoop->loop();
    EXPECT_TRUE(pt1->IsComleted());
    EXPECT_EQ(0, oQuestManager.GetEventObserverSize());
}

TEST(QuestTest, JingyingGuai)
{
    AvatarQuest::QuestList oQuestManager;
   
    NotRepetitionActivity(oQuestManager,  E_ACTIVITY_ZHANXING);
}

//TEST(QuestTest, AcceptNextQuest)
//{
//    AvatarQuest::QuestManager oQuestManager;
//    int32_t iq = 151021;
//    int32_t nq = 61001;
//    oQuestManager.AcceptQuest(iq);
//    oQuestManager.DirectToCompleteQuest(iq);
//    EXPECT_FALSE(oQuestManager.FindAcceptedQuest(61001));
//
// 
//    EXPECT_EQ(OR_QUEST_COMPLETED, oQuestManager.AcceptQuest(nq));
//}

TEST(QuestTest, AcceptNextQuest)
{
    //AvatarQuest::QuestManager oQuestManager;
    //int32_t iq = 151021;
    //int32_t nq = 61001;
    //oQuestManager.AcceptQuest(iq);
    //oQuestManager.DirectToCompleteQuest(iq);
    //EXPECT_FALSE(oQuestManager.FindAcceptedQuest(61001));


    //EXPECT_EQ(OR_QUEST_COMPLETED, oQuestManager.AcceptQuest(nq));
}

TEST(QuestTest, AcceptCehua)
{
    AvatarQuest::QuestList oQuestManager;
    EXPECT_EQ(OR_OK, oQuestManager.AcceptQuest(41011));
    EXPECT_TRUE(oQuestManager.FindAcceptedQuest(41011));
}


TEST(QuestTest, PrevAcceptQuest)
{
   /*  AvatarQuest::QuestManager oQuestManager;
     int32_t iq = 200125;
     int32_t nq = 200126;
     EXPECT_EQ(OR_OK, oQuestManager.AcceptQuest(iq));
     EXPECT_EQ(OR_QUEST_PREV_QUEST_UNCOMPLETE, oQuestManager.AcceptQuest(nq));*/

}

TEST(QuestTest, CompletQuestAddProgress)
{
    AvatarQuest::QuestList oQuestManager;
    int32_t iq = 11020;
    int32_t nq = 11001;
    EXPECT_EQ(OR_OK, oQuestManager.AcceptQuest(iq));
    EXPECT_EQ(OR_OK, oQuestManager.AcceptQuest(nq));
    EXPECT_EQ(OR_OK, oQuestManager.DirectToCompleteQuest(nq));
    EXPECT_EQ(QuestTable::Instance().GetElement(nq)->complete_quest_add_progress_type_, oQuestManager.GetQuest(iq)->GetJustOneTagetQuestProgress());
}

TEST(QuestTest, CompletQuestAddProgress1)
{
    AvatarQuest::QuestList oQuestManager;
    int32_t iq = 11021;
    int32_t nq = 11010;
    int32_t nq1 = 11011;
    oQuestManager.SetCheckUniqueQuest(false);
    EXPECT_EQ(OR_OK, oQuestManager.AcceptQuest(iq));
    EXPECT_EQ(OR_OK, oQuestManager.AcceptQuest(nq));
    EXPECT_EQ(OR_OK, oQuestManager.AcceptQuest(nq1));
    EXPECT_EQ(OR_OK, oQuestManager.DirectToCompleteQuest(nq));
    EXPECT_EQ(OR_OK, oQuestManager.DirectToCompleteQuest(nq1));
    EXPECT_TRUE(oQuestManager.FindCompletedQuest(iq));
}
TEST(QuestTest, NewDayQuest)
{
    AvatarQuest::QuestList oQuestManager;
    DailyQuestManager dQuest(&oQuestManager);
    ActivityQuestManger o(&oQuestManager);
    o.OnDBComplete();
    o.OnDBComplete();
    o.OnNewDayCome();
    dQuest.OnNewDayCome();
    for (auto && it : LevelTable::Instance().GetElement(oQuestManager.GetLevel())->daily_quest_id)
    {
        EXPECT_EQ(OR_OK, oQuestManager.DirectToCompleteQuest(it));
    }
    o.OnDBComplete();
    o.OnNewDayCome();
    dQuest.OnNewDayCome();
    for (auto && it : LevelTable::Instance().GetElement(oQuestManager.GetLevel())->daily_quest_id)
    {
        EXPECT_TRUE(oQuestManager.FindAcceptedQuest(it));
    }
}

TEST(QuestTest, EventTrigger)
{
    AvatarQuest::QuestList oQuestManager;
    int32_t nQuestId = 11009;
    EXPECT_EQ(OR_OK, oQuestManager.AcceptQuest(nQuestId));
    oQuestManager.DirectToCompleteQuest(nQuestId);
    nQuestId = 11010;
    EXPECT_EQ(OR_OK, oQuestManager.AcceptQuest(nQuestId));
    oQuestManager.DirectToCompleteQuest(nQuestId);
    EXPECT_EQ(0, oQuestManager.GetEventObserverSize());
}

TEST(QuestTest, Quest70190)
{
    /*  AvatarQuest::QuestManager oQuestManager;
      oQuestManager.LevelUp(1);
      int32_t nQuestId = 70190;
      EXPECT_EQ(OR_OK, oQuestManager.AcceptQuest(nQuestId));
      QuestRpcCompleteQuestStepAsk ask;
      QuestRpcCompleteQuestStepReply reply;
      ask.set_questconfigid(nQuestId);
      ask.set_queststepeventtype(AvatarQuest::E_QUEST_TALKING_WITH_NPC);
      ask.set_queststep(0);
      ask.set_targetid(QuestTable::Instance().GetElement(nQuestId)->target_1_id[0]);
      EXPECT_EQ(OR_OK, oQuestManager.RpcCompleteQuestStep(ask, reply));
      EXPECT_TRUE(oQuestManager.FindCompletedQuest(nQuestId));
      EXPECT_FALSE(oQuestManager.FindAchievedQuest(nQuestId));
      nQuestId = 70195;
      EXPECT_FALSE(oQuestManager.FindCompletedQuest(nQuestId));
      EXPECT_FALSE(oQuestManager.FindAchievedQuest(nQuestId));
      ask.set_questconfigid(nQuestId);
      ask.set_queststepeventtype(AvatarQuest::E_QUEST_TALKING_WITH_NPC);
      ask.set_queststep(0);
      ask.set_targetid(QuestTable::Instance().GetElement(nQuestId)->target_1_id[0]);
      EXPECT_EQ(OR_OK, oQuestManager.RpcCompleteQuestStep(ask, reply));
      EXPECT_TRUE(oQuestManager.FindCompletedQuest(nQuestId));
      EXPECT_FALSE(oQuestManager.FindAchievedQuest(nQuestId));

      nQuestId = 70197;
      EXPECT_FALSE(oQuestManager.FindCompletedQuest(nQuestId));
      EXPECT_FALSE(oQuestManager.FindAchievedQuest(nQuestId));

      oQuestManager.LevelUp(QuestTable::Instance().GetElement(nQuestId)->target_1_amount);

      EXPECT_TRUE(oQuestManager.FindCompletedQuest(nQuestId));
      EXPECT_FALSE(oQuestManager.FindAchievedQuest(nQuestId));

      nQuestId = 70200;
      EXPECT_FALSE(oQuestManager.FindCompletedQuest(nQuestId));
      EXPECT_FALSE(oQuestManager.FindAchievedQuest(nQuestId));
      ask.set_questconfigid(nQuestId);
      ask.set_queststepeventtype(AvatarQuest::E_QUEST_TALKING_WITH_NPC);
      ask.set_queststep(0);
      ask.set_targetid(QuestTable::Instance().GetElement(nQuestId)->target_1_id[0]);
      EXPECT_EQ(OR_OK, oQuestManager.RpcCompleteQuestStep(ask, reply));
      EXPECT_TRUE(oQuestManager.FindCompletedQuest(nQuestId));
      EXPECT_FALSE(oQuestManager.FindAchievedQuest(nQuestId));

      nQuestId = 70210;
      EXPECT_FALSE(oQuestManager.FindCompletedQuest(nQuestId));
      EXPECT_FALSE(oQuestManager.FindAchievedQuest(nQuestId));
      ask.set_questconfigid(nQuestId);
      ask.set_queststepeventtype(AvatarQuest::E_QUEST_TALKING_WITH_NPC);
      ask.set_queststep(0);
      ask.set_targetid(QuestTable::Instance().GetElement(nQuestId)->target_1_id[0]);
      EXPECT_EQ(OR_OK, oQuestManager.RpcCompleteQuestStep(ask, reply));
      EXPECT_TRUE(oQuestManager.FindCompletedQuest(nQuestId));
      EXPECT_FALSE(oQuestManager.FindAchievedQuest(nQuestId));*/
}

TEST(QuestTest, AccountQuestSize)
{
    AvatarQuest::QuestList oQuestManager;
    ActivityQuestManger o(&oQuestManager);
    o.OnDBComplete();
    oQuestManager.LevelUp(60);
    EXPECT_EQ(OR_OK, o.ParticipateActivity(eActivityID::E_ACTIVITY_ZHANXING));
    EXPECT_EQ(OR_QUEST_TYPE_UNIQUE, o.ParticipateActivity(eActivityID::E_ACTIVITY_ZHANXING));
    
    EXPECT_EQ(1, oQuestManager.GetAcceptQuestSizeFromQuestType(AvatarQuest::E_ZHANXING));
    EXPECT_EQ(OR_OK, o.ParticipateActivity(eActivityID::E_ACTIVITY_HUNT));
    EXPECT_EQ(OR_QUEST_TYPE_UNIQUE, o.ParticipateActivity(eActivityID::E_ACTIVITY_HUNT));
    EXPECT_EQ(1, oQuestManager.GetAcceptQuestSizeFromQuestType(AvatarQuest::E_LIEMO));
    oQuestManager.CompletetQuestFromQuestType(AvatarQuest::E_ZHANXING);
    oQuestManager.CompletetQuestFromQuestType(AvatarQuest::E_LIEMO);
    EXPECT_EQ(0, oQuestManager.GetAcceptQuestSizeFromQuestType(AvatarQuest::E_ZHANXING));
    EXPECT_EQ(0, oQuestManager.GetAcceptQuestSizeFromQuestType(AvatarQuest::E_LIEMO));
}


TEST(QuestTest, Order)
{
    AvatarQuest::QuestList oQuestManager;
    int32_t quest_id = 11026;
    const QuestElement * pQuestElement = QuestTable::Instance().GetElement(quest_id);


    oQuestManager.AcceptQuest(quest_id);
    AvatarQuest::QuestEvent oQuestEvent(pQuestElement->target_1_type, pQuestElement->target_1_id[0], pQuestElement->target_1_amount);
    AvatarQuest::QuestEvent oQuestEvent1(pQuestElement->target_2_type, pQuestElement->target_2_id[0], pQuestElement->target_2_amount);
    AvatarQuest::QuestEvent oQuestEvent2(pQuestElement->target_3_type, pQuestElement->target_3_id[0], pQuestElement->target_3_amount);
    AvatarQuest::QuestEvent oQuestEvent3(pQuestElement->target_4_type, pQuestElement->target_4_id[0], pQuestElement->target_4_amount);
    AvatarQuest::QuestEvent oQuestEvent4(pQuestElement->target_5_type, pQuestElement->target_5_id[0], pQuestElement->target_5_amount);

    std::vector<AvatarQuest::QuestEvent> v_quest_event{ oQuestEvent , oQuestEvent1, oQuestEvent2,oQuestEvent3, oQuestEvent4};
    for (int32_t i = 5; i > 0; --i) 
    {
        for (int32_t j = i; j > 0; --j)
        {
            auto& oevent = v_quest_event[j - 1];
            oQuestManager.TriggerEvent(oevent.GetEventType(), oevent.GetTargetId(), oevent.GetCount());
        }
    }

    
    EXPECT_FALSE(oQuestManager.FindCompletedQuest(quest_id));

    for (int32_t i = 0; i < 5; ++i)
    {
        auto& oevent = v_quest_event[i];
        oQuestManager.TriggerEvent(oevent.GetEventType(), oevent.GetTargetId(), oevent.GetCount());
    }
    EXPECT_TRUE(oQuestManager.FindCompletedQuest(quest_id));
}

TEST(QuestTest, ResetQuest)
{
    AvatarQuest::QuestList oQuestManager;

    int32_t nConfigId = 11015;


    const QuestElement * pQuestElement = QuestTable::Instance().GetElement(nConfigId);
    QuestElement * pChageQuestElement = const_cast<QuestElement *>(pQuestElement);
    pChageQuestElement->target_2_id.push_back(0);
    pChageQuestElement->target_3_id.push_back(0);
    pChageQuestElement->target_4_id.push_back(0);
    pChageQuestElement->target_5_id.push_back(0);

    oQuestManager.AcceptQuest(nConfigId);

    oQuestManager.TriggerEvent(pQuestElement->target_1_type, pQuestElement->target_1_id[0], pQuestElement->target_1_amount);
    EXPECT_EQ(OR_QUEST_ACHIVE_ERROR, oQuestManager.CompleteQuest(nConfigId));


    oQuestManager.TriggerEvent(pQuestElement->target_2_type, pQuestElement->target_2_id[0], pQuestElement->target_2_amount);
    EXPECT_EQ(OR_QUEST_ACHIVE_ERROR, oQuestManager.CompleteQuest(nConfigId));

    oQuestManager.TriggerEvent(pQuestElement->target_3_type, pQuestElement->target_3_id[0], pQuestElement->target_3_amount);
    EXPECT_EQ(OR_QUEST_ACHIVE_ERROR, oQuestManager.CompleteQuest(nConfigId));

    oQuestManager.TriggerEvent(pQuestElement->target_4_type, pQuestElement->target_4_id[0], pQuestElement->target_4_amount);
    EXPECT_EQ(OR_QUEST_ACHIVE_ERROR, oQuestManager.CompleteQuest(nConfigId));

    oQuestManager.ResetQuest(nConfigId);
    EXPECT_EQ(OR_QUEST_ACHIVE_ERROR, oQuestManager.CompleteQuest(nConfigId));

    oQuestManager.TriggerEvent(pQuestElement->target_1_type, pQuestElement->target_1_id[0], pQuestElement->target_1_amount);
    oQuestManager.TriggerEvent(pQuestElement->target_2_type, pQuestElement->target_2_id[0], pQuestElement->target_2_amount);
    oQuestManager.TriggerEvent(pQuestElement->target_3_type, pQuestElement->target_3_id[0], pQuestElement->target_3_amount);
    oQuestManager.TriggerEvent(pQuestElement->target_4_type, pQuestElement->target_4_id[0], pQuestElement->target_4_amount);
    oQuestManager.TriggerEvent(pQuestElement->target_5_type, pQuestElement->target_5_id[0], pQuestElement->target_5_amount);
    
    EXPECT_EQ(OR_QUEST_COMPLETED, oQuestManager.CompleteQuest(nConfigId));
    
    EXPECT_EQ(0, oQuestManager.GetEventObserverSize());
}

TEST(QuestTest, ResetQuestPB)
{
    AvatarQuest::QuestList oQuestManager;
    int32_t nConfigId = 11015;
    

    const QuestElement * pQuestElement = QuestTable::Instance().GetElement(nConfigId);
    QuestElement * pChageQuestElement = const_cast<QuestElement *>(pQuestElement);
    pChageQuestElement->target_2_id.push_back(0);
    pChageQuestElement->target_3_id.push_back(0);
    pChageQuestElement->target_4_id.push_back(0);
    pChageQuestElement->target_5_id.push_back(0);

    oQuestManager.AcceptQuest(nConfigId);
    QuestData pb = oQuestManager.GetQuestPb(nConfigId);
    oQuestManager.TriggerEvent(pQuestElement->target_1_type, pQuestElement->target_1_id[0], pQuestElement->target_1_amount);
    EXPECT_EQ(OR_QUEST_ACHIVE_ERROR, oQuestManager.CompleteQuest(nConfigId));

  
    oQuestManager.TriggerEvent(pQuestElement->target_2_type, pQuestElement->target_2_id[0], pQuestElement->target_2_amount);
    EXPECT_EQ(OR_QUEST_ACHIVE_ERROR, oQuestManager.CompleteQuest(nConfigId));
    oQuestManager.TriggerEvent(pQuestElement->target_3_type, pQuestElement->target_3_id[0], pQuestElement->target_3_amount);
    EXPECT_EQ(OR_QUEST_ACHIVE_ERROR, oQuestManager.CompleteQuest(nConfigId));
    oQuestManager.TriggerEvent(pQuestElement->target_4_type, pQuestElement->target_4_id[0], pQuestElement->target_4_amount);
    EXPECT_EQ(OR_QUEST_ACHIVE_ERROR, oQuestManager.CompleteQuest(nConfigId));
    AvatarQuest::QuestEvent oQuestEvent5(pQuestElement->target_5_type, pQuestElement->target_5_id[0], pQuestElement->target_5_amount);

    oQuestManager.ResetQuest(pb);
    EXPECT_EQ(OR_QUEST_ACHIVE_ERROR, oQuestManager.CompleteQuest(nConfigId));

    oQuestManager.TriggerEvent(pQuestElement->target_1_type, pQuestElement->target_1_id[0], pQuestElement->target_1_amount);
    oQuestManager.TriggerEvent(pQuestElement->target_2_type, pQuestElement->target_2_id[0], pQuestElement->target_2_amount);
    oQuestManager.TriggerEvent(pQuestElement->target_3_type, pQuestElement->target_3_id[0], pQuestElement->target_3_amount);
    oQuestManager.TriggerEvent(pQuestElement->target_4_type, pQuestElement->target_4_id[0], pQuestElement->target_4_amount);
    oQuestManager.TriggerEvent(pQuestElement->target_5_type, pQuestElement->target_5_id[0], pQuestElement->target_5_amount);

    EXPECT_EQ(OR_QUEST_COMPLETED, oQuestManager.CompleteQuest(nConfigId));


    EXPECT_EQ(0, oQuestManager.GetEventObserverSize());
}

TEST(QuestTest, ForTrigger)
{
    AvatarQuest::QuestList oQuestManager;
    int32_t firstId = QuestTable::Instance().GetAllID()[11];
    int32_t secondId = QuestTable::Instance().GetAllID()[12];
    oQuestManager.AcceptQuest(firstId);
    {
        auto ptr =  oQuestManager.CreateForTrigger();
        EXPECT_TRUE(oQuestManager.ForTrigger());
        for (size_t i = 0; i < 11; i++)
        {
            oQuestManager.TriggerEvent(QuestTable::Instance().GetElement(firstId)->target_1_type, QuestTable::Instance().GetElement(firstId)->target_1_id[0], QuestTable::Instance().GetElement(firstId)->target_1_amount);
            EXPECT_TRUE(oQuestManager.ForTrigger());
        }
        EXPECT_TRUE(oQuestManager.ForTrigger());
    }
    EXPECT_FALSE(oQuestManager.ForTrigger());
}

TEST(QuestTest, prequeest)
{
    AvatarQuest::QuestList oQuestManager;

    EXPECT_EQ(OR_QUEST_PREV_QUEST_UNCOMPLETE, oQuestManager.AcceptQuest(190014));
}


class AchievementCalaclatorMock
{
public:

    AchievementCalaclatorMock(AvatarQuest::QuestList * pQuestManager)
        :m_pQuestManager(pQuestManager)
    {
        auto cb = std::bind(&AchievementCalaclatorMock::OnAcceptQuest, this, std::placeholders::_1);
        m_pQuestManager->SetQuestAcceptCalculator(cb);
    }

    void OnAcceptQuest(int32_t quest_id)
    {
        auto pQuestElement = AchievementDataTable::Instance().GetElement(quest_id);
        if (nullptr == pQuestElement)
        {
            return;
        }
        TriggerType(quest_id, pQuestElement->type1);
        TriggerType(quest_id, pQuestElement->type2);
        TriggerType(quest_id, pQuestElement->type3);
        TriggerType(quest_id, pQuestElement->type4);
    }

    void   TriggerType(int32_t quest_id, int32_t nType)
    {
        auto pQuestElement = AchievementDataTable::Instance().GetElement(quest_id);
        if (nullptr == pQuestElement)
        {
            return;
        }
        switch (nType)
        {
        case AvatarQuest::E_ACHIEVEMENT_PARENT_ACHIVEMENT:
        {

            for (auto& sub_quest_id : pQuestElement->achievement_ids)
            {
                bool check = m_pQuestManager->FindCompletedQuest(sub_quest_id) ||
                    m_pQuestManager->FindAchievedQuest(sub_quest_id);
                if (!check)
                {
                    continue;
                }
                using ia = std::array<int32_t, 5>;
                ia  params = { sub_quest_id, 0, 0, 0, 0 };
                AvatarQuest::QuestEvent oEvent1(AvatarQuest::E_ACHIEVEMENT_PARENT_ACHIVEMENT, params, 1);
                m_pQuestManager->TriggerEvent(oEvent1);
            }

            break;
        }
        }
    }

    AvatarQuest::QuestList * m_pQuestManager{ nullptr };
};


TEST(QuestTest, baoshiachievment)
{
    auto p = AvatarQuest::AchievementFactory::GetInstance();
    AvatarQuest::AchievmentList oQuestManager(nullptr, p);

    AchievementCalaclatorMock oc(&oQuestManager);
    int32_t second_size = 5;
    int32_t first_achievment = 3093;
    int32_t second_achievment = 3094;

    auto p_first = AchievementDataTable::Instance().GetElement(first_achievment);
    for (auto& it : p_first->achievement_ids)
    {
        EXPECT_EQ(OR_OK, oQuestManager.AcceptQuest(it));
    }
    EXPECT_EQ(OR_OK, oQuestManager.AcceptQuest(first_achievment));

    {
        int32_t tempsecond = second_achievment;
        for (int32_t i = 0; i < second_size; ++i)
        {
            auto p_second = AchievementDataTable::Instance().GetElement(tempsecond);
            for (auto& it : p_second->achievement_ids)
            {
                EXPECT_EQ(OR_OK, oQuestManager.AcceptQuest(it));
            }
            ++tempsecond;
        }
    }
    
    std::size_t i = 0; 
    for (auto& it : p_first->achievement_ids)
    {
        auto p_condition = AchievementDataTable::Instance().GetElement(it);
        AvatarQuest::QuestEvent::param_vector_type params = { p_condition->type1_parameter1[0], 0, 0, 0, 0 };
        AvatarQuest::QuestEvent oEvent(AvatarQuest::E_ACHIEVEMENT_GET_ITEM, params, 1);
        oQuestManager.TriggerEvent(oEvent);
        if (++i >= p_first->achievement_ids.size())
        {
            break;
        }
        EXPECT_FALSE(oQuestManager.FindAchievedQuest(first_achievment));
        EXPECT_FALSE(oQuestManager.FindCompletedQuest(first_achievment));
    }
    EXPECT_TRUE(oQuestManager.FindAchievedQuest(first_achievment));
    EXPECT_FALSE(oQuestManager.FindCompletedQuest(first_achievment));

    {
        int32_t tempsecond = second_achievment;
        for (int32_t j = 0; j < second_size; ++j)
        {  
            EXPECT_FALSE(oQuestManager.FindAchievedQuest(tempsecond));
            EXPECT_FALSE(oQuestManager.FindCompletedQuest(tempsecond));

            auto p_second = AchievementDataTable::Instance().GetElement(tempsecond);
            std::size_t i = 0;
            for (auto& it : p_second->achievement_ids)
            {
                auto p_condition = AchievementDataTable::Instance().GetElement(it);
                AvatarQuest::QuestEvent::param_vector_type params = { p_condition->type1_parameter1[0], 0, 0, 0, 0 };
                AvatarQuest::QuestEvent oEvent(AvatarQuest::E_ACHIEVEMENT_GET_ITEM, params, 1);
                oQuestManager.TriggerEvent(oEvent);
                if (++i >= p_first->achievement_ids.size())
                {
                    break;
                }
                EXPECT_FALSE(oQuestManager.FindAchievedQuest(tempsecond));
                EXPECT_FALSE(oQuestManager.FindCompletedQuest(tempsecond));
            }
            //EXPECT_TRUE(oQuestManager.FindAchievedQuest(tempsecond));
            ++tempsecond;
        }
    }
    
   
}

TEST(QuestTest, calcaacceptevment)
{
    auto p = AvatarQuest::AchievementFactory::GetInstance();
    AvatarQuest::AchievmentList oQuestManager(nullptr, p);

    AchievementCalaclatorMock oc(&oQuestManager);

    int32_t first_achievment = 3093;
    int32_t second_achievment = 3094;
    int32_t second_size = 6;
    auto p_first = AchievementDataTable::Instance().GetElement(first_achievment);
    for (auto& it : p_first->achievement_ids)
    {
        EXPECT_EQ(OR_OK, oQuestManager.AcceptQuest(it));
    }
    EXPECT_EQ(OR_OK, oQuestManager.AcceptQuest(first_achievment));

    {
        int32_t tachievment = second_achievment;
        for (int32_t i = 0; i < second_size; ++i, ++tachievment)
        {
            auto p_second = AchievementDataTable::Instance().GetElement(tachievment);
            for (auto& it : p_second->achievement_ids)
            {
                EXPECT_EQ(OR_OK, oQuestManager.AcceptQuest(it));
            }
        }

    }

    {
        int32_t tachievment = second_achievment;

        std::size_t i = 0;
        for (int32_t j = 0; j < second_size; ++j, ++tachievment)
        {
            auto p_second = AchievementDataTable::Instance().GetElement(tachievment);
            for (auto& it : p_second->achievement_ids)
            {
                auto p_condition = AchievementDataTable::Instance().GetElement(it);
                AvatarQuest::QuestEvent::param_vector_type params = { p_condition->type1_parameter1[0], 0, 0, 0, 0 };
                AvatarQuest::QuestEvent oEvent(AvatarQuest::E_ACHIEVEMENT_GET_ITEM, params, 1);
                oQuestManager.TriggerEvent(oEvent);
                EXPECT_FALSE(oQuestManager.FindAchievedQuest(tachievment));
                EXPECT_FALSE(oQuestManager.FindCompletedQuest(tachievment));
            }
        }

    }


    std::size_t i = 0;
    for (auto& it : p_first->achievement_ids)
    {
        auto p_condition = AchievementDataTable::Instance().GetElement(it);
        AvatarQuest::QuestEvent::param_vector_type params = { p_condition->type1_parameter1[0], 0, 0, 0, 0 };
        AvatarQuest::QuestEvent oEvent(AvatarQuest::E_ACHIEVEMENT_GET_ITEM, params, 1);
        oQuestManager.TriggerEvent(oEvent);
    }

    EXPECT_TRUE(oQuestManager.FindAchievedQuest(first_achievment));
    EXPECT_FALSE(oQuestManager.FindCompletedQuest(first_achievment));

    {
        int32_t tachievment = second_achievment;
        for (int32_t j = 0; j < second_size; ++j, ++tachievment)
        {
            //EXPECT_TRUE(oQuestManager.FindAchievedQuest(tachievment));
            EXPECT_FALSE(oQuestManager.FindCompletedQuest(tachievment));
        }
    }
}

int main(int argc, char **argv)
{
    muduo::net::EventLoop loop;
    pLoop = &loop;
    BaseModule::SetThreadLocalStorageLoop(pLoop);
    ModuleConfig::Instance().Initialize();
    testing::InitGoogleTest(&argc, argv);
    ActivityQuestRelation::Instance().OnConfigLoaded();
    AvatarQuest::AchievementFactory::OnAchievmentDataTableLoaded();
    while (true)
    {
        RUN_ALL_TESTS();
    }

    return RUN_ALL_TESTS();
}

