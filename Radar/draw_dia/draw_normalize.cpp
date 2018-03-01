
#include "E15_queue.h"
#include "E15_string_array.h"
#include "E15_value.h"
#include "E15_map.h"

#include <string.h>

#include "data_mgr.h"

#include "draw_dia.h"
extern E15_Queue g_draw_config_list;
extern bool one_second_out;
extern int g_kavg;
extern int g_twist1;
extern int g_twist2;
extern int g_merge;
void draw_normalize_data(DiagramView_Control *view, MarketMVC * mvc);
void draw_time_scale(DiagramView_Control *view,MarketMVC * mvc,cairo_t* cr);
void draw_normalize_volume(DiagramView_Control *view, MarketMVC * mvc,
		unsigned long vol_max);
void draw_kline_scale(DiagramView_Control *draw, MarketMVC * mvc);
void draw_time(int begin_date,int begin_time,int x,DiagramView_Control view);

void draw_callback_normalize(DiagramView_Control *draw, MarketMVC * mvc)
{
	//首先自动滚动到最新的显示tick
	DiagramDataItem * node;
	node = (DiagramDataItem *) mvc->handler->PeekDataItem(
            mvc->data_control.offset[mvc->_data_type-1]);
	if (!node)
		return;

	DrawRGBA * color = &mvc->draw_helper->draw_config.color_line;
	cairo_set_source_rgba (draw->cr, color->red,color->green,color->blue,color->alpha);
	cairo_set_line_width(draw->cr,mvc->draw_helper->draw_config.line_width);

	//计算最大值和最小值
	mvc->data_control.value_min = node->base._value;
	mvc->data_control.value_max = mvc->data_control.value_min + 100;

	unsigned long vol_max = 1;

//	node = (DiagramDataItem *) node->Next();
	int c = 1;

	long long price;
	unsigned long volume;

    long long OpenInterest_max = node->base.OpenInterest;
    long long OpenInterest_min = OpenInterest_max;
	while (node)
	{
		price = node->base._value;
		volume = node->base._volume_tick;

		if (price > mvc->data_control.value_max)
			mvc->data_control.value_max = price;
		if (price < mvc->data_control.value_min)
			mvc->data_control.value_min = price;

		if (vol_max < volume)
			vol_max = volume;
        if(OpenInterest_max < node->base.OpenInterest)
            OpenInterest_max = node->base.OpenInterest;
        if(OpenInterest_min > node->base.OpenInterest)
            OpenInterest_min = node->base.OpenInterest;

		c++;
        if(draw->zoom_in>0)
        {
            if ((c *(((mvc->data_control.width_unit)<< draw->zoom_in)+SPACEING)) > (int) draw->width_main)
                break;
        }
        else{
            if((c *(mvc->data_control.width_unit+SPACEING) >> draw->zoom_out) > (int) draw->width_main)
                break;
        }
		node = (DiagramDataItem *) node->Next();
	}
    double dashed2[] = { 1, 0 };
    cairo_set_dash(draw->cr_volume, dashed2, 2, 1);
    cairo_set_dash(draw->cr_main, dashed2, 2, 1);
	draw->cr = draw->cr_main;
	draw_normalize_data(draw, mvc);
    if(draw->zoom_in_out)
    {
        cairo_new_path(draw->cr_scale);
        draw_kline_scale(draw,mvc);
        draw_time_scale(draw,mvc,draw->cr_main);
    }
    double dashed1[] = { 2.0, 0};
    cairo_set_dash(draw->cr_volume,dashed1,2,1);
    cairo_set_dash(draw->cr_main,dashed1,2,1);
    cairo_new_path(draw->cr_volume);
    mvc->data_control.OpenInterest_max = OpenInterest_max;
    mvc->data_control.OpenInterest_min = OpenInterest_min;
    mvc->data_control.volume_max = vol_max;
//	draw->cr = draw->cr_sub;
    draw->cr = draw->cr_volume;
	draw_normalize_volume(draw, mvc, vol_max);

}

void Price2Str(E15_String * str, long price)
{
//	unsigned long p1 = price / 10000;
//	unsigned int p2 = price % 10000;

//	str->Sprintf("%lu", p1);
//	if (p2 == 0)
//		return;
//	str->Append(".%04u", p2);
}

