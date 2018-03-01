#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include<E15_value.h>
#include <string.h>
#include "E15_debug.h"
#include"E15_thread.h"
#include "md_client.h"
#include "ui.h"
#include<E15_server.h>
#include<QSemaphore>
#include "data_view.h"
#include "stock_msg.h"
#include"stock_signal.h"
#include<stock_data.h>
extern int g_klineCount ;
void strategy_t1706(const char * id);
extern Stock_Signal *g_signal;
extern int g_historyCnt;
extern E15_Id infoSeverId;//记录服务器ID
extern long long g_begin_date;
extern long long g_end_date;
extern long long g_begin_time;
extern long long g_end_time;
extern int g_top_button_index;
extern QSemaphore semaphore;
extern  QVector<DiagramDrawCallbackData *> g_draw_list;
extern QVector<QStringList > g_optional_id_list;//0:全部1:黑色2:金属3:轻工业品4:5;6: 自选
QList<MarketDepthData> T1706_List;
extern int g_one_key_cycle;//一键多周期
void load_buy(E15_String buy);
E15_ThreadFunc sql_save();
QMap<QString, int> g_description_index;//5biricktick
MD_Client::MD_Client()
{
    m_log.Init("buy.log",100);
}

MD_Client::~MD_Client()
{
}

void MD_Client::OnLoginOk(E15_ClientInfo * info,E15_String *& json)
{
    E15_Debug::Printf(0," %s [%x:%x] (name=%s:role=%s) 上线\n",__FUNCTION__,info->id.h,info->id.l,info->name,info->role);
    E15_Debug::Log(0,"[%x:%x] (name=%s:role=%s) 上线\n",info->id.h,info->id.l,info->name,info->role);

}

int MD_Client::OnLogout(E15_ClientInfo * info)
{
    E15_Debug::Printf(0,"%s  [%x:%x] (name=%s:role=%s) 下线\n",__FUNCTION__,info->id.h,info->id.l,info->name,info->role);
    E15_Debug::Log(0,"[%x:%x] (name=%s:role=%s) 下线\n",info->id.h,info->id.l,info->name,info->role);

    return 1000;//自动重联
}

int MD_Client::OnLoginFailed(E15_ClientInfo * info,int status,const char * errmsg)
{
    E15_Debug::Printf(0,"%s  连接 (name=%s:role=%s) 失败 %s\n",__FUNCTION__,info->name,info->role,errmsg);
    E15_Debug::Log(0,"连接 (name=%s:role=%s) 失败 %s\n",info->name,info->role,errmsg);
    return 2000;
}


void MD_Client::OnRequest(E15_ClientInfo * info,E15_ClientMsg * cmd,E15_String *&  json)
{
    E15_Debug::Printf(0,"[%x:%x] (name=%s:role=%s) OnRequest\n",info->id.h,info->id.l,info->name,info->role);
    E15_Debug::Log(0,"[%x:%x] (name=%s:role=%s) OnRequest\n",info->id.h,info->id.l,info->name,info->role);
}


