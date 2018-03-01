#include "E15_ini.h"


void replace_view_data_index(const char * id,int index,int data_type,int data_level,int zoom);

void LoadDiagramInstruments()
{
	E15_Ini   ini_file;
	E15_Ini * ini = &ini_file;
	ini->Read("./ini/diagram.ini");

	ini->SetSection("diagram");
	int c = ini->GetChildSectionCount("view");
	int i;

	const char * id ;
	int data_type;
	int data_level;
	int zoom;
	int pos;

	//首先清理原有的数据
	for(i=0; i< 9; i++)
	{
		if( !ini->ToChildSection("view",i) )
			continue;

		data_type = 0;
		data_level = 0;
		zoom = 0;
		pos = -1;

		id = ini->ReadString("id",0);
		ini->Read("data_type",data_type);
		ini->Read("data_level",data_level);
		ini->Read("zoom",zoom);
		ini->Read("pos",pos);

		replace_view_data_index(id,pos,data_type,data_level,zoom);

		ini->ToParentSection();
	}

	for(i=0; i< c; i++)
	{
		if( !ini->ToChildSection("view",i) )
			continue;

		data_type = 0;
		data_level = 0;
		zoom = 0;
		pos = -1;

		id = ini->ReadString("id",0);
		ini->Read("data_type",data_type);
		ini->Read("data_level",data_level);
		ini->Read("zoom",zoom);
		ini->Read("pos",pos);

		replace_view_data_index(id,pos,data_type,data_level,zoom);

		ini->ToParentSection();
	}
}


