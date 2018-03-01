#include <gtk/gtk.h>

#include "E15_queue.h"
#include "E15_value.h"
#include "E15_debug.h"

GtkWidget * g_market_view = 0;

void drag_drop_ondata (GtkWidget        *widget,
               GdkDragContext   *context,
               gint              x,
               gint              y,
               GtkSelectionData *data,
               guint             info,
               guint             time,
               gpointer          user_data)
{
	guchar * str = gtk_selection_data_get_text (data);
	printf("%d:%d insert %s\n",x,y,str);
}


GtkWidget * Create_Market_View()
{
    GtkWidget *sw;
    GtkWidget * frm = gtk_frame_new (0);

	//gtk_widget_set_can_focus (frm,TRUE);

    sw = gtk_scrolled_window_new (NULL, NULL);


	g_market_view = frm;

	gtk_drag_dest_set (frm,
                   GTK_DEST_DEFAULT_ALL,//,
                   0,//drag_target,
                   0,
                   GDK_ACTION_COPY); //GDK_ACTION_COPY

	g_signal_connect (frm, "drag-data-received", (GCallback)drag_drop_ondata, 0);


	gtk_drag_dest_add_text_targets (frm);

	gtk_container_add (GTK_CONTAINER (sw), frm);

    return sw;
}