void price_to_str(E15_String * str, long price ,int decimal)
{
    E15_String s;
    int size;
    switch (decimal) {
    case 0:
        s.Memcpy("%lu",3);
        size = 10000;
        break;
    case 1:
        s.Memcpy(".%01u",5);
        size = 1000;
        break;
    case 2:
        s.Memcpy(".%02u",5);
        size = 100;
        break;
    case 3:
        s.Memcpy(".%03u",5);
        size = 10;
        break;
    default:
        break;
    }
    unsigned int p1 = price / 10000;
    unsigned int p2 = price % 10000;
    unsigned int p3 = p2/size;

    str->Sprintf("%lu", p1);
    if (p3 == 0)
        return;
    str->Append(s.c_str(), p3);
}
void normalize_draw_focus_info(DiagramView_Control *draw, MarketMVC * mvc)
{
	if (!draw->mouse_in)
		return;
	//鼠标指向的元素相关信息
	DiagramDataItem * node;
	node = (DiagramDataItem *) mvc->handler->PeekDataItem(
            mvc->data_control.focus);

	if (!node)
		return;

//	E15_String str;
//	E15_String price_str;
//	E15_String bid_str;
//	E15_String ask_str;

//	Price2Str(&price_str, node->base._value);
//	unsigned long volume = node->base._volume_tick;

//	MarketDepthData * depth = (MarketDepthData *)node->pri->c_str();


//	Price2Str(&bid_str, depth->base.bid_ask.Bid);
//	Price2Str(&ask_str, depth->base.bid_ask.Ask);

//	str.Sprintf("%u:%06u [%u]  成交[%s/%u] 委买[%s/%4u] 委卖[%s/%4u]", depth->base.nActionDay,  depth->base.nTime/1000,node->base._seq,
//			price_str.c_str(), volume,
//			bid_str.c_str(), depth->base.bid_ask.BidVolume,
//			ask_str.c_str(), depth->base.bid_ask.AskVolume);

//	cairo_show_text(draw->cr, str.c_str());
    DrawHelper_Config * tag_conf;
    int cnt = mvc->handler->GetTagCount();
    int i;
    E15_String s;
    MarketDataType * dt;
    for (i = 0; i < cnt; i++)
    {
        dt = mvc->handler->GetTagType(i);
        if (!dt)
            continue;

        tag_conf = (DrawHelper_Config *) g_draw_conf_hash->LookupSI(dt->name, dt->param);
        if (!tag_conf)
        {
            tag_conf = (DrawHelper_Config *) g_draw_config_list.Head(0);
        }
        if(strcmp(dt->name,"均线")==0)
        {
            if(!g_kavg)
                continue;
        }
        else
        {
            if(strcmp(dt->name,"中枢")==0)
            {
                if(!g_twist1)
                    continue;
            }
            else
            {
                if(!g_twist2)
                    continue;
            }
        }
        if (!tag_conf->show_hide)
            continue;
        DiagramTag * tag = node->PeekTag(i);
        if(tag)
        {
            s.Sprintf(dt->name);
            s.Append(" %d:",dt->param);
            s.Append("%d ",tag->base._value/10000);
            double xx = 0;
            double yy = 0;
            cairo_get_current_point(draw->cr,&xx,&yy);
            cairo_move_to(draw->cr, xx, g_draw_font.tip.Size);
            cairo_set_source_rgba(draw->cr,tag_conf->color_line.red,tag_conf->color_line.green,tag_conf->color_line.blue,tag_conf->color_line.alpha);
            cairo_show_text(draw->cr, s.c_str());
        }
    }
    cairo_stroke(draw->cr);

}