void MD_Client::OnResponse(E15_ClientInfo * info,E15_ClientMsg * cmd,E15_String *&  json)
{
    E15_Debug::Log(0,"[%x:%x] 响应(cmd=%u,seq=%u,status=%d)\n",info->id.h,info->id.l,cmd->cmd,cmd->seq,cmd->status);
    E15_Debug::Printf(0,"[%x:%x] 响应(cmd=%u,seq=%u,status=%d)\n",info->id.h,info->id.l,cmd->cmd,cmd->seq,cmd->status);
    E15_ValueTable vt;
//    E15_Value * v = vt.InsertS("id_list");
//    v->Init(E15_Value::Value_STRINGARRAY);
//    v->AppendBytes(id,strlen(id));
//    E15_String s;
//    vt.Dump(&s);

    switch (cmd->cmd) {
    case Stock_Msg_DiagramHistoryReq:
    {
        int index = cmd->receiver.l;
        if(index<0||index>8)
            return;
        //加载买卖点历史数据
        E15_String ss;
        E15_ValueTable vt;
        QString bt = QString("%1").arg(g_begin_time, 9, 10, QChar('0'));
        QString et = QString("%1").arg(g_end_time, 9, 10, QChar('0'));
        QString begin = QString::number(g_begin_date);
        begin += bt;
        QString end = QString::number(g_end_date);
        end += et;
        if(end.toLongLong() < begin.toLongLong())
            return;
        vt.SetSInt64("begin_time",begin.toLongLong());
        vt.SetSInt64("end_time",end.toLongLong());
        ss.Strcpy(g_draw_list[index]->m_ins_id.c_str());
        if(g_draw_list[index]->view.m_strategy_list.size()==0)
            return;
        vt.InsertS("stg_id_list")->SetIntArray(&g_draw_list[index]->view.m_strategy_list[0], g_draw_list[index]->view.m_strategy_list.size());
        ss.Resize(InstrumentID_MaxLengh,0);
//        vt.Print();
        vt.Dump(&ss);
        load_buy(ss);
    }
        break;
    case Stock_Msg_ReqDiagramCache:
    {
        vt.Import(json->c_str(),json->Length());
//        vt.Print();
//        E15_StringArray *sa  = vt.ValueS("id_list")->GetStringArray();
//        for (unsigned long i = 0; i < sa->Size(); ++i){
//            const char *id = sa->At(i)->c_str();
//            int market = MarketCodeById(id);
//            DiagramDataMgr * stock = DiagramDataMgr_PeekData(market,id);
//            if( !stock )
//                break;
//            semaphore.release();
//        }
         break;
    }
    case Trade_Msg_SnatchSet:
    {
        E15_String s = *json;
        g_signal->clientToStrategy(s);
        break;
    }
    default:
        break;
    }
}
HintInfo read_id_speed(QString id);

static int handler_contract_info(E15_Key * key,E15_Value * info,E15_ClientInfo * p)
{
	const char * id = key->GetS();
    if( !id)
		return 0;
    static int count = 0;

	const char * name = info->BytesS("name",0);
	if( !name )
		return 0;
      int market = MarketCodeById(id);////info->BaseS("market");

	DiagramDataMgr * data = DiagramDataMgr_GetData(market,id,name,0);
	if( !(DiagramDataMgrView*)data->m_params )
		data->m_params = new  DiagramDataMgrView;
	((DiagramDataMgrView*)data->m_params)->m_market_src = p->id;

	data->info.price_tick = info->BaseS("tick");
	data->info.Multiple = info->BaseS("Multiple");
    data->info.product = info->BytesS("product",0);
    data->market_flag = info->BaseS("amplification");//1 外盘
    data->info.master_flag = info->BaseS("pri");//主力标记
    if(data->info.master_flag==1)
     {
        count++;
//        E15_Debug::Printf(0,"主力 id = %s ,count = %d\n",id,count);
    }
//    E15_Debug::Printf(0,"data->market_flag   =%d \n",data->market_flag );
//    E15_Debug::Printf(0,"data->info.price_tick = %d  id = %s ,name = %s,data->info.product =%s\n",data->info.price_tick,id,name,data->info.product );
    data->hint = read_id_speed(QString(id));
	return 0;
}

int client_handler_diagram_item(E15_Key * key,E15_Value * info,DiagramDataMgr * stock)
{
	stock->lock.Lock();
	do
	{
		if( !stock->factory )
			break;
		unsigned int parent_index = info->BaseS("pi");
        unsigned int index = info->BaseS("di");
		int mode = info->BaseS("mode");
		E15_String * s = info->ValueS("data")->GetString();
        if( parent_index == (unsigned int)0) //这个是数据
        {
             stock->factory->OnData(stock->depth,mode,index,s);
        }
       else
        {
            stock->factory->OnTag(stock->depth,mode,parent_index,index,s);
        }
	}while(0);
    stock->lock.Unlock();
	return 0;
}

