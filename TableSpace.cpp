#include "stdafx.h"
#include "TableSpace.h"

	TableSpace::TableSpace()
	{
		InitializeDatabaseFile(std::ios::in|std::ios::out|std::ios::binary);
	}

    TableSpace::~TableSpace()
	{

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
        char buffer[sizeof(SystemBlock)];
        tableSpaceFile.seekg(0,std::ios::beg);
        tableSpaceFile.read(buffer,sizeof(SystemBlock));

        return buffer;
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
        tableSpaceFile.seekp(dir,std::ios::beg);
        tableSpaceFile.write(blockData,defaultBlockSize);
        tableSpaceFile.flush();
	}

	void TableSpace::CreateTableSpace(char DatabaseName[256])
	{
        CreateSystemBlock(DatabaseName);

        GeneralHeader generalBlock;
		generalBlock.BlockType = Blank;
		generalBlock.TombStone = false;

		for(int i=1;i<defaultBlockCount;i++)
		{
			generalBlock.BlockId = i;
            generalBlock.PreviousBlockId = i-1;
			generalBlock.NextBlockId = i+1;

            char charBlock[sizeof(GeneralHeader)];
            memcpy(charBlock, &generalBlock, sizeof(GeneralHeader));

            writeData(i*defaultBlockSize, charBlock, sizeof(GeneralHeader));
		}
        tableSpaceFile.flush();
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


    //Gadiel
    long TableSpace::getNextFreeBlockAndUseIt()
    {
        char * rawSysBlock = GetSystemBlock();
        SystemBlock sysBlock;
        memcpy(&sysBlock, rawSysBlock, sizeof(SystemBlock));

        long emptyBlockID = sysBlock.FirstEmptyBlockId;
        char * rawGenBlock = GetGeneralHeaderData(emptyBlockID);

        GeneralHeader generalBlock;
        memcpy(&generalBlock, rawGenBlock, sizeof(GeneralHeader));

        sysBlock.FirstEmptyBlockId = generalBlock.NextBlockId;
        memcpy(rawSysBlock, &sysBlock, sizeof(SystemBlock));

        UpdateSystemBlock(rawSysBlock);

        return emptyBlockID;
    }

    long TableSpace::addNewBlock(long id)
    {
        return 0;
    }

    //Gadiel
    long TableSpace::addNewBlock()
    {
        char * rawSysBlock = GetSystemBlock();
        SystemBlock sysBlock;
        memcpy(&sysBlock, rawSysBlock, sizeof(SystemBlock));
        GeneralHeader generalBlock;
        generalBlock.NextBlockId = NULL;
        generalBlock.TombStone = false;
        generalBlock.BlockType = Blank;
        generalBlock.BlockId = sysBlock.PhysicalBlockCount+1;
        generalBlock.PreviousBlockId = sysBlock.LastEmptyBlockId;

        GeneralHeader lastBlock;
        char * rawLastBlock = GetGeneralHeaderData(sysBlock.LastEmptyBlockId);
        memcpy(&lastBlock, rawLastBlock, sizeof(GeneralHeader));
        lastBlock.NextBlockId = generalBlock.BlockId;

        memcpy(rawSysBlock, &sysBlock, sizeof(SystemBlock));
        char charBlock[sizeof(GeneralHeader)];
        memcpy(charBlock, &generalBlock, sizeof(GeneralHeader));
        memcpy(rawLastBlock, &lastBlock, sizeof(GeneralHeader));

        sysBlock.PhysicalBlockCount+=1;
        sysBlock.LastEmptyBlockId = generalBlock.BlockId;

        writeData(lastBlock.BlockId*defaultBlockSize,rawLastBlock,sizeof(GeneralHeader));
        writeData(generalBlock.BlockId*defaultBlockSize,charBlock, sizeof(GeneralHeader));
        writeData(0, rawSysBlock, sizeof(SystemBlock));
    }

    char * TableSpace::GetGeneralHeaderData(long blockId)
    {
        char buffer[sizeof(GeneralHeader)];
        tableSpaceFile.seekg(blockId*defaultBlockSize,std::ios::beg);
        tableSpaceFile.read(buffer,sizeof(GeneralHeader));
        return buffer;
    }

    void TableSpace::writeData(long dir, char * data, int size)
    {
        tableSpaceFile.seekp(dir,std::ios::beg);
        tableSpaceFile.write(data,size);
        tableSpaceFile.flush();
    }

    long TableSpace::getLastTableMetadataBlockId(long BlockID)
    {
        if(BlockID==0)
        {
            return 0;
        }

        char * rawPreviousGenBlock = GetGeneralHeaderData(BlockID);
        GeneralHeader genBlock;
        memcpy(&genBlock, rawPreviousGenBlock, sizeof(GeneralHeader));
        if(genBlock.BlockType!=TableMetadata)
        {
            return 0;
        }

        if(genBlock.NextBlockId==0)
        {
            return BlockID;
        }

        return getLastTableMetadataBlockId(genBlock.NextBlockId);
    }

    long TableSpace::CreateNewTable(TableMetadataHeader tableMetadata)
    {
        char * rawSysBlock = GetSystemBlock();
        SystemBlock sysBlock;
        memcpy(&sysBlock, rawSysBlock, sizeof(SystemBlock));

        long emptyBlockID = getNextFreeBlockAndUseIt();
        char * rawGenBlock = GetGeneralHeaderData(emptyBlockID);
        GeneralHeader generalBlock;
        memcpy(&generalBlock, rawGenBlock, sizeof(GeneralHeader));

        generalBlock.BlockType = TableMetadata;
        generalBlock.NextBlockId = 0;
        generalBlock.PreviousBlockId = getLastTableMetadataBlockId(sysBlock.FirstTableMetadataBlockId);

        memcpy(rawGenBlock,&generalBlock,sizeof(GeneralHeader));

        writeData(emptyBlockID*defaultBlockSize,rawGenBlock,sizeof(GeneralHeader));
        long metadataDir = emptyBlockID*defaultBlockSize+sizeof(GeneralHeader);
        char rawMetadataBlock[sizeof(TableMetadataHeader)];
        memcpy(rawMetadataBlock,&tableMetadata,sizeof(GeneralHeader));
        writeData(metadataDir,rawMetadataBlock,sizeof(TableMetadataHeader));
        return emptyBlockID;
    }


    //Wendy
    bool TableSpace::UpdateSystemBlock(char* newSystemBlock)
    {
        writeData(0,newSystemBlock,sizeof(SystemBlock));
        return true;
    }

    char * TableSpace::GetData(long positionInFile, long sizeToRead)
    {
        char* buffer;
        tableSpaceFile.seekg(positionInFile,std::ios::beg);
        tableSpaceFile.read(buffer,sizeToRead);

        return buffer;
    }

    char* TableSpace::GetTableMetadataHeader(long blockId){
          int posInicial= (blockId*defaultBlockSize)+(sizeof(GeneralHeader));
          return GetData(posInicial,sizeof(TableMetadataHeader));
      }

    bool TableSpace::UpdateMetadataField(long blockId,long fieldId, char* newMetadataField){

          int offset=(blockId*defaultBlockSize)+sizeof(GeneralHeader)+sizeof(TableMetadataHeader)+(fieldId*sizeof(MetadataField));
          writeData(offset,newMetadataField,sizeof(MetadataField));
          return true;
    }

