#include <string.h>

#include "E15_queue.h"
#include "E15_debug.h"
#include<QDateTime>
#include "draw_dia.h"

#include "data_mgr.h"
#include"mydef.h"
#define MAX_ZOOM 10  //最大放大缩小倍数2^MAX_ZOOM
extern double clarity;
bool read_data_from_db(const char * id,My_Date date,DiagramDataHandler * h,int maxitem,int seq);

int dia_on_key_page_down(DiagramDrawCallbackData * cbdata, int ctrl_flag)
{
    int max_item = ((cbdata->view.width_main << cbdata->view.zoom_out) - 1) / ((cbdata->mvc->data_control.width_unit) << cbdata->view.zoom_in+SPACEING);
	int cnt = 0;
	E15_AutoLock lock(&cbdata->data->lock);

	if( !cbdata->data->factory)
		return 0;

	E15_Queue * q = cbdata->mvc->handler->PeekData();
	cnt = q->Count();

	if (cnt == 0)
		return 0;

	if (ctrl_flag)
        cbdata->mvc->data_control.offset[cbdata->mvc->_data_type-1] += max_item;
	else
        cbdata->mvc->data_control.offset[cbdata->mvc->_data_type-1] += max_item ;

    if ((cbdata->mvc->data_control.offset[cbdata->mvc->_data_type-1] + (max_item >> 1)) > cnt)
	{
        cbdata->mvc->data_control.offset[cbdata->mvc->_data_type-1] = cnt - (max_item >> 1);
        if (cbdata->mvc->data_control.offset[cbdata->mvc->_data_type-1] < 0)
            cbdata->mvc->data_control.offset[cbdata->mvc->_data_type-1] = 0;
	}
	cbdata->mvc->data_control.redraw_by_data_change = 1;

	return 1;
}

int dia_on_key_page_up(DiagramDrawCallbackData *cbdata, int ctrl_flag)
{
    int max_item = ((cbdata->view.width_main << cbdata->view.zoom_out) - 1) / ((cbdata->mvc->data_control.width_unit << cbdata->view.zoom_in)+SPACEING);

	if (ctrl_flag)
	{
        cbdata->mvc->data_control.offset[cbdata->mvc->_data_type-1] -= max_item;
	}
	else
	{
        cbdata->mvc->data_control.offset[cbdata->mvc->_data_type-1] -= max_item /*/ 4*/;
	}
    if (cbdata->mvc->data_control.offset[cbdata->mvc->_data_type-1] < 0)
	{
//        dia_load_history(cbdata->data->info.id, cbdata->mvc->handler, 512,cbdata->index);
        cbdata->mvc->data_control.offset[cbdata->mvc->_data_type-1] = 0;

        My_Date date;
        QDateTime time = QDateTime::currentDateTime();
        date.year = time.toString("yyyy").toInt(); //设置显示格式
        date.month = time.toString("MM").toInt(); //设置显示格式
        date.day = time.toString("dd").toInt(); //设置显示格式
        DiagramDataItem * data = cbdata->mvc->handler->PeekDataItem(0);
        if(data)
        {
            date.year = data->base._date/10000; //设置显示格式
            date.month = (data->base._date/100)%100; //设置显示格式
            date.day = data->base._date%100; //设置显示格式
            read_data_from_db(cbdata->m_ins_id.c_str(),date,cbdata->mvc->handler,512,data->base._seq);
        }
        else
            {
            read_data_from_db(cbdata->m_ins_id.c_str(),date,cbdata->mvc->handler,512,-1);
        }
	}
	cbdata->mvc->data_control.redraw_by_data_change = 1;

	return 1;
}

int dia_on_key_up(DiagramDrawCallbackData * cbdata, int ctrl_flag)
{
    if(ctrl_flag)//开多仓
    {
        return 0;
    }
	if (cbdata->view.zoom_out > 0)
		cbdata->view.zoom_out--;
	else
	{
		cbdata->view.zoom_in++;
		if (cbdata->view.zoom_in > MAX_ZOOM)
			cbdata->view.zoom_in = MAX_ZOOM;
	}

	int cnt = 0;
    E15_AutoLock lock(&cbdata->data->lock);


	if( !cbdata->data->factory)
		return 0;
	E15_Queue * q = cbdata->mvc->handler->PeekData();

	cnt = q->Count();


    int max_item = ((cbdata->view.width_main << cbdata->view.zoom_out) - 1) / (((cbdata->mvc->data_control.width_unit) << cbdata->view.zoom_in)+SPACEING);

    cbdata->mvc->data_control.offset[cbdata->mvc->_data_type-1] = cbdata->mvc->data_control.focus - ((cbdata->view.mouse_x << cbdata->view.zoom_out) / ((cbdata->mvc->data_control.width_unit << cbdata->view.zoom_in)+SPACEING));
    if ((cbdata->mvc->data_control.offset[cbdata->mvc->_data_type-1] + (max_item >> 1)) > cnt)
    {
        cbdata->mvc->data_control.offset[cbdata->mvc->_data_type-1] = cnt - (max_item >> 1);
    }

    if (cbdata->mvc->data_control.offset[cbdata->mvc->_data_type-1] < 0)
	{
        cbdata->mvc->data_control.offset[cbdata->mvc->_data_type-1] = 0;
        cbdata->view.mouse_x = (cbdata->mvc->data_control.focus - cbdata->mvc->data_control.offset[cbdata->mvc->_data_type-1]) * (((cbdata->mvc->data_control.width_unit) << cbdata->view.zoom_in)+SPACEING);
        cbdata->view.mouse_x += (((cbdata->mvc->data_control.width_unit) << cbdata->view.zoom_in)) >> 1;
		cbdata->view.mouse_x >>= cbdata->view.zoom_out;
	}

	cbdata->mvc->data_control.redraw_by_data_change = 1;

	return 1;
}

