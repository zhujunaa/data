#include <string.h>

#include "E15_queue.h"
#include "E15_value.h"
#include "E15_debug.h"
#include "E15_ini.h"

#include "draw_dia.h"

#include "data_mgr.h"
extern double clarity;
static E15_Queue g_draw_helper_list;
extern int g_one_key_cycle;//一键多周期
extern ALL_ID_LIST g_id_list_info;
void draw_tag(int index, DiagramView_Control *view, MarketMVC * mvc, DrawHelper_Config * draw_config);
void draw_boll(int type_index, DiagramView_Control *view, MarketMVC * mvc, DrawHelper_Config * draw_config);
void draw_tip_background(DiagramView_Control *draw, MarketMVC * mvc);
void draw_tip_info(DiagramView_Control *draw, DiagramDrawCallbackData * cbdata);
extern void Price2Str(E15_String * str, long price);
extern QMap<QString ,Graph_info> g_id_info;//0: 自选1:黑色2:金属3:轻工业品4:5;6:extern bool one_second_out;
extern bool one_second_out;
extern int g_kavg;
extern int g_twist1;
extern int g_twist2;
extern int g_merge;
void draw_mouse_price(cairo_t *cr, DiagramDrawCallbackData * cbdata)
{
	//显示最新价格
	//鼠标所在位置的价格
	if (cbdata->view.mouse_y < cbdata->h_title)
		return ;
	if (cbdata->view.mouse_y > (cbdata->h_title + cbdata->h_main))
		return ;

    if( cbdata->mvc->data_control.value_max <= cbdata->mvc->data_control.value_min)
		return ;

	double adjust = 0;
	long tick_price = cbdata->data->info.price_tick;
	if( tick_price < 100)
		tick_price = 100;


	adjust = (cbdata->mvc->data_control.value_max - cbdata->mvc->data_control.value_min);
	adjust /= tick_price;

    adjust = (cbdata->h_main /2) / adjust;

	double price = cbdata->h_main - (cbdata->view.mouse_y - cbdata->h_title) + adjust;
	price /= cbdata->h_main;

	price *= (cbdata->mvc->data_control.value_max - cbdata->mvc->data_control.value_min);
	price += cbdata->mvc->data_control.value_min;

	unsigned long p = price;

	{
		unsigned long t = p % tick_price;
		p -= t;
	}

	E15_String p_str;
	Price2Str(&p_str, p);

	cairo_select_font_face(cr, "文泉驿等宽正黑", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_source_rgba(cr, g_draw_font.tip.Color.red,g_draw_font.tip.Color.green,g_draw_font.tip.Color.blue,g_draw_font.tip.Color.alpha);

	cairo_set_font_size(cr, g_draw_font.tip.Size);
    int x = cbdata->view.width - g_draw_font.tip.Size * p_str.Length()/2;
	cairo_move_to(cr, x, cbdata->view.mouse_y);
	cairo_show_text(cr, p_str.c_str());
}

void draw_callback_focus(cairo_t *cr,DiagramDrawCallbackData * cbdata)
{
	if (!cbdata->view.mouse_in)
		return ;

    if( !cbdata->view.zoom_in_out )
        return ;

    cbdata->view.cr = cr;
	cairo_select_font_face(cr, "文泉驿等宽正黑", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_source_rgba(cr, g_draw_font.id.Color.red,g_draw_font.id.Color.green,g_draw_font.id.Color.blue,g_draw_font.id.Color.alpha);
	cairo_set_font_size(cr, g_draw_font.tip.Size);

    cairo_move_to(cr, cbdata->m_focus_x+10 +TIP_WIDTH, g_draw_font.tip.Size);

	cbdata->mvc->draw_helper->draw_focus(&cbdata->view, cbdata->mvc);
}

void draw_callback_title(DiagramDrawCallbackData * cbdata)
{
    //显示名称(左上角)
    cairo_select_font_face(cbdata->cr_title, "文泉驿等宽正黑", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);

    if(cbdata->view.has_trade)
    {
        if(one_second_out)
        {
            cairo_set_source_rgba(cbdata->cr_title, 0.0,0.0,1.0,g_draw_font.id.Color.alpha);
        }
        else {
            cairo_set_source_rgba(cbdata->cr_title, g_draw_font.id.Color.red,g_draw_font.id.Color.green,g_draw_font.id.Color.blue,g_draw_font.id.Color.alpha);
        }
    }
    else
    {
        cairo_set_source_rgba(cbdata->cr_title, g_draw_font.id.Color.red,g_draw_font.id.Color.green,g_draw_font.id.Color.blue,g_draw_font.id.Color.alpha);
    }

	cairo_set_font_size(cbdata->cr_title, g_draw_font.id.Size);
	cairo_move_to(cbdata->cr_title, 2, g_draw_font.id.Size);
	cairo_show_text(cbdata->cr_title, cbdata->mvc->info->id); //合约ID
	cairo_show_text(cbdata->cr_title, ":");
	cairo_show_text(cbdata->cr_title, cbdata->mvc->info->name); //合约名称
	cairo_show_text(cbdata->cr_title, ":");

	cbdata->tempstr.Sprintf("zoom(%d)", cbdata->view.zoom_in - cbdata->view.zoom_out); //放大缩小倍数
	cairo_show_text(cbdata->cr_title, cbdata->tempstr.c_str());
    //cairo_surface_write_to_png(cbdata->pixbuf_title,"./title.png"); 存图
	double xx = 0;
	double yy = 0;

	cairo_get_current_point(cbdata->cr_title,&xx,&yy);

	//显示图样名称（右上角）
    MarketDataType * t = cbdata->mvc->handler->GetDataType();
    if(!t)
        return;
    cbdata->tempstr.Sprintf("%s:%s:%ld", t->class_name, t->name, t->param);
    cairo_set_font_size(cbdata->cr_title, g_draw_font.tip.Size);
    int x = cbdata->view.width - TIP_WIDTH-g_draw_font.tip.Size * cbdata->tempstr.Length() / 2;
//    if(g_ce)
//    {
//        switch (cbdata->view.index) {
//        case 0:
//            cairo_set_source_rgba(cbdata->cr_title,1,0,0,1);
//            break;
//        case 1:
//            cairo_set_source_rgba(cbdata->cr_title,1,1,0,1);
//            break;
//        case 2:
//            cairo_set_source_rgba(cbdata->cr_title,1,0,1,1);
//            break;
//        case 3:
//            cairo_set_source_rgba(cbdata->cr_title,0.2,0.5,0.5,1);
//            break;
//        case 4:
//            cairo_set_source_rgba(cbdata->cr_title,0,1,0,1);
//            break;
//        case 5:
//            cairo_set_source_rgba(cbdata->cr_title,0,0,1,1);
//            break;
//        case 6:
//            cairo_set_source_rgba(cbdata->cr_title,0,1,1,1);
//            break;
//        case 7:
//            cairo_set_source_rgba(cbdata->cr_title,0.5,0,0.5,1);
//            break;
//        case 8:
//            cairo_set_source_rgba(cbdata->cr_title,0.5,0,0,1);
//            break;
//        case 9:
//            cairo_set_source_rgba(cbdata->cr_title,0.5,0.5,0,1);
//            break;
//        case 10:
//            cairo_set_source_rgba(cbdata->cr_title,0,0.5,0,1);
//            break;
//        case 11:
//            cairo_set_source_rgba(cbdata->cr_title,0,0,0.5,1);
//            break;
//        default:
//            break;
//        }
//    }

    cairo_move_to(cbdata->cr_title, x, g_draw_font.tip.Size);
	cairo_show_text(cbdata->cr_title, cbdata->tempstr.c_str());
    cbdata->m_focus_x = xx;
    MarketDepthData * depth = cbdata->data->depth;
//    if(cbdata->view.zoom_in_out)
//        return;
    if(depth)
    {
        if(depth->ext.UpperLimitPrice<=depth->base.nPrice)//涨停
        {
            cairo_set_source_rgba(cbdata->cr_title, 1,0,0,1);
            cairo_move_to(cbdata->cr_title, xx, g_draw_font.id.Size);
            cairo_show_text(cbdata->cr_title, "涨停");
        }
        else if(depth->ext.LowerLimitPrice>=depth->base.nPrice)//跌停
        {
            cairo_move_to(cbdata->cr_title, xx, g_draw_font.id.Size );
            cairo_set_source_rgba(cbdata->cr_title, 0,1,0,1);
            cairo_show_text(cbdata->cr_title, "跌停");
        }
    }
//    cairo_move_to(cbdata->cr_title, xx, g_draw_font.id.Size );
//    if(cbdata->mvc->data_control.auto_scroller)
//        cairo_show_text(cbdata->cr_title, "暂停");
//    else
//        cairo_show_text(cbdata->cr_title, "继续");

}




extern E15_Queue g_draw_config_list;
extern QMap<QString ,Graph_info> g_id_info;//图表参数

void draw_callback_datas(DiagramDrawCallbackData * cbdata)
{
//    draw_callback_title(cbdata);

	//显示对应类型的数据
	//自动滚动的位置调整(滚动控制与窗口绑定还是与数据绑定？？？？？？)
	if (cbdata->mvc->data_control.auto_scroller)
    {
        long max_item = ((cbdata->view.width_main << cbdata->view.zoom_out) - 1) / ((cbdata->mvc->data_control.width_unit << cbdata->view.zoom_in)+ SPACEING);
         unsigned long cnt = cbdata->mvc->handler->PeekData()->Count();
        if ((long) cnt > max_item)
		{
            cbdata->mvc->data_control.offset[cbdata->mvc->_data_type-1] = cnt - max_item ;
            cbdata->mvc->data_control.focus = cbdata->mvc->data_control.offset[cbdata->mvc->_data_type-1] + (((cbdata->view.mouse_x) << cbdata->view.zoom_out) / ((cbdata->mvc->data_control.width_unit << cbdata->view.zoom_in)+SPACEING));
		}
		else
		{
            cbdata->mvc->data_control.offset[cbdata->mvc->_data_type-1] = 0;
		}
//        g_id_info[cbdata->m_ins_id.c_str()].offset[cbdata->mvc->_data_type-1] =0;
        g_id_list_info.id_info[cbdata->m_ins_id.c_str()].offset[cbdata->mvc->_data_type-1] =0;
	}
    else
    {
//        g_id_info[cbdata->m_ins_id.c_str()].offset[cbdata->mvc->_data_type-1] = cbdata->mvc->data_control.offset[cbdata->mvc->_data_type-1];
        g_id_list_info.id_info[cbdata->m_ins_id.c_str()].offset[cbdata->mvc->_data_type-1] = cbdata->mvc->data_control.offset[cbdata->mvc->_data_type-1];
    }
    cairo_show_text(cbdata->cr_main, cbdata->mvc->info->id); //合约ID
    cairo_show_text(cbdata->cr_main, ":");
    cairo_show_text(cbdata->cr_main, cbdata->mvc->info->name); //合约名称
    cairo_stroke(cbdata->cr_main);
	//数据显示函数调用
	int zoom_out = cbdata->view.zoom_out <= 2 ? cbdata->view.zoom_out : 2;
	cairo_set_line_width(cbdata->view.cr, 1.0 / (1 << zoom_out));

	cbdata->mvc->draw_helper->draw_callback(&cbdata->view, cbdata->mvc);
	//附加标签数据绘制
	zoom_out = 0;    // cbdata->view.zoom_out < 2 ? cbdata->view.zoom_out : 1;
	DrawHelper_Config * tag_conf;
	int cnt = cbdata->mvc->handler->GetTagCount();
	int i;
	MarketDataType * dt;
    for (i = 1; i <= cnt; i++)
	{
        dt = cbdata->mvc->handler->GetTagType(i);
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
        if(strcmp(dt->name,"布林通道")==0)
        {
            draw_boll(i, &cbdata->view, cbdata->mvc, tag_conf);
        }
        else if(strcmp(dt->name,"cci")!=0){
            draw_tag(i, &cbdata->view, cbdata->mvc, tag_conf);
        }
	}
}

int draw_dia(cairo_t *cr, int w,int h,double scale,DiagramDrawCallbackData * cbdata)
{
	DrawRGBA color = { 0, 0, 0, 1 };
	if (h < (g_draw_font.id.Size + g_draw_font.tip.Size + 20))
		return 0;
    cairo_set_source_rgba(cr, color.red,color.green,color.blue,color.alpha);
    cairo_rectangle(cr, 0, 0, w * scale, h * scale);
    cairo_fill(cr);
    if (!cbdata->mvc ||!cbdata->m_ins_id.Length())
	{
		//用底色清除绘图区即可
		return 0;
	}

	int redraw_data = 0;
    int scale_width = 0;
	redraw_data = cbdata->mvc->data_control.redraw_by_data_change;
	cbdata->mvc->data_control.redraw_by_data_change = 0;
	cbdata->view.cr = cr;
    cbdata->view.cr_all  = cr;
	cbdata->view_w = w;

    if ((w != cbdata->view.width) || (h != cbdata->view.height)||cbdata->view.rebuild&4)
	{
		//需要重新创建绘图缓冲区

		if (cbdata->pixbuf_main)
		{
			cairo_destroy(cbdata->cr_main);
			cairo_surface_destroy(cbdata->pixbuf_main);
			cbdata->cr_main = 0;
			cbdata->pixbuf_main = 0;

            cairo_destroy(cbdata->cr_time);
            cairo_surface_destroy(cbdata->pixbuf_time);
            cbdata->cr_time = 0;
            cbdata->pixbuf_time = 0;

            cairo_destroy(cbdata->cr_macd);
            cairo_surface_destroy(cbdata->pixbuf_macd);
            cbdata->cr_macd = 0;
            cbdata->pixbuf_macd = 0;

            cairo_destroy(cbdata->cr_volume);
            cairo_surface_destroy(cbdata->pixbuf_volume);
            cbdata->cr_volume = 0;
            cbdata->pixbuf_volume = 0;

            cairo_destroy(cbdata->cr_scale);
            cairo_surface_destroy(cbdata->pixbuf_scale);
            cbdata->cr_scale = 0;
            cbdata->pixbuf_scale = 0;

			cairo_destroy(cbdata->cr_title);
			cairo_surface_destroy(cbdata->pixbuf_title);
			cbdata->cr_title = 0;
			cbdata->pixbuf_title = 0;

            cairo_destroy(cbdata->cr_tip);
            cairo_surface_destroy(cbdata->pixbuf_tip);
            cbdata->cr_tip = 0;
            cbdata->pixbuf_tip = 0;
		}
		redraw_data = 1;
		cbdata->view.width = w;
		cbdata->view.height = h;
        if(cbdata->view.zoom_in_out)
        {
            scale_width = SCALE_WITH;
        }
        if(cbdata->view.zoom_in_out)
            cbdata->h_time = 10;
        else
            cbdata->h_time = 0;
        cbdata->w_tip = TIP_WIDTH;
        cbdata->h_tip  = h;
        cbdata->view.height_time = cbdata->h_time;
        cbdata->h_title = g_draw_font.id.Size ;
        cbdata->h_main =  cbdata->view.main_precent_height*(h - cbdata->h_title - cbdata->h_time);//去除h_title后的2/3
        cbdata->w_main = w - scale_width - TIP_WIDTH;
        cbdata->view.width_main = cbdata->w_main;


        cbdata->view.rebuild = cbdata->view.rebuild &(~4);
        switch (cbdata->view.rebuild) {
        case 2://只画volume
        {
            cbdata->h_volume = (1-cbdata->view.main_precent_height) * (h - cbdata->h_title ); //去除h_title后的1/3
            int offset = h - cbdata->h_main - cbdata->h_volume - cbdata->h_title -cbdata->h_time;
            cbdata->h_volume  += offset;
            cbdata->h_macd = 0;
        }
            break;
        case 3://画volume和macd
        {
            cbdata->h_macd = cbdata->view.macd_precent_height* (h - cbdata->h_title );//去除h_title后的1/3
            cbdata->h_volume = cbdata->view.volume_precent_height * (h - cbdata->h_title );//去除h_title后的1/3
            int offset = h - cbdata->h_main - cbdata->h_volume - cbdata->h_macd - cbdata->h_title -cbdata->h_time;
            cbdata->h_macd  += offset;
        }
            break;
        case 1://只画macd
        {
            cbdata->h_macd = (1-cbdata->view.volume_precent_height) * (h - cbdata->h_title );//去除h_title后的1/3
            int offset = h - cbdata->h_main - cbdata->h_macd - cbdata->h_title-cbdata->h_time;
            cbdata->h_macd  += offset;
            cbdata->h_volume = 0;
        }
            break;
        case 0://都不画
        {
            cbdata->h_main = h - cbdata->h_title -cbdata->h_time;//去除h_title后的2/3
            cbdata->h_volume = 0;
            cbdata->h_macd = 0;
        }
            break;
        default:
            break;
        }
        cbdata->h_title+=1;
        cbdata->view.height_main = cbdata->h_main;
        cbdata->h_scale = cbdata->h_main;
        cbdata->w_scale = scale_width;
        cbdata->view.width_scale = cbdata->w_scale;
        cbdata->view.height_scale = cbdata->h_main;
        cbdata->view.height_macd = cbdata->h_macd;
        cbdata->view.height_volume = cbdata->h_volume;
        cbdata->view.height_tip = cbdata->h_tip;

	}
	//创建一个新的缓冲区
	if (!cbdata->pixbuf_main)
		redraw_data = 1;

	if (redraw_data)
	{
		if (!cbdata->pixbuf_main)
		{
            cbdata->pixbuf_main = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, (w -scale_width-TIP_WIDTH)* scale, cbdata->h_main * scale);
            cbdata->pixbuf_scale  = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, scale_width * scale, cbdata->h_scale * scale);
            cbdata->pixbuf_time = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, (w-TIP_WIDTH) * scale, cbdata->h_time * scale);
            cbdata->pixbuf_macd = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, (w-TIP_WIDTH) * scale, cbdata->h_macd * scale);
            cbdata->pixbuf_volume = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, (w-TIP_WIDTH) * scale, cbdata->h_volume * scale);
            cbdata->pixbuf_title = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, (w-TIP_WIDTH) * scale, cbdata->h_title * scale);
            cbdata->pixbuf_tip = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, TIP_WIDTH * scale, cbdata->h_tip * scale);

			cairo_surface_set_device_scale(cbdata->pixbuf_main,scale,scale);
            cairo_surface_set_device_scale(cbdata->pixbuf_scale,scale,scale);
            cairo_surface_set_device_scale(cbdata->pixbuf_time,scale,scale);
            cairo_surface_set_device_scale(cbdata->pixbuf_macd,scale,scale);
            cairo_surface_set_device_scale(cbdata->pixbuf_volume,scale,scale);
			cairo_surface_set_device_scale(cbdata->pixbuf_title,scale,scale);
            cairo_surface_set_device_scale(cbdata->pixbuf_tip,scale,scale);


			cbdata->cr_main = cairo_create(cbdata->pixbuf_main);
            cbdata->cr_scale = cairo_create(cbdata->pixbuf_scale);
            cbdata->cr_time = cairo_create(cbdata->pixbuf_time);
            cbdata->cr_macd = cairo_create(cbdata->pixbuf_macd);
            cbdata->cr_volume = cairo_create(cbdata->pixbuf_volume);
			cbdata->cr_title = cairo_create(cbdata->pixbuf_title);
            cbdata->cr_tip = cairo_create(cbdata->pixbuf_tip);

			cbdata->view.cr_main = cbdata->cr_main;
            cbdata->view.cr_scale = cbdata->cr_scale;
            cbdata->view.cr_time = cbdata->cr_time;
            cbdata->view.cr_macd = cbdata->cr_macd;
            cbdata->view.cr_volume = cbdata->cr_volume;
			cbdata->view.cr_title = cbdata->cr_title;
            cbdata->view.cr_tip = cbdata->cr_tip;
		}

		cairo_set_source_rgba(cbdata->cr_main, color.red,color.green,color.blue,color.alpha);
        cairo_set_source_rgba(cbdata->cr_scale, color.red,color.green,color.blue,color.alpha);
        cairo_set_source_rgba(cbdata->cr_time, color.red,color.green,color.blue,color.alpha);
        cairo_set_source_rgba(cbdata->cr_macd, color.red,color.green,color.blue,color.alpha);
        cairo_set_source_rgba(cbdata->cr_volume, color.red,color.green,color.blue,color.alpha);
		cairo_set_source_rgba(cbdata->cr_title, color.red,color.green,color.blue,color.alpha);
        cairo_set_source_rgba(cbdata->cr_tip, color.red,color.green,color.blue,color.alpha);

        cairo_rectangle(cbdata->cr_main, 0, 0, cbdata->w_main, cbdata->h_main);
        cairo_fill(cbdata->cr_main);

        cairo_rectangle(cbdata->cr_scale, 0, 0, cbdata->w_scale, cbdata->h_scale);
        cairo_fill(cbdata->cr_scale);

        cairo_rectangle(cbdata->cr_time, 0, 0, w-TIP_WIDTH, cbdata->h_time);
        cairo_fill(cbdata->cr_time);

        cairo_rectangle(cbdata->cr_macd, 0, 0, w-TIP_WIDTH, cbdata->h_macd);
        cairo_fill(cbdata->cr_macd);

        cairo_rectangle(cbdata->cr_volume, 0, 0, w-TIP_WIDTH, cbdata->h_volume);
        cairo_fill(cbdata->cr_volume);

        cairo_rectangle(cbdata->cr_title, 0, 0, w-TIP_WIDTH, cbdata->h_title);
        cairo_fill(cbdata->cr_title);

        cairo_rectangle(cbdata->cr_tip, 0, 0, TIP_WIDTH, cbdata->h_tip);
        cairo_fill(cbdata->cr_tip);

		//真正的绘制数据
		cbdata->data->lock.Lock();
		if( cbdata->data->factory)
        {
			draw_callback_datas(cbdata);
        }
		cbdata->data->lock.Unlock();

        cairo_surface_flush(cbdata->pixbuf_main);
        cairo_surface_flush(cbdata->pixbuf_scale);
        cairo_surface_flush(cbdata->pixbuf_time);
        cairo_surface_flush(cbdata->pixbuf_macd);
        cairo_surface_flush(cbdata->pixbuf_volume);
        cairo_surface_flush(cbdata->pixbuf_title);
        cairo_surface_flush(cbdata->pixbuf_tip);
	}
    cairo_set_source_rgba(cbdata->cr_title, color.red,color.green,color.blue,color.alpha);
    cairo_rectangle(cbdata->cr_title, 0, 0, w-TIP_WIDTH, cbdata->h_title);
    cairo_fill(cbdata->cr_title);
    draw_callback_title(cbdata);
    cbdata->data->lock.Lock();
    if( cbdata->data->factory)
    {
        draw_tip_background(&cbdata->view,cbdata->mvc);
        draw_tip_info(&cbdata->view,cbdata);
    }
    cbdata->data->lock.Unlock();
	//贴图到窗口视图
    cairo_set_source_surface(cr, cbdata->pixbuf_title, TIP_WIDTH, 0);
    cairo_paint(cr);

    cairo_set_source_surface(cr, cbdata->pixbuf_main, TIP_WIDTH, cbdata->h_title);
	cairo_paint(cr);

    cairo_set_source_surface(cr, cbdata->pixbuf_scale, TIP_WIDTH+cbdata->w_main, cbdata->h_title);
    cairo_paint(cr);

    cairo_set_source_surface(cr, cbdata->pixbuf_volume, TIP_WIDTH, cbdata->h_title + cbdata->h_main);
    cairo_paint(cr);

    cairo_set_source_surface(cr, cbdata->pixbuf_macd, TIP_WIDTH, cbdata->h_title + cbdata->h_main + cbdata->h_volume);
    cairo_paint(cr);

    cairo_set_source_surface(cr, cbdata->pixbuf_time, TIP_WIDTH, cbdata->h_title + cbdata->h_main+cbdata->h_volume +cbdata->h_macd);
    cairo_paint(cr);

    cairo_set_source_surface(cr, cbdata->pixbuf_tip, 0, 0);
    cairo_paint(cr);
	//在主图上绘制焦点信息
	cbdata->data->lock.Lock();
    if( cbdata->data->factory){
		draw_callback_focus(cr,cbdata);
    }
	cbdata->data->lock.Unlock();
