#ifndef __Data_View_H
#define __Data_View_H

#include "data_mgr.h"
//--------------------------------------------------------------------

class DiagramDataMgrView : public DiagramDataMgrParams
{
public:
	E15_Strmap  			view_hash;	//当前在哪个显示窗口中显示
	int 							draw_flag;

	E15_Id					m_market_src;	//行情数据服务器

	int 						m_reinit;


    long                    row;
public:
    DiagramDataMgrView(){draw_flag = 0;m_reinit =1; row = -1;}
    virtual ~DiagramDataMgrView(){}
};


void DiagramDataMgr_UpdateAllView(DiagramDataMgr *stock);


#endif
