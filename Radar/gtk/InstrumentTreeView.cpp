#include <gtk/gtk.h>

#include "E15_queue.h"
#include "E15_value.h"
#include "E15_debug.h"

#include "StockDataCache.h"

#include "data_view.h"

enum
{
	ID_COLUMN = 0, //ID
	NAME_COLUMN,
	NUM_COLUMNS
};

static GtkTreeModel * create_model(void)
{
	GtkTreeStore *model;

	/* create tree store */
	model = gtk_tree_store_new(NUM_COLUMNS, G_TYPE_STRING, G_TYPE_STRING);

	return GTK_TREE_MODEL(model);
}

static void add_columns(GtkTreeView *treeview)
{
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	/* 合约ID列 */
	renderer = gtk_cell_renderer_text_new();

	column = gtk_tree_view_column_new_with_attributes("ID", renderer, "text", ID_COLUMN, NULL);
	gtk_tree_view_column_set_sort_column_id(column, ID_COLUMN);

	gtk_tree_view_column_set_sort_order(column, GTK_SORT_DESCENDING);
	//gtk_tree_view_column_set_clickable(column,FALSE);
	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_AUTOSIZE); //GTK_TREE_VIEW_COLUMN_AUTOSIZE //GTK_TREE_VIEW_COLUMN_FIXED
	gtk_tree_view_column_set_alignment(column, 0.0);
	gtk_tree_view_append_column(treeview, column);

	/* 合约名称列 */
	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("名称", renderer, "text", NAME_COLUMN, NULL);
	gtk_tree_view_column_set_sort_column_id(column, NAME_COLUMN);
	gtk_tree_view_column_set_sort_order(column, GTK_SORT_DESCENDING);
	//gtk_tree_view_column_set_clickable(column,FALSE);
	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width(column, 10);
	gtk_tree_view_column_set_alignment(column, 0.0);
	gtk_tree_view_append_column(treeview, column);
}

static GtkTreeView * g_future_treeview = 0;
static GtkTreeView * g_cna_treeview = 0;


static void drag_drop_getdata(GtkWidget *widget, GdkDragContext *context, GtkSelectionData *data, guint info, guint time, gpointer user_data)
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

GtkWidget * Create_Contract_Listview_Sub(GtkTreeView * & treeview)
{
	GtkWidget *sw;
	GtkTreeModel *model;

	sw = gtk_scrolled_window_new(NULL, NULL);
	/*
	 gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),
	 GTK_SHADOW_ETCHED_IN);
	 gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
	 GTK_POLICY_NEVER,
	 GTK_POLICY_AUTOMATIC);
	 */

	/* create tree model */
	model = create_model();

	/* create tree view */
	treeview = (GtkTreeView*)gtk_tree_view_new_with_model(model);
	gtk_tree_view_set_search_column(GTK_TREE_VIEW(treeview), ID_COLUMN);
	//gtk_tree_view_set_fixed_height_mode (GTK_TREE_VIEW (treeview),TRUE);
	gtk_tree_view_enable_model_drag_source(treeview, GDK_BUTTON1_MASK, 0, 0, GDK_ACTION_COPY);

	gtk_drag_source_add_text_targets((GtkWidget *)treeview);

	g_signal_connect((GtkWidget *)treeview, "drag-data-get", (GCallback) drag_drop_getdata, 0);

	g_object_unref(model);

	gtk_container_add(GTK_CONTAINER(sw), (GtkWidget *)treeview);

	/* add columns to the tree view */
	add_columns(GTK_TREE_VIEW(treeview));

	return sw;
}

GtkWidget * Create_Contract_Listview()
{
	GtkWidget * notepad = gtk_notebook_new();
	GtkWidget * f_ins = Create_Contract_Listview_Sub(g_future_treeview);
	GtkWidget * tab_label = gtk_label_new("期货");
	gtk_notebook_append_page((GtkNotebook *) notepad, f_ins, tab_label);


	GtkWidget * cna_ins = Create_Contract_Listview_Sub(g_cna_treeview);
	tab_label = gtk_label_new("A股");
	gtk_notebook_append_page((GtkNotebook *) notepad, cna_ins, tab_label);

	return notepad;

}

static int on_contract_info(E15_Key * key, E15_Value * info, GtkTreeStore **model_list)
{
	const char * id = key->GetS();
	if (!id)
		return 0;

	int market = MarketCodeById(id);

	DiagramDataMgr * data = DiagramDataMgr_PeekData(market, id);
	if (!data)
		return 0;

	if (((DiagramDataMgrView*) data->m_params)->view_hash.LookupSI("list", -1)) //已经加入列表中了
		return 0;

	((DiagramDataMgrView*) data->m_params)->view_hash.SetAtSI(data, "list", -1);

	GtkTreeIter iter;
	if( market == 4 )
	{
		gtk_tree_store_append(model_list[0], &iter, NULL);
		gtk_tree_store_set(model_list[0], &iter, ID_COLUMN, data->info.id, NAME_COLUMN, data->info.name, -1);
	}
	else
	{
		gtk_tree_store_append(model_list[1], &iter, NULL);
		gtk_tree_store_set(model_list[1], &iter, ID_COLUMN, data->info.id, NAME_COLUMN, data->info.name, -1);
	}
	return 0;
}

void insert_contract_data(E15_ValueTable * t)
{
	GtkTreeStore *model[2];
	model[0] = (GtkTreeStore*) gtk_tree_view_get_model(g_future_treeview);
	model[1] = (GtkTreeStore*) gtk_tree_view_get_model(g_cna_treeview);

	t->each((int (*)(E15_Key * key, E15_Value * info, void *) )on_contract_info, model);
}
