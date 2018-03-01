#include <gtk/gtk.h>

#include "E15_string.h"
#include "E15_system.h"
#include "E15_ini.h"

#include "ui.h"

#include "E15_value.h"

class UI_Handler : public E15_DispatchHandler
{
public:
    UI_Handler();
    virtual ~UI_Handler();

    virtual long OnMessage(E15_Msg * msg) ;
    virtual void Notify();
};


UI_Handler::UI_Handler()
{

}

UI_Handler::~UI_Handler()
{

}

void view_msg_printf(int level,E15_String * s);
void view_msg_set_tag_color(int level,GdkRGBA * color);
void insert_contract_data(E15_ValueTable * t);
void Update_Diagram_View(int idx);
void insert_speed_data(E15_ValueTable * t);

void LoadDiagramInstruments();
void clear_view_data(unsigned int h,unsigned int l);

void ReSubScribeAllView();

long UI_Handler::OnMessage(E15_Msg * msg)
{
	switch( msg->msg )
	{
	case E15_UI_Message_InstrumentList:
		insert_contract_data( (E15_ValueTable *)msg->params);
		//开始把配置好的显示
		//自选股
		//图形监控配置
		ReSubScribeAllView();
		break;
	case E15_UI_Message_DiagramInfo:
		clear_view_data(msg->sender->h,msg->sender->l);
		LoadDiagramInstruments();

		break;
	case E15_UI_Message_DepthMarket:
		Update_Diagram_View(msg->submsg);

		break;
	case E15_UI_Message_Printf:
		view_msg_printf(msg->submsg,(E15_String *)msg->params);
		break;

	case E15_UI_Message_SpeedTop:
	{
		E15_ValueTable vt;
		vt.Import(((E15_String *)msg->params)->c_str(),((E15_String *)msg->params)->Length());
		insert_speed_data(&vt);
	}
		break;
	}
	return 1;
}

gboolean ui_msg_handler(E15_Dispatch * dispatch)
{
	if( !dispatch)
		return FALSE;
    dispatch->TryMsg(0);
    return FALSE; //只执行一次
}

E15_Dispatch * g_ui_loop = 0;
UI_Handler * g_ui_handler = 0;


void UI_Handler::Notify()
{
	if( !g_ui_loop )
		return;
	g_main_context_invoke(NULL,( GSourceFunc)ui_msg_handler,g_ui_loop);
}


gboolean ui_msg_handler_init(E15_Dispatch * dispatch)
{
	g_ui_handler = new UI_Handler;
	g_ui_loop = new E15_Dispatch;

	unsigned long gtk_thd_id = E15_SystemApi::CurrentThreadId();
	//printf("gtk_thd_id = %lu\n",gtk_thd_id);

	g_ui_loop->Start(g_ui_handler, gtk_thd_id );

    return FALSE; //只执行一次
}


void E15_UI_Start()
{
	ui_msg_handler_init(0);
	//g_main_context_invoke(NULL,( GSourceFunc)ui_msg_handler,g_ui_loop);
}

void E15_UI_Stop()
{
	g_ui_loop->Stop();
	delete g_ui_handler;
	delete g_ui_loop;

	g_ui_loop = 0;
	g_ui_handler = 0;

}

static StockDataCache g_market_data_obj;
StockDataCache 	* g_market_data = &g_market_data_obj;