//在main和sub间画红色分界线
	double dashed1[] = { 4.0, 2.0 };

	cairo_set_dash(cr, dashed1, 2, 1);
	color.red = 1.0;
	color.green = 0.0;
	color.blue = 0.0;
	color.alpha = 0.3;
	cairo_set_source_rgba(cr, color.red,color.green,color.blue,color.alpha);

	cairo_set_line_width(cr, 2);

    cairo_move_to(cr, TIP_WIDTH, cbdata->h_title + cbdata->h_main);
	cairo_line_to(cr, w, cbdata->h_title + cbdata->h_main);
    cairo_move_to(cr, TIP_WIDTH, cbdata->h_title + cbdata->h_main +cbdata->h_volume);
    cairo_line_to(cr, w, cbdata->h_title + cbdata->h_main +cbdata->h_volume);

	cairo_stroke(cr);

	if (!cbdata->view.mouse_in)
	{//鼠标不在窗口内
		return 0;
	}

	//鼠标十字架
//    color.red = 0.8;
//    color.green = 0.8;
//    color.blue = 0;
//    color.alpha = 0.8;
//    cairo_set_source_rgba(cr, color.red,color.green,color.blue,color.alpha);
//    cairo_set_line_width(cr, 0.5);

//    cairo_new_path(cr);
//    cairo_move_to(cr, 0, cbdata->view.mouse_y);
//    cairo_line_to(cr, cbdata->view.width, cbdata->view.mouse_y);
//    cairo_stroke(cr);
//    cairo_move_to(cr, cbdata->view.mouse_x, 0);
//    cairo_line_to(cr, cbdata->view.mouse_x, cbdata->view.height);
//    cairo_stroke(cr);