void draw_normalize_data(DiagramView_Control *view, MarketMVC * mvc)
{
	DrawRGBA color;
	int x = 0;
	int y = 0;

	DiagramDataItem * node;
	node = (DiagramDataItem *) mvc->handler->PeekDataItem(
            mvc->data_control.offset[mvc->_data_type-1]);
	if (!node)
		return;

	color.red = 1.0;
	color.green = 1.0;
	color.blue = 1.0;
	color.alpha = 1.0;

	cairo_set_source_rgba (view->cr, color.red,color.green,color.blue,color.alpha);

	cairo_set_line_width(view->cr, 1.0);

	double dy;
	long delta = 0;

	unsigned long long price;

	cairo_new_path(view->cr);
	while (node)
	{
		price = node->base._value;

		delta = price - mvc->data_control.value_min;
        dy = delta * (view->height_main- 2*MAIN_SPACEING);
		dy /= (mvc->data_control.value_max - mvc->data_control.value_min);
        y = view->height_main - dy - MAIN_SPACEING;

		int x_pos = (x + ((mvc->data_control.width_unit << view->zoom_in) >> 1))
				>> view->zoom_out;
        cairo_line_to(view->cr, x_pos, y);
        x += ((mvc->data_control.width_unit << view->zoom_in)+ SPACEING);

        if (x_pos >= view->width_main)
			break;

		node = (DiagramDataItem *) node->Next();
	}
	cairo_stroke(view->cr);
}

int normalize_on_key(DiagramView_Control *, MarketMVC * , int )
{
	return 0;
}

extern DrawRGBA positively_color;
extern DrawRGBA negative_color;

void draw_normalize_volume(DiagramView_Control *view, MarketMVC * mvc,
		unsigned long vol_max)
{
	DiagramDataItem * node;
	node = (DiagramDataItem *) mvc->handler->PeekDataItem(
            mvc->data_control.offset[mvc->_data_type-1]);
	if (!node)
		return;

    DiagramDataItem * node_k = node;
	DiagramDataItem * pre = (DiagramDataItem *) node->Pre();
	if (!pre)
		pre = node;

	cairo_set_line_width(view->cr, 0.6);
	DrawRGBA color;

	color.red = 1.0;
	color.green = 0.0;
	color.blue = 0.0;
	color.alpha = 1.0;

	int x = 0;
	cairo_set_source_rgba (view->cr, color.red,color.green,color.blue,color.alpha);
	float h = 0;

	unsigned long long price = node->base._value;
	unsigned long long price_pre = pre->base._value;
	while (node)
	{
		h = node->base._volume_tick;
		h /= vol_max;
//		h *= view->height_sub;
        h *= view->height_volume-RESERVE_PIXEL;

		if (price >= price_pre)
		{
			cairo_set_source_rgba (view->cr, positively_color.red,positively_color.green,positively_color.blue,positively_color.alpha);
		}
		else
		{
			cairo_set_source_rgba (view->cr, negative_color.red,negative_color.green,negative_color.blue,negative_color.alpha);
		}

//		cairo_rectangle(view->cr, (x >> view->zoom_out), view->height_sub,
//				((mvc->data_control.width_unit << view->zoom_in) - 1)
//						>> view->zoom_out, -h);

        cairo_rectangle(view->cr, (x >> view->zoom_out), view->height_volume-RESERVE_PIXEL,
                ((mvc->data_control.width_unit << view->zoom_in))
                        >> view->zoom_out, -h);
		cairo_stroke(view->cr);

		node = (DiagramDataItem *) node->Next();
		price_pre = price;
		if (!node)
			break;

		price = node->base._value;
        x += ((mvc->data_control.width_unit << view->zoom_in) + SPACEING);
        if ((x >> view->zoom_out) > view->width_main)
			break;

	}

    if(mvc->data_control.OpenInterest_max==0)
        return;
    cairo_set_line_width(view->cr, 0.8);
    cairo_set_source_rgba(view->cr, 1, 1, 1, 1);
    int height = view->height_volume;
    double diff  =0;
    x = 0;
    node = node_k;
    while(node)
    {
        diff = node->base.OpenInterest -mvc->data_control.OpenInterest_min ;
        diff *= height;
        diff /= mvc->data_control.OpenInterest_max - mvc->data_control.OpenInterest_min;

        cairo_line_to(view->cr, (x + (mvc->data_control.width_unit << view->zoom_in) / 2) >> view->zoom_out, height - diff);
        node = (DiagramDataItem *) node->Next();
        if (!node)
            break;
        x += ((mvc->data_control.width_unit << view->zoom_in)+ SPACEING);
        if ((x >> view->zoom_out) > view->width_main)
            break;
    }
    cairo_stroke(view->cr);
}



void string_to_float(E15_String * str, long price,int decimal)
{
    unsigned long p1 = price / 10000;
    unsigned int p2 = price % 10000;

    str->Sprintf("%lu", p1);
    if (p2 == 0)
        return;
    str->Append(".%04u", p2);
}