int getKlineCount(MarketDataType * obj,DiaTypeMap * param)
{
    if(obj->parent_index==0)
    {
        QString dia = QString::number(obj->param)+obj->class_name+obj->name;
        g_description_index[dia] = obj->type_index;
        g_klineCount++;
    }
    return 0;
}
E15_ValueTable *g_id_vt = new E15_ValueTable;
void MD_Client::OnNotify(E15_ClientInfo * info,E15_ClientMsg * cmd,E15_String *&  json)
{
  switch(cmd->cmd)
 {
	case Stock_Msg_InstrumentList:
	{
      E15_Value vt;
		m_unzip_buffer.Reset();
		m_unzip.unzip_start(&m_unzip_buffer);
		m_unzip.unzip(json->c_str(),json->Length());
		m_unzip.unzip_end();       
        vt.Init(E15_Value::Value_Table);
        vt.GetValueTable()->Import(m_unzip_buffer.c_str(),m_unzip_buffer.Length() );
        g_id_vt->Import(m_unzip_buffer.c_str(),m_unzip_buffer.Length() );
        vt.Print();
        E15_String ss;
        vt.Json_encode(&ss);
        E15_Debug::Printf(0,"Stock_Msg_InstrumentList json len = %s\n",ss.c_str());

        g_id_vt->each((int (*)(E15_Key * key,E15_Value * info,void *) )handler_contract_info,info);
//        E15_Debug::Printf(0,"Stock_Msg_InstrumentList json len = %ld\n",json->Length());
//        t->Print();
        g_ui_loop->Push(0,0,E15_UI_Message_InstrumentList,0,g_id_vt,0);
        m_unzip_buffer.Reset();
        E15_ClientMsg c;
        c.cmd = Stock_Msg_SubscribeAll;
//        c.status = 0;
//        E15_ValueTable vt;
//        E15_Value * v = vt.InsertS("id_list");
//        v->Init(E15_Value::Value_STRINGARRAY);
//        v->AppendBytes(id,-1);
//        E15_String s;
//        vt.Dump(&s);

        g_client->Request(&info->id,&c,0,0);
		return;
	}
	break;
    case Stock_Msg_DiagramInfo: //分析处理后数据的描述信息
    {
        E15_Debug::Printf(0,"Stock_Msg_DiagramInfo    %s \n",json->c_str());
//        E15_ValueTable vt;
//        vt.Import(json->c_str(),json->Length() );
//        vt.Print();
//        vt.each((int (*)(E15_Key * key,E15_Value * info,void *))on_data_desc,0);
        if(g_data_tag_info.Count()==0)
        {
            g_data_tag_info.Import(json->c_str(),json->Length());
            g_data_tag_info.Each((int (*)(MarketDataType * obj,void * param))getKlineCount,&g_data_tag_info);
        }
        else
        {
            DiaTypeMap typeMap;
            typeMap.Import(json->c_str(),json->Length());
        }
        g_ui_loop->Push(&info->id,0,E15_UI_Message_DiagramInfo,0,0);
    }
        break;
    case Stock_Msg_DiagramGroup:
    {
//       E15_Debug::Printf(0,"Stock_Msg_DiagramGroup \n");
      //图表数据的合集
       if( json->Length() < InstrumentID_MaxLengh)
       {
           return;
       }
       const char * id = json->c_str();
       int market = MarketCodeById(id);
       DiagramDataMgr * stock = DiagramDataMgr_PeekData(market,id);
       if( !stock )
       {
           return;
       }
       m_unzip_buffer.Reset();
       m_unzip.unzip_start(&m_unzip_buffer);
       m_unzip.unzip(id+InstrumentID_MaxLengh,json->Length() - InstrumentID_MaxLengh);
       m_unzip.unzip_end();
       stock->lock.Lock();
       stock->m_vt.Import(m_unzip_buffer.c_str(),m_unzip_buffer.Length());
       unsigned long depth_len = 0;
       stock->depth = (MarketDepthData *)stock->m_vt.BytesS("depth",&depth_len);
       if(stock->depth->ext.HighestPrice<0)
           stock->depth->ext.HighestPrice = 0;
       if(stock->depth->ext.LowestPrice<0)
           stock->depth->ext.LowestPrice = 0;
       if(stock->depth->ext.OpenPrice<0)
           stock->depth->ext.OpenPrice = 0;
       if( stock->date != stock->depth->base.nActionDay)
       {
           stock->date = stock->depth->base.nActionDay;
           stock->pre_close = stock->depth->ext.PreClose;
       }
       if(DrawMarket::s_market_info_map.contains(id))
       {
           DrawMarket::s_market_info_map[id]->pre_depth_data = DrawMarket::s_market_info_map[id]->depth_data;
           DrawMarket::s_market_info_map[id]->depth_data = *stock->depth;
       }
       stock->lock.Unlock();
       E15_ValueTable * dia =  stock->m_vt.TableS("dia");
       if( depth_len != sizeof(MarketDepthData))
       {
           if(dia)
           {
               return;
           }
           dia = &stock->m_vt;
       }
       else
       {
           if(g_top_button_index==0)
           {
               E15_String * s = new E15_String;
               s->Memcpy(id,InstrumentID_MaxLengh);
               s->Memcat((const char *)stock->depth,depth_len);
               g_ui_loop->Push(&info->id,0,E15_UI_Message_DepthMarketDetail,0,s);
               json = 0;
           }
       }
       if( !dia )
           break;
       if(g_top_button_index==1)
       {
           for(int i = 0;i<g_draw_list.size();i++)
           {

               if(g_draw_list[i]->m_ins_id.Length()<2)
                   continue;
               if(g_draw_list[i]->m_ins_id.Equal(id))
               {
                   dia->each((int (*)(E15_Key * key,E15_Value * info,void *) )client_handler_diagram_item, stock);
                   DiagramDataMgr_UpdateAllView(stock);
                   break;
               }
           }
       }
       else if(g_one_key_cycle&&Cycle::cycle_id== QString(id))
       {
           dia->each((int (*)(E15_Key * key,E15_Value * info,void *) )client_handler_diagram_item, stock);
           DiagramDataMgr_UpdateAllView(stock);
       }
       if(strcmp("T1706",id)==0)
       {
           if(T1706_List.size()<10)
           {
               T1706_List.append(*stock->depth);
           }
           else
           {
               T1706_List.pop_front();
               T1706_List.push_back(*stock->depth);
               strategy_t1706(id);
           }
       }
        break;
    }
    case Stock_Msg_DiagramHistoryData:
    case Stock_Msg_DiagramHistoryTag:
    {
        if( json->Length() < (long)(InstrumentID_MaxLengh + sizeof(HistoryRequest) ) )
            return;
        const char * id = json->c_str();

        int market = MarketCodeById(id);
        DiagramDataMgr * stock = DiagramDataMgr_PeekData(market,id);
        if( !stock )
            return;
        m_unzip_buffer.Reset();
        m_unzip.unzip_start(&m_unzip_buffer);
        m_unzip.unzip(id+InstrumentID_MaxLengh+sizeof(HistoryRequest),json->Length() - InstrumentID_MaxLengh-sizeof(HistoryRequest));
        m_unzip.unzip_end();

        HistoryRequest * req = (HistoryRequest*)(id+InstrumentID_MaxLengh);
        semaphore.acquire();
        stock->lock.Lock();
        do
        {
            if( !stock->factory)
                break;
            if( cmd->cmd == Stock_Msg_DiagramHistoryData)
            {
                stock->factory->LoadHistoryData(req,m_unzip_buffer.c_str() ,m_unzip_buffer.Length(), cmd->receiver.l,cmd->status);
            }
            else
                stock->factory->LoadHistoryTag(req,m_unzip_buffer.c_str() ,m_unzip_buffer.Length(), cmd->receiver.l,cmd->receiver.h,cmd->status);
        }while(0);
        stock->lock.Unlock();
        semaphore.release();
        DiagramDataMgr_UpdateAllView(stock);
        break;
    }
    case Stock_Msg_DiagramCacheData:
    case Stock_Msg_DiagramCacheTag:
    {
        if( json->Length() < InstrumentID_MaxLengh )
                    return;
        const char * id = json->c_str();
        int market = MarketCodeById(id);
        DiagramDataMgr * stock = DiagramDataMgr_PeekData(market,id);
        if( !stock )
            return;

        m_unzip_buffer.Reset();
        m_unzip.unzip_start(&m_unzip_buffer);
        m_unzip.unzip(id+InstrumentID_MaxLengh,json->Length() - InstrumentID_MaxLengh);
        m_unzip.unzip_end();

        stock->lock.Lock();
        do
        {
            if( !stock->factory )
                break;
            if( cmd->cmd == Stock_Msg_DiagramCacheData)
                stock->factory->LoadCacheData(m_unzip_buffer.c_str() ,m_unzip_buffer.Length(), cmd->receiver.l,cmd->status);
            else
                stock->factory->LoadCacheTag(m_unzip_buffer.c_str() ,m_unzip_buffer.Length(), cmd->receiver.l,cmd->receiver.h,cmd->status);
        }while(0);
        stock->lock.Unlock();
        DiagramDataMgr_UpdateAllView(stock);
    }
    break;

    case Stock_Msg_SpeedTop20:
    {
      return;
      E15_String * s = json;
      //期货为4，A股为1
      if(cmd->status==1)
          g_ui_loop->Push(&info->id,0,E15_UI_Message_speedTopAShares,cmd->status,s);
      else
          g_ui_loop->Push(&info->id,0,E15_UI_Message_speedTopFutures,cmd->status,s);
      json = 0;
    }
        break;
  case Stock_Msg_Trade_Ok:
  case Trade_Msg_SnatchOver:{
      return;
       if( json->Length() < InstrumentID_MaxLengh)
           return;
       E15_String * s = json;
       g_ui_loop->Push(&info->id,0,Trade_Msg_SnatchOver,0,s);
       json = 0;
//       g_signal->clientToStrategy(*s);
   }
       break;
  case Trade_Msg_StrategeOpen:
  case Trade_Msg_StrategeClose:
  {
      const char *id = json->c_str();
      int market = MarketCodeById(id);
      DiagramDataMgr * stock = DiagramDataMgr_PeekData(market,id);
      if( !stock )
          return;
      if(!stock->factory)
          return;
          order_instruction *oi = (order_instruction*)(json->c_str()+InstrumentID_MaxLengh);
          int type_index = g_description_index[oi->dia_name];
          stock->lock.Lock();
          stock->factory->onBuy(type_index,oi);
          stock->lock.Unlock();
          DiagramDataMgr_UpdateAllView(stock);
  }
        break;
  case Trade_Msg_StrategeTradeResult:
  {
    return;
      if( json->Length() < InstrumentID_MaxLengh)
          return;
      g_signal->clientToTradeOpen(*json);

  }
      break;
  case 5000:
   {
     E15_Debug::Printf(0,"buy history %s len  = %d \n",json->c_str(),json->Length());
     if( json->Length() < (long)(InstrumentID_MaxLengh + sizeof(order_instruction) ) )
            return;
      const char * id = json->c_str();
      int market = MarketCodeById(id);

      DiagramDataMgr * stock = DiagramDataMgr_PeekData(market,id);
      if( !stock )
          return;
      stock->lock.Lock();
      do
      {
          if( !stock->factory)
              break;
          stock->factory->LoadHistoryBuy(json);
      }while(0);
      stock->lock.Unlock();
      DiagramDataMgr_UpdateAllView(stock);
  }
      break;
//  case Stock_Msg_GetIdMarket://获取合约行情(如:昨收)
//  {
//      if( json->Length() < 16)
//          return;
//  }
//      break;
	default:
        break;
	}
}

