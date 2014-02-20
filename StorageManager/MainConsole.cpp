// StorageManager.cpp: define el punto de entrada de la aplicación de consola.
#include "stdafx.h"
#include "TableSpace.cpp"

int _tmain(int argc, _TCHAR* argv[])
{
	/*std::fstream tableSpaceFile;
	tableSpaceFile.open("prueba.db",std::fstream::app);
	if(!tableSpaceFile.is_open())
	{
		SystemBlock sBlocsk;
		strcpy_s(sBlocsk.DatabaseName, "holis");
	}
	else
	{
		SystemBlock sBlocsk;
		strcpy_s(sBlocsk.DatabaseName, "holis");
	}
	tableSpaceFile.seekp(0,std::fstream::beg);
	SystemBlock sBlock;
	strcpy_s(sBlock.DatabaseName, "holis");
	sBlock.FirstEmptyBlockId = defaultBlockSize;
	sBlock.Version = DatabaseEngineVersion;

	tableSpaceFile.write((char*)&sBlock,sizeof(sBlock));
	tableSpaceFile.close();*/
	
	TableSpace tbspace;
	tbspace.createTableSpace("prueba1");
	return 0;
}