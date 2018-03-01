#include <gtk/gtk.h>

#include <string.h>

#include "E15_queue.h"
#include "E15_value.h"
#include "E15_debug.h"
#include "E15_ini.h"

#include "draw_dia.h"

#include "data_mgr.h"

static E15_Queue g_draw_helper_list;

GtkWidget * E15_create_eventbox(DiagramDrawCallbackData * cbdata);

gboolean draw_callback(GtkWidget *widget, cairo_t *cr, DiagramDrawCallbackData * cbdata);

GtkWidget * Create_Diagram_View(DiagramDrawCallbackData * cbdata)
{
	GtkWidget * diagram = gtk_drawing_area_new();

	cbdata->draw_area = diagram;
	cbdata->event_box = E15_create_eventbox(cbdata);
	gtk_container_add((GtkContainer *) cbdata->event_box, diagram);

	g_signal_connect(G_OBJECT(diagram), "draw", G_CALLBACK(draw_callback), cbdata);

	gtk_drag_dest_add_text_targets((GtkWidget*)cbdata->event_box);
	return (GtkWidget*)cbdata->event_box;
}


gboolean draw_callback(GtkWidget *widget, cairo_t *cr, DiagramDrawCallbackData * cbdata)
{

	int w = gtk_widget_get_allocated_width(widget);
	int h = gtk_widget_get_allocated_height(widget);

	if (h < (g_draw_font.id.Size + g_draw_font.tip.Size + 20))
		return FALSE; //窗口太小，不绘制

	gint scale = gdk_window_get_scale_factor(gtk_widget_get_window(widget));


	//绘制主图到缓冲区
	return draw_dia(cr, w, h,scale,cbdata );

}


