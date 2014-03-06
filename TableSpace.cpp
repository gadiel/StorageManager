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


//        char buffer[sizeof(SystemBlock)];
//        tableSpaceFile.seekp(0,std::ios::beg);
//        tableSpaceFile.read(buffer,sizeof(SystemBlock));

        return GetData(0,sizeof(SystemBlock));

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

        GeneralHeader generalHeader;
        generalHeader.BlockType = Blank;
        generalHeader.TombStone = false;

		for(int i=1;i<defaultBlockCount;i++)
		{
            generalHeader.BlockId = i;
            generalHeader.NextBlockId = i+1;

            AllocateBlock(i*defaultBlockSize,(char*)&generalHeader);
		}

		CloseDatabaseFile();
	}
	
    char* TableSpace::GetTableMetadataHeader(long blockId){
        int posInicial= (blockId*defaultBlockSize)+(sizeof(GeneralHeader));
        return GetData(posInicial,sizeof(TableMetadataHeader));
    }

    char* TableSpace::GetTableMetadataBlock(long blockId){
        char* tableMetadataHeader=GetTableMetadataHeader(blockId);

        TableMetadataHeader header;
        memcpy(&header, tableMetadataHeader, sizeof(TableMetadataHeader));

        int tamanioBuffer=(header.LogicalColumnsCount*sizeof(MetadataField))+sizeof(TableMetadataHeader);

        char buffer[tamanioBuffer];

        for(int i=1;i<=header.PhysicalColumnsCount;i++){
            //GetMetadataField(long blockId, long fieldId);

            //if(!field.IsDeleted){
            //  append to buffer
            //}
        }

        return NULL;

    }

    bool TableSpace::UpdateMetadataField(long blockId,long fieldId, char* newMetadataField){

        int offset=(blockId*defaultBlockSize)+sizeof(GeneralHeader)+sizeof(TableMetadataHeader)+(fieldId*sizeof(MetadataField));
        AllocateBlock(offset,newMetadataField);
        return true;
    }

	void TableSpace::CloseDatabaseFile()
	{
		tableSpaceFile.close();
	}
