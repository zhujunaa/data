#include <gtk/gtk.h>
#include <string.h>

#include "E15_ini.h"
#include "E15_debug.h"

#include "draw_dia.h"
#include "data_view.h"

static DiagramDrawCallbackData * g_draw_list[9] = { 0 };

static DrawHelper_Config __g_draw_default_config;

MarketMVC::MarketMVC()
{
	factory = 0;
	memset(&data_control, 0, sizeof(data_control));
	_data_type = 1;

	//draw_config = &__g_draw_default_config;

	handler = 0;
	draw_helper = 0;
	data_control.width_unit = 1;
	data_control.auto_scroller = 1;
	data_control.redraw_by_data_change = 1;

}

MarketMVC::~MarketMVC()
{
}


GtkWidget * Create_Diagram_View(DiagramDrawCallbackData * cbdata);

static GtkWidget * g_col_box[3] = { 0 };

GtkWidget * Create_Diagram_ListView()
{
	int i;
	GtkWidget * hbox = 0;
	GtkWidget * vbox = 0;

	GtkWidget * draw;
	GtkWidget * frm;
//    GtkWidget * handle_box;

	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);

	for (i = 0; i < 9; i++)
	{
		if (!vbox)
		{
			vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
			gtk_box_pack_start((GtkBox*) hbox, vbox, true, true, 0);
			g_col_box[i / 3] = vbox;
		}
		else
		{
			GList * rows = gtk_container_get_children((GtkContainer *) vbox);
			if ((int) g_list_length(rows) >= 3)
			{
				vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
				gtk_box_pack_start((GtkBox*) hbox, vbox, true, true, 0);
				g_col_box[i / 3] = vbox;
			}
		}

		frm = gtk_frame_new(0);
		gtk_frame_set_shadow_type((GtkFrame *) frm, GTK_SHADOW_NONE);

		DiagramDrawCallbackData * cbdata = Create_DiagramDrawCallbackData(i);
		g_draw_list[i] = cbdata;
		cbdata->frame = frm;

		g_object_set_data_full(G_OBJECT(frm), "draw_cbdata", cbdata, (void (*)(void *))Delete_DiagramDrawCallbackData);

		draw = Create_Diagram_View(cbdata);
		gtk_container_add((GtkContainer *) frm, draw);

		gtk_box_pack_start((GtkBox*) vbox, frm, true, true, 0);
	}

	return hbox;
}

void DiagramMaxMin(DiagramDrawCallbackData *cbdata)
{
	int i;
	if (cbdata->zoom_in_out) //放大
	{
		for (i = 0; i < 9; i++)
		{
			if (i != cbdata->index)
				gtk_widget_hide((GtkWidget *)g_draw_list[i]->frame);
		}
		for (i = 0; i < 3; i++)
		{
			if (i != (cbdata->index / 3))
			{
				gtk_widget_hide(g_col_box[i]);
			}
		}
		return;
	}
	for (i = 0; i < 9; i++)
	{
		if (i != cbdata->index)
			gtk_widget_show( (GtkWidget *)g_draw_list[i]->frame);
	}
	for (i = 0; i < 3; i++)
	{
		if (i != (cbdata->index / 3))
		{
			gtk_widget_show(g_col_box[i]);
		}
	}
	//缩小
}

void SaveDiagramInstruments()
{
	E15_Ini ini;

	ini.AddSection("diagram");
	ini.ToChildSection("diagram");

	int i;
	int pos = 0;
	for (i = 0; i < 9; i++)
	{
		if (!g_draw_list[i])
			continue;

		if (g_draw_list[i]->m_ins_id.Length() == 0)
			continue;

		ini.SetSection("diagram");

		ini.AddSection("view");
		ini.ToChildSection("view", pos++);

		ini.Write("id", g_draw_list[i]->m_ins_id.c_str());

		ini.Write("data_type", g_draw_list[i]->mvc_obj._data_type);
		ini.Write("pos", i);
		ini.Write("zoom", (g_draw_list[i]->view.zoom_in - g_draw_list[i]->view.zoom_out));
	}

	ini.Dump("./ini/diagram.ini");
}

void SubScribeMarketData(DiagramDataMgr * mgr, const char * id, int yes);

void clear_view_data(unsigned int h,unsigned int l)
{
	int i;
	DiagramDrawCallbackData * cbdata;
	for (i = 0; i < 9; i++)
	{
		cbdata = g_draw_list[i];
		if (!cbdata->data)
			continue;

		DiagramDataMgr * mgr = (DiagramDataMgr*) cbdata->data;
		if( ((DiagramDataMgrView*) mgr->m_params)->m_market_src.h != h )
			continue;
		if( ((DiagramDataMgrView*) mgr->m_params)->m_market_src.l != l )
			continue;

		cbdata->mvc = 0;
		cbdata->data = 0;

		((DiagramDataMgrView*) mgr->m_params)->view_hash.RemoveSI("draw", cbdata->index);
		((DiagramDataMgrView*) mgr->m_params)->draw_flag = 0;
		mgr->lock.Lock();
		delete mgr->factory;
		mgr->factory = 0;
		mgr->lock.Unlock();

		SubScribeMarketData(mgr, cbdata->m_ins_id.c_str(), 0);
	}
}

