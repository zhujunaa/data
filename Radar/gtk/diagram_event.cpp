#include <gtk/gtk.h>

#include "E15_queue.h"
#include "E15_string_array.h"
#include "E15_value.h"
#include "E15_map.h"

#include <string.h>
#include "draw_dia.h"

#include "data_view.h"



int get_key_value(GdkEventKey *event)
{
	if (event->keyval < GDK_KEY_0)
	{
		if (event->keyval == GDK_KEY_plus)
			return '+';
		if (event->keyval == GDK_KEY_minus)
			return '-';
		return -1;
	}

	if (event->keyval < GDK_KEY_z)
		return '0' + event->keyval - GDK_KEY_0;

	return -1;
}

gboolean key_value(GtkWidget *widget, GdkEventKey *event, DiagramDrawCallbackData * cbdata)
{
	if (!cbdata->mvc)
		return TRUE;

	if (cbdata->view.width < 1)
		cbdata->view.width = gtk_widget_get_allocated_width(widget);

	if (cbdata->view.width < 10)
		return TRUE; //太小

	int ctrl_flag = event->state & GDK_CONTROL_MASK;
	int key = get_key_value(event);

	if (key >= '0' && key <= 'z' )
	{ //切换显示图表类型
		if( dia_on_key_ch(cbdata, key, ctrl_flag) )
			gtk_widget_queue_draw((GtkWidget *)cbdata->draw_area);
		return TRUE;
	}

	int redraw = 0;
	switch (event->keyval)
	{
	case GDK_KEY_Home:
		redraw = dia_on_key_home(cbdata,ctrl_flag);
		break;
	case GDK_KEY_End:
		redraw = dia_on_key_end(cbdata,ctrl_flag);
		break;
	case GDK_KEY_Page_Up:
		redraw = dia_on_key_page_up(cbdata,ctrl_flag);
		break;
	case GDK_KEY_Page_Down:
		redraw = dia_on_key_page_down(cbdata,ctrl_flag);
		break;
	case GDK_KEY_Up:
		redraw = dia_on_key_up(cbdata,ctrl_flag);
		break;
	case GDK_KEY_Down:
		redraw = dia_on_key_down(cbdata,ctrl_flag);
		break;
	case GDK_KEY_Left:
		redraw = dia_on_key_left(cbdata,ctrl_flag);
		break;
	case GDK_KEY_Right:
		redraw = dia_on_key_right(cbdata,ctrl_flag);
		break;
	default:
		break;
	}
	//显示发生变化，重新绘制
	if( redraw)
		gtk_widget_queue_draw((GtkWidget *)cbdata->draw_area);
	return TRUE;
}

gboolean leave_event(GtkWidget *widget, GdkEventCrossing *event, DiagramDrawCallbackData * cbdata)
{
	if (GDK_CROSSING_NORMAL != event->mode)
		return FALSE;

	cbdata->view.mouse_in = 0;
	gtk_widget_queue_draw( (GtkWidget *)cbdata->draw_area);
	return TRUE;
}

gboolean enter_event(GtkWidget *widget, GdkEventCrossing *event, DiagramDrawCallbackData * cbdata)
{
	if (GDK_CROSSING_NORMAL != event->mode)
		return FALSE;
	cbdata->view.mouse_in = 1;
	gtk_widget_queue_draw( (GtkWidget *)cbdata->draw_area);
	gtk_widget_grab_focus(widget);
	return TRUE;
}

gboolean motion_notify_event(GtkWidget *widget, GdkEventMotion *event, DiagramDrawCallbackData * cbdata) // 鼠标移动事件
{
	if (!cbdata->mvc)
		return TRUE;

	if( dia_on_mouse_move(cbdata,event->x, event->y) )
		gtk_widget_queue_draw((GtkWidget *)cbdata->draw_area);
	return FALSE;
}

//响应双击事件，切换放大和缩小

void DiagramMaxMin(DiagramDrawCallbackData *cbdata);

gboolean on_mouse_press(GtkWidget *widget, GdkEvent *event, DiagramDrawCallbackData * cbdata)
{
	GdkEventButton * button = (GdkEventButton*) event;

	if (button->button != 1)
		return FALSE;
	if (button->type != GDK_2BUTTON_PRESS)
		return FALSE;

	cbdata->zoom_in_out = 1 - cbdata->zoom_in_out;
	DiagramMaxMin(cbdata);
	//双击事件，进行切换
	return TRUE;
}

extern int replace_view_data(const char * id, DiagramDrawCallbackData * cbdata);
extern void SaveDiagramInstruments();

void on_instrument_change(GtkWidget *widget, GdkDragContext *context, gint x, gint y, GtkSelectionData *data, guint info, guint time, DiagramDrawCallbackData * cbdata)
{

	const char * id = (const char *) gtk_selection_data_get_text(data);
	if (!id)
		return;
	replace_view_data(id, cbdata);
	SaveDiagramInstruments();
}

//创建event box

GtkWidget * E15_create_eventbox(DiagramDrawCallbackData * cbdata)
{
	GtkWidget * event_box = gtk_event_box_new();

	gtk_widget_add_events(event_box, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_MOTION_MASK | GDK_POINTER_MOTION_MASK | GDK_LEAVE_NOTIFY_MASK | GDK_ENTER_NOTIFY_MASK);

	g_signal_connect(G_OBJECT(event_box), "key-press-event", G_CALLBACK(key_value), cbdata);
	g_signal_connect(G_OBJECT(event_box), "button-press-event", G_CALLBACK(on_mouse_press), cbdata);
	g_signal_connect(G_OBJECT(event_box), "leave_notify_event", G_CALLBACK(leave_event), cbdata);
	g_signal_connect(G_OBJECT(event_box), "enter_notify_event", G_CALLBACK(enter_event), cbdata);
	g_signal_connect(G_OBJECT(event_box), "motion_notify_event", G_CALLBACK(motion_notify_event), cbdata);

	gtk_widget_set_can_focus(event_box, TRUE);

	gtk_drag_dest_set(event_box, GTK_DEST_DEFAULT_ALL,         //,
	0,         //drag_target,
	0, GDK_ACTION_COPY); //GDK_ACTION_COPY

	g_signal_connect(event_box, "drag-data-received", (GCallback) on_instrument_change, cbdata);

	return event_box;
}