int dia_on_key_down(DiagramDrawCallbackData * cbdata, int ctrl_flag)
{
    if(ctrl_flag)//开空仓
    {
        return 0;
    }
	int cnt = 0;

//	E15_AutoLock lock(&cbdata->data->lock);
    cbdata->data->lock.Lock();
	if( !cbdata->data->factory)
    {
        cbdata->data->lock.Unlock();
		return 0;
    }
	E15_Queue * q = cbdata->mvc->handler->PeekData();
	cnt = q->Count();
    int max_item = ((cbdata->view.width_main << cbdata->view.zoom_out) - 1) / ((cbdata->mvc->data_control.width_unit << cbdata->view.zoom_in)+SPACEING);

	if (cbdata->view.zoom_in > 0)
		cbdata->view.zoom_in--;
	else
	{
		cbdata->view.zoom_out++;
		if (cbdata->view.zoom_out > MAX_ZOOM)
			cbdata->view.zoom_out = MAX_ZOOM;
	}

	if (cbdata->mvc->data_control.focus < max_item)
    {
//        dia_load_history(cbdata->data->info.id, cbdata->mvc->handler, max_item - cbdata->mvc->data_control.focus,cbdata->index);
        My_Date date;
        QDateTime time = QDateTime::currentDateTime();
        date.year = time.toString("yyyy").toInt(); //设置显示格式
        date.month = time.toString("MM").toInt(); //设置显示格式
        date.day = time.toString("dd").toInt(); //设置显示格式
        DiagramDataItem * data = cbdata->mvc->handler->PeekDataItem(0);
        if(data)
        {
            date.year = data->base._date/10000; //设置显示格式
            date.month = (data->base._date/100)%100; //设置显示格式
            date.day = data->base._date%100; //设置显示格式
            read_data_from_db(cbdata->m_ins_id.c_str(),date,cbdata->mvc->handler,max_item - cbdata->mvc->data_control.focus,data->base._seq);
        }
        else
            {
            read_data_from_db(cbdata->m_ins_id.c_str(),date,cbdata->mvc->handler,max_item - cbdata->mvc->data_control.focus,-1);
        }
    }

    cbdata->mvc->data_control.offset[cbdata->mvc->_data_type-1] = cbdata->mvc->data_control.focus - (cbdata->view.mouse_x << cbdata->view.zoom_out) / ((cbdata->mvc->data_control.width_unit << cbdata->view.zoom_in)+SPACEING);
    if ((cbdata->mvc->data_control.offset[cbdata->mvc->_data_type-1] + (max_item >> 1)) > cnt)
        cbdata->mvc->data_control.offset[cbdata->mvc->_data_type-1] = cnt - (max_item >> 1);

    if (cbdata->mvc->data_control.offset[cbdata->mvc->_data_type-1] < 0)
	{
        cbdata->mvc->data_control.offset[cbdata->mvc->_data_type-1] = 0;
        cbdata->view.mouse_x = (cbdata->mvc->data_control.focus - cbdata->mvc->data_control.offset[cbdata->mvc->_data_type-1]) * (((cbdata->mvc->data_control.width_unit << cbdata->view.zoom_in))+SPACEING);
        cbdata->view.mouse_x += (((cbdata->mvc->data_control.width_unit) << cbdata->view.zoom_in)) >> 1;
		cbdata->view.mouse_x >>= cbdata->view.zoom_out;
	}

	cbdata->mvc->data_control.redraw_by_data_change = 1;
    cbdata->data->lock.Unlock();
	return 1;
}

