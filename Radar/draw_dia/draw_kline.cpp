#include "E15_queue.h"
#include "E15_string_array.h"
#include "E15_value.h"
#include "E15_map.h"
#include <string.h>

#include "data_mgr.h"
#include"mydef.h"
#include "draw_dia.h"
void price_to_str(E15_String * str, long price ,int decimal);
extern long long g_today_open_price;
extern E15_Queue g_draw_config_list;
extern bool one_second_out;
extern int g_kavg;
extern int g_twist1;
extern int g_twist2;
extern int g_merge;
void draw_kline_data(DiagramView_Control *view, MarketMVC * mvc);
void draw_kline_macd(DiagramView_Control *view, MarketMVC * mvc, int diff_max, int bar_max,double);
void draw_buy( int,DiagramView_Control *view, MarketMVC * mvc, DiagramDataItem * node, int x);
void draw_kline_volume(DiagramView_Control *view, MarketMVC * mvc, unsigned long vol_max,double);
void draw_kline_scale(DiagramView_Control *draw, MarketMVC * mvc);
void draw_time(int begin_date,int begin_time,int x,DiagramView_Control* view);
void draw_line_segment(DiagramView_Control *draw, MarketMVC * mvc);
void draw_tip_background(DiagramView_Control *draw, MarketMVC * mvc);
void draw_now_price(DiagramView_Control *draw, MarketMVC * mvc);

DrawRGBA getColor(long long cur,long long pre)
{
    DrawRGBA co;
    if(cur > pre)
    {
        co.red = 1.0;
        co.green = 0;
        co.blue = 0;
        co.alpha = 1;
    }
    else if(pre>cur)
    {
        co.red = 0;
        co.green = 1.0;
        co.blue = 0;
        co.alpha = 1;
    }
    else
    {
        co.red = 1.0;
        co.green = 1.0;
        co.blue = 1.0;
        co.alpha = 1;
    }
    return co;
}
void draw_time_scale(DiagramView_Control *view,MarketMVC * mvc,cairo_t* cr)
{
    DiagramDataItem * node;
    node = (DiagramDataItem *) mvc->handler->PeekDataItem(mvc->data_control.offset[mvc->_data_type-1]);
    int i = 0;
    int x = 0;
    int end_x= view->width_main/10;
    if(end_x<100)
        end_x = 100;
    cairo_set_line_width(view->cr_macd, 0.4);
    cairo_set_source_rgba(view->cr_macd,1,0,0,0.4);
    cairo_set_line_width(view->cr_main, 0.4);
    cairo_set_source_rgba(view->cr_main,1,0,0,0.4);
    cairo_set_line_width(view->cr_time, 0.4);
    cairo_set_source_rgba(view->cr_time,1,0,0,0.4);
    cairo_set_line_width(view->cr_volume, 0.4);
    cairo_set_source_rgba(view->cr_volume,1,0,0,0.4);
    double dashed1[] = { 2.0, 2.0 };
    cairo_set_dash(view->cr_macd, dashed1, 2, 1);
    cairo_set_dash(view->cr_main, dashed1, 2, 1);
    cairo_set_dash(view->cr_volume, dashed1, 2, 1);
    cairo_set_dash(view->cr_time, dashed1, 2, 1);
    cairo_rectangle(view->cr_time, 0, 0, view->width, view->height_time);
    cairo_set_source_rgba(view->cr_time, 0,0,0,1);
    cairo_fill(view->cr_time);
    while(node)
    {
        node = (DiagramDataItem *) node->Next();
        if (!node)
            break;
        x += ((mvc->data_control.width_unit << view->zoom_in) + SPACEING);
        if ((x >>view->zoom_out) >view->width_main)
            break;
        if(((x >>view->zoom_out))>=(end_x*i))
        {
            draw_time(node->base._date,node->base._time,x,view);
            i++;
        }
    }
    cairo_stroke(cr);
    return;
}

