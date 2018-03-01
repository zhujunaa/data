#include <string.h>
#include <math.h>

#include "E15_queue.h"
#include "E15_string_array.h"
#include "E15_value.h"
#include "E15_map.h"
#include "E15_debug.h"

#include "data_mgr.h"
#include "draw_dia.h"
void price_to_str(E15_String * str, long price ,int decimal);
void draw_tag_up_arrow(cairo_t * cr, long x, long y);
void draw_tag_down_arrow(cairo_t * cr, long x, long y);
void draw_tag_rect(DiagramDataItem * node, DiagramTag * tag, DiagramView_Control *view, MarketMVC * mvc, long x, long y);

void draw_boll(int type_index, DiagramView_Control *view, MarketMVC * mvc, DrawHelper_Config * draw_config)
{
    DiagramDataItem * node,*node_c;
    node = (DiagramDataItem *) mvc->handler->PeekDataItem(mvc->data_control.offset[mvc->_data_type-1]);
    if (!node)
        return;
    node_c = node;
    view->cr = view->cr_main; //附加在主图上
    cairo_set_source_rgba(view->cr, 1,0,0,1);
    cairo_set_line_width(view->cr, 1);
    DiagramTag * tag = 0;
    double dy;
    long long delta = 0;
    long long y;
    long long x = (mvc->data_control.width_unit << view->zoom_in) / 2;
    while (node)
    {
        tag = node->PeekTag(type_index);
        if (!tag)
            break;
        if (tag->base._date == 0)
            break;
        if (tag->base._value == 0)
            break;
        BollDataTag*data = (BollDataTag*)tag->pri->c_str();
        delta = data->up_price - mvc->data_control.value_min;
        dy = delta * (view->height_main-2*MAIN_SPACEING);
        dy /= (mvc->data_control.value_max - mvc->data_control.value_min);
        y = (view->height_main) - dy-MAIN_SPACEING;
        cairo_line_to(view->cr, (x >> view->zoom_out), y);
        x += ((mvc->data_control.width_unit << view->zoom_in)+SPACEING);
        if ((x >> view->zoom_out) > view->width_main)
            break;
        node = (DiagramDataItem *) node->Next();
        E15_Debug::Printf(0,"close_price[%lld],down_price[%lld],mb_price[%lld],up_price[%lld]\n",data->close_price,data->down_price,data->mb_price,data->up_price);
    }
    cairo_stroke(view->cr);
    x = (mvc->data_control.width_unit << view->zoom_in) / 2;
    node = node_c;
    while (node)
    {
        tag = node->PeekTag(type_index);
        if (!tag)
            break;
        if (tag->base._date == 0)
            break;
        if (tag->base._value == 0)
            break;
        BollDataTag*data = (BollDataTag*)tag->pri->c_str();
        delta = data->mb_price - mvc->data_control.value_min;
        dy = delta * (view->height_main-2*MAIN_SPACEING);
        dy /= (mvc->data_control.value_max - mvc->data_control.value_min);
        y = (view->height_main) - dy-MAIN_SPACEING;
        cairo_line_to(view->cr, (x >> view->zoom_out), y);
        x += ((mvc->data_control.width_unit << view->zoom_in)+SPACEING);
        if ((x >> view->zoom_out) > view->width_main)
            break;
        node = (DiagramDataItem *) node->Next();
    }
    cairo_stroke(view->cr);
    x = (mvc->data_control.width_unit << view->zoom_in) / 2;
    node = node_c;
    while (node)
    {
        tag = node->PeekTag(type_index);
        if (!tag)
            break;
        if (tag->base._date == 0)
            break;
        if (tag->base._value == 0)
            break;
        BollDataTag*data = (BollDataTag*)tag->pri->c_str();
        delta = data->down_price - mvc->data_control.value_min;
        dy = delta * (view->height_main-2*MAIN_SPACEING);
        dy /= (mvc->data_control.value_max - mvc->data_control.value_min);
        y = (view->height_main) - dy-MAIN_SPACEING;
        cairo_line_to(view->cr, (x >> view->zoom_out), y);
        x += ((mvc->data_control.width_unit << view->zoom_in)+SPACEING);
        if ((x >> view->zoom_out) > view->width_main)
            break;
        node = (DiagramDataItem *) node->Next();
    }
    cairo_stroke(view->cr);
}