//    draw_mouse_price(cr, cbdata);


	return 0;
}



static E15_Strmap g_draw_helper_hash_obj;
E15_Strmap * g_draw_helper_hash = &g_draw_helper_hash_obj;

int on_key_nothing(DiagramView_Control *, MarketMVC * , int )
{
	return 0;
}

void draw_callback_raw(DiagramView_Control *view, MarketMVC * mvc);
void raw_draw_focus_info(DiagramView_Control *view, MarketMVC * mvc);

void draw_callback_normalize(DiagramView_Control *view, MarketMVC * mvc);
void normalize_draw_focus_info(DiagramView_Control *view, MarketMVC * mvc);
int normalize_on_key(DiagramView_Control *view, MarketMVC * mvc, int key);
void draw_callback_kline(DiagramView_Control *view, MarketMVC * mvc);
void kline_draw_focus_info(DiagramView_Control *view, MarketMVC * mvc);
int kline_on_key(DiagramView_Control *view, MarketMVC * mvc, int key);

void draw_callback_brick(DiagramView_Control *view, MarketMVC * mvc);
void brick_draw_focus_info(DiagramView_Control *view, MarketMVC * mvc);
int brick_on_key(DiagramView_Control *view, MarketMVC * mvc, int key);
void Init_Draw_Helper()
{
	MarketDataDraw_Helper * dh = new MarketDataDraw_Helper;

//    dh->draw_callback = draw_callback_normalize;
//    dh->draw_focus = normalize_draw_focus_info;
//    dh->draw_mode_set = normalize_on_key;
//    dh->item_width = 5;
//    g_draw_helper_list.PutTail(dh);
//    g_draw_helper_hash->SetAtS(dh, "sys::def");

	dh->draw_callback = draw_callback_kline;
	dh->draw_focus = kline_draw_focus_info;
	dh->draw_mode_set = kline_on_key;
    dh->item_width = 5;
	g_draw_helper_list.PutTail(dh);
	g_draw_helper_hash->SetAtS(dh, "kline");

    dh = new MarketDataDraw_Helper;
    dh->draw_callback = draw_callback_brick;
    dh->draw_focus = brick_draw_focus_info;
    dh->draw_mode_set = brick_on_key;
    dh->item_width = 5;
    g_draw_helper_list.PutTail(dh);
    g_draw_helper_hash->SetAtS(dh, "brick");

}