void draw_callback_kline(DiagramView_Control *draw, MarketMVC * mvc)
{
	//首先自动滚动到最新的显示tick
	DiagramDataItem * node;
    node = (DiagramDataItem *) mvc->handler->PeekDataItem(mvc->data_control.offset[mvc->_data_type-1]);
	if (!node)
		return;
	MarketAnalyseKline * kdata = (MarketAnalyseKline*) node->pri->c_str();

	//计算最大值和最小值
	mvc->data_control.value_max = kdata->max_item.price + 100;
	mvc->data_control.value_min = kdata->min_item.price;
	int diff_max = kdata->diff;
	int bar_max = kdata->bar;
    int dea_max  = kdata->dea;
	long long volume_max = node->base._volume_tick + 1;
    long long OpenInterest_max = node->base.OpenInterest;
    long long OpenInterest_min = OpenInterest_max;
	node = (DiagramDataItem *) node->Next();
	int c = 1;
	while (node)
	{
		kdata = (MarketAnalyseKline*) node->pri->c_str();
		if (kdata->max_item.price > mvc->data_control.value_max)
			mvc->data_control.value_max = kdata->max_item.price;
		if (kdata->min_item.price < mvc->data_control.value_min)
			mvc->data_control.value_min = kdata->min_item.price;
		c++;
		if (kdata->diff > diff_max)
			diff_max = kdata->diff;
		else if (-kdata->diff > diff_max)
			diff_max = -kdata->diff;

		if (kdata->bar > bar_max)
			bar_max = kdata->bar;
		else if (-kdata->bar > bar_max)
			bar_max = -kdata->bar;

        if (kdata->dea > dea_max)
            dea_max = kdata->dea;
        else if (-kdata->dea > dea_max)
            dea_max = -kdata->dea;

		if (volume_max < node->base._volume_tick)
			volume_max = node->base._volume_tick;
        if(OpenInterest_max < node->base.OpenInterest)
            OpenInterest_max = node->base.OpenInterest;
        if(OpenInterest_min > node->base.OpenInterest)
            OpenInterest_min = node->base.OpenInterest;
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

    diff_max += 1;
    bar_max += 1;
    mvc->data_control.bar_max = bar_max;
    mvc->data_control.diff_max = diff_max;
    mvc->data_control.dea_max = dea_max;
    mvc->data_control.volume_max = volume_max;
    mvc->data_control.OpenInterest_max = OpenInterest_max;
    mvc->data_control.OpenInterest_min = OpenInterest_min;
	draw->cr = draw->cr_main;
    if(draw->zoom_in_out)
    {
        cairo_new_path(draw->cr_scale);
        draw_kline_scale(draw,mvc);
        draw_time_scale(draw,mvc,draw->cr_main);
    }

    double dashed1[] = { 2.0, 0};
    cairo_set_dash(draw->cr,dashed1,2,1);
	cairo_new_path(draw->cr);

	draw_kline_data(draw, mvc);

//	draw->cr = draw->cr_sub;
//	cairo_new_path(draw->cr);


    //附属图 1
    double dashed2[] = { 1, 0 };
    cairo_set_dash(draw->cr_volume, dashed2, 2, 1);
    draw->cr = draw->cr_volume;
    cairo_new_path(draw->cr);
    if(draw->volume_type==0)
    {
        draw_kline_volume(draw, mvc, volume_max,draw->height_volume);
    }
    else if(draw->volume_type==1)
    {
        draw_kline_macd(draw, mvc, diff_max, bar_max,draw->height_volume);
    }
    else
    {
        draw_kline_macd(draw, mvc, diff_max, bar_max,draw->height_volume);
    }
    //附属图 2
    draw->cr = draw->cr_macd;
    cairo_set_dash(draw->cr, dashed2, 2, 1);
    cairo_new_path(draw->cr);
    if(draw->macd_type==0)
    {
        draw_kline_volume(draw, mvc, volume_max,draw->height_macd);
    }
    else if(draw->macd_type==1)
    {
        draw_kline_macd(draw, mvc, diff_max, bar_max,draw->height_macd);
    }
    else
    {
        draw_kline_macd(draw, mvc, diff_max, bar_max,draw->height_macd);
    }

//    draw->cr = draw->cr_macd;
//    cairo_new_path(draw->cr);
//    draw_kline_macd(draw, mvc, diff_max, bar_max);
//    double dashed2[] = { 1, 0 };
//    cairo_set_dash(draw->cr_volume, dashed2, 2, 1);
//    draw->cr = draw->cr_volume;
//    cairo_new_path(draw->cr);
//    draw_kline_volume(draw, mvc, volume_max);
//    draw_line_segment(draw ,mvc);
}
int get_begin(bool direct,unsigned int date,unsigned int seq,DiagramDataItem * node)
{
    int count = 0;
    if(direct)
    {
        while(node)
        {
            if(node->base._date==date&&node->base._seq==seq)
            {
                return count;
            }
            count++;
            node = (DiagramDataItem*)node->Next();
        }
    }
    else
    {
        while(node)
        {
            if(node->base._date==date&&node->base._seq==seq)
            {
                return count;
            }
            count--;
            node = (DiagramDataItem*)node->Pre();
        }
    }
    return -1;
}

void draw_line_segment(DiagramView_Control *draw, MarketMVC * mvc)
{
    DiagramDataItem * node;
    int x= 0;
    int has  =false;
    node = (DiagramDataItem *) mvc->handler->PeekData()->At(mvc->data_control.offset[mvc->_data_type-1], 0);
    while(node)
    {
        if(node->m_lines)
        {
            Line_Point_Item *line_node = (Line_Point_Item*)node->m_lines->At(0,0);
            while(line_node)
            {
                cairo_set_source_rgba(draw->cr_main, 1, 0, 0, 1);
                long long dy = 0;
                long long delta = line_node->line.end_price - mvc->data_control.value_min;
                dy = delta;
                dy *= (long long) (draw->height_main- MAIN_SPACEING*2);
                dy /= (long long) (mvc->data_control.value_max - mvc->data_control.value_min);
                int y1 = (long long) draw->height_main - dy- MAIN_SPACEING*2;
                cairo_move_to(draw->cr_main,(x + (mvc->data_control.width_unit << draw->zoom_in) / 2) >> draw->zoom_out,y1 +MAIN_SPACEING);
                int count = get_begin(line_node->line.direct,line_node->line.begin_date,line_node->line.begin_seq,node);
                int b_x = 0;
                dy = line_node->line.begin_price - mvc->data_control.value_min;
                dy *= (long long) (draw->height_main- MAIN_SPACEING*2);
                dy /= (long long) (mvc->data_control.value_max - mvc->data_control.value_min);
                long long y2 = (long long) draw->height_main - dy- MAIN_SPACEING;
                if(draw->zoom_in>=0)
                {
                    b_x = count*(((mvc->data_control.width_unit << draw->zoom_in))+SPACEING);
                    cairo_line_to(draw->cr_main,(((x+b_x) + (mvc->data_control.width_unit << draw->zoom_in) / 2) >> draw->zoom_out),y2);
                }
                else
                {
                    b_x = count*((mvc->data_control.width_unit+SPACEING));
                    cairo_line_to(draw->cr_main,(((x+b_x) + (mvc->data_control.width_unit << draw->zoom_in) / 2) >> draw->zoom_out),y2);
                }
                cairo_stroke(draw->cr_main);
                //如果发生点击事件
                if(draw->mouse_press)
                {
                    DiagramDataItem* node_cur = (DiagramDataItem*)mvc->handler->PeekData()->At(mvc->data_control.focus, 0);
                    if(node_cur)
                    {
                        long long price = 0;
                        int co1 =  get_begin(line_node->line.direct,node_cur->base._date,node_cur->base._seq ,node);
                        if(count>0&&co1<=count)
                        {
                            price = (line_node->line.end_price - line_node->line.begin_price);
                            price = (long long) (line_node->line.end_price - price*((double)co1/count));
                        }
                        else if(count<0&&co1>=count&&co1!=-1)
                        {
                            price = (line_node->line.end_price - line_node->line.begin_price);
                            price = (long long) (line_node->line.end_price - price*((double)co1/count));
                        }
                        int pos;
                        long long delta = price - mvc->data_control.value_min;
                        dy = delta;
                        dy *= (draw->height_main-2*MAIN_SPACEING);
                        dy /= (mvc->data_control.value_max - mvc->data_control.value_min);
                        pos = draw->height_main - dy- MAIN_SPACEING+g_draw_font.id.Size + 2;
                        if(draw->mouse_y<=pos+5&&draw->mouse_y>=pos-5)
                        {
                            line_node->line.is_select =true;
                            draw->mouse_press = false;
                        }
                        else
                        {
                            line_node->line.is_select =false;
                        }
                    }
                }
                if(line_node->line.is_select)
                {
                    cairo_set_source_rgba(draw->cr_main, 0, 1, 0, 1);
                    has = true;
                    int x1 = (((x+b_x) + (mvc->data_control.width_unit << draw->zoom_in) / 2) >> draw->zoom_out);
                    int x2 = (x + (mvc->data_control.width_unit << draw->zoom_in) / 2) >> draw->zoom_out;
                    cairo_rectangle(draw->cr_main,x1,y2,1,1);
                    dy = ((line_node->line.end_price-line_node->line.begin_price)/2 +line_node->line.begin_price -mvc->data_control.value_min);
                    dy *= (long long) (draw->height_main- MAIN_SPACEING*2);
                    dy /= (long long) (mvc->data_control.value_max - mvc->data_control.value_min);
                    y2 = (long long) (draw->height_main - dy- MAIN_SPACEING);
                    cairo_rectangle(draw->cr_main,(x1-x2)/2 +x2,y2,1,1);
                    cairo_rectangle(draw->cr_main,x2,y1 +MAIN_SPACEING,1,1);
                    cairo_stroke(draw->cr_main);
                }
                line_node = (Line_Point_Item*)line_node->Next();
            }
        }
        node = (DiagramDataItem*)node->Next();
        if (!node)
            break;
        x += ((mvc->data_control.width_unit << draw->zoom_in)+SPACEING);
        if ((x >> draw->zoom_out) > draw->width_main)
            break;
    }
    if(has)
        draw->has_select  =true;
    else
        draw->has_select  =false;
    cairo_stroke(draw->cr_main);

}

void draw_kline_info(DiagramView_Control *draw, MarketMVC * mvc, DiagramDataItem * node);

void kline_draw_focus_info(DiagramView_Control *draw, MarketMVC * mvc)
{

    DiagramDataItem * node;
    node = (DiagramDataItem *) mvc->handler->PeekData()->At(mvc->data_control.focus, 0);

    if (!node)
        return;

	if (!draw->mouse_in)
		return;
	draw_kline_info(draw, mvc, node);

}

extern void Price2Str(E15_String * str, long price);

void draw_kline_info(DiagramView_Control *draw, MarketMVC * mvc, DiagramDataItem * node)
{

    DrawHelper_Config * tag_conf;
    int cnt = mvc->handler->GetTagCount();
    int i;
    E15_String ss;
    double xx = 0;
    double yy = 0;
    ss.Append("%d seq:%d  ",node->base._date,node->base._seq);
    cairo_get_current_point(draw->cr,&xx,&yy);
    cairo_move_to(draw->cr, xx, g_draw_font.tip.Size);
    cairo_set_source_rgba(draw->cr,1,1,1,1);
    cairo_show_text(draw->cr, ss.c_str());
    MarketDataType * dt;
    for (i = 1; i <= cnt; i++)
    {
        E15_String s;
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
                    continue;
            }
        }
        if (!tag_conf->show_hide)
            continue;
        DiagramTag * tag = node->PeekTag(i);
        if(tag)
        {
            s.Append("%s %d :%d ",dt->name,dt->param,tag->base._value/10000);
            cairo_get_current_point(draw->cr,&xx,&yy);
            cairo_move_to(draw->cr, xx, g_draw_font.tip.Size);
            cairo_set_source_rgba(draw->cr,tag_conf->color_line.red,tag_conf->color_line.green,tag_conf->color_line.blue,tag_conf->color_line.alpha);
            cairo_show_text(draw->cr, s.c_str());
        }
    }
    cairo_stroke(draw->cr);
}