void draw_tag(int type_index, DiagramView_Control *view, MarketMVC * mvc, DrawHelper_Config * draw_config)
{
	DiagramDataItem * node;
    node = (DiagramDataItem *) mvc->handler->PeekDataItem(mvc->data_control.offset[mvc->_data_type-1]);
	if (!node)
		return;

	view->cr = view->cr_main; //附加在主图上
	cairo_set_source_rgba(view->cr, draw_config->color_line.red, draw_config->color_line.green, draw_config->color_line.blue, draw_config->color_line.alpha);
	cairo_set_line_width(view->cr, draw_config->line_width);

	DiagramTag * tag = 0;
    long long x = (mvc->data_control.width_unit << view->zoom_in) / 2;
    long long offset = 0;
	DiagramDataItem * node2;

	if (draw_config->line)
	{
		//找到上一个连线点
		node2 = node;
		while (node2)
		{
			if (node2->PeekTag(type_index))
			{
				node = node2;
				x -= offset;
				break;
			}
			node2 = (DiagramDataItem *) node2->Pre();
            offset += ((mvc->data_control.width_unit << view->zoom_in) +SPACEING);
			if (!node2)
				break;
		}
	}

	cairo_new_path(view->cr);

    double dy;
    long long delta = 0;

    long long y;

	double dashed1[] = { 4.0, 4.0 };
	if (draw_config->dash)
	{
		cairo_set_dash(view->cr, dashed1, 2, 1);
	}
	while (node)
	{
		tag = node->PeekTag(type_index);
		do
		{
			if (!tag)
				break;
			if (tag->base._date == 0)
				break;
			if (tag->base._value == 0)
				break;
			delta = tag->base._value - mvc->data_control.value_min;

            dy = delta * (view->height_main-2*MAIN_SPACEING);
			dy /= (mvc->data_control.value_max - mvc->data_control.value_min);
            y = (view->height_main) - dy-MAIN_SPACEING;

			if (draw_config->mode == 4)
			{
                draw_tag_rect(node, tag, view, mvc, x, y);
				break;
			}
            double yy = 0;
            double xx = 0;
            bool isPositive  =true;
            cairo_get_current_point(view->cr,&xx,&yy);
			if (draw_config->line)
			{
                cairo_line_to(view->cr, (x >> view->zoom_out), y);
                if(yy - y>0)
                    isPositive = false;
			}
            double diameter = draw_config->diameter / (1 << view->zoom_out);
            double triangleYL = y+(sqrt(3)/4) * diameter;
            double triangleBYH = y-(sqrt(3)/4) * diameter;
            double half = diameter / 2;

			switch (draw_config->mode)
			{
			case 1:
				if (view->zoom_out < 2)
				{
                    cairo_move_to(view->cr, (x >> view->zoom_out)+draw_config->diameter / (1 << view->zoom_out), y);
                    cairo_arc(view->cr, x >> view->zoom_out, y, draw_config->diameter / (1 << view->zoom_out), 0, 2 * M_PI);
				}
				break;
			case 2:
                cairo_move_to(view->cr,x - half,triangleYL );
                cairo_line_to(view->cr,x + half,triangleYL);
                cairo_line_to(view->cr,x,triangleBYH);
                cairo_line_to(view->cr,x - half,triangleYL);

                draw_tag_up_arrow(view->cr, x >> view->zoom_out, y);
				break;
			case 3:
                cairo_move_to(view->cr,x - half,triangleBYH );
                cairo_line_to(view->cr,x + half,triangleBYH);
                cairo_line_to(view->cr,x,triangleYL);
                cairo_line_to(view->cr,x - half,triangleBYH);
                draw_tag_down_arrow(view->cr, x >> view->zoom_out, y);
				break;
            case 5:{//画价格
                if(view->zoom_out>2)
                    break;
                cairo_set_font_size(view->cr, g_draw_font.tip.Size);
                if(isPositive)
                    cairo_move_to(view->cr, x >> view->zoom_out, y+9);
                else
                    cairo_move_to(view->cr, x >> view->zoom_out, y-3);
                E15_String tempstr;
                if(mvc->info->price_tick>=10000)
                    price_to_str(&tempstr,tag->base._value,0);
                else if(mvc->info->price_tick>=1000)
                    price_to_str(&tempstr,tag->base._value,1);
                else if(mvc->info->price_tick>=100)
                        price_to_str(&tempstr,tag->base._value,2);
                else
                    price_to_str(&tempstr,tag->base._value,3);
                cairo_show_text(view->cr,tempstr.c_str());
            }
                break;
			default:
				break;
			}
            cairo_move_to(view->cr, x >> view->zoom_out, y);
		} while (0);

        x += ((mvc->data_control.width_unit << view->zoom_in)+SPACEING);
        if ((x >> view->zoom_out) > view->width_main)
            break;
		node = (DiagramDataItem *) node->Next();
	}

	if (draw_config->line && !tag && node)
	{
		//找到上一个连线点
		node2 = (DiagramDataItem *) node->Next();

		while (node2)
		{
            x += ((mvc->data_control.width_unit << view->zoom_in)+SPACEING);
			tag = node2->PeekTag(type_index);
			if (!tag)
			{
				node2 = (DiagramDataItem *) node2->Next();
				continue;
			}

			delta = tag->base._value - mvc->data_control.value_min;

            dy = delta * (view->height_main-MAIN_SPACEING*2);
			dy /= (mvc->data_control.value_max - mvc->data_control.value_min);
            y = (view->height_main) - dy-MAIN_SPACEING;

			if (draw_config->line)
			{
                cairo_line_to(view->cr, x >> view->zoom_out, y);
			}
			break;
		}
	}

	cairo_stroke(view->cr);

	if (draw_config->dash)
	{
		cairo_set_dash(view->cr, dashed1, 0, 0);
	}

}

