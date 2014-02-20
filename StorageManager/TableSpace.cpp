#include "stdafx.h"
#include "TableSpace.h"

	TableSpace::TableSpace()
	{
		initializeDatabaseFile(std::ios::in|std::ios::out|std::ios::binary);
	}

	TableSpace::~TableSpace()
	{

	}

	char * TableSpace::getData(long positionInFile, long sizeToRead)
	{

	}

	void TableSpace::CreateDatabaseFile(char * fileName)
	{
		tableSpaceFile.open(fileName,std::fstream::app);
		tableSpaceFile.close();
	}

	void TableSpace::initializeDatabaseFile(std::ios_base::openmode openmode)
	{
		if(tableSpaceFile.is_open()){
			tableSpaceFile.close();
		}
		else
		{
			CreateDatabaseFile(defaultTableSpaceFileName);
		}
		tableSpaceFile.open(defaultTableSpaceFileName,openmode);
	}

	void TableSpace::createSystemBlock(char DatabaseName[256])
	{
		SystemBlock sBlock;
		strcpy_s(sBlock.DatabaseName, DatabaseName);
		sBlock.FirstEmptyBlockId = defaultBlockSize;
		sBlock.Version = DatabaseEngineVersion;

		AllocateBlock(0, (char*)&sBlock);
	}

	void TableSpace::verifyTableSpaceFile()
	{
		if(!tableSpaceFile.is_open())
		{
			initializeDatabaseFile(std::ios::in|std::ios::out|std::ios::binary);
		}
	}

	void TableSpace::AllocateBlock(long dir, char * blockData)
	{
		verifyTableSpaceFile();
		initializeDatabaseFile(std::ios::in|std::ios::out|std::ios::binary);
		tableSpaceFile.seekp(dir,std::ios::beg);
		tableSpaceFile.write(blockData,defaultBlockSize);
	}

	void TableSpace::createTableSpace(char DatabaseName[256])
	{
		initializeDatabaseFile(std::ios::in|std::ios::out|std::ios::binary);
		createSystemBlock(DatabaseName);

		GeneralBlock generalBlock;
		generalBlock.BlockType = Blank;
		generalBlock.TombStone = false;

		for(int i=1;i<defaultBlockCount;i++)
		{
			generalBlock.BlockId = i;
			generalBlock.NextBlockId = i+1;
			AllocateBlock(i*defaultBlockSize,(char*)&generalBlock);
		}

		closeDatabaseFile();
	}
	void TableSpace::closeDatabaseFile()
	{
		tableSpaceFile.close();
	}
