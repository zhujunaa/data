#ifndef __ui_H
#define __ui_H

#include "E15_dispatch.h"
#include "StockDataCache.h"


enum E15_UI_Message
{
	E15_UI_Message_InstrumentList = 0, //合约列表更新
	E15_UI_Message_DepthMarket ,    //深度行情
	E15_UI_Message_Printf,
	E15_UI_Message_DiagramInfo,
	E15_UI_Message_SpeedTop,
};

void E15_UI_Start();
void E15_UI_Stop();

extern E15_Dispatch * g_ui_loop; //后台事件与界面交互的消息循环


#endif