DiagramDrawCallbackData *Create_DiagramDrawCallbackData(int index)
{
	DiagramDrawCallbackData * cbdata = new DiagramDrawCallbackData;
	cbdata->data = 0;
	cbdata->mvc = 0;
//view
	cbdata->view.cr = 0;
    cbdata->view.cr_all = 0;

    cbdata->view.cr_main = 0;
    cbdata->view.height_main = 0;
    cbdata->view.width_main = 0;

    cbdata->view.cr_time = 0;
    cbdata->view.height_time = 0;

    cbdata->view.cr_macd = 0;
    cbdata->view.height_macd = 0;

    cbdata->view.cr_volume = 0;
    cbdata->view.height_volume = 0;

    cbdata->view.cr_scale = 0;
    cbdata->view.height_scale = 0;
    cbdata->view.width_scale = 0;

    cbdata->view.cr_tip = 0;
    cbdata->view.height_tip =0;
    cbdata->view.width_tip = 0;

    cbdata->view.cr_title = 0;

    cbdata->view.height = 0;
    cbdata->view.width = 0;

    cbdata->view.mouse_in = 0;
    cbdata->view.mouse_x = 0;
    cbdata->view.mouse_y = 0;
    cbdata->view.begin_mouse_x = 0;
    cbdata->view.begin_mouse_y = 0;

    cbdata->view.zoom_in = 0; //放大倍数
    cbdata->view.zoom_out = 0; //缩小倍数
    cbdata->view.zoom_in_out = 0;
    cbdata->view.has_trade = 0;
    cbdata->view.kavg_show = 0;
    cbdata->view.has_trade =false;
    cbdata->view.kavg_show = 0;//显示均线
    cbdata->view.rebuild = 3;
    cbdata->view.index = index;
    cbdata->view.m_paintEventFlag = 0;
    cbdata->view.mouse_press = 0;
    cbdata->view.has_select = 0;
    cbdata->view.main_precent_height = 0.6;
    cbdata->view.volume_precent_height = 0.2;
    cbdata->view.macd_precent_height = 0.2;



    cbdata->draw_area = 0;
	cbdata->pixbuf_main = 0; //绘图缓存，避免鼠标移动时的大量重绘

//	cbdata->pixbuf_sub = 0;
    cbdata->pixbuf_macd = 0;
    cbdata->pixbuf_volume = 0;
	cbdata->cr_main = 0;
//	cbdata->cr_sub = 0;
    cbdata->cr_macd = 0;
    cbdata->cr_volume = 0;

    cbdata->view_w = 0;
    cbdata->m_focus_x = 0;

    cbdata->pixbuf_main = 0;
    cbdata->cr_main = 0;
    cbdata->h_main = 0;
    cbdata->w_main = 0;

    cbdata->pixbuf_time = 0;
    cbdata->cr_time = 0;
    cbdata->h_time = 0;

    cbdata->pixbuf_macd = 0;
    cbdata->cr_macd = 0;
    cbdata->h_macd = 0;
    cbdata->pixbuf_volume = 0;
    cbdata->cr_volume = 0;
    cbdata->h_volume = 0;

    cbdata->pixbuf_scale = 0;
    cbdata->cr_scale = 0;
    cbdata->h_scale = 0;
    cbdata->w_scale = 0;

    cbdata->cr_title = 0;
	cbdata->pixbuf_title = 0; //绘图缓存，避免鼠标移动时的大量重绘
    cbdata->h_title = 0;

    cbdata->pixbuf_tip = 0;
    cbdata->cr_tip = 0;
    cbdata->h_tip = 0;
    cbdata->w_tip = 0;

    cbdata->m_ins_id  =NULL;
    cbdata->view.volume_type = 0;
    cbdata->view.macd_type = 1;
	return cbdata;
}