MD_Client * g_client = 0;
E15_Id    _g_history_id;
E15_Id * g_history_srv = &_g_history_id;

void dia_load_history(const char * id,DiagramDataHandler * h,int max_item,int index)
{
	if( h->m_history_over ) //没有更多历史数据了
		return ;

	if( max_item < 256 )
		max_item = 256;

	E15_ClientMsg cmd;
//    E15_ValueTable vt;
//    vt.SetSI("index",index);
	cmd.cmd = Stock_Msg_DiagramHistoryReq;
	E15_String req_data;
	req_data.Strcpy(id,-1);
    req_data.Resize(InstrumentID_MaxLengh+sizeof(HistoryRequest),0);

    HistoryRequest * req = (HistoryRequest*)(req_data.c_str() + InstrumentID_MaxLengh);

	DiagramDataItem * data = h->PeekDataItem(0);

	if( !data )
	{
		req->date = -1; //最新数据
		req->seq = -1;
	}
	else
	{
		req->date = data->base._date;
		req->seq = data->base._seq;
        E15_Debug::Printf(0,"####################### 向服务器申请数据  #####################\n" );
        E15_Debug::Printf(0," 日期 = %d ， seq = %d \n",req->date,req->seq );
        if(data->base._date==0)
        {
            E15_Debug::Printf(0,"error error 向服务器申请数据  error error error\n" );
            return;
        }
	}
	req->direct = -1;
	req->cnt = max_item;

	h->BuildHistoryReq(&req_data);
    cmd.sender.l = h->DataType()->type_index;
    cmd.receiver.l = index%9;
//    vt.Dump(&req_data);

//    const char *dd = req_data.c_str();

//    E15_ValueTable tt;
//    tt.Import(dd+16+sizeof(HistoryRequest),req_data.Length()-16-sizeof(HistoryRequest));
//    int i = tt.BaseS("index");

//    E15_Debug::Printf(0," index = %d；dd === %s;length  ===  %d\n\n",i,dd,req_data.Length());


	g_client->Request(g_history_srv,&cmd,req_data.c_str(),req_data.Length() );
}
