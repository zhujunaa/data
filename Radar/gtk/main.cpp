#include <gtk/gtk.h>
#include <string.h>

#include "E15_thread.h"

#include "E15_debug.h"
#include "E15_queue.h"
#include "E15_map.h"
#include "E15_socket.h"
#include "E15_ini.h"

#include "ui.h"

#include "md_client.h"

#include "draw_config.h"


E15_Socket * g_socket = 0;


void E15_OnExit(GtkWidget *object, gpointer user_data)
{
	if (g_client)
		g_client->Stop();
	if (g_socket)
		g_socket->Stop();
	E15_UI_Stop();
	//SaveDiagramInstruments();

	delete g_client;

	delete g_socket;

	g_client = 0;
	gtk_main_quit();
}

void InitClient(GtkWidget *widget, gpointer user_data)
{
	g_socket = new E15_Socket;
	g_socket->Start();

	g_client = new MD_Client;
	g_client->Start(g_socket);

	E15_ClientInfo client;
	memset(&client, 0, sizeof(E15_ClientInfo));

	E15_Ini ini; //
	ini.Read("./ini/server.ini");
	//历史文件获取
	ini.SetSection("history");
	const char * addr = ini.ReadString("addr", "127.0.0.1");
	int port = 80;
	ini.Read("port", port);

	client.name = ini.ReadString("user", "test");
	client.pwd = ini.ReadString("password", "123456");

	client.role = "history";
	g_client->Login(&client, "", addr, port);
	*g_history_srv = client.id;
	//行情指标服务器地址

	ini.SetSection("realtime");
	int cnt = ini.GetChildSectionCount("server");

	client.role = "view";
	int i;
	for (i = 0; i < cnt; i++)
	{
		ini.ToChildSection("server", i);

		addr = ini.ReadString("addr", "127.0.0.1");

		port = 9001 + i;
		ini.Read("port", port);


		client.name = ini.ReadString("user", "test");
		client.pwd = ini.ReadString("password", "123456");

		g_client->Login(&client, "", addr, port);

		ini.ToParentSection();
	}
}

GtkWidget * E15_create_message();
GtkWidget * Create_Contract_Listview();
GtkWidget * Create_Market_View();
GtkWidget * Create_Diagram_ListView();
GtkWidget * Create_Speed_Listview();

void Init_Draw_Helper();
void InitFonts();
int main(int argc, char *argv[])
{
	GtkWidget *win = NULL; //主窗口
	GtkWidget *speed_win = NULL; //主窗口

	GtkWidget * notepad;
	GtkWidget * paned_v;      //上下分割显示,上面为图形区域/合约列表，下面为文字输出
	GtkWidget * paned_h;

	GtkWidget * msg;        //文字消息显示窗口

	GtkWidget * contracts_list;

	GtkWidget * speed_view;
	GtkWidget * diagram_view;
	GtkWidget *tab_label;

	/* Initialize GTK+ */
	g_log_set_handler("Gtk", G_LOG_LEVEL_WARNING, (GLogFunc) gtk_false, NULL);
	gtk_init(&argc, &argv);
	g_log_set_handler("Gtk", G_LOG_LEVEL_WARNING, g_log_default_handler, NULL);

	InitFonts();
	InitDrawConfig();

	E15_UI_Start();
	notepad = gtk_notebook_new();

	/*
	 market_view = Create_Market_View();
	 tab_label = gtk_label_new ("自选行情");
	 gtk_notebook_append_page ( (GtkNotebook *)notepad,market_view,tab_label);
	 */
	diagram_view = Create_Diagram_ListView();
	tab_label = gtk_label_new("行情图表");
	gtk_notebook_append_page((GtkNotebook *) notepad, diagram_view, tab_label);

	contracts_list = Create_Contract_Listview();
	paned_h = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);

	gtk_paned_pack1((GtkPaned *) paned_h, contracts_list, TRUE, FALSE);
	gtk_paned_pack2((GtkPaned *) paned_h, notepad, TRUE, FALSE);

	gtk_paned_set_position((GtkPaned *) paned_h, 120);

	paned_v = gtk_paned_new(GTK_ORIENTATION_VERTICAL);

	gtk_paned_pack1((GtkPaned *) paned_v, paned_h, TRUE, FALSE);

	msg = E15_create_message();
	gtk_paned_pack2((GtkPaned *) paned_v, msg, FALSE, TRUE);

	/* Create the main window */
	win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width(GTK_CONTAINER(win), 8);
	gtk_window_set_title(GTK_WINDOW(win), "合约雷达");
	gtk_window_set_position(GTK_WINDOW(win), GTK_WIN_POS_CENTER);
	gtk_widget_realize(win);

	Init_Draw_Helper();

	g_signal_connect(win, "destroy", (GCallback) E15_OnExit, 0);
	g_signal_connect(msg, "realize", (GCallback) InitClient, 0);

	gtk_window_set_default_size(GTK_WINDOW(win), 960, 600);
	/* Enter the main loop */

	gtk_container_add((GtkContainer *) win, paned_v);

	speed_view = Create_Speed_Listview();


	gtk_widget_show_all(win);

	speed_win = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	gtk_container_add((GtkContainer *) speed_win, speed_view);

	gtk_window_set_default_size(GTK_WINDOW(speed_win), 200, 150);
	gtk_window_set_title (GTK_WINDOW(speed_win),"涨速排行榜");
	gtk_window_set_deletable (GTK_WINDOW(speed_win),false);
	gtk_widget_show_all(speed_win);

	E15_Debug::Printf(1, "启动完成...\n");
	gtk_main();

	return 0;
}

