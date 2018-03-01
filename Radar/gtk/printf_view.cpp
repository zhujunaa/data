#include <gtk/gtk.h>

#include "E15_debug.h"

#include "E15_string.h"
#include "ui.h"

struct MessageView
{
    GtkTextBuffer *buffer;
    GtkWidget     *view;
};

static MessageView * g_msg_view = 0;


static E15_String g_tag_name;

const char * get_msg_tag(int level)
{
	if(level < 1 )
		return 0;
	if( level >4 )
		return 0;
    g_tag_name.Sprintf("%d",level);
    return g_tag_name.c_str();
}

void view_msg_set_tag_color(int level,GdkRGBA * color)
{

	gtk_text_buffer_create_tag (g_msg_view->buffer, get_msg_tag(level),"foreground-rgba", color, NULL);
}


void view_msg_printf(int level,E15_String * s)
{

    GtkTextIter iter,iter2;
    if( gtk_text_buffer_get_line_count (g_msg_view->buffer) > 1000 )
    {
		//文字信息太多，自动删除前面的部分数据
        gtk_text_buffer_get_iter_at_line (g_msg_view->buffer,&iter,0);
        gtk_text_buffer_get_iter_at_line (g_msg_view->buffer,&iter2,100);
        gtk_text_buffer_delete(g_msg_view->buffer,&iter,&iter2);
    }

    gtk_text_buffer_get_iter_at_offset(g_msg_view->buffer,&iter,-1);
    const char * data = s->c_str();
    int len = s->Length();

    const char * tag = get_msg_tag(level);
    if( tag )
        gtk_text_buffer_insert_with_tags_by_name(g_msg_view->buffer,&iter,data,len ,tag,NULL);
    else
        gtk_text_buffer_insert(g_msg_view->buffer,&iter,data,len );


    //gtk_text_buffer_get_iter_at_line(g_msg_view->buffer,&iter,0x7fffffff);
    gtk_text_buffer_get_end_iter(g_msg_view->buffer,&iter);
    gtk_text_view_scroll_to_iter ((GtkTextView*)g_msg_view->view,&iter,0,0,0,0);
}


MessageView * Create_MessageView()
{
    MessageView * ret = new MessageView;
    ret->buffer = 0;
    ret->view = 0;

    E15_Debug::SetPrintObj( ret );
    return ret;
}

void Delete_MessageView(MessageView * obj)
{
    E15_Debug::SetPrint(0);
    E15_Debug::SetPrintObj(0);

    delete obj;
}

void MainPrintf(int level,const char * fmt,va_list ap)
{
	E15_String * s = new E15_String ;

	s->FormatDate(time(0),"[yyyy-mm-dd hh:mi:ss] ");
	s->AppendV(fmt,ap);
	if( s->tail(1)[0] != '\n')
		s->Addch('\n');
	g_ui_loop->Post(0,0,E15_UI_Message_Printf,level,s);
}

GtkWidget * E15_create_message()
{
	if( g_msg_view )
		return 0;
	GtkWidget *sw;

	g_msg_view = Create_MessageView();


	g_msg_view->view = gtk_text_view_new ();

	gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (g_msg_view->view), GTK_WRAP_WORD);
	gtk_text_view_set_editable (GTK_TEXT_VIEW (g_msg_view->view),false);

	sw = gtk_scrolled_window_new (NULL, NULL);

	g_object_set_data_full( G_OBJECT(sw),"msg_data",g_msg_view, (void (*)(void *) )Delete_MessageView);


	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
								  GTK_POLICY_AUTOMATIC,
								  GTK_POLICY_AUTOMATIC);

	g_msg_view->buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (g_msg_view->view));
	gtk_widget_set_size_request (g_msg_view->view,-1,50);

	GdkRGBA color;
	color.alpha = 1;

	color.red = 1;
	color.green = 0;
	color.blue = 0;
	view_msg_set_tag_color(1,&color);

	color.red = 0;
	color.green = 1;
	color.blue = 0;
	view_msg_set_tag_color(2,&color);


	color.red = 0;
	color.green = 0;
	color.blue = 1;
	view_msg_set_tag_color(3,&color);

	color.red = 0;
	color.green = 1;
	color.blue = 1;
	view_msg_set_tag_color(4,&color);


	gtk_container_add (GTK_CONTAINER (sw), g_msg_view->view);

	//E15_Debug::SetPrint( MainPrintf );

    return sw;

}
