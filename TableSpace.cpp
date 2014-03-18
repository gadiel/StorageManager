#include "stdafx.h"
#include "TableSpace.h"
#include <iostream>

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
        char * rawData = GetSystemBlock();
        SystemBlock sb;
        memcpy(&sb, rawData, sizeof(SystemBlock));
        long freeBlockId = sb.FirstEmptyBlockId;

        rawData = GetGeneralHeader(freeBlockId);
        GeneralHeader gh;
        memcpy(&gh, rawData, sizeof(GeneralHeader));

        sb.FirstEmptyBlockId = gh.NextBlockId;
        memcpy(rawData, &sb, sizeof(SystemBlock));

        UpdateSystemBlock(rawData);

        return freeBlockId;
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
        char * rawLastBlock = GetGeneralHeader(sysBlock.LastEmptyBlockId);
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

    char * TableSpace::GetGeneralHeader(long blockId)
    {
        long offset=blockId*defaultBlockSize;
        std::cout<<"fuck you";
        char * rawData= GetData(offset,sizeof(GeneralHeader));
        std::cout<<"shit";
        return rawData;
    }

    void TableSpace::writeData(long dir, char * data, int size)
    {
        tableSpaceFile.seekp(dir,std::ios::beg);
        tableSpaceFile.write(data,size);
        tableSpaceFile.flush();
    }

    long TableSpace::getLastTableMetadataBlockId()
    {
        return 0;
    }

    //Alex
    long TableSpace::CreateMetadataTable(char name[256]){

        long id= getNextFreeBlockAndUseIt();
        printf("Id: %d",id);
        long lastId = getLastTableMetadataBlockId();
        printf("Last Id: %d",lastId);

        char * rawData= GetGeneralHeader(id);
        std::cout << "Crashea aqui";
        GeneralHeader newBlockGeneralHeader;
        std::cout << "Crashea aqui";
        memcpy(&newBlockGeneralHeader,rawData,sizeof(GeneralHeader));
        std::cout << "Crashea aqui";
        newBlockGeneralHeader.BlockType= TableMetadata;
        std::cout << "Crashea aqui";
        newBlockGeneralHeader.PreviousBlockId=lastId;
        std::cout << "Crashea aqui";

        rawData= GetGeneralHeader(lastId);
        GeneralHeader lastBlockGeneralHeader;
        memcpy(&lastBlockGeneralHeader,rawData,sizeof(GeneralHeader));
        lastBlockGeneralHeader.NextBlockId=id;
        std::cout << "Crashea aqui";

        TableMetadataHeader newBlockMetadataHeader= TableMetadataHeader();
        strcpy(newBlockMetadataHeader.TableName,name);
        newBlockMetadataHeader.FreeFields= (defaultBlockSize-sizeof(GeneralHeader)-sizeof(TableMetadataHeader))/sizeof(MetadataField);
        newBlockMetadataHeader.LogicalColumnsCount=0;
        newBlockMetadataHeader.PhysicalColumnsCount=0;
        newBlockMetadataHeader.NextMetadataExtensionBlockId=0;
        newBlockMetadataHeader.Identity=0;
        std::cout << "Crashea aqui";

        UpdateGeneralHeader(id,newBlockGeneralHeader);
        CreateMetadataTableHeader(id,newBlockMetadataHeader);
        UpdateGeneralHeader(lastId,lastBlockGeneralHeader);
        std::cout << "Crashea aqui";

        return id;
    }

    void TableSpace::CreateMetadataTableHeader(long blockId,TableMetadataHeader header){
        long offset=(defaultBlockSize*blockId)+sizeof(GeneralHeader);
        writeData(offset,(char*)&header,sizeof(TableMetadataHeader));
    }

    void TableSpace::UpdateGeneralHeader(long blockId,GeneralHeader header){
        long offset=defaultBlockSize*blockId;
        writeData(offset,(char*)&header,sizeof(GeneralHeader));
    }

    bool TableSpace::CreateMetadataField(long blockId, char *metadataField){

        TableMetadataHeader header;
        do{

        if(&header!=NULL)
            blockId=header.NextMetadataExtensionBlockId;
        char* tableMetadataHeader=GetTableMetadataHeader(blockId);
        memcpy(&header, tableMetadataHeader, sizeof(TableMetadataHeader));

        }while(header.NextMetadataExtensionBlockId!=0);

        if(header.FreeFields>0)
        {
            long offset=defaultBlockSize*blockId+sizeof(GeneralHeader)+sizeof(TableMetadata)+(sizeof(MetadataField)*header.PhysicalColumnsCount);
            writeData(offset,metadataField,sizeof(MetadataField));

            header.FreeFields--;
            header.LogicalColumnsCount++;
            header.PhysicalColumnsCount++;
            header.Identity++;

            offset=(defaultBlockSize*blockId)+sizeof(GeneralHeader);
            writeData(offset,(char*)&header,sizeof(TableMetadataHeader));
            return true;
        }else
        {
            //Crear Extension de Metadata
        }
    }


    //Alex


    //Wendy
    bool TableSpace::UpdateSystemBlock(char* newSystemBlock)
    {
        writeData(0,newSystemBlock,sizeof(SystemBlock));
        return true;
    }

    char * TableSpace::GetData(long positionInFile, long sizeToRead)
    {
        char buffer[sizeToRead];
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

