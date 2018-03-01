#include <gtk/gtk.h>

#include "E15_queue.h"
#include "E15_value.h"
#include "E15_debug.h"

#include "StockDataCache.h"
#include "draw_dia.h"
#include "data_view.h"

enum
{
	ID_COLUMN = 0, //ID
	NAME_COLUMN,
	SPEED_COLUMN,
	FIX_COLUMN,
	NUM_COLUMNS
};

static GtkTreeModel * create_model(void)
{
	GtkTreeStore *model;

	/* create tree store */
	model = gtk_tree_store_new(NUM_COLUMNS, G_TYPE_STRING, G_TYPE_STRING,G_TYPE_INT,G_TYPE_INT);

	return GTK_TREE_MODEL(model);
}

static void add_columns(GtkTreeView *treeview)
{
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	/* 合约ID列 */
	renderer = gtk_cell_renderer_text_new();

	column = gtk_tree_view_column_new_with_attributes("ID", renderer, "text",ID_COLUMN, NULL);
	gtk_tree_view_column_set_clickable(column, FALSE);
	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_AUTOSIZE); //GTK_TREE_VIEW_COLUMN_AUTOSIZE //GTK_TREE_VIEW_COLUMN_FIXED
	gtk_tree_view_column_set_alignment(column, 0.0);
	gtk_tree_view_append_column(treeview, column);

	/* 合约名称列 */
	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("名称", renderer, "text",NAME_COLUMN, NULL);

	gtk_tree_view_column_set_clickable(column, FALSE);
	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
	//gtk_tree_view_column_set_fixed_width(column, 10);
	gtk_tree_view_column_set_alignment(column, 0.0);
	gtk_tree_view_append_column(treeview, column);

	/* 合约涨速列*/
	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("涨速", renderer, "text",SPEED_COLUMN, NULL);
	gtk_tree_view_column_set_sort_column_id(column, SPEED_COLUMN);
	gtk_tree_view_column_set_sort_order(column, GTK_SORT_DESCENDING);
	//gtk_tree_view_column_set_clickable(column,FALSE);
	gtk_tree_view_append_column(treeview, column);
}

static GtkTreeView * g_speed_treeview = 0;

static void drag_drop_getdata(GtkWidget *widget, GdkDragContext *context,
		GtkSelectionData *data, guint info, guint time, gpointer user_data)
{
	GtkTreeIter iter;
	GtkTreeModel *model = 0;
	GtkTreeView * treeview = (GtkTreeView *) widget;

	GtkTreeSelection * sel = gtk_tree_view_get_selection(treeview);
	if (!sel)
		return;
	if (!gtk_tree_selection_get_selected(sel, &model, &iter))
		return;

	gchar * id = 0;
	gtk_tree_model_get(model, &iter, ID_COLUMN, &id, -1);

	gtk_selection_data_set_text(data, id, -1);
}

GtkWidget * Create_Speed_Listview()
{

	GtkWidget *sw;
	GtkWidget *treeview;
	GtkTreeModel *model;

	sw = gtk_scrolled_window_new(NULL, NULL);

	/* create tree model */
	model = create_model();

	/* create tree view */
	treeview = gtk_tree_view_new_with_model(model);
	g_speed_treeview = (GtkTreeView *) treeview;
	gtk_tree_view_set_search_column(GTK_TREE_VIEW(treeview), ID_COLUMN);
	//gtk_tree_view_set_fixed_height_mode (GTK_TREE_VIEW (treeview),TRUE);
	gtk_tree_view_enable_model_drag_source(g_speed_treeview,
			GDK_BUTTON1_MASK, 0, 0, GDK_ACTION_COPY);

	gtk_drag_source_add_text_targets(treeview);

	g_signal_connect(treeview, "drag-data-get", (GCallback) drag_drop_getdata,
			0);

	g_object_unref(model);

	gtk_container_add(GTK_CONTAINER(sw), treeview);

	/* add columns to the tree view */
	add_columns(GTK_TREE_VIEW(treeview));

	return sw;
}

struct GtkSpeedTreeViewParam
{
	GtkTreeStore *model;
	int cnt;
};

static int on_contract_info(E15_Key * key, E15_Value * info, GtkSpeedTreeViewParam *p)
{
	const char * id = key->GetS();
	if (!id)
		return 0;

	int market = MarketCodeById(id);

	DiagramDataMgr * data = DiagramDataMgr_PeekData(market, id);
	if (!data)
		return 0;

	gint speed = info->GetLong();

	E15_String s;
	s.Sprintf("%ld",speed);

	GtkTreeIter iter;
	gtk_tree_store_append(p->model, &iter, NULL);
	gtk_tree_store_set(p->model, &iter,
			ID_COLUMN, data->info.id,
			NAME_COLUMN,data->info.name,
			SPEED_COLUMN,speed,
			-1);

	p->cnt++;
	if( p->cnt >=5 )
		return 1;

	return 0;
}

void insert_speed_data(E15_ValueTable * t)
{
	if (!g_speed_treeview)
		return;

	GtkSpeedTreeViewParam pp;
	pp.model = (GtkTreeStore*) gtk_tree_view_get_model(g_speed_treeview);
	pp.cnt = 0;

	gtk_tree_store_clear(pp.model);

	E15_ValueTable * speed = t->TableS("speed_up");
	if( speed )
		speed->each((int (*)(E15_Key * key, E15_Value * info, void *) )on_contract_info, &pp );

	pp.cnt = 0;
	speed = t->TableS("speed_down");
	if( !speed )
		return ;
	speed->each((int (*)(E15_Key * key, E15_Value * info, void *) )on_contract_info, &pp );



}
