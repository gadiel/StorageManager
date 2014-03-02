#include "stdafx.h"
#include "TableSpace.h"

	TableSpace::TableSpace()
	{
		InitializeDatabaseFile(std::ios::in|std::ios::out|std::ios::binary);
	}

	TableSpace::~TableSpace()
	{

	}

	char * TableSpace::GetData(long positionInFile, long sizeToRead)
	{
		return NULL;
	}

	void TableSpace::CreateDatabaseFile(char * fileName)
	{
		tableSpaceFile.open(fileName,std::fstream::app);
		tableSpaceFile.close();
	}

	void TableSpace::InitializeDatabaseFile(std::ios_base::openmode openmode)
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

	void TableSpace::CreateSystemBlock(char DatabaseName[256])
	{
		SystemBlock sBlock;
        strcpy(sBlock.DatabaseName, DatabaseName);
		sBlock.FirstEmptyBlockId = 1;
		sBlock.Version = DatabaseEngineVersion;
		
        AllocateBlock(0, (char*)&sBlock);
	}

    char* TableSpace::GetSystemBlock(){
        InitializeDatabaseFile(std::ios::in|std::ios::out|std::ios::binary);

        char buffer[sizeof(SystemBlock)];
        tableSpaceFile.seekp(0,std::ios::beg);
        tableSpaceFile.read(buffer,sizeof(SystemBlock));

        return buffer;
    }

    bool TableSpace::UpdateSystemBlock(char* newSystemBlock){

        AllocateBlock(0,newSystemBlock);
        //CloseDatabaseFile();
        return true;
    }

	void TableSpace::VerifyTableSpaceFile()
	{
		if(!tableSpaceFile.is_open())
		{
			InitializeDatabaseFile(std::ios::in|std::ios::out|std::ios::binary);
		}
	}

	void TableSpace::AllocateBlock(long dir, char * blockData)
	{
        //VerifyTableSpaceFile();
		InitializeDatabaseFile(std::ios::in|std::ios::out|std::ios::binary);
		tableSpaceFile.seekp(dir,std::ios::beg);		
        tableSpaceFile.write(blockData,defaultBlockSize);
	}

	void TableSpace::CreateTableSpace(char DatabaseName[256])
	{
		InitializeDatabaseFile(std::ios::in|std::ios::out|std::ios::binary);
		CreateSystemBlock(DatabaseName);

		GeneralBlock generalBlock;
		generalBlock.BlockType = Blank;
		generalBlock.TombStone = false;

		for(int i=1;i<defaultBlockCount;i++)
		{
			generalBlock.BlockId = i;
			generalBlock.NextBlockId = i+1;

            AllocateBlock(i*defaultBlockSize,(char*)&generalBlock);
		}

		CloseDatabaseFile();
	}
	
	void TableSpace::CloseDatabaseFile()
	{
		tableSpaceFile.close();
	}
