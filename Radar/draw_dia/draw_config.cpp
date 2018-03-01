#include <string.h>

#include "E15_ini.h"
#include "E15_debug.h"
#include "E15_finder.h"
#include "E15_file.h"

#include "E15_string_array.h"

#include "draw_config.h"

E15_Queue g_draw_config_list;
E15_Strmap g_draw_conf_hash_obj;
E15_Strmap * g_draw_conf_hash = &g_draw_conf_hash_obj;

DrawFontConf g_draw_font;

void ReadColor(E15_Ini * ini, DrawRGBA * color, const char * key,
		const char * def)
{
	E15_StringArray sa;
	const char * data = ini->ReadString(key, def);
	sa.Split(data, ',', 0);
	if (sa.Size() != 4)
	{
		color->red = 1.0;
		color->green = 1.0;
		color->blue = 1.0;
		color->alpha = 1.0;
		return;
	}
	color->red = sa.At(0)->ToFloat();
	color->green = sa.At(1)->ToFloat();
	color->blue = sa.At(2)->ToFloat();
	color->alpha = sa.At(3)->ToFloat();
}

void InitFonts()
{
	E15_Ini ini;
	ini.Read("./ini/font.ini");

	ini.SetSection("font.ID");
	ini.Read("size", g_draw_font.id.Size);
	ReadColor(&ini, &g_draw_font.id.Color, "color", "1.0,1.0,1.0,1.0");

	ini.SetSection("font.price");
	ini.Read("size", g_draw_font.price.Size);
	ReadColor(&ini, &g_draw_font.price.Color, "color", "1.0,1.0,1.0,1.0");

	ini.SetSection("font.tip");
	ini.Read("size", g_draw_font.tip.Size);
	ReadColor(&ini, &g_draw_font.tip.Color, "color", "1.0,1.0,1.0,1.0");

	ini.SetSection("font.open_buy");
	ini.Read("size", g_draw_font.open_buy.Size);
	ReadColor(&ini, &g_draw_font.open_buy.Color, "color", "1.0,1.0,1.0,1.0");

	ini.SetSection("font.open_sale");
	ini.Read("size", g_draw_font.open_sale.Size);
	ReadColor(&ini, &g_draw_font.open_sale.Color, "color", "1.0,1.0,1.0,1.0");

	ini.SetSection("font.close_buy");
	ini.Read("size", g_draw_font.close_buy.Size);
	ReadColor(&ini, &g_draw_font.close_buy.Color, "color", "1.0,1.0,1.0,1.0");

	ini.SetSection("font.close_sale");
	ini.Read("size", g_draw_font.close_sale.Size);
	ReadColor(&ini, &g_draw_font.close_sale.Color, "color", "1.0,1.0,1.0,1.0");
}

DrawHelper_Config::DrawHelper_Config()
{
	show_hide = 1;
	mode = 0;
	line = 1;
	line_width = 0.6;
	diameter = 3;
	dash = 0;

	color.red = 1.0;
	color.green = 1.0;
	color.blue = 1.0;
	color.alpha = 1.0;

	color_fill.red = 0;
	color_fill.green = 0;
	color_fill.blue = 0;
	color_fill.alpha = 0;

	color_point.red = 1.0;
	color_point.green = 1.0;
	color_point.blue = 1.0;
	color_point.alpha = 1.0;

	color_line.red = 1.0;
	color_line.green = 1.0;
	color_line.blue = 1.0;
	color_line.alpha = 1.0;

}

DrawHelper_Config::~DrawHelper_Config()
{

}

DrawHelper_Config & DrawHelper_Config::operator =(
		const DrawHelper_Config & conf)
{
	if (this == &conf)
		return *this;

	show_hide = conf.show_hide;
	mode = conf.mode;
	line = conf.line;
	line_width = conf.line_width;
	diameter = conf.diameter;

	color = conf.color;
	color_fill = conf.color_fill;
	color_point = conf.color_point;
	color_line = conf.color_line;
	dash = conf.dash;
	return *this;
}

////////////////////////////////////////////

int ReadCnfColor(E15_Ini * ini, const char * key, DrawRGBA * color)
{
	unsigned int c = 0;
	if (!ini->Read(key, c))
		return 0;

	color->red = (float) ((c >> 24) & 0xff) / 0xff;
	color->green = (float) ((c >> 16) & 0xff) / 0xff;
	color->blue = (float) ((c >> 8) & 0xff) / 0xff;
	color->alpha = (float) ((c) & 0xff) / 0xff;

	return 1;
}

void LoadDrawConfig(E15_Ini * ini, DrawHelper_Config * info)
{
	ini->Read("show", info->show_hide);
	ini->Read("mode", info->mode);

	int ret = ini->Read("line", info->line_width);
	if (ret)
		info->line = 1;
	else
		info->line = 0;

	ini->Read("dash", info->dash);

	ini->Read("diameter", info->diameter);

	if (ReadCnfColor(ini, "color", &info->color))
	{
		info->color_point = info->color;
		info->color_line = info->color;
	}

	ReadCnfColor(ini, "color_point", &info->color_point);
	ReadCnfColor(ini, "color_line", &info->color_line);
	ReadCnfColor(ini, "color_fill", &info->color_fill);
}

void InitDefaultDrawConfig()
{
	DrawHelper_Config * info = new DrawHelper_Config;
	g_draw_config_list.PutTail(info);
	g_draw_conf_hash->SetAtSI(info, "::default", 0);

	E15_Ini ini;
	ini.Read("./ini/draw.ini");
	ini.SetSection("draw_config");
	LoadDrawConfig(&ini, info);
}

void InitDrawConfig(bool flag)
{
    g_draw_config_list.Clear();
    g_draw_conf_hash->RemoveAll();

	InitDefaultDrawConfig();

	E15_Finder flist;
	E15_File * draw_file;

	E15_Ini ini;
    if(flag)
        flist.Find("./ini/draw/*.ini");
    else
        flist.Find("./ini/draw/default/*.ini");
	draw_file = flist.First();

	int i;
	const char * name;
	long type = 0;
	DrawHelper_Config * conf;

	DrawHelper_Config * def = (DrawHelper_Config*) g_draw_config_list.Head(0);

	while (draw_file)
	{
		if (draw_file->IsDir())
		{
			draw_file = flist.Next();
			continue;
		}

		ini.Read(draw_file->FullName());
		if (!ini.SetSection("draw_config"))
		{
			draw_file = flist.Next();
			continue;
		}

		int c = ini.GetChildSectionCount("item");

		for (i = 0; i < c; i++)
		{
			ini.ToChildSection("item", i);

			do
			{
				name = ini.ReadString("name", 0);
				if (!name)
					break;

				if (!ini.Read("type", type))
					break;

				conf = (DrawHelper_Config *) g_draw_conf_hash->LookupSI(name,type);
				if (conf)
					break;

				conf = new DrawHelper_Config;
				g_draw_config_list.PutTail(conf);
				g_draw_conf_hash->SetAtSI(conf, name, type);
				*conf = *def;

				LoadDrawConfig(&ini, conf);
			} while (0);

			ini.ToParentSection();
		}
		draw_file = flist.Next();
	}
}

