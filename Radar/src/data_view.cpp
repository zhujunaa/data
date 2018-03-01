#include <stdio.h>
#include <string.h>

#include"stock_signal.h"
#include "data_view.h"
#include "md_client.h"
#include"strategy_base.h"
#include "ui.h"

#include "stock_msg.h"
extern  QVector<DiagramDrawCallbackData *> g_draw_list;
void SubScribeMarketData(DiagramDataMgr * mgr,const char * id,int yes)
{
    E15_ClientMsg cmd;
    if( yes )
        cmd.cmd = Stock_Msg_SubscribeById;
    else
        cmd.cmd = Stock_Msg_UnSubscribeById;
//    cmd.cmd = Stock_Msg_SubscribeAll;
    cmd.status = 0;
    E15_ValueTable vt;
    E15_Value * v = vt.InsertS("id_list");
    v->Init(E15_Value::Value_STRINGARRAY);
    v->AppendBytes(id,-1);
    E15_String s;
    vt.Dump(&s);
    g_client->Request(& ( (DiagramDataMgrView*)mgr->m_params)->m_market_src,&cmd,s.c_str(),s.Length() );
}


void get_cache_data(DiagramDataMgr * mgr,const char * id)
{
    E15_ClientMsg cmd;
    cmd.cmd = Stock_Msg_ReqDiagramCache;
    cmd.status = 0;
    E15_ValueTable vt;
    E15_Value * v = vt.InsertS("id_list");
    v->Init(E15_Value::Value_STRINGARRAY);
    v->AppendBytes(id,strlen(id));
    E15_String s;
    vt.Dump(&s);
    g_client->Request(& ( (DiagramDataMgrView*)mgr->m_params)->m_market_src,&cmd,s.c_str(),s.Length() );
}

int UpdateDiagramView(const char * key,unsigned long ukey,void * data,void * param)
{
//    E15_Debug::Printf(0," key = %s   ukey = %d\n",key,ukey);
    if( !strcmp(key,"draw") == 0&&!strcmp(key,"cycle") == 0 )
		return 0;
    g_ui_loop->Post(0,0,E15_UI_Message_DepthMarket,ukey,0);
//     E15_Debug::Printf(0," wwwwwwwwwwwwwwwwwwwwwwwwwwwww\n");
	return 0;
}
extern int g_top_button_index;
extern int g_one_key_cycle;//一键多周期
void DiagramDataMgr_UpdateAllView(DiagramDataMgr * stock)
{
    if(g_top_button_index==1)
    {
        for(int i =0;i<g_draw_list.size();i++)
        {
            if(g_draw_list[i]->m_ins_id.cObj)
            {
                if(g_draw_list[i]->m_ins_id.Equal(stock->info.id))
                    ((DiagramDataMgrView*)stock->m_params)->view_hash.each(UpdateDiagramView,0);
            }
        }
    }
    if(g_one_key_cycle)
    {
        if(Cycle::cycle_id==QString(stock->info.id))
            ((DiagramDataMgrView*)stock->m_params)->view_hash.each(UpdateDiagramView,0);
    }
}


