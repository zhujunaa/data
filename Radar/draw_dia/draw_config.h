#ifndef __draw_config_H
#define __draw_config_H

#include "E15_queue.h"
#include "E15_string.h"
#include "E15_map.h"

struct DrawRGBA
{
	double red;
	double green;
	double blue;
	double alpha;
};

struct FontConf
{
    const char * Name;
    int         Size;
    DrawRGBA     Color;
};

struct DrawFontConf
{
    FontConf  id; //合约ID的字体
    FontConf  price; //价格
    FontConf  tip;  //动态提示

    FontConf  open_buy;//开仓买入提示
    FontConf  open_sale;//开仓卖出提示

    FontConf  close_buy;//平仓买入提示
    FontConf  close_sale;//开仓卖出提示
};

class DrawHelper_Config : public E15_QueueItem
{
public:
	DrawHelper_Config();
	virtual ~DrawHelper_Config();

	short  show_hide;
	short  mode;
	char   line;
	short	 dash;

	float  line_width;
	short  diameter;

	DrawRGBA color;
	DrawRGBA color_fill;
	DrawRGBA color_point;
	DrawRGBA color_line;

	DrawHelper_Config & operator = (const DrawHelper_Config & data);


};

void InitFonts();
void InitDrawConfig(bool);

extern DrawFontConf g_draw_font;
extern E15_Strmap * g_draw_conf_hash;

#endif
