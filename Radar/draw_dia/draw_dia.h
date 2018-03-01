#ifndef __Draw_Dia_H
#define __Draw_Dia_H


#include <cairo/cairo.h>

#include "E15_map.h"

#include "draw_config.h"
#include "data_mgr.h"
#include<QVector>
#include"mydef.h"
struct HintInfo;
#define RESERVE_PIXEL (5)
#define SPACEING (2)

#define SPACEING1 (1)
#define SCALE_WITH (36)
#define MAIN_SPACEING (10)
#define TIP_WIDTH (50)
struct BollDataTag{
    long long up_price;
    long long down_price;
    long long mb_price;//中轨线
    long long close_price;
};
//每种数据特有的配置
struct DiagramView_DataControl
{
	unsigned int item_begin; //起始元素的编号 YYYYMMDDHHMISS
	unsigned int item_end;   //结束
	unsigned int item_focus; //鼠标选中


    long offset[11];    //视图第一个元素在存储队列中位置
    long focus;     //选中的元素位置

	int width_unit; //界面元素占用屏幕像素的宽度(原始)
	int unit_offset; //界面元素间距离

	short redraw_by_data_change;  //全图重绘,否则只绘制鼠标和对应元素的文字信息
	int auto_scroller;  					//当数据超出显示范围后是否自动卷动屏幕

	__int64 value_max;						//视图中元素价格最大值
	__int64 value_min;						//视图中元素价格最小值


    int diff_max ;
    int bar_max;
    int dea_max;
    long long volume_max;//交易量最大值
    long long OpenInterest_max = 0;//持仓量最大值
    long long OpenInterest_min = 0;//持仓量最小值
    bool is_line_drawing;
    __int64 cci_max = 0;
    __int64 cci_min = 0;
//    HintInfo hint;//涨速提醒

};

struct MarketDataDraw_Helper;

//显示视图对应与数据的配对
class MarketMVC
{
public:
	ContractInfo *info;										//合约的信息（ID,名称等）
	DiagramView_DataControl data_control;	//与绘制相关的参数

	DiagramDataFactory *factory;					//数据指标集合

	DiagramDataHandler *handler;					//当前正在绘制的指标数据类型
	MarketDataDraw_Helper *draw_helper;		//当前数据绘制器
	int _data_type;												//数据类型，对应其在数据工厂中的序号
	int _sub_type;												//数据子类型，例如成交量或者MACD

public:
	MarketMVC();
	virtual ~MarketMVC();
};

class DiagramView_Control;

struct MarketDataDraw_Helper: public E15_QueueItem
{
	void (*draw_callback)(DiagramView_Control *view, MarketMVC * mvc);	//绘制主图
	void (*draw_focus)(DiagramView_Control *view, MarketMVC * mvc);			//绘制鼠标所在位置的元素信息
	int (*draw_mode_set)(DiagramView_Control *view, MarketMVC * mvc, int mode); //改变内部显示模式，如成交量/macd切换等

	int item_width;
	DrawHelper_Config draw_config;
};

extern E15_Strmap * g_draw_helper_hash;

//显示的部分，用来处理绘制
struct DiagramView_Control
{
	cairo_t *cr;        //all
    cairo_t *cr_all;

	cairo_t *cr_main;
    double height_main;
    int width_main;

    cairo_t *cr_time;
    double height_time;

    cairo_t *cr_macd;
    double height_macd;

    cairo_t *cr_volume;
    double height_volume;

    cairo_t *cr_scale;
    double height_scale;
    int width_scale;

    cairo_t *cr_tip;
    double height_tip;
    int width_tip;

	cairo_t *cr_title;

    double width;
	int height;

	int mouse_in;
	long mouse_x;
	long mouse_y;
    long begin_mouse_y;
    long begin_mouse_x;

	int zoom_in; //放大倍数
	int zoom_out; //缩小倍数
    int zoom_in_out;
    bool has_trade;//存在交易
    int kavg_show;//显示均线
    QVector <int> m_strategy_list;//策略列表
    int rebuild;//
    bool m_paintEventFlag;

    bool mouse_press;
    int index;									//该数据对应的显示视图编号
    bool has_select ;
    double main_precent_height;
    double volume_precent_height;
    double macd_precent_height;

        int macd_type = 0;//0 volume 1 macd 2CCI
        int volume_type = 0;
};

class DiagramDataMgr;

struct DiagramDrawCallbackData
{
	DiagramDataMgr * data;			//行情数据的存储对象
	MarketMVC * mvc;						//显示模型
	DiagramView_Control view;		//绘图控制参数
	MarketMVC mvc_obj;					//显示模型的真实存储空间，对应mvc变量

    double view_w;								//视图的宽度
    long m_focus_x;		//焦点元素信息在文字栏中的开始显示横坐标

	cairo_surface_t *pixbuf_main; //主数据绘图缓存，避免非数据改变引起的大量重绘
    cairo_t * cr_main;
    double h_main;
    double w_main;

    cairo_surface_t *pixbuf_time;  //主数据绘图缓存，避免非数据改变引起的大量重绘
    cairo_t * cr_time;
    double h_time;

    cairo_surface_t *pixbuf_macd;  //主数据绘图缓存，避免非数据改变引起的大量重绘
    cairo_t * cr_macd;
    double h_macd;

    cairo_surface_t *pixbuf_volume;  //主数据绘图缓存，避免非数据改变引起的大量重绘
    cairo_t * cr_volume;
    double h_volume;


    cairo_surface_t *pixbuf_scale;  //主数据绘图缓存，避免非数据改变引起的大量重绘
    cairo_t * cr_scale;
    double h_scale;
    double w_scale;

	cairo_surface_t *pixbuf_title; //标题栏
	cairo_t * cr_title;
    double h_title;

    cairo_surface_t *pixbuf_tip;  //主数据绘图缓存，避免非数据改变引起的大量重绘
    cairo_t * cr_tip;
    double h_tip;
    double w_tip;

	E15_String tempstr;
    E15_String type_string;


//	int zoom_in_out;						//当前视图放大缩小状态(小图 or 全屏)
	E15_String m_ins_id;				//当前视图绑定的合约id

	void * frame;								//视图的父窗口
	void * event_box;						//视图的事件触发器
	void * draw_area;						//视图句柄

//    bool valid;//有效窗口

};

DiagramDrawCallbackData * Create_DiagramDrawCallbackData(int index);
void Delete_DiagramDrawCallbackData(DiagramDrawCallbackData*cbdata);

int draw_dia(cairo_t *cr, int w,int h,double scale,DiagramDrawCallbackData * cbdata);

int dia_on_key_page_down(DiagramDrawCallbackData * cbdata,int ctrl_flag);
int dia_on_key_page_up(DiagramDrawCallbackData *cbdata ,int ctrl_flag);
int dia_on_key_down(DiagramDrawCallbackData * cbdata,int ctrl_flag);
int dia_on_key_up(DiagramDrawCallbackData * cbdata,int ctrl_flag);
int dia_on_key_left(DiagramDrawCallbackData *cbdata,int ctrl_flag );
int dia_on_key_right(DiagramDrawCallbackData * cbdata,int ctrl_flag);
int dia_on_key_home(DiagramDrawCallbackData * cbdata,int ctrl_flag);
int dia_on_key_end(DiagramDrawCallbackData * cbdata,int ctrl_flag);
int dia_on_key_ch(DiagramDrawCallbackData * cbdata,int ch,int ctrl_flag);

int dia_on_mouse_move(DiagramDrawCallbackData * cbdata,long x,long y);

void dia_load_history(const char * id,DiagramDataHandler * h,int max_item,int win_id);

#endif
