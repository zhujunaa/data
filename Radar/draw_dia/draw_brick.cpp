#include"draw_dia.h"
#include"E15_value.h"
void draw_brick_data(DiagramView_Control *draw, MarketMVC * mvc);
void price_to_str(E15_String * str, long price ,int decimal);
void draw_buy(int, DiagramView_Control *view, MarketMVC * mvc, DiagramDataItem * node, int x);
void draw_brick_cci(DiagramView_Control *draw, MarketMVC * mvc,double height_volume)
{
    DiagramDataItem * node_k ,*node;
    node_k = (DiagramDataItem *) mvc->handler->PeekData()->At(mvc->data_control.offset[mvc->_data_type-1], 0);
    if (!node_k)
        return;
    node = node_k;
    int x = 0;
    int height = height_volume - RESERVE_PIXEL;
    double cci  =0;
    cairo_set_line_width(draw->cr, 0.8);
    cairo_set_source_rgba(draw->cr, 0.1,0.2,1,0.5);
    double dashed1[] = { 2.0, 2.0 };
    cairo_set_dash(draw->cr, dashed1, 2, 1);
    cci = 100 -mvc->data_control.cci_min ;
    cci *= height;
    cci /= mvc->data_control.cci_max - mvc->data_control.cci_min;
    cairo_move_to(draw->cr,0, height - cci);
    cairo_line_to(draw->cr,draw->width_main+draw->width_scale-20, height - cci);
    cairo_show_text(draw->cr, "100");
    cci = -100 -mvc->data_control.cci_min ;
    cci *= height;
    cci /= mvc->data_control.cci_max - mvc->data_control.cci_min;
    cairo_move_to(draw->cr,0, height - cci);
    cairo_line_to(draw->cr,draw->width_main+draw->width_scale-20, height - cci);
    cairo_show_text(draw->cr, "-100");
    cairo_stroke(draw->cr);

    double dashed2[] = { 2.0, 0 };
    cairo_set_dash(draw->cr, dashed2, 2, 1);
    cairo_set_line_width(draw->cr, 0.8);
    cairo_set_source_rgba(draw->cr, 1,0,0,1);
    MarketAnalyseBrick * kdata;
    DiagramTag * tag = 0;
    while (node_k)
    {
        break;
        tag = node_k->PeekTag(1);
        if (!tag)
            break;
        if (tag->base._date == 0)
            break;
        cci = tag->base._value - mvc->data_control.cci_min;
        kdata = (MarketAnalyseBrick*) node_k->pri->c_str();
        cci = kdata->cci -mvc->data_control.cci_min ;
        cci *= height;
        cci /= mvc->data_control.cci_max - mvc->data_control.cci_min;
        cairo_line_to(draw->cr, (x + (mvc->data_control.width_unit << draw->zoom_in) / 2) >> draw->zoom_out, height - cci);
        node_k = (DiagramDataItem *) node_k->Next();
        if (!node_k)
            break;
        x += ((mvc->data_control.width_unit << draw->zoom_in)+SPACEING);
        if((x >> draw->zoom_out) > draw->width_main)
            break;
    }
    cairo_stroke(draw->cr);
    x = 0;
    node_k = node;
    cairo_set_source_rgba(draw->cr, 1,1,0,1);
    while (node_k)
    {
        kdata = (MarketAnalyseBrick*) node_k->pri->c_str();
        cci = kdata->cciAverage -mvc->data_control.cci_min ;
        cci *= height;
        cci /= mvc->data_control.cci_max - mvc->data_control.cci_min;
        cairo_line_to(draw->cr, (x + (mvc->data_control.width_unit << draw->zoom_in) / 2) >> draw->zoom_out, height - cci);
        node_k = (DiagramDataItem *) node_k->Next();
        if (!node_k)
            break;
        x += ((mvc->data_control.width_unit << draw->zoom_in)+SPACEING);
        if((x >> draw->zoom_out) > draw->width_main)
            break;
    }
    cairo_stroke(draw->cr);
}

