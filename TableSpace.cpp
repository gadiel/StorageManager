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
        char buffer[sizeToRead];
        tableSpaceFile.seekp(positionInFile,std::ios::beg);
        tableSpaceFile.read(buffer,sizeToRead);

        return buffer;
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

    void TableSpace::CreateSystemBlock(char DatabaseName[64])
	{
		SystemBlock sBlock;
        strcpy(sBlock.DatabaseName, DatabaseName);
		sBlock.FirstEmptyBlockId = 1;
        sBlock.FirstMetadataBlockId = 0;
		sBlock.Version = DatabaseEngineVersion;
		
        char charBlock[sizeof(SystemBlock)];
        memcpy(charBlock, &sBlock, sizeof(SystemBlock));

        AllocateBlock(0, charBlock);
	}

    char* TableSpace::GetSystemBlock(){
        //InitializeDatabaseFile(std::ios::in|std::ios::out|std::ios::binary);

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
        //InitializeDatabaseFile(std::ios::in|std::ios::out|std::ios::binary);
		tableSpaceFile.seekp(dir,std::ios::beg);		
        tableSpaceFile.write(blockData,defaultBlockSize);
        tableSpaceFile.flush();
	}

	void TableSpace::CreateTableSpace(char DatabaseName[256])
	{
        //InitializeDatabaseFile(std::ios::in|std::ios::out|std::ios::binary);
		CreateSystemBlock(DatabaseName);

		GeneralBlock generalBlock;
		generalBlock.BlockType = Blank;
		generalBlock.TombStone = false;

		for(int i=1;i<defaultBlockCount;i++)
		{
			generalBlock.BlockId = i;
            generalBlock.PreviousBlockId = i-1;
			generalBlock.NextBlockId = i+1;

            char charBlock[sizeof(GeneralBlock)];
            memcpy(charBlock, &generalBlock, sizeof(GeneralBlock));


            AllocateBlock(i*defaultBlockSize, charBlock);
		}
        tableSpaceFile.flush();
        //CloseDatabaseFile();
	}
	
	void TableSpace::CloseDatabaseFile()
	{
		tableSpaceFile.close();
	}

    long TableSpace::getNextFreeBlock()
    {
        char* systemC=tbspace.GetSystemBlock();

        SystemBlock system;
        memcpy(&system, systemC, sizeof(SystemBlock));
        return system.FirstEmptyBlockId();
    }