int dia_on_key_left(DiagramDrawCallbackData *cbdata, int ctrl_flag)
{
    if(ctrl_flag)//平仓
    {
        return 0;
    }
	cbdata->mvc->data_control.focus--;
    if (cbdata->mvc->data_control.focus < cbdata->mvc->data_control.offset[cbdata->mvc->_data_type-1])
	{
        cbdata->mvc->data_control.offset[cbdata->mvc->_data_type-1]--;

        if (cbdata->mvc->data_control.offset[cbdata->mvc->_data_type-1] < 0)
		{
//            dia_load_history(cbdata->data->info.id, cbdata->mvc->handler, 256,cbdata->index);
            cbdata->mvc->data_control.offset[cbdata->mvc->_data_type-1] = 0;
            My_Date date;
            QDateTime time = QDateTime::currentDateTime();
            DiagramDataItem * data = cbdata->mvc->handler->PeekDataItem(0);
            if(data)
            {
                date.year = data->base._date/10000; //设置显示格式
                date.month = (data->base._date/100)%100; //设置显示格式
                date.day = data->base._date%100; //设置显示格式
                read_data_from_db(cbdata->m_ins_id.c_str(),date,cbdata->mvc->handler,512,data->base._seq);
            }
            else{
                read_data_from_db(cbdata->m_ins_id.c_str(),date,cbdata->mvc->handler,512,-1);
            }


		}
        cbdata->mvc->data_control.focus = cbdata->mvc->data_control.offset[cbdata->mvc->_data_type-1];

		cbdata->mvc->data_control.redraw_by_data_change = 1;
	}

    cbdata->view.mouse_x = (cbdata->mvc->data_control.focus - cbdata->mvc->data_control.offset[cbdata->mvc->_data_type-1]) * (((cbdata->mvc->data_control.width_unit << cbdata->view.zoom_in)+SPACEING));
    cbdata->view.mouse_x += (((cbdata->mvc->data_control.width_unit) << cbdata->view.zoom_in)) >> 1;
	cbdata->view.mouse_x >>= cbdata->view.zoom_out;
	return 1;
}
int dia_on_key_right(DiagramDrawCallbackData * cbdata, int ctrl_flag)
{
    if(ctrl_flag)//平仓
    {
        return 0;
    }
	int cnt = 0;
	E15_AutoLock lock(&cbdata->data->lock);
	if( !cbdata->data->factory)
		return 0;

	E15_Queue * q = cbdata->mvc->handler->PeekData();

	cnt = q->Count();

    int max_item = ((cbdata->view.width_main << cbdata->view.zoom_out) - 1) / ((cbdata->mvc->data_control.width_unit << cbdata->view.zoom_in)+SPACEING);
	if (cbdata->mvc->data_control.focus >= cnt)
		return 0;

	cbdata->mvc->data_control.focus++;
    if ((cbdata->mvc->data_control.focus - cbdata->mvc->data_control.offset[cbdata->mvc->_data_type-1]) > max_item)
	{
		cbdata->mvc->data_control.redraw_by_data_change = 1;
        cbdata->mvc->data_control.offset[cbdata->mvc->_data_type-1]++;
	}

    cbdata->view.mouse_x = (cbdata->mvc->data_control.focus - cbdata->mvc->data_control.offset[cbdata->mvc->_data_type-1]) * (((cbdata->mvc->data_control.width_unit << cbdata->view.zoom_in)+SPACEING));
    cbdata->view.mouse_x += (((cbdata->mvc->data_control.width_unit) << cbdata->view.zoom_in)) >> 1;
	cbdata->view.mouse_x >>= cbdata->view.zoom_out;
	return 1;
}

int dia_on_key_home(DiagramDrawCallbackData * cbdata, int )
{
    cbdata->mvc->data_control.offset[cbdata->mvc->_data_type-1] = 0;
	cbdata->mvc->data_control.redraw_by_data_change = 1;
	return 1;
}

int dia_on_key_end(DiagramDrawCallbackData * cbdata, int )
{
	int cnt = 0;
	E15_AutoLock lock(&cbdata->data->lock);
	if( !cbdata->data->factory)
		return 0;

	E15_Queue * q = cbdata->mvc->handler->PeekData();

	cnt = q->Count();

    int max_item = ((cbdata->view.width_main << cbdata->view.zoom_out) - 1) / ((cbdata->mvc->data_control.width_unit << cbdata->view.zoom_in)+SPACEING);
	if (cnt > (max_item >> 1))
	{
        cbdata->mvc->data_control.offset[cbdata->mvc->_data_type-1] = cnt - (max_item >> 1);
        if (cbdata->mvc->data_control.offset[cbdata->mvc->_data_type-1] < 0)
            cbdata->mvc->data_control.offset[cbdata->mvc->_data_type-1] = 0;
	}

	cbdata->mvc->data_control.redraw_by_data_change = 1;
	return 1;
}