void Delete_DiagramDrawCallbackData(DiagramDrawCallbackData*cbdata)
{
	if (cbdata->pixbuf_main)
	{
		cairo_destroy(cbdata->cr_main);
		cairo_surface_destroy(cbdata->pixbuf_main);
	}
    if (cbdata->pixbuf_macd)
    {
        cairo_destroy(cbdata->cr_macd);
        cairo_surface_destroy(cbdata->pixbuf_macd);
    }

    if (cbdata->pixbuf_volume)
    {
        cairo_destroy(cbdata->cr_volume);
        cairo_surface_destroy(cbdata->pixbuf_volume);
    }

	if (cbdata->pixbuf_title)
	{
		cairo_destroy(cbdata->cr_title);
		cairo_surface_destroy(cbdata->pixbuf_title);
	}
    if(cbdata->pixbuf_scale)
    {
        cairo_destroy(cbdata->cr_scale);
        cairo_surface_destroy(cbdata->pixbuf_scale);
    }
    if(cbdata->pixbuf_time)
    {
        cairo_destroy(cbdata->cr_time);
        cairo_surface_destroy(cbdata->pixbuf_time);
    }
    if(cbdata->pixbuf_tip)
    {
        cairo_destroy(cbdata->cr_tip);
        cairo_surface_destroy(cbdata->pixbuf_tip);
    }
	delete cbdata;
}