void draw_brick_volume(DiagramView_Control *view, MarketMVC * mvc, unsigned long vol_max,double height_volume)
{
    DiagramDataItem * node_k;
    node_k = (DiagramDataItem *) mvc->handler->PeekData()->At(mvc->data_control.offset[mvc->_data_type-1], 0);
    if (!node_k)
        return;
    int x = 0;
    cairo_set_line_width(view->cr, 0.8);
    DiagramDataItem * node_k_raw = node_k;
    DiagramDataItem * pre = (DiagramDataItem *) node_k->Pre();
    if (!pre)
        pre = node_k;

    //首先0轴
    cairo_set_line_width(view->cr, 0.6);

    float h = 0;

    MarketAnalyseBrick * kdata;
    int begin_time = node_k->base._time;
    int begin_date = node_k->base._date;
    int end_time = begin_time;
    int end_date = begin_date;
    while (node_k)
    {
        kdata = (MarketAnalyseBrick*) node_k->pri->c_str();
        h = node_k->base._volume_tick;
        end_time = node_k->base._time;
        end_date = node_k->base._date;
        h /= vol_max;
        h *= height_volume - RESERVE_PIXEL;

        if ( node_k->base._type==1)
        {
            cairo_set_source_rgba(view->cr, 1,0,0,1);
        }
        else
        {
            cairo_set_source_rgba(view->cr, 0,1,0,1);
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
void draw_brick_macd(DiagramView_Control *draw, MarketMVC * mvc, int diff_max, int bar_max,double height_macd)
{
    DiagramDataItem * node_k;
    node_k = (DiagramDataItem *) mvc->handler->PeekData()->At(mvc->data_control.offset[mvc->_data_type-1], 0);
    if (!node_k)
        return;
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

    int h = height_macd/2;

    cairo_move_to(draw->cr, 0, h);
    cairo_line_to(draw->cr, draw->width, h);
    cairo_stroke(draw->cr);
    cairo_set_dash(draw->cr, dashed1, 0, 0);
    color.alpha = 1.0;
    //diff
    cairo_set_line_width(draw->cr, 0.6);
    x = 0;

    float diff;

    MarketAnalyseBrick * kdata;

    //bar
    x = 0;
    node_k = node_k_raw;

    cairo_set_line_width(draw->cr, 1.0);
    while (node_k)
    {
        kdata = (MarketAnalyseBrick*) node_k->pri->c_str();
        diff = kdata->bar;

        diff *= h;
        diff /= bar_max;

        if (diff > 0)
        {
            cairo_set_source_rgba(draw->cr, 1,0,0,1);
        }
        else
        {
            cairo_set_source_rgba(draw->cr, 0,1,0,1);
        }
        cairo_move_to(draw->cr, (x + (mvc->data_control.width_unit << draw->zoom_in) / 2) >> draw->zoom_out, h);
        cairo_line_to(draw->cr, (x + (mvc->data_control.width_unit << draw->zoom_in) / 2) >> draw->zoom_out, h - diff);
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
        kdata = (MarketAnalyseBrick*) node_k->pri->c_str();
        diff = kdata->diff;

        diff *= h;
        diff /= diff_max;

        cairo_line_to(draw->cr, (x + (mvc->data_control.width_unit << draw->zoom_in) / 2) >> draw->zoom_out, h - diff);
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
        kdata = (MarketAnalyseBrick*) node_k->pri->c_str();
        diff = kdata->dea;
        diff *= h;
        diff /= diff_max;

        cairo_line_to(draw->cr, (x + (mvc->data_control.width_unit << draw->zoom_in) / 2) >> draw->zoom_out, h - diff);
        node_k = (DiagramDataItem *) node_k->Next();
        if (!node_k)
            break;
        x += ((mvc->data_control.width_unit << draw->zoom_in)+SPACEING);
        if ((x >> draw->zoom_out) > draw->width_main)
            break;
    }
    cairo_stroke(draw->cr);
}
void draw_brick_scale(DiagramView_Control *draw, MarketMVC * mvc)
{
    cairo_set_line_width(draw->cr, 0.4);
    double dashed1[] = { 2.0, 2.0 };
    cairo_set_dash(draw->cr, dashed1, 2, 1);
    double h = (double)(draw->height_main-2*MAIN_SPACEING) /10;
    cairo_set_font_size(draw->cr_scale, g_draw_font.tip.Size);
    cairo_set_source_rgba(draw->cr, 1,0,0,1);
    cairo_set_source_rgba(draw->cr_scale, 0,1,0,1);
    cairo_text_extents_t text;
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

void draw_callback_brick(DiagramView_Control *draw, MarketMVC * mvc)
{
    //计算最大值最小值范围
    DiagramDataItem * node;
    node = (DiagramDataItem *) mvc->handler->PeekDataItem(mvc->data_control.offset[mvc->_data_type-1]);
    if (!node||!node->pri)
        return;
        MarketAnalyseBrick * kdata = (MarketAnalyseBrick*) node->pri->c_str();
        mvc->data_control.value_max = kdata->max_item.price ;
        mvc->data_control.value_min = kdata->min_item.price;
        int diff_max = kdata->diff;
        int bar_max = kdata->bar;
        int dea_max  = kdata->dea;
        long long volume_max = node->base._volume_tick + 1;
        long long OpenInterest_max = node->base.OpenInterest;
        long long OpenInterest_min = OpenInterest_max;
        __int64 cci_max = kdata->cci+1;
        __int64 cci_min = kdata->cci;
        int c = 1;
        node = (DiagramDataItem *) node->Next();
        while (node)
        {
            kdata = (MarketAnalyseBrick*) node->pri->c_str();
            if(mvc->data_control.value_max<kdata->max_item.price)
                mvc->data_control.value_max = kdata->max_item.price;
            if(mvc->data_control.value_min>kdata->min_item.price)
                mvc->data_control.value_min=kdata->min_item.price;
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
            if(cci_max<kdata->cci)
                cci_max = kdata->cci;
            if(cci_min>kdata->cci)
                cci_min = kdata->cci;
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
        MarketDataType * t = mvc->handler->GetDataType();
        mvc->data_control.bar_max = bar_max;
        mvc->data_control.diff_max = diff_max;
        mvc->data_control.dea_max = dea_max;
        mvc->data_control.volume_max = volume_max;
        mvc->data_control.OpenInterest_max = OpenInterest_max;
        mvc->data_control.OpenInterest_min = OpenInterest_min;
        mvc->data_control.cci_max = cci_max;
        mvc->data_control.cci_min = cci_min;

        draw->cr = draw->cr_main;
        if(draw->zoom_in_out)
        {
            cairo_new_path(draw->cr_scale);
            draw_brick_scale(draw,mvc);
//            draw_time_scale(draw,mvc,draw->cr_main);
        }
        double dashed2[] = { 1, 0 };
        cairo_set_dash(draw->cr_main, dashed2, 2, 1);
        draw->cr = draw->cr_main;
        draw_brick_data(draw,mvc);
        //附属图 1
        cairo_set_dash(draw->cr_volume, dashed2, 2, 1);
        draw->cr = draw->cr_volume;
        cairo_new_path(draw->cr);
        if(draw->volume_type==0)
        {
            draw_brick_volume(draw, mvc, volume_max,draw->height_volume);
        }
        else if(draw->volume_type==1)
        {
            draw_brick_macd(draw, mvc, diff_max, bar_max,draw->height_volume);
        }
        else
        {
            draw_brick_cci(draw, mvc, draw->height_volume);
        }
        //附属图 2
        draw->cr = draw->cr_macd;
        cairo_set_dash(draw->cr, dashed2, 2, 1);
        cairo_new_path(draw->cr);
        if(draw->macd_type==0)
        {
            draw_brick_volume(draw, mvc, volume_max,draw->height_macd);
        }
        else if(draw->macd_type==1)
        {
            draw_brick_macd(draw, mvc, diff_max, bar_max,draw->height_macd);
        }
        else
        {
            draw_brick_cci(draw, mvc,draw->height_macd);
        }

}

void draw_b_negative(DiagramView_Control *draw, MarketMVC * mvc, MarketAnalyseBrick * kdata ,int x)
{
    //阴线
    cairo_set_source_rgba(draw->cr, 0,1,0,1);
    double y_min =0;
    double y_max = 0;
    double y_currunt = 0;
    double dy = 0;
    int delta;
    delta = kdata->max_item.price - mvc->data_control.value_min;
    dy = delta;
    dy *= (double) (draw->height_main - MAIN_SPACEING*2);
    dy /= (double) (mvc->data_control.value_max - mvc->data_control.value_min);
    y_max = (double) draw->height_main - dy - MAIN_SPACEING;

    delta = kdata->currunt_item.price - mvc->data_control.value_min;
    dy = delta;
    dy *= (double) (draw->height_main - MAIN_SPACEING*2);
    dy /= (double) (mvc->data_control.value_max - mvc->data_control.value_min);
    y_currunt = (double) draw->height_main - dy - MAIN_SPACEING;

    delta =kdata->min_item.price - mvc->data_control.value_min;
    dy = delta;
    dy *= (double) (draw->height_main- MAIN_SPACEING*2);
    dy /= (double) (mvc->data_control.value_max - mvc->data_control.value_min);
    y_min = (double) draw->height_main - dy - MAIN_SPACEING;

    cairo_rectangle(draw->cr, x >> draw->zoom_out, y_max,
                    ((mvc->data_control.width_unit << draw->zoom_in) - SPACEING1) >> draw->zoom_out,(y_currunt-y_max));
    cairo_fill(draw->cr);
    cairo_rectangle(draw->cr, x >> draw->zoom_out, y_max,
                    ((mvc->data_control.width_unit << draw->zoom_in) - SPACEING1) >> draw->zoom_out,(y_min-y_max));

    cairo_stroke(draw->cr);
}
void draw_b_positively(DiagramView_Control *draw, MarketMVC * mvc, MarketAnalyseBrick * kdata, int x)
{//阳线
    cairo_set_source_rgba(draw->cr, 1,0,0,1);
    double y_min = 0;
    double y_currunt;
    double y_max = 0;
    double dy = 0;
    int delta;
    delta = kdata->max_item.price- mvc->data_control.value_min;
    dy = delta;
    dy *= (double) (draw->height_main - MAIN_SPACEING*2);
    dy /= (double) (mvc->data_control.value_max - mvc->data_control.value_min);
    y_max = (double) draw->height_main - dy - MAIN_SPACEING;

    delta = kdata->currunt_item.price - mvc->data_control.value_min;
    dy = delta;
    dy *= (double) (draw->height_main- MAIN_SPACEING*2);
    dy /= (double) (mvc->data_control.value_max - mvc->data_control.value_min);
    y_currunt = (double) draw->height_main - dy - MAIN_SPACEING;

    delta = kdata->min_item.price - mvc->data_control.value_min;
    dy = delta;
    dy *= (double) (draw->height_main- MAIN_SPACEING*2);
    dy /= (double) (mvc->data_control.value_max - mvc->data_control.value_min);
    y_min = (double) draw->height_main - dy - MAIN_SPACEING;

        cairo_rectangle(draw->cr, x >> draw->zoom_out, y_currunt, ((mvc->data_control.width_unit << draw->zoom_in) - SPACEING1) >> draw->zoom_out,(y_min-y_currunt));
        cairo_fill(draw->cr);
    cairo_rectangle(draw->cr, x >> draw->zoom_out, y_max, ((mvc->data_control.width_unit << draw->zoom_in) - SPACEING1) >> draw->zoom_out,(y_min-y_max));

    cairo_stroke(draw->cr);
}

void brick_draw_focus_info(DiagramView_Control *draw, MarketMVC * mvc)
{
    DiagramDataItem * node;
    node = (DiagramDataItem *) mvc->handler->PeekData()->At(mvc->data_control.focus, 0);
    if (!node)
        return;
    if (!draw->mouse_in)
        return;
    E15_String ss;
    double xx = 0;
    double yy = 0;
    DiagramTag * tag = node->PeekTag(1);
    if(!tag)
        return ;
    ss.Append("%d seq:%d cci%d ",node->base._date,node->base._seq,tag->base._value);
    cairo_get_current_point(draw->cr,&xx,&yy);
    cairo_move_to(draw->cr, xx, g_draw_font.tip.Size);
    cairo_set_source_rgba(draw->cr,1,1,1,1);
    cairo_show_text(draw->cr, ss.c_str());
}

void draw_brick_data(DiagramView_Control *draw, MarketMVC * mvc)
{
    DiagramDataItem * node_b;
    node_b = (DiagramDataItem *) mvc->handler->PeekData()->At(mvc->data_control.offset[mvc->_data_type-1], 0);
     cairo_set_line_width(draw->cr, 1);
     int x = 0;
     MarketAnalyseBrick * kdata;
     while (node_b)
     {
         kdata = (MarketAnalyseBrick*) node_b->pri->c_str();
        if(node_b->base._type==-1)
        {
            draw_b_negative(draw, mvc, kdata, x);
        }
        else
        {
            draw_b_positively(draw, mvc, kdata, x);
        }
        if(node_b->m_buys)
        {
            draw_buy(0,draw,mvc,node_b,x);
        }
         x += ((mvc->data_control.width_unit << draw->zoom_in) + SPACEING);
         if ((x >> draw->zoom_out) > draw->width_main)
             break;
         node_b = (DiagramDataItem *) node_b->Next();
         if (!node_b)
             break;
    }
}

int brick_on_key(DiagramView_Control *draw, MarketMVC * mvc, int key)
{
    return 1;
}