int dia_on_key_ch(DiagramDrawCallbackData * cbdata, int key, int ctrl_flag)
{
	if (ctrl_flag ) //交给数据自定义快捷键
	{
		if (!cbdata->mvc->draw_helper)
			return 0;

		int change = cbdata->mvc->draw_helper->draw_mode_set(&cbdata->view, cbdata->mvc, key);
		if (!change)
			return 0;

		cbdata->mvc->data_control.redraw_by_data_change = 1;
		return 1;
	}

//	int index = -1;

//	if (key >= '0' && key <= '9')
//	{ //切换显示图表类型
//		index = key - '0';
//        E15_AutoLock lock(&cbdata->data->lock);
//        if( !cbdata->data->factory)
//            return 0;
//        DiagramDataHandler * h = cbdata->mvc->factory->GetDataHandler(index);
//        if (!h)
//            return 0;

//        if (h == cbdata->mvc->handler)
//        {
//            //数据源未变化
//            return 0;
//        }
//        MarketDataDraw_Helper * dh = (MarketDataDraw_Helper *) g_draw_helper_hash->LookupS(h->GetDataType()->class_name);
//        if (!dh)
//            return 0;

//        //切换数据源和绘图函数
//        cbdata->mvc->_data_type-1 = key - '0';

//        cbdata->mvc->handler = h;
//        cbdata->mvc->draw_helper = dh;

//        cbdata->mvc->data_control.width_unit = dh->item_width;

//        cbdata->mvc->data_control.redraw_by_data_change = 1;
//        return 1;
//	}
//	else if (key >= 'a' && key <= 'f')
//	{
//		index = key - 'a' + 10;
//	}

//	switch(key)
//	{
//    case 'm':
//    case 'M':
//		if (cbdata->mvc->_sub_type == 'm')
//			return 0;
//		cbdata->mvc->_sub_type = 'm';
//		cbdata->mvc->data_control.redraw_by_data_change = 1;
//		return 1;
//		break;
//    case 'v':
//    case 'V':
//		if (cbdata->mvc->_sub_type == 'v')
//			return 0;
//		cbdata->mvc->_sub_type = 'v';
//		cbdata->mvc->data_control.redraw_by_data_change = 1;
//		return 1;
//	case 'p':
//    case 'P':
//		cbdata->mvc->data_control.auto_scroller = 0;
//		return 0;
//		break;
//	case 'r':
//    case 'R':
//		cbdata->mvc->data_control.auto_scroller = 1;
//        cbdata->mvc->data_control.redraw_by_data_change = 1;
//		return 0;
//    case 'l':
//    case 'L':
//        cbdata->view.kavg_show = !cbdata->view.kavg_show;
//        cbdata->mvc->data_control.redraw_by_data_change = 1;
//        return 0;
//	default:
//		return 0;
//	}
	return 0;
}


int dia_on_mouse_move(DiagramDrawCallbackData * cbdata,long x,long y)
{
	if (!cbdata->mvc)
		return 0;

    if( cbdata->view.mouse_x  == x-TIP_WIDTH && cbdata->view.mouse_y == y)
		return 0;
    if(x<TIP_WIDTH)
        return 0;

    cbdata->view.mouse_x = x-TIP_WIDTH;
	cbdata->view.mouse_y = y;

	int cnt = 0;
	E15_AutoLock lock(&cbdata->data->lock);
	if( !cbdata->data->factory)
		return 0;
	E15_Queue * q = cbdata->mvc->handler->PeekData();

	cnt = q->Count();


	if (cnt == 0)
		return 0;
    int focus =cbdata->mvc->data_control.focus;
//	cbdata->view.mouse_in = 1;
    int mouse_x = cbdata->view.mouse_x;

    int offset = (mouse_x << cbdata->view.zoom_out) / ((cbdata->mvc->data_control.width_unit << cbdata->view.zoom_in)+ SPACEING);
    if ((offset + cbdata->mvc->data_control.offset[cbdata->mvc->_data_type-1]) > cnt)
        offset = cnt - cbdata->mvc->data_control.offset[cbdata->mvc->_data_type-1];

    cbdata->mvc->data_control.focus = offset + cbdata->mvc->data_control.offset[cbdata->mvc->_data_type-1];

    cbdata->view.mouse_x = (cbdata->mvc->data_control.focus - cbdata->mvc->data_control.offset[cbdata->mvc->_data_type-1]) * ((cbdata->mvc->data_control.width_unit << cbdata->view.zoom_in)+SPACEING);
    cbdata->view.mouse_x += (((cbdata->mvc->data_control.width_unit) << cbdata->view.zoom_in)) >> 1;
	cbdata->view.mouse_x >>= cbdata->view.zoom_out;

    if(focus==cbdata->mvc->data_control.focus)
        return 0;
	return 1;
}