void ReSubScribeAllView()
{
	int i;
	DiagramDrawCallbackData * cbdata;
	for (i = 0; i < 9; i++)
	{
		cbdata = g_draw_list[i];
		if (!cbdata->data)
			continue;

		DiagramDataMgr * mgr = (DiagramDataMgr*) cbdata->data;

		if (!mgr->factory)
			continue;

		if (!((DiagramDataMgrView*) mgr->m_params)->draw_flag)
			continue;
		SubScribeMarketData(mgr, cbdata->m_ins_id.c_str(), 1);
	}
}

int replace_view_data(const char * id, DiagramDrawCallbackData * cbdata)
{
	if (!id)
		return 0;
	if (!cbdata)
		return 0;

	cbdata->m_ins_id.Strcpy(id);

	if (cbdata->data)
	{
		do
		{
			if (strcmp(cbdata->data->info.id, id) == 0)
			{
				if( !cbdata->data->factory )
					break;

				//printf("窗口当前显示与目标一致，不需要切换\n");
				//return 0;
				return 1;

			}

			//源显示内容与新内容不一致，删除旧显示内容
			if( ((DiagramDataMgrView*) cbdata->data->m_params)->draw_flag > 0 )
				((DiagramDataMgrView*) cbdata->data->m_params)->draw_flag--;
			((DiagramDataMgrView*) cbdata->data->m_params)->view_hash.RemoveSI("draw", cbdata->index);

			if (((DiagramDataMgrView*) cbdata->data->m_params)->draw_flag == 0) //不需要绘图了
			{

				printf("delete factory for %s\n", cbdata->data->info.id);
				delete cbdata->data->factory;
				cbdata->data->factory = 0;

				SubScribeMarketData(cbdata->data, cbdata->data->info.id, 0);
			}
		}while(0);
	}

	int market = MarketCodeById(id);
	DiagramDataMgr * stock = DiagramDataMgr_PeekData( market,id );
	if (!stock)
		return 0;

	if (!(DiagramDataMgrView*) stock->m_params)
		return 0;

	if( ((DiagramDataMgrView*) stock->m_params)->m_market_src.l == 0 )
		return 0;

	if (!stock->factory)
	{
		stock->CreateFactory();
	}

	DiagramDataHandler * h = stock->factory->GetDataHandler(cbdata->mvc_obj._data_type);
	if (!h)
	{
		E15_Debug::Printf(0, "未找到数据处理器 %d\n", cbdata->mvc_obj._data_type);
		return 0;
	}
	MarketDataDraw_Helper * dh = (MarketDataDraw_Helper*) g_draw_helper_hash->LookupS(h->GetDataType()->class_name);
	if (!dh)
	{
		E15_Debug::Printf(0, "未找到数据绘制器 %s \n", h->GetDataType()->class_name);
		return 0;
	}
	((DiagramDataMgrView*) stock->m_params)->draw_flag++;
	printf("窗口 %d <--> (%s:%s ) 切换数据源\n", cbdata->index, stock->info.id, stock->info.name);


	cbdata->mvc_obj.factory = stock->factory;
	cbdata->mvc_obj.info = &stock->info;
	cbdata->mvc_obj.handler = h;
	cbdata->mvc_obj.draw_helper = dh;
	cbdata->mvc_obj.data_control.width_unit = cbdata->mvc_obj.draw_helper->item_width;

	cbdata->mvc = &cbdata->mvc_obj;

	((DiagramDataMgrView*) stock->m_params)->view_hash.SetAtSI(cbdata->draw_area, "draw", cbdata->index);
	cbdata->data = stock;

	cbdata->mvc->data_control.redraw_by_data_change = 1;
	gtk_widget_queue_draw( (GtkWidget *)cbdata->draw_area);

	if (h->PeekData()->Count() < 512)
		dia_load_history(id, h, 512); //加载历史数据

	SubScribeMarketData(cbdata->data, cbdata->data->info.id, 1); //订阅新数据

	//int max_item = ( (cbdata->view.width << cbdata->view.zoom_out) -1) / (cbdata->mvc->data_control.width_unit<<cbdata->view.zoom_in);

	return 1;
}

void replace_view_data_index(const char * id, int index, int data_type, int data_level, int zoom)
{
	if (index < 0)
		return;
	if (index > 8)
		return;

	g_draw_list[index]->mvc_obj._data_type = data_type;

	g_draw_list[index]->view.zoom_in = 0;
	g_draw_list[index]->view.zoom_out = 0;

	g_draw_list[index]->mvc_obj.data_control.redraw_by_data_change = 1;
	if (!replace_view_data(id, g_draw_list[index]))
		return;
	if (zoom == 0)
		return;

	if (zoom > 10)
		zoom = 10;
	else if (zoom < -10)
		zoom = -10;

	if (zoom > 0)
	{
		g_draw_list[index]->view.zoom_in = zoom;
	}
	else if (zoom < 0)
	{
		g_draw_list[index]->view.zoom_out = -zoom;
	}
}

void Update_Diagram_View(int idx)
{
	if (idx < 0)
		return;
	if (idx > 8)
		return;
	g_draw_list[idx]->mvc_obj.data_control.redraw_by_data_change = 1;
	gtk_widget_queue_draw((GtkWidget *)g_draw_list[idx]->draw_area);
}
