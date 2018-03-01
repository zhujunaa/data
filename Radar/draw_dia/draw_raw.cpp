#include "E15_queue.h"
#include "E15_string_array.h"
#include "E15_value.h"
#include "E15_map.h"

#include <string.h>

#include "data_mgr.h"

#include "draw_dia.h"

void draw_raw_data(DiagramView_Control *view, MarketMVC * mvc);
void draw_raw_volume(DiagramView_Control *view, MarketMVC * mvc, unsigned long vol_max);

void draw_callback_raw(DiagramView_Control *draw, MarketMVC * mvc)
{
	//首先自动滚动到最新的显示tick
	DiagramDataItem * node;
    node = (DiagramDataItem *) mvc->handler->PeekDataItem(mvc->data_control.offset[mvc->_data_type-1]);
	if (!node)
		return;

	//计算最大值和最小值
	mvc->data_control.value_max = node->base._value;
	mvc->data_control.value_min = mvc->data_control.value_max;

	__int64 vol_max = 1;

	node = (DiagramDataItem *) node->Next();
	int c = 1;

	while (node)
	{
		if (node->base._value > mvc->data_control.value_max)
			mvc->data_control.value_max = node->base._value;
		if (node->base._value < mvc->data_control.value_min)
			mvc->data_control.value_min = node->base._value;

		if (vol_max < node->base._volume_tick)
			vol_max = node->base._volume_tick;

		c++;
        if ((((c * mvc->data_control.width_unit) << draw->zoom_in) >> draw->zoom_out) > (int) draw->width_main)
			break;
		node = (DiagramDataItem *) node->Next();
	}

	//int h = mvc->data_control.value_max - mvc->data_control.value_min;

	draw->cr = draw->cr_main;
	draw_raw_data(draw, mvc);

//	draw->cr = draw->cr_sub;
    draw->cr = draw->cr_macd;
	draw_raw_volume(draw, mvc, vol_max);
}

extern void Price2Str(E15_String * str, long price);

void raw_draw_focus_info(DiagramView_Control *draw, MarketMVC * mvc)
{
	if (!draw->mouse_in)
		return;
	//鼠标指向的元素相关信息
	DiagramDataItem * node;
	node = (DiagramDataItem *) mvc->handler->PeekDataItem(mvc->data_control.focus);

	if (!node)
		return;

	E15_String str;
	E15_String price_str;
	E15_String bid_str;
	E15_String ask_str;

	Price2Str(&price_str, node->base._value);
	unsigned long volume = node->base._volume_tick;

	MarketDepthData * depth = (MarketDepthData *) node->pri->c_str();

	Price2Str(&bid_str, depth->base.bid_ask.Bid);
	Price2Str(&ask_str, depth->base.bid_ask.Ask);

	str.Sprintf("%u %u 成交[%s/%u] 委买[%s/%4u] 委卖[%s/%4u]", depth->base.nActionDay, depth->base.nTime, price_str.c_str(), volume, bid_str.c_str(), depth->base.bid_ask.BidVolume, ask_str.c_str(), depth->base.bid_ask.AskVolume);

	cairo_show_text(draw->cr, str.c_str());
}

void draw_raw_data(DiagramView_Control *view, MarketMVC * mvc)
{
	DrawRGBA color;
	DiagramDataItem * node;
	int x = 0;
	int y = 0;

	view->cr = view->cr_main;

	cairo_new_path(view->cr);
    node = (DiagramDataItem *) mvc->handler->PeekDataItem(mvc->data_control.offset[mvc->_data_type-1]);

	color.red = 1.0;
	color.green = 1.0;
	color.blue = 1.0;
	color.alpha = 1.0;

	cairo_set_source_rgba(view->cr, color.red, color.green, color.blue, color.alpha);

	cairo_set_line_width(view->cr, 1.0);

	double dy;
    long long delta = 0;
	unsigned long long price;
	while (node)
	{
		price = node->base._value;
		delta = price - mvc->data_control.value_min;
		dy = delta * view->height_main;
		dy /= (mvc->data_control.value_max - mvc->data_control.value_min);
		y = view->height_main - dy;

        long long x_pos = (x + ((mvc->data_control.width_unit << view->zoom_in) >> 1)) >> view->zoom_out;
		cairo_line_to(view->cr, x_pos, y);
		x += mvc->data_control.width_unit << view->zoom_in;

        if (x_pos >= view->width_main)
			break;

		node = (DiagramDataItem *) node->Next();
	}
	cairo_stroke(view->cr);
}

extern DrawRGBA positively_color;
extern DrawRGBA negative_color;

void draw_raw_volume(DiagramView_Control *view, MarketMVC * mvc, unsigned long vol_max)
{
	DiagramDataItem * node;
    node = (DiagramDataItem *) mvc->handler->PeekDataItem(mvc->data_control.offset[mvc->_data_type-1]);
	if (!node)
		return;

	cairo_set_line_width(view->cr, 0.8);
	int x = 0;
    double h = 0;

	unsigned long long price = node->base._value;
	unsigned long long price_pre = price;

	DiagramDataItem * node_pre = (DiagramDataItem *) node->Pre();
	if (node_pre)
	{
		price_pre = node->base._value;
	}

	while (node)
	{
		price = node->base._value;
		h = node->base._volume_tick;

		h /= vol_max;
//		h *= view->height_sub;
        h *= view->height_macd;

		if (price >= price_pre)
		{
			cairo_set_source_rgba(view->cr, positively_color.red, positively_color.green, positively_color.blue, positively_color.alpha);
		}
		else
		{
			cairo_set_source_rgba(view->cr, negative_color.red, negative_color.green, negative_color.blue, negative_color.alpha);
		}

//		cairo_rectangle(view->cr, (x >> view->zoom_out), view->height_sub, ((mvc->data_control.width_unit << view->zoom_in) - 1) >> view->zoom_out, -h);
        cairo_rectangle(view->cr, (x >> view->zoom_out), view->height_macd, ((mvc->data_control.width_unit << view->zoom_in) - 1) >> view->zoom_out, -h);
		cairo_stroke(view->cr);

		node = (DiagramDataItem *) node->Next();
		if (!node)
			break;

		price_pre = price;

		x += (mvc->data_control.width_unit << view->zoom_in);
        if ((x >> view->zoom_out) > view->width_main)
			break;

	}
}
