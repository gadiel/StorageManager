#include "stdafx.h"
#include "TableSpace.h"
#include <iostream>
using namespace std;

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
        sBlock.FirstEmptyBlockId = 0;
        sBlock.FirstTableMetadataBlockId = 0;
		sBlock.Version = DatabaseEngineVersion;
        sBlock.LastEmptyBlockId=0;
        sBlock.PhysicalBlockCount=0;
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

        for(int i=1;i<=10;i++)
        {
            addNewBlock();
        }
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
        char *rawData = GetData(0,sizeof(SystemBlock));
        SystemBlock sb;
        memcpy(&sb, rawData, sizeof(SystemBlock));
        long freeBlockId = sb.FirstEmptyBlockId;

        rawData = GetGeneralHeader(freeBlockId);
        GeneralHeader gh;
        memcpy(&gh, rawData, sizeof(GeneralHeader));

        sb.FirstEmptyBlockId = gh.NextBlockId;
        memcpy(rawData, &sb, sizeof(SystemBlock));

        UpdateSystemBlock((char*)&sb);

        return freeBlockId;
    }

    //Gadiel : Modified by Alex
    long TableSpace::addNewBlock()
    {
        char * rawData = GetSystemBlock();
        SystemBlock systemBlock;
        memcpy(&systemBlock, rawData, sizeof(SystemBlock));

        GeneralHeader generalHeaderNewBlock;
        generalHeaderNewBlock.NextBlockId = 0;        
        generalHeaderNewBlock.BlockType = Blank;
        generalHeaderNewBlock.BlockId = systemBlock.PhysicalBlockCount+1;
        generalHeaderNewBlock.PreviousBlockId = systemBlock.LastEmptyBlockId;

        UpdateGeneralHeader(generalHeaderNewBlock.BlockId,generalHeaderNewBlock);

        cout << "New Block: " << generalHeaderNewBlock.PreviousBlockId << "<==" << generalHeaderNewBlock.BlockId << "==>" << generalHeaderNewBlock.NextBlockId << "\n";

        long offset=(generalHeaderNewBlock.BlockId*defaultBlockSize)+defaultBlockSize-1;
        rawData="\0";
        writeData(offset,rawData,1);

        if(systemBlock.LastEmptyBlockId>0){
            GeneralHeader generalHeaderLastBlock;
            rawData = GetGeneralHeader(systemBlock.LastEmptyBlockId);
            memcpy(&generalHeaderLastBlock, rawData, sizeof(GeneralHeader));
            generalHeaderLastBlock.NextBlockId = generalHeaderNewBlock.BlockId;
            UpdateGeneralHeader(generalHeaderLastBlock.BlockId,generalHeaderLastBlock);
            cout<< "Last Block: " << generalHeaderLastBlock.PreviousBlockId << "<==" << generalHeaderLastBlock.BlockId << "==>" << generalHeaderLastBlock.NextBlockId << "\n";
        }

        if(systemBlock.LastEmptyBlockId==0)
            systemBlock.FirstEmptyBlockId=generalHeaderNewBlock.BlockId;

        systemBlock.PhysicalBlockCount+=1;
        systemBlock.LastEmptyBlockId = generalHeaderNewBlock.BlockId;
        writeData(0,(char*)&systemBlock,sizeof(SystemBlock));
    }

    char * TableSpace::GetGeneralHeader(long blockId)
    {
        long offset=blockId*defaultBlockSize;
        char * rawData= GetData(offset,sizeof(GeneralHeader));
        return rawData;
    }

    void TableSpace::writeData(long dir, char * data, streamsize size)
    {
        tableSpaceFile.seekp(dir,std::ios::beg);
        tableSpaceFile.write(data,size);
        tableSpaceFile.flush();

        /*Just for control. this code can be deleted later.
        tableSpaceFile.seekp(0,ios_base::end);
        cout<<"Size of TableSpace: "<< tableSpaceFile.tellp()<<"\n";
        */
    }

    //Alex
    long TableSpace::CreateMetadataTable(char name[256]){

        long id= getNextFreeBlockAndUseIt();
        long lastId = GetLastBlockId(id);

        char * rawData= GetGeneralHeader(id);
        GeneralHeader newBlockGeneralHeader;
        memcpy(&newBlockGeneralHeader,rawData,sizeof(GeneralHeader));
        newBlockGeneralHeader.BlockType= TableMetadata;
        newBlockGeneralHeader.PreviousBlockId=lastId;

        rawData= GetGeneralHeader(lastId);
        GeneralHeader lastBlockGeneralHeader;
        memcpy(&lastBlockGeneralHeader,rawData,sizeof(GeneralHeader));
        lastBlockGeneralHeader.NextBlockId=id;

        TableMetadataHeader newBlockMetadataHeader= TableMetadataHeader();
        strcpy(newBlockMetadataHeader.TableName,name);
        newBlockMetadataHeader.FreeFields= (defaultBlockSize-sizeof(GeneralHeader)-sizeof(TableMetadataHeader))/sizeof(MetadataField);
        newBlockMetadataHeader.LogicalColumnsCount=0;
        newBlockMetadataHeader.PhysicalColumnsCount=0;
        newBlockMetadataHeader.NextMetadataExtensionBlockId=0;
        newBlockMetadataHeader.Identity=0;

        UpdateGeneralHeader(id,newBlockGeneralHeader);
        CreateMetadataTableHeader(id,newBlockMetadataHeader);
        UpdateGeneralHeader(lastId,lastBlockGeneralHeader);

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

    char* TableSpace::GetData(long positionInFile, long sizeToRead)
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

    bool TableSpace::AddNewRecord(long blockId, char *record, int recordSize){
        //blockId es el TableMetadataBlockId
        int offset=0;
        char* headerChars=GetTableMetadataHeader(blockId);
        TableMetadataHeader tableHeader;
        memcpy(&tableHeader,headerChars,sizeof(TableMetadataHeader));

        if(tableHeader.FirstDataBlock==0)
        {
            //Crear primer bloque de datos
            long newDataBlockId= getNextFreeBlockAndUseIt();

            //Actualizar GeneralHeader
            char* generalHeaderChars= GetGeneralHeader(newDataBlockId);
            GeneralHeader generalHeader;
            memcpy(&generalHeader,generalHeaderChars,sizeof(GeneralHeader));

            generalHeader.BlockType=Data;
            generalHeader.PreviousBlockId=blockId;
            generalHeader.NextBlockId=0;

            UpdateGeneralHeader(newDataBlockId,generalHeader);

            //Insertar DataBlockHeader
            DataBlockHeader dataBlockHeader;
            dataBlockHeader.LogicalRowsCount=1;
            dataBlockHeader.PhysicalRowsCount=1;
            dataBlockHeader.TrailSize=0;

            InsertDataBlockHeader(newDataBlockId, (char*)&dataBlockHeader);

            //Insertar RowHeader
            RowHeader rowHeader;
            rowHeader.TombStone=false;

            offset=(newDataBlockId*defaultBlockSize)+sizeof(GeneralHeader)+sizeof(DataBlockHeader);
            writeData(offset,(char*)&rowHeader,sizeof(RowHeader));

            //Insertar registro
            offset+=sizeof(RowHeader);
            writeData(offset,record,recordSize);

            //Actualizar TableMetadataHeader
            tableHeader.FirstDataBlock=newDataBlockId;
            CreateMetadataTableHeader(blockId,tableHeader);

            return true;
        }

        //Buscar ultimo bloque de datos de la tabla
        long lastDataBlockId=GetLastBlockId(tableHeader.FirstDataBlock);

        char* dataBlockHeaderChars= GetDataBlockHeader(lastDataBlockId);
        DataBlockHeader dataBlockHeader;
        memcpy(&dataBlockHeader,dataBlockHeaderChars,sizeof(DataBlockHeader));

        RowHeader rowHeader;
        rowHeader.TombStone=false;

        int wholeRecordSize=recordSize+sizeof(RowHeader);
        int freeSpace=defaultBlockSize-(sizeof(GeneralHeader))-(sizeof(DataBlockHeader))-(dataBlockHeader.PhysicalRowsCount*wholeRecordSize);

        if(freeSpace>wholeRecordSize)//Cabe el registro completo
        {
            //Insertar RowHeader
            offset=(lastDataBlockId*defaultBlockSize)+sizeof(GeneralHeader)+(dataBlockHeader.PhysicalRowsCount*wholeRecordSize);
            writeData(offset,(char*)&rowHeader,sizeof(RowHeader));

            //Insertar Registro
            offset=offset+sizeof(RowHeader);
            writeData(offset,record,recordSize);

            //Actualizar DataBlockHeader
            dataBlockHeader.LogicalRowsCount++;
            dataBlockHeader.PhysicalRowsCount++;
            InsertDataBlockHeader(lastDataBlockId, (char*)&dataBlockHeader);

            return true;
        }
        else{
            if(freeSpace>=sizeof(RowHeader))//Solo se inserta el encabezado
            {
                //Insertar RowHeader
                writeData(offset,(char*)&rowHeader,sizeof(RowHeader));

                long newDataBlockId=CreateNewDataBlock(lastDataBlockId,recordSize);

                //Insertar Registro
                offset=(newDataBlockId*defaultBlockSize)+sizeof(GeneralHeader)+sizeof(DataBlockHeader);
                writeData(offset,record,recordSize);

                return true;
            }
            else//Se crea un nuevo bloque
            {
                long newDataBlockId=CreateNewDataBlock(lastDataBlockId,0);

                dataBlockHeaderChars="\0";
                dataBlockHeaderChars=GetDataBlockHeader(newDataBlockId);
                memcpy(&dataBlockHeader,dataBlockHeaderChars,sizeof(DataBlockHeader));

                offset=(newDataBlockId*defaultBlockSize)+sizeof(GeneralHeader)+sizeof(DataBlockHeader);
                writeData(offset,(char*)&rowHeader,sizeof(RowHeader));

                offset+=sizeof(RowHeader);
                writeData(offset,record,recordSize);

                return true;
            }
        }


    }

    char* TableSpace::GetDataBlockHeader(long blockId){
        int posInicial= (blockId*defaultBlockSize)+(sizeof(GeneralHeader));
        return GetData(posInicial,sizeof(DataBlockHeader));
    }

    bool TableSpace::InsertDataBlockHeader(long blockId, char *dataBlockHeader){
         int offset=(blockId*defaultBlockSize)+sizeof(GeneralHeader);
         writeData(offset,dataBlockHeader,sizeof(DataBlockHeader));
         return true;
    }

    long TableSpace::GetLastBlockId(long blockId){
        char* generalHeaderChars= GetGeneralHeader(blockId);
        GeneralHeader generalHeader;
        memcpy(&generalHeader,generalHeaderChars,sizeof(GeneralHeader));

        if(generalHeader.NextBlockId==0)
            return blockId;
        return GetLastBlockId(generalHeader.NextBlockId);
    }

    long TableSpace::CreateNewDataBlock(long lastDataBlockId, int trailSize){
        //Crear nuevo DataBlock
        long newDataBlockId= getNextFreeBlockAndUseIt();

        //Obtener GeneralHeader del nuevo bloque de Datos y actualizarlo
        char* generalHeaderChars= GetGeneralHeader(lastDataBlockId);
        GeneralHeader generalHeader;
        memcpy(&generalHeader,generalHeaderChars,sizeof(GeneralHeader));

        generalHeader.PreviousBlockId=lastDataBlockId;
        generalHeader.NextBlockId=0;
        generalHeader.BlockType=Data;

        UpdateGeneralHeader(newDataBlockId,generalHeader);

        //Insertar DataBlockHeader
        DataBlockHeader newHeader;
        newHeader.LogicalRowsCount=1;
        newHeader.PhysicalRowsCount=1;
        newHeader.TrailSize=trailSize;

        InsertDataBlockHeader(newDataBlockId,(char*)&newHeader);

        //Obtener GeneralHeader del ultimo bloque de Datos y actualizarlo
        generalHeaderChars="\0";
        generalHeaderChars= GetGeneralHeader(newDataBlockId);
        memcpy(&generalHeader,generalHeaderChars,sizeof(GeneralHeader));
        generalHeader.NextBlockId=newDataBlockId;

        UpdateGeneralHeader(lastDataBlockId,generalHeader);
    }