void draw_tag_rect(DiagramDataItem * node, DiagramTag * tag, DiagramView_Control *view, MarketMVC * mvc, long x, long y)
{
	double dy;

	if (!tag->pri)
		return;

	if (tag->pri->Length() != sizeof(MarketAnalyseDpo))
		return;
	//获得矩形的终止坐标
	DiagramDataItem * node2 = (DiagramDataItem *) node->Next();
	MarketAnalyseDpo * dpo = (MarketAnalyseDpo *) tag->pri->c_str();

    long long diff = 0;
	while (node2)
	{
		if (node2->base._date > dpo->date)
			break;
		if (node2->base._date == dpo->date)
		{
			if (node2->base._seq > dpo->seq)
				break;
		}
		diff++;
		node2 = (DiagramDataItem *) node2->Next();

        if (((x + diff * (mvc->data_control.width_unit << view->zoom_in)) >> view->zoom_out) >= view->width_main)
			break;
	}

    long long x2 = diff * ((mvc->data_control.width_unit << view->zoom_in)+SPACEING);

    long long delta = dpo->price;
	delta -= mvc->data_control.value_min;

    dy = delta * (view->height_main - MAIN_SPACEING);
	dy /= (mvc->data_control.value_max - mvc->data_control.value_min);
    long long y2 = (view->height_main) - dy-MAIN_SPACEING;

    cairo_rectangle(view->cr, x >> view->zoom_out, y, (x2 >> view->zoom_out), y2 - y);
	cairo_stroke(view->cr);

}

void draw_tag_up_arrow(cairo_t * cr, long x, long y)
{
	//cairo_move_to(cr,x>>view->zoom_out,y);

}

void draw_tag_down_arrow(cairo_t * cr, long x, long y)
{
	//cairo_move_to(cr,x>>view->zoom_out,y);
}