void draw_tip_info(DiagramView_Control *draw, DiagramDrawCallbackData * cbdata)
{
    int decimal = 0;
    if(cbdata->data->info.price_tick>=10000)
        decimal = 0;
    else if(cbdata->data->info.price_tick>=1000)
        decimal = 1;
    else if(cbdata->data->info.price_tick>=100)
        decimal = 2;
    else
        decimal = 3;
    cairo_text_extents_t text;
    cairo_select_font_face(draw->cr_tip, "文泉驿等宽正黑", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(draw->cr_tip, g_draw_font.tip.Size);
    cairo_set_source_rgba(draw->cr_tip,1,0,0,1);
    DiagramDataMgr * stock = cbdata->data;
    DrawRGBA color;
    do
    {
        if(!stock)
            break;
        MarketDepthData * depth = stock->depth;
        if(!depth)
            break;
        int adjust = 0;
        if(!draw->zoom_in_out)
            adjust = 2;
        long long pre_price = depth->ext.PreSettlementPrice;
        color = getColor(depth->base.bid_ask.Ask,pre_price);
        cairo_set_source_rgba(draw->cr_tip,color.red,color.green,color.blue,color.alpha);
        E15_String price;
        price_to_str( &price, depth->base.bid_ask.Ask , decimal);
        cairo_text_extents(draw->cr_tip,price.c_str(),&text);
        cairo_move_to(draw->cr_tip,TIP_WIDTH-text.width-SPACEING,g_draw_font.tip.Size*(adjust+Ask_Price));
        cairo_show_text(cbdata->cr_tip,price.c_str());

        price_to_str( &price, depth->base.bid_ask.Bid , decimal);
        color = getColor(depth->base.bid_ask.Bid,pre_price);
        cairo_set_source_rgba(draw->cr_tip,color.red,color.green,color.blue,color.alpha);
        cairo_text_extents(draw->cr_tip,price.c_str(),&text);
        cairo_move_to(draw->cr_tip,TIP_WIDTH-text.width-SPACEING,g_draw_font.tip.Size*(adjust+Bid_Price));
        cairo_show_text(cbdata->cr_tip,price.c_str());

        price_to_str( &price, depth->base.nPrice , decimal);
        color = getColor(depth->base.nPrice,pre_price);
        cairo_set_source_rgba(draw->cr_tip,color.red,color.green,color.blue,color.alpha);
        cairo_text_extents(draw->cr_tip,price.c_str(),&text);
        cairo_move_to(draw->cr_tip,TIP_WIDTH-text.width-SPACEING,g_draw_font.tip.Size*(adjust+Latest_Price));
        cairo_show_text(cbdata->cr_tip,price.c_str());

        cairo_set_source_rgba(draw->cr_tip,1,1,0,1);
        price.Sprintf("%ld",depth->base.bid_ask.AskVolume);
        cairo_text_extents(draw->cr_tip,price.c_str(),&text);
        cairo_move_to(draw->cr_tip,TIP_WIDTH-text.width-SPACEING,g_draw_font.tip.Size*(adjust+Ask_Volume));
        cairo_show_text(cbdata->cr_tip,price.c_str());

        price.Sprintf("%ld",depth->base.bid_ask.BidVolume);
        cairo_text_extents(draw->cr_tip,price.c_str(),&text);
        cairo_move_to(draw->cr_tip,TIP_WIDTH-text.width-SPACEING,g_draw_font.tip.Size*(adjust+Bid_Volume));
        cairo_show_text(cbdata->cr_tip,price.c_str());


        price_to_str( &price, depth->ext.UpperLimitPrice , decimal);//涨停板
        cairo_set_source_rgba(draw->cr_tip,1,0,0,1);
        cairo_text_extents(draw->cr_tip,price.c_str(),&text);
        cairo_move_to(draw->cr_tip,TIP_WIDTH-text.width-SPACEING,g_draw_font.tip.Size*(adjust+UpperLimitPrice));
        cairo_show_text(cbdata->cr_tip,price.c_str());

        price_to_str( &price, depth->ext.LowerLimitPrice , decimal);//跌停板
        cairo_set_source_rgba(draw->cr_tip,0,1,0,1);
        cairo_text_extents(draw->cr_tip,price.c_str(),&text);
        cairo_move_to(draw->cr_tip,TIP_WIDTH-text.width-SPACEING,g_draw_font.tip.Size*(adjust+LowerLimitPrice));
        cairo_show_text(cbdata->cr_tip,price.c_str());

//        price_to_str( &price, depth->base.iVolume*10000 , decimal);//跌停板
        price.Sprintf("%ld",depth->base.iVolume);
        cairo_set_source_rgba(draw->cr_tip,0,1,0,1);
        cairo_text_extents(draw->cr_tip,price.c_str(),&text);
        cairo_move_to(draw->cr_tip,TIP_WIDTH-text.width-SPACEING,g_draw_font.tip.Size*(adjust+Today_Turnover_Volume));
        cairo_show_text(cbdata->cr_tip,price.c_str());
    }while(0);
    if(cbdata->view.zoom_in_out==1)//放大
    {
        DiagramDataItem * node;
        node = (DiagramDataItem *) cbdata->mvc->handler->PeekData()->At(cbdata->mvc->data_control.focus, 0);
        if (!node)
        {
            return;
        }
        if(strcmp(cbdata->mvc->handler->GetDataType()->class_name,"normalize")==0)
        {
            MarketDepthData * depth = (MarketDepthData *)node->pri->c_str();
            long long time = depth->base.nTime/1000;
            long long pre_price = 0;
            if(!stock ||!stock->depth)
            {
                pre_price = g_today_open_price;
            }
            else
            {
                pre_price = stock->depth->ext.PreClose;
            }
            E15_String date;
            date.Sprintf("%ld",depth->base.nActionDay);
            date.Insert(4,'-');
            date.Insert(7,'-');
            cairo_text_extents(draw->cr_tip,date.c_str(),&text);
            cairo_move_to(draw->cr_tip,TIP_WIDTH-text.width -SPACEING,g_draw_font.tip.Size*Date);
            cairo_show_text(draw->cr_tip,date.c_str());
            E15_String t;
            t.Sprintf("%06ld",time);
            t.Insert(2,':');
            t.Insert(5,':');
            cairo_text_extents(draw->cr_tip,t.c_str(),&text);
            cairo_move_to(draw->cr_tip,TIP_WIDTH-text.width-SPACEING,g_draw_font.tip.Size*Time);
            cairo_show_text(draw->cr_tip,t.c_str());
            color = getColor(node->base._value,pre_price);
            cairo_set_source_rgba(draw->cr_tip,color.red,color.green,color.blue,color.alpha);

            E15_String temp;
            price_to_str(&temp,node->base._value,decimal);
            cairo_text_extents(draw->cr_tip,temp.c_str(),&text);
            cairo_move_to(draw->cr_tip,TIP_WIDTH-text.width-SPACEING,g_draw_font.tip.Size*Open_Price);
            cairo_show_text(draw->cr_tip,temp.c_str());
            cairo_move_to(draw->cr_tip,TIP_WIDTH-text.width-SPACEING,g_draw_font.tip.Size*Close_Price);
            cairo_show_text(draw->cr_tip,temp.c_str());
            cairo_move_to(draw->cr_tip,TIP_WIDTH-text.width-SPACEING,g_draw_font.tip.Size*Max_Price);
            cairo_show_text(draw->cr_tip,temp.c_str());
            cairo_move_to(draw->cr_tip,TIP_WIDTH-text.width-SPACEING,g_draw_font.tip.Size*Min_Price);
            cairo_show_text(draw->cr_tip,temp.c_str());
            temp.Sprintf("%ld%",node->base._pv_tick);
            cairo_text_extents(draw->cr_tip,temp.c_str(),&text);
            cairo_move_to(draw->cr_tip,TIP_WIDTH-text.width-SPACEING,g_draw_font.tip.Size*Turnover_Value);
            cairo_show_text(draw->cr_tip,temp.c_str());

            DiagramDataMgr * stock = cbdata->data;
            if( !stock )
            {
                return;
            }
            if(stock->depth && stock->pre_close)
            {
                double speed = (((double)depth->base.nPrice - stock->pre_close)/stock->pre_close) *100;
                if(speed>0.000001)
                {
                    color.red = 1;
                    color.green = 0;
                    color.blue  = 0;
                    color.alpha = 1;
                }
                else if(speed<-0.000001)
                {
                    color.red = 0;
                    color.green = 1;
                    color.blue  = 0;
                    color.alpha = 1;
                }
                else
                {
                    color.red = 1;
                    color.green = 1;
                    color.blue  = 1;
                    color.alpha = 1;
                }
                cairo_set_source_rgba(draw->cr_tip,color.red,color.green,color.blue,color.alpha);
                temp.Sprintf("%.2f",speed);
                cairo_text_extents(draw->cr_tip,temp.c_str(),&text);
                cairo_move_to(draw->cr_tip,TIP_WIDTH-text.width-SPACEING,g_draw_font.tip.Size*Speed);
                cairo_show_text(draw->cr_tip,temp.c_str());
                cairo_show_text(draw->cr_tip,"%");
            }

            cairo_set_source_rgba(draw->cr_tip,1,1,0,1);
            temp.Sprintf("%ld%",node->base.OpenInterest);
            cairo_text_extents(draw->cr_tip,temp.c_str(),&text);
            cairo_move_to(draw->cr_tip,TIP_WIDTH-text.width-SPACEING,g_draw_font.tip.Size*Open_Interest);
            cairo_show_text(draw->cr_tip,temp.c_str());
            temp.Sprintf("%ld%",node->base._volume_tick);
            cairo_text_extents(draw->cr_tip,temp.c_str(),&text);
            cairo_move_to(draw->cr_tip,TIP_WIDTH-text.width-SPACEING,g_draw_font.tip.Size*Turnover_Volume);
            cairo_show_text(draw->cr_tip,temp.c_str());
        }
        else if(strcmp(cbdata->mvc->handler->GetDataType()->class_name,"kline")==0)
        {
            MarketAnalyseKline * kdata = (MarketAnalyseKline*) node->pri->c_str();
            long long pre_price = 0;
            if(!stock->depth)
            {
                pre_price = g_today_open_price;
            }
            else
            {
                pre_price = stock->depth->ext.PreClose;
            }
           cairo_set_source_rgba(draw->cr_tip,1,1,1,1);
           E15_String date;
           date.Sprintf("%ld",node->base._trade_date);
           date.Insert(4,'-');
           date.Insert(7,'-');
           cairo_text_extents(draw->cr_tip,date.c_str(),&text);
           cairo_move_to(draw->cr_tip,TIP_WIDTH-text.width - SPACEING,g_draw_font.tip.Size*Date);
           cairo_show_text(draw->cr_tip,date.c_str());
           E15_String t;
           t.Sprintf("%06ld",kdata->close_item.time/1000);
           t.Insert(2,':');
           t.Insert(5,':');
           cairo_text_extents(draw->cr_tip,t.c_str(),&text);
           cairo_move_to(draw->cr_tip,TIP_WIDTH-text.width-SPACEING,g_draw_font.tip.Size*Time);
           cairo_show_text(draw->cr_tip,t.c_str());

           E15_String temp;
           price_to_str(&temp,kdata->open_item.price,decimal);
           cairo_text_extents(draw->cr_tip,temp.c_str(),&text);
           color = getColor(kdata->open_item.price,pre_price);
           cairo_set_source_rgba(draw->cr_tip,color.red,color.green,color.blue,color.alpha);
           cairo_move_to(draw->cr_tip,TIP_WIDTH-text.width-SPACEING,g_draw_font.tip.Size*Open_Price);
           cairo_show_text(draw->cr_tip,temp.c_str());
           price_to_str(&temp,kdata->close_item.price,decimal);
           cairo_text_extents(draw->cr_tip,temp.c_str(),&text);
           cairo_move_to(draw->cr_tip,TIP_WIDTH-text.width-SPACEING,g_draw_font.tip.Size*Close_Price);
           cairo_show_text(draw->cr_tip,temp.c_str());
           color = getColor(kdata->close_item.price,pre_price);
           cairo_set_source_rgba(draw->cr_tip,color.red,color.green,color.blue,color.alpha);

           price_to_str(&temp,kdata->max_item.price,decimal);
           cairo_text_extents(draw->cr_tip,temp.c_str(),&text);
           cairo_move_to(draw->cr_tip,TIP_WIDTH-text.width-SPACEING,g_draw_font.tip.Size*Max_Price);
           cairo_show_text(draw->cr_tip,temp.c_str());
           color = getColor(kdata->max_item.price,pre_price);
           cairo_set_source_rgba(draw->cr_tip,color.red,color.green,color.blue,color.alpha);

           price_to_str(&temp,kdata->min_item.price,decimal);
           cairo_text_extents(draw->cr_tip,temp.c_str(),&text);
           cairo_move_to(draw->cr_tip,TIP_WIDTH-text.width-SPACEING,g_draw_font.tip.Size*Min_Price);
           cairo_show_text(draw->cr_tip,temp.c_str());
           color = getColor(kdata->min_item.price,pre_price);
           cairo_set_source_rgba(draw->cr_tip,color.red,color.green,color.blue,color.alpha);

           temp.Sprintf("%ld",node->base._pv_tick);
           cairo_text_extents(draw->cr_tip,temp.c_str(),&text);
           cairo_move_to(draw->cr_tip,TIP_WIDTH-text.width-SPACEING,g_draw_font.tip.Size*Turnover_Value);
           cairo_show_text(draw->cr_tip,temp.c_str());

           cairo_set_source_rgba(draw->cr_tip,1,1,1,1);
           temp.Sprintf("%ld",node->base._pv_tick);
           cairo_text_extents(draw->cr_tip,temp.c_str(),&text);
           cairo_move_to(draw->cr_tip,TIP_WIDTH - g_draw_font.tip.Size,g_draw_font.tip.Size*Speed);
           cairo_show_text(draw->cr_tip,"0");
           cairo_set_source_rgba(draw->cr_tip,1,1,0,1);
           temp.Sprintf("%ld",node->base._volume_tick);
           cairo_text_extents(draw->cr_tip,temp.c_str(),&text);
           cairo_move_to(draw->cr_tip,TIP_WIDTH-text.width-SPACEING,g_draw_font.tip.Size*Turnover_Volume);
           cairo_show_text(draw->cr_tip,temp.c_str());
           temp.Sprintf("%ld",node->base.OpenInterest);
           cairo_text_extents(draw->cr_tip,temp.c_str(),&text);
           cairo_move_to(draw->cr_tip,TIP_WIDTH-text.width-SPACEING,g_draw_font.tip.Size*Open_Interest);
           cairo_show_text(draw->cr_tip,temp.c_str());
        }
        else
        {
            MarketAnalyseBrick * kdata = (MarketAnalyseBrick*) node->pri->c_str();
            long long pre_price = 0;
            if(!stock->depth)
            {
                pre_price = g_today_open_price;
            }
            else
            {
                pre_price = stock->depth->ext.PreClose;
            }
            cairo_set_source_rgba(draw->cr_tip,1,1,1,1);
            E15_String date;
            date.Sprintf("%ld",node->base._trade_date);
            date.Insert(4,'-');
            date.Insert(7,'-');
            cairo_text_extents(draw->cr_tip,date.c_str(),&text);
            cairo_move_to(draw->cr_tip,TIP_WIDTH-text.width - SPACEING,g_draw_font.tip.Size*Date);
            cairo_show_text(draw->cr_tip,date.c_str());
            E15_String t;
            t.Sprintf("%06ld",kdata->currunt_item.time/1000);
            t.Insert(2,':');
            t.Insert(5,':');
            cairo_text_extents(draw->cr_tip,t.c_str(),&text);
            cairo_move_to(draw->cr_tip,TIP_WIDTH-text.width-SPACEING,g_draw_font.tip.Size*Time);
            cairo_show_text(draw->cr_tip,t.c_str());

            E15_String temp;
            price_to_str(&temp,kdata->max_item.price,decimal);
            cairo_text_extents(draw->cr_tip,temp.c_str(),&text);
            color = getColor(kdata->max_item.price,pre_price);
            cairo_set_source_rgba(draw->cr_tip,color.red,color.green,color.blue,color.alpha);
            cairo_move_to(draw->cr_tip,TIP_WIDTH-text.width-SPACEING,g_draw_font.tip.Size*Open_Price);
            cairo_show_text(draw->cr_tip,temp.c_str());
            price_to_str(&temp,kdata->min_item.price,decimal);
            cairo_text_extents(draw->cr_tip,temp.c_str(),&text);
            cairo_move_to(draw->cr_tip,TIP_WIDTH-text.width-SPACEING,g_draw_font.tip.Size*Close_Price);
            cairo_show_text(draw->cr_tip,temp.c_str());

            price_to_str(&temp,kdata->currunt_item.price,decimal);
            cairo_text_extents(draw->cr_tip,temp.c_str(),&text);
            cairo_move_to(draw->cr_tip,TIP_WIDTH-text.width-SPACEING,g_draw_font.tip.Size*Max_Price);
            cairo_show_text(draw->cr_tip,temp.c_str());
        }
    }
    cairo_stroke(draw->cr_tip);
}

void draw_tip_background(DiagramView_Control *draw, MarketMVC * mvc)
{
    cairo_set_source_rgba(draw->cr_tip, 0,0,0,1);
    cairo_rectangle(draw->cr_tip, 0, 0, TIP_WIDTH, draw->height_tip);
    cairo_fill(draw->cr_tip);
    cairo_new_path(draw->cr_tip);
    cairo_select_font_face(draw->cr_tip, "文泉驿等宽正黑", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(draw->cr_tip, g_draw_font.tip.Size);
    cairo_set_source_rgba(draw->cr_tip,1,0,0,1);
    int adjust = 0;
    cairo_text_extents_t text;
    if(draw->zoom_in_out==1)//放大
    {
        cairo_move_to(draw->cr_tip, 0, g_draw_font.tip.Size*Date_Text);
        cairo_show_text(draw->cr_tip, "日期");
        cairo_move_to(draw->cr_tip, 0, g_draw_font.tip.Size*Time_Text);
        cairo_show_text(draw->cr_tip, "时间");
        cairo_move_to(draw->cr_tip, 0, g_draw_font.tip.Size*Open_Price_Text);
        cairo_show_text(draw->cr_tip, "开盘价");
        cairo_move_to(draw->cr_tip, 0, g_draw_font.tip.Size*Close_Price_Text);
        cairo_show_text(draw->cr_tip, "收盘价");
        cairo_move_to(draw->cr_tip, 0, g_draw_font.tip.Size*Max_Price_Text);
        cairo_show_text(draw->cr_tip, "最高价");

        cairo_move_to(draw->cr_tip, 0, g_draw_font.tip.Size*Min_Price_Text);
        cairo_show_text(draw->cr_tip, "最低价");
        cairo_move_to(draw->cr_tip, 0, g_draw_font.tip.Size*Turnover_Volume_Text);
        cairo_show_text(draw->cr_tip, "成交量");
        cairo_move_to(draw->cr_tip, 0, g_draw_font.tip.Size*Turnover_Value_Text);
        cairo_show_text(draw->cr_tip, "成交额");
        cairo_move_to(draw->cr_tip, 0, g_draw_font.tip.Size*Open_Interest_Text);
        cairo_show_text(draw->cr_tip, "持仓量");
        cairo_move_to(draw->cr_tip, 0, g_draw_font.tip.Size*Speed_Text);
        cairo_show_text(draw->cr_tip, "涨速");

        cairo_move_to(draw->cr_tip,0,Split_line1*g_draw_font.tip.Size - g_draw_font.tip.Size/1.5);
        cairo_line_to(draw->cr_tip,TIP_WIDTH,Split_line1*g_draw_font.tip.Size- g_draw_font.tip.Size/1.5);
        cairo_move_to(draw->cr_tip,0,Split_line2*g_draw_font.tip.Size- g_draw_font.tip.Size/1.5);
        cairo_line_to(draw->cr_tip,TIP_WIDTH,Split_line2*g_draw_font.tip.Size- g_draw_font.tip.Size/1.5);
    }
    else
    {
        adjust =2;
        E15_String s;
        s.Sprintf("%d",draw->index);
        cairo_text_extents(draw->cr_tip,s.c_str(),&text);
        cairo_move_to(draw->cr_tip, 0, g_draw_font.tip.Size*(Ask_Price_Text));
        cairo_show_text(draw->cr_tip, "窗口");
        cairo_move_to(draw->cr_tip, TIP_WIDTH- text.width-SPACEING, g_draw_font.tip.Size*(Ask_Price_Text+1));
        cairo_show_text(draw->cr_tip, s.c_str());
    }
    cairo_move_to(draw->cr_tip, 0, g_draw_font.tip.Size*(Ask_Price_Text+adjust));
    cairo_show_text(draw->cr_tip, "卖出");
    cairo_move_to(draw->cr_tip, 0, g_draw_font.tip.Size*(Ask_Volume_Text+adjust));
    cairo_show_text(draw->cr_tip, "卖量");
    cairo_move_to(draw->cr_tip, 0, g_draw_font.tip.Size*(Bid_Price_Text+adjust));
    cairo_show_text(draw->cr_tip, "买入");
    cairo_move_to(draw->cr_tip, 0, g_draw_font.tip.Size*(Bid_Volume_Text+adjust));
    cairo_show_text(draw->cr_tip, "买量");
    cairo_move_to(draw->cr_tip, 0, g_draw_font.tip.Size*(Latest_Price_Text+adjust));
    cairo_show_text(draw->cr_tip, "最新价");
    cairo_move_to(draw->cr_tip, 0, g_draw_font.tip.Size*(UpperLimitPriceText+adjust));
    cairo_show_text(draw->cr_tip, "涨停板");
    cairo_move_to(draw->cr_tip, 0, g_draw_font.tip.Size*(LowerLimitPriceText+adjust));
    cairo_show_text(draw->cr_tip, "跌停板");
    cairo_move_to(draw->cr_tip, 0, g_draw_font.tip.Size*(Today_Turnover_Volume_Text+adjust));
    cairo_show_text(draw->cr_tip, "今成交量");
    cairo_set_line_width(draw->cr_tip, 1);
    double dashed1[] = { 2.0, 2.0 };
    cairo_set_dash(draw->cr_tip, dashed1, 2, 1);
    cairo_move_to(draw->cr_tip,TIP_WIDTH,0);
    cairo_line_to(draw->cr_tip,TIP_WIDTH,draw->height_tip);
    cairo_stroke(draw->cr_tip);

}

DrawRGBA star_color = { 1.0, 1.0, 1.0, 1.0 };
DrawRGBA negative_color = { 0.5, 1.0, 1.0, 1.0 };
DrawRGBA positively_color = { 1.0, 0.0, 0.0, 1.0 };

DrawRGBA star_color_a = { 1.0, 1.0, 1.0, 0.4 };
DrawRGBA negative_color_a = { 0.5, 1.0, 1.0, 0.4 };
DrawRGBA positively_color_a = { 1.0, 0.0, 0.0, 0.4 };

void draw_k_star(DiagramView_Control *draw, MarketMVC * mvc, DiagramDataItem * node, int x)
{
    double y = 0;
	double dy = 0;
	int delta;

    double y_min;
    double y_max;

	MarketAnalyseKline * kdata = (MarketAnalyseKline*) node->pri->c_str();

	//计算最大值和最小值

	delta = kdata->open_item.price - mvc->data_control.value_min;
	dy = delta;
    dy *= (double) (draw->height_main-2*MAIN_SPACEING);
    dy /= (double) (mvc->data_control.value_max - mvc->data_control.value_min);
    y = (double) draw->height_main - dy-MAIN_SPACEING;

	if (mvc->draw_helper->draw_config.mode && node->base._type != 0)
        cairo_set_source_rgba(draw->cr, star_color_a.red, star_color_a.green, star_color_a.blue, star_color_a.alpha);
    else
        cairo_set_source_rgba(draw->cr, star_color.red, star_color.green, star_color.blue, star_color.alpha);

    cairo_move_to(draw->cr, (x >> draw->zoom_out), y);
    cairo_line_to(draw->cr, (x + ((mvc->data_control.width_unit) << draw->zoom_in) - SPACEING1) >> draw->zoom_out, y);

	cairo_stroke(draw->cr);
	delta = kdata->max_item.price - kdata->min_item.price;
	if (delta == 0)
		return;

	delta = kdata->max_item.price - mvc->data_control.value_min;
	dy = delta;
    dy *= (double) (draw->height_main-2*MAIN_SPACEING);
    dy /= (double) (mvc->data_control.value_max - mvc->data_control.value_min);
    y_max = (double) draw->height_main - dy-MAIN_SPACEING;

	delta = kdata->min_item.price - mvc->data_control.value_min;
	dy = delta;
    dy *= (double) (draw->height_main-2*MAIN_SPACEING);
    dy /= (double) (mvc->data_control.value_max - mvc->data_control.value_min);
    y_min = (double) draw->height_main - dy-MAIN_SPACEING;

	cairo_stroke(draw->cr);
    cairo_move_to(draw->cr, (x + ((mvc->data_control.width_unit) << draw->zoom_in) / 2) >> draw->zoom_out, y_max);
    cairo_line_to(draw->cr, (x + ((mvc->data_control.width_unit) << draw->zoom_in) / 2) >> draw->zoom_out, y_min);

	cairo_stroke(draw->cr);

}

void draw_k_positively(DiagramView_Control *draw, MarketMVC * mvc, DiagramDataItem * node, int x)
{ //阳线
    double y_open;
    double y_close;
    double y_min;
    double y_max;

	double dy = 0;
	int delta;

	MarketAnalyseKline * kdata = (MarketAnalyseKline*) node->pri->c_str();
	//计算最大值和最小值

	if (mvc->draw_helper->draw_config.mode && node->base._type != 0)
		cairo_set_source_rgba(draw->cr, positively_color_a.red, positively_color_a.green, positively_color_a.blue, positively_color_a.alpha);
	else
		cairo_set_source_rgba(draw->cr, positively_color.red, positively_color.green, positively_color.blue, positively_color.alpha);

	delta = kdata->open_item.price - mvc->data_control.value_min;
	dy = delta;
    dy *= (double) (draw->height_main - MAIN_SPACEING*2);
    dy /= (double) (mvc->data_control.value_max - mvc->data_control.value_min);
    y_open = (double) draw->height_main - dy - MAIN_SPACEING;

	delta = kdata->close_item.price - mvc->data_control.value_min;
	dy = delta;
    dy *= (double) (draw->height_main- MAIN_SPACEING*2);
    dy /= (double) (mvc->data_control.value_max - mvc->data_control.value_min);
    y_close = (double) draw->height_main - dy - MAIN_SPACEING;

	delta = kdata->max_item.price - mvc->data_control.value_min;
	dy = delta;
    dy *= (double) (draw->height_main- MAIN_SPACEING*2);
    dy /= (double) (mvc->data_control.value_max - mvc->data_control.value_min);
    y_max = (double) draw->height_main - dy- MAIN_SPACEING;

	delta = kdata->min_item.price - mvc->data_control.value_min;
	dy = delta;
    dy *= (double) (draw->height_main- MAIN_SPACEING*2);
    dy /= (double) (mvc->data_control.value_max - mvc->data_control.value_min);
    y_min = (double) draw->height_main - dy- MAIN_SPACEING;

    cairo_rectangle(draw->cr, x >> draw->zoom_out, y_close, ((mvc->data_control.width_unit << draw->zoom_in) - SPACEING1) >> draw->zoom_out,(y_open - y_close));
	cairo_stroke(draw->cr);
	//下影线
	if (y_min != y_open)
	{
        cairo_move_to(draw->cr, (x + (mvc->data_control.width_unit << draw->zoom_in) / 2) >> draw->zoom_out, y_open);
        cairo_line_to(draw->cr, (x + (mvc->data_control.width_unit << draw->zoom_in) / 2) >> draw->zoom_out, y_min);
		cairo_stroke(draw->cr);
	}
	//上影线
	if (y_max != y_close)
	{
        cairo_move_to(draw->cr, (x + (mvc->data_control.width_unit << draw->zoom_in) / 2) >> draw->zoom_out, y_close );
        cairo_line_to(draw->cr, (x + (mvc->data_control.width_unit << draw->zoom_in) / 2) >> draw->zoom_out, y_max);
	}
	cairo_stroke(draw->cr);
}

void draw_k_negative(DiagramView_Control *draw, MarketMVC * mvc, DiagramDataItem * node, int x)
{ //阴线
    double y_open;
    double y_close;
    double y_min;
    double y_max;

	MarketAnalyseKline * kdata = (MarketAnalyseKline*) node->pri->c_str();
	//计算最大值和最小值


	double dy = 0;
	int delta;

	if (mvc->draw_helper->draw_config.mode && node->base._type != 0)
        cairo_set_source_rgba(draw->cr, negative_color_a.red, negative_color_a.green, negative_color_a.blue, negative_color_a.alpha);
	else
		cairo_set_source_rgba(draw->cr, negative_color.red, negative_color.green, negative_color.blue, negative_color.alpha);

	delta = kdata->open_item.price - mvc->data_control.value_min;
	dy = delta;
    dy *= (draw->height_main-2*MAIN_SPACEING);
    dy /= (mvc->data_control.value_max - mvc->data_control.value_min);
    y_open =  draw->height_main - dy -MAIN_SPACEING;

	delta = kdata->close_item.price - mvc->data_control.value_min;
	dy = delta;
    dy *=  (draw->height_main-2*MAIN_SPACEING);
    dy /=  (mvc->data_control.value_max - mvc->data_control.value_min);
    y_close =  draw->height_main - dy-MAIN_SPACEING;

	delta = kdata->max_item.price - mvc->data_control.value_min;
	dy = delta;
    dy *=  (draw->height_main-2*MAIN_SPACEING);
    dy /=  (mvc->data_control.value_max - mvc->data_control.value_min);
    y_max =  draw->height_main - dy-MAIN_SPACEING;
	delta = kdata->min_item.price - mvc->data_control.value_min;
	dy = delta;
    dy *= (draw->height_main-2*MAIN_SPACEING);
    dy /=  (mvc->data_control.value_max - mvc->data_control.value_min);
    y_min =  draw->height_main - dy-MAIN_SPACEING;

	//首先是矩形框
    cairo_rectangle(draw->cr, (x >> draw->zoom_out), y_open, ((mvc->data_control.width_unit << draw->zoom_in) - SPACEING1) >> draw->zoom_out, y_close - y_open);
	cairo_fill(draw->cr);

	//下影线
	if (y_min != y_close)
	{
        cairo_move_to(draw->cr, (x + (mvc->data_control.width_unit << draw->zoom_in) / 2) >> draw->zoom_out, y_close);
        cairo_line_to(draw->cr, (x + (mvc->data_control.width_unit << draw->zoom_in) / 2) >> draw->zoom_out, y_min);
		cairo_stroke(draw->cr);
	}
	//上影线
	if (y_max != y_open)
	{
        cairo_move_to(draw->cr, (x + (mvc->data_control.width_unit << draw->zoom_in) / 2) >> draw->zoom_out, y_open);
        cairo_line_to(draw->cr, (x + (mvc->data_control.width_unit << draw->zoom_in) / 2) >> draw->zoom_out, y_max);
	}

	cairo_stroke(draw->cr);
}

void draw_kline_graph(DiagramView_Control *draw, MarketMVC * mvc, DiagramDataItem * node_k);
void draw_macd_graph(DiagramView_Control *draw, MarketMVC * mvc, DiagramDataItem * node_k, int diff_max, int bar_max,double);

void draw_kline_data(DiagramView_Control *draw, MarketMVC * mvc)
{
	DiagramDataItem * node_k;
    node_k = (DiagramDataItem *) mvc->handler->PeekData()->At(mvc->data_control.offset[mvc->_data_type-1], 0);

	draw_kline_graph(draw, mvc, node_k);
    draw_now_price(draw,mvc);

}
void draw_now_price(DiagramView_Control *draw, MarketMVC * mvc)
{
    DiagramDataItem * node_k;
    node_k = (DiagramDataItem *) mvc->handler->PeekData()->Tail(10);
    if(!node_k)
        return;
    MarketAnalyseKline * kdata = (MarketAnalyseKline*) node_k->pri->c_str();
    int delta;
    double dy = 0;
    double y_open;
    delta = kdata->close_item.price - mvc->data_control.value_min;
    dy = delta;
    dy *= (double) (draw->height_main - MAIN_SPACEING*2);
    dy /= (double) (mvc->data_control.value_max - mvc->data_control.value_min);
    y_open = (double) draw->height_main - dy - MAIN_SPACEING;

    cairo_move_to(draw->cr,draw->width_main-10,y_open);
    cairo_line_to(draw->cr,draw->width_main,y_open);
    cairo_stroke(draw->cr);
}

void draw_kline_scale(DiagramView_Control *draw, MarketMVC * mvc)
{
    cairo_set_line_width(draw->cr, 0.4);
    double dashed1[] = { 2.0, 2.0 };
    cairo_set_dash(draw->cr, dashed1, 2, 1);
    double h = (double)(draw->height_main-2*MAIN_SPACEING) /10;
    cairo_set_font_size(draw->cr_scale, g_draw_font.tip.Size);
    cairo_set_source_rgba(draw->cr, positively_color_a.red, positively_color_a.green, positively_color_a.blue, positively_color_a.alpha);
    cairo_set_source_rgba(draw->cr_scale, positively_color_a.red, positively_color_a.green, positively_color_a.blue, positively_color_a.alpha);
    cairo_text_extents_t text;
//    cairo_text_extents(draw->cr_scale,"012345678",&text);
//    double i = text.width;
    for(int i = 10;i>=0;i--)
    {
        cairo_move_to(draw->cr,0,h*(10-i)+MAIN_SPACEING);
        long tick_price = mvc->info->price_tick;

        double price = h*i;
        price /= (draw->height_main-2*MAIN_SPACEING);

        price *= (mvc->data_control.value_max -mvc->data_control.value_min);
        price += mvc->data_control.value_min;

        long long p = price;
        {
            unsigned long t = p % tick_price;
            p -= t;
        }

        E15_String tempstr;
        if(mvc->info->price_tick>=10000)
            price_to_str(&tempstr,(int)price,0);
        else if(mvc->info->price_tick>=1000)
            price_to_str(&tempstr,(int)price,1);
        else if(mvc->info->price_tick>=100)
                price_to_str(&tempstr,(int)price,2);
        else
            price_to_str(&tempstr,(int)price,3);
        cairo_text_extents(draw->cr_scale,tempstr.c_str(),&text);
        cairo_line_to(draw->cr,draw->width_main,h*(10-i)+MAIN_SPACEING);
        cairo_move_to(draw->cr_scale,(SCALE_WITH - text.width-2),h*(10-i)+(g_draw_font.tip.Size-1)/2 +MAIN_SPACEING);
        cairo_show_text(draw->cr_scale,tempstr.c_str());
    }
    cairo_stroke(draw->cr);
    cairo_stroke(draw->cr_scale);
}
DrawRGBA twist_color_a = { 1.0, 1.0, 0.0, 1.0 };
DrawRGBA twist_color_b = { 1.0, 1.0, 0.0, 0.4 };

void draw_k_twist(DiagramView_Control *draw, MarketMVC * mvc, DiagramDataItem * node, int x)
{ //合并后的

    return;
//    if(node->base._type < 0)
//        return;
//    double y_min;
//    double y_max;

//	double dy = 0;
//	int delta;

//	MarketAnalyseKline * kdata = (MarketAnalyseKline*) node->pri->c_str();
//	//计算最大值和最小值

////    if (mvc->draw_helper->draw_config.mode && node->base._type > 0)
////        cairo_set_source_rgba(draw->cr, twist_color_a.red, twist_color_a.green, twist_color_a.blue, twist_color_a.alpha);
////    else
////        cairo_set_source_rgba(draw->cr, twist_color_b.red, twist_color_b.green, twist_color_b.blue, twist_color_b.alpha);

//	delta = kdata->twist.price_max - mvc->data_control.value_min;
//	dy = delta;
//    dy *= (double) (draw->height_main-MAIN_SPACEING*2);
//    dy /= (double) (mvc->data_control.value_max - mvc->data_control.value_min);
//    y_max = (double) draw->height_main - dy-MAIN_SPACEING*2;

//	delta = kdata->twist.price_min - mvc->data_control.value_min;
//	dy = delta;
//    dy *= (double) (draw->height_main-MAIN_SPACEING*2);
//    dy /= (double) (mvc->data_control.value_max - mvc->data_control.value_min);
//    y_min = (double) draw->height_main - dy-MAIN_SPACEING*2;

//	double dashed1[] = { 4.0, 4.0 };

////    if (node->base._type < 0)
////        cairo_set_dash(draw->cr, dashed1, 2, 1);

////	cairo_rectangle(draw->cr, x >> draw->zoom_out, y_max,
////			((mvc->data_control.width_unit << draw->zoom_in) - 1)
////					>> draw->zoom_out, y_min - y_max);

//    if (kdata->close_item.price == kdata->open_item.price)
//    {
//        //平白线
////        draw_k_star(draw, mvc, node_k, x);
//        cairo_set_source_rgba(draw->cr, star_color.red, star_color.green, star_color.blue, star_color.alpha);
//    }
//    else if (kdata->close_item.price > kdata->open_item.price)
//    { //阳线
////        draw_k_positively(draw, mvc, node_k, x);
//        cairo_set_source_rgba(draw->cr, positively_color.red, positively_color.green, positively_color.blue, positively_color.alpha);
//    }
//    else
//    { //阴线
////        draw_k_negative(draw, mvc, node_k, x);
//        cairo_set_source_rgba(draw->cr, negative_color.red, negative_color.green, negative_color.blue, negative_color.alpha);
//    }

//    cairo_rectangle(draw->cr, x >> draw->zoom_out, y_max+MAIN_SPACEING, ((mvc->data_control.width_unit << draw->zoom_in) - 1) >> draw->zoom_out, y_min - y_max);
//    cairo_fill(draw->cr);
//	cairo_stroke(draw->cr);
//	cairo_set_dash(draw->cr, dashed1, 0, 0);
}

void draw_kline_graph(DiagramView_Control *draw, MarketMVC * mvc, DiagramDataItem * node_k)
{
	int x = 0;

    cairo_set_line_width(draw->cr, 1);

	MarketAnalyseKline * kdata;
	while (node_k)
	{
		kdata = (MarketAnalyseKline*) node_k->pri->c_str();

        if (mvc->draw_helper->draw_config.mode && node_k->base._type != 0)
        {
            //绘制合并后的K线
            draw_k_twist(draw, mvc, node_k, x);
        }
        else if (kdata->close_item.price == kdata->open_item.price)
        {
            //平白线
            draw_k_star(draw, mvc, node_k, x);
        }
        else if (kdata->close_item.price > kdata->open_item.price)
        { //阳线
            draw_k_positively(draw, mvc, node_k, x);
        }
        else
        { //阴线
            draw_k_negative(draw, mvc, node_k, x);
        }

        if(node_k->m_buys)
        {
            draw_buy(1,draw,mvc,node_k,x);
        }

		node_k = (DiagramDataItem *) node_k->Next();
		if (!node_k)
			break;
        x += ((mvc->data_control.width_unit << draw->zoom_in) + SPACEING);
        if ((x >> draw->zoom_out) > draw->width_main)
            break;

	}
    if(mvc->data_control.is_line_drawing)
    {
        cairo_move_to(draw->cr,draw->begin_mouse_x,draw->begin_mouse_y);
        cairo_line_to(draw->cr,draw->mouse_x,draw->mouse_y-14);
        cairo_stroke(draw->cr);
    }
}

void draw_kline_macd(DiagramView_Control *draw, MarketMVC * mvc, int diff_max, int bar_max,double height_macd)
{
	DiagramDataItem * node_k;
    node_k = (DiagramDataItem *) mvc->handler->PeekData()->At(mvc->data_control.offset[mvc->_data_type-1], 0);
	if (!node_k)
		return;
    draw_macd_graph(draw, mvc, node_k, diff_max, bar_max,height_macd);
}

void draw_macd_graph(DiagramView_Control *draw, MarketMVC * mvc, DiagramDataItem * node_k, int diff_max, int bar_max,double height_macd)
{
    int x = 0;
    //diff_max += 5;
    cairo_set_line_width(draw->cr, 0.8);

    DiagramDataItem * node_k_raw = node_k;

    DrawRGBA color = { 0.0, 0.0, 0.0, 1.0 };

    //首先0轴
    cairo_set_line_width(draw->cr, 0.6);
    double dashed1[] = { 1.0, 1.0 };

    cairo_set_dash(draw->cr, dashed1, 2, 1);
    color.red = 1.0;
    color.green = 0.0;
    color.blue = 0.0;
    color.alpha = 0.5;

    cairo_set_source_rgba(draw->cr, color.red, color.green, color.blue, color.alpha);

//	int h = draw->height_sub >> 1;

    int height = height_macd/2;

    cairo_move_to(draw->cr, 0, height);
    cairo_line_to(draw->cr, draw->width, height);
    cairo_stroke(draw->cr);

    cairo_set_dash(draw->cr, dashed1, 0, 0);

    color.alpha = 1.0;
    //diff
    cairo_set_line_width(draw->cr, 0.6);
    x = 0;

    float diff;

    MarketAnalyseKline * kdata;

    //bar
    x = 0;
    node_k = node_k_raw;

    cairo_set_line_width(draw->cr, 1.0);
    while (node_k)
    {
        kdata = (MarketAnalyseKline*) node_k->pri->c_str();
        diff = kdata->bar;

        diff *= height;
        diff /= bar_max;

        if (diff > 0)
        {
            cairo_set_source_rgba(draw->cr, positively_color.red, positively_color.green, positively_color.blue, positively_color.alpha);
        }
        else
        {
            cairo_set_source_rgba(draw->cr, negative_color.red, negative_color.green, negative_color.blue, negative_color.alpha);
        }
        cairo_move_to(draw->cr, (x + (mvc->data_control.width_unit << draw->zoom_in) / 2) >> draw->zoom_out, height);
        cairo_line_to(draw->cr, (x + (mvc->data_control.width_unit << draw->zoom_in) / 2) >> draw->zoom_out, height - diff);
        cairo_stroke(draw->cr);
        node_k = (DiagramDataItem *) node_k->Next();
        if (!node_k)
            break;
        x += ((mvc->data_control.width_unit << draw->zoom_in)+SPACEING);
        if ((x >> draw->zoom_out) > draw->width_main)
            break;
    }
    cairo_stroke(draw->cr);

    color.red = 1.0;
    color.green = 1.0;
    color.blue = 1.0;
    cairo_set_source_rgba(draw->cr, color.red, color.green, color.blue, color.alpha);

    x = 0;
    node_k = node_k_raw;
    while (node_k)
    {
        kdata = (MarketAnalyseKline*) node_k->pri->c_str();
        diff = kdata->diff;

        diff *= height;
        diff /= diff_max;

        cairo_line_to(draw->cr, (x + (mvc->data_control.width_unit << draw->zoom_in) / 2) >> draw->zoom_out, height - diff);
        node_k = (DiagramDataItem *) node_k->Next();
        if (!node_k)
            break;
        x += ((mvc->data_control.width_unit << draw->zoom_in)+SPACEING);
        if ((x >> draw->zoom_out) > draw->width_main)
            break;
    }
    cairo_stroke(draw->cr);
    //dea

    color.red = 1.0;
    color.green = 1.0;
    color.blue = 0.0;

    cairo_set_source_rgba(draw->cr, color.red, color.green, color.blue, color.alpha);

    x = 0;
    node_k = node_k_raw;
    while (node_k)
    {
        kdata = (MarketAnalyseKline*) node_k->pri->c_str();
        diff = kdata->dea;
        diff *= height;
        diff /= diff_max;
        cairo_line_to(draw->cr, (x + (mvc->data_control.width_unit << draw->zoom_in) / 2) >> draw->zoom_out, height - diff);
        node_k = (DiagramDataItem *) node_k->Next();
        if (!node_k)
            break;
        x += ((mvc->data_control.width_unit << draw->zoom_in)+SPACEING);
        if ((x >> draw->zoom_out) > draw->width_main)
            break;
    }
    cairo_stroke(draw->cr);
}

void draw_kline_volume_graph(DiagramView_Control *view, MarketMVC * mvc, DiagramDataItem * node_k, unsigned long vol_max,double height_volume)
{
	int x = 0;
	cairo_set_line_width(view->cr, 0.8);
    DiagramDataItem * node_k_raw = node_k;
	DiagramDataItem * pre = (DiagramDataItem *) node_k->Pre();
	if (!pre)
		pre = node_k;

	//首先0轴
	cairo_set_line_width(view->cr, 0.6);
	float h = 0;

	MarketAnalyseKline * kdata;
    int begin_time = node_k->base._time;
    int begin_date = node_k->base._date;
    int end_time = begin_time;
    int end_date = begin_date;
	while (node_k)
	{
		kdata = (MarketAnalyseKline*) node_k->pri->c_str();
		h = node_k->base._volume_tick;
        end_time = node_k->base._time;
        end_date = node_k->base._date;
		h /= vol_max;
        h *= height_volume - RESERVE_PIXEL;

		if (kdata->close_item.price >= kdata->open_item.price)
		{
			cairo_set_source_rgba(view->cr, positively_color.red, positively_color.green, positively_color.blue, positively_color.alpha);
		}
		else
		{
			cairo_set_source_rgba(view->cr, negative_color.red, negative_color.green, negative_color.blue, negative_color.alpha);
		}
        cairo_rectangle(view->cr, (x >> view->zoom_out), height_volume - RESERVE_PIXEL, ((mvc->data_control.width_unit << view->zoom_in) - 1) >> view->zoom_out, -h);
		cairo_stroke(view->cr);
		pre = node_k;
		node_k = (DiagramDataItem *) node_k->Next();
		if (!node_k)
			break;

        x += ((mvc->data_control.width_unit << view->zoom_in)+SPACEING);
        if ((x >> view->zoom_out) > view->width_main)
			break;

	}
    cairo_stroke(view->cr);
    if(mvc->data_control.OpenInterest_max==0)
        return;
    cairo_set_line_width(view->cr, 0.8);
    cairo_set_source_rgba(view->cr, 1, 1, 1, 1);
//    int height = view->height_sub;
    int height = height_volume - RESERVE_PIXEL;
    double diff  =0;
    x = 0;
    node_k = node_k_raw;
    cairo_set_line_width(view->cr_all, 0.4);
    cairo_set_source_rgba(view->cr_all,0.3,0,0,0.4);
    double dashed1[] = { 2.0, 2.0 };
    cairo_set_dash(view->cr_all, dashed1, 2, 1);
    while(node_k)
    {
        diff = node_k->base.OpenInterest -mvc->data_control.OpenInterest_min ;
        diff *= height;
        diff /= mvc->data_control.OpenInterest_max - mvc->data_control.OpenInterest_min;
        cairo_line_to(view->cr, (x + (mvc->data_control.width_unit << view->zoom_in) / 2) >> view->zoom_out, height - diff);
        node_k = (DiagramDataItem *) node_k->Next();
        if (!node_k)
            break;
        x += ((mvc->data_control.width_unit << view->zoom_in)+SPACEING);
        if((x >> view->zoom_out) > view->width_main)
            break;
    }
    cairo_stroke(view->cr);
}
void draw_time(int begin_date,int begin_time,int x1,DiagramView_Control* view)
{
    int x =(x1 + (5 << view->zoom_in) / 2) >> view->zoom_out;
    int d1 = begin_date%10000;
    int t1 = begin_time/1000;

    E15_String d2;
    d2.Sprintf("%04d",d1);
    d2.Insert(2,'-');
    E15_String t2 ;
    t2.Sprintf("%06d",t1);
    t2.Insert(2,':');
    t2.Insert(5,':');
    d2.Append("/");
    d2.Append(t2.c_str());
    cairo_select_font_face(view->cr_time, "文泉驿等宽正黑", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_source_rgba(view->cr_time, g_draw_font.tip.Color.red,g_draw_font.tip.Color.green,g_draw_font.tip.Color.blue,g_draw_font.tip.Color.alpha);

    cairo_set_font_size(view->cr_time, 7);
    cairo_move_to(view->cr_time, x, 7);
    cairo_show_text(view->cr_time, d2.c_str());



    cairo_move_to(view->cr_macd,x, 0);
    cairo_line_to(view->cr_macd,x,view->height_macd);
    cairo_move_to(view->cr_main,x, 0);
    cairo_line_to(view->cr_main,x,view->height_main);
    cairo_move_to(view->cr_volume,x, 0);
    cairo_line_to(view->cr_volume,x,view->height_volume);
    cairo_move_to(view->cr_time,x, 0);
    cairo_line_to(view->cr_time,x,view->height_time);
    cairo_stroke(view->cr_time);
    cairo_stroke(view->cr_macd);
    cairo_stroke(view->cr_main);
    cairo_stroke(view->cr_volume);
}

void draw_kline_volume(DiagramView_Control *draw, MarketMVC * mvc, unsigned long vol_max,double height_volume)
{
	DiagramDataItem * node_k;
    node_k = (DiagramDataItem *) mvc->handler->PeekData()->At(mvc->data_control.offset[mvc->_data_type-1], 0);
	if (!node_k)
		return;
    draw_kline_volume_graph(draw, mvc, node_k, vol_max,height_volume);

}

int kline_on_key(DiagramView_Control *draw, MarketMVC * mvc, int key)
{
	mvc->draw_helper->draw_config.mode = !mvc->draw_helper->draw_config.mode;
	return 1;
}
