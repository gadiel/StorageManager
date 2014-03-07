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
        tableSpaceFile.seekg(positionInFile,std::ios::beg);
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
        sBlock.FirstTableMetadataBlockId = 0;
		sBlock.Version = DatabaseEngineVersion;
		
        char charBlock[sizeof(SystemBlock)];
        memcpy(charBlock, &sBlock, sizeof(SystemBlock));

        writeData(0, charBlock,sizeof(SystemBlock));
	}

    char* TableSpace::GetSystemBlock(){
        //InitializeDatabaseFile(std::ios::in|std::ios::out|std::ios::binary);

        char buffer[sizeof(SystemBlock)];
        tableSpaceFile.seekg(0,std::ios::beg);
        tableSpaceFile.read(buffer,sizeof(SystemBlock));

        return buffer;
    }

    bool TableSpace::UpdateSystemBlock(char* newSystemBlock){

        writeData(0,newSystemBlock,sizeof(SystemBlock));
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

    void TableSpace::WriteBlock(long dir, char * blockData)
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

            writeData(i*defaultBlockSize, charBlock, sizeof(GeneralBlock));
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
        char* systemC=GetSystemBlock();

        SystemBlock system;
        memcpy(&system, systemC, sizeof(SystemBlock));

        return system.FirstEmptyBlockId;
    }

    long TableSpace::getNextFreeBlockAndUseIt()
    {
        char * rawSysBlock = GetSystemBlock();
        SystemBlock sysBlock;
        memcpy(&sysBlock, rawSysBlock, sizeof(SystemBlock));

         long emptyBlockID = sysBlock.FirstEmptyBlockId;

        sysBlock.FirstEmptyBlockId = generalBlock.NextBlockId;
        memcpy(rawSysBlock, &sysBlock, sizeof(SystemBlock));
        UpdateSystemBlock(rawSysBlock);

        return emptyBlockID;
    }

    char * TableSpace::GetGeneralBlockData( long blockId)
    {
        char buffer[sizeof(GeneralBlock)];
        tableSpaceFile.seekg(blockId*defaultBlockSize,std::ios::beg);
        tableSpaceFile.read(buffer,sizeof(GeneralBlock));
        return buffer;
    }

    long TableSpace::writeTableMetadata( long blockID,char * tableMetadata)
    {

    }

    void TableSpace::writeData(long dir, char * data, int size)
    {
        tableSpaceFile.seekp(dir,std::ios::beg);
        tableSpaceFile.write(data,size);
        tableSpaceFile.flush();
    }

    long TableSpace::getLastTableMetadataBlockId(long firstMDId)
    {
        char * rawPreviousGenBlock = GetGeneralBlockData(firstMDId);
        GeneralBlock genBlock;
        memcpy(&genBlock, rawPreviousGenBlock, sizeof(GeneralBlock));
        if(genBlock.BlockType!=TableMetadata)
        {
            return NULL;
        }

        long lastMDblockId = genBlock.FirstTableMetadataBlockId;

        while(genBlock.NextBlockId !=0)
        {
            lastMDblockId = genBlock.BlockId;
            rawPreviousGenBlock = GetGeneralBlockData(genBlock.NextBlockId);
            memcpy(&genBlock, rawPreviousGenBlock, sizeof(GeneralBlock));

            if(genBlock.BlockType!=TableMetadata)
            {
                return NULL;
            }
        }
        return lastMDblockId;

    }


    long TableSpace::CreateNewTable(TableMetadataBlock tableMetadata)
    {
        char * rawSysBlock = GetSystemBlock();
        SystemBlock sysBlock;
        memcpy(&sysBlock, rawSysBlock, sizeof(SystemBlock));

        long emptyBlockID = getNextFreeBlockAndUseIt();
        char * rawGenBlock = GetGeneralBlockData(emptyBlockID);
        GeneralBlock generalBlock;
        memcpy(&generalBlock, rawGenBlock, sizeof(GeneralBlock));
        generalBlock.BlockType = TableMetadata;
        generalBlock.NextBlockId = 0;

        if(sysBlock.FirstTableMetadataBlockId==0)
        {
            generalBlock.PreviousBlockId = 0;
        }
        else
        {
            char * rawPreviousGenBlock = GetGeneralBlockData(sysBlock.FirstTableMetadataBlockId);
            GeneralBlock previousGeneralBlock;
            memcpy(&previousGeneralBlock, rawPreviousGenBlock, sizeof(GeneralBlock));
            previousGeneralBlock.NextBlockId = emptyBlockID;

            generalBlock.PreviousBlockId =
        }
    }



