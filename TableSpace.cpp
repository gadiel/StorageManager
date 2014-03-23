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


        for(int i=1;i<=defaultBlockCount;i++)
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

        if(gh.NextBlockId==0){
            gh.NextBlockId=addNewBlock();
            rawData = GetData(0,sizeof(SystemBlock));
            memcpy(&sb, rawData, sizeof(SystemBlock));
        }
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

        //cout << "New Block: " << generalHeaderNewBlock.PreviousBlockId << "<==" << generalHeaderNewBlock.BlockId << "==>" << generalHeaderNewBlock.NextBlockId << "\n";

        long offset=(generalHeaderNewBlock.BlockId*defaultBlockSize)+defaultBlockSize-1;
        rawData="\0";
        writeData(offset,rawData,1);

        if(systemBlock.LastEmptyBlockId>0){
            GeneralHeader generalHeaderLastBlock;
            rawData = GetGeneralHeader(systemBlock.LastEmptyBlockId);
            memcpy(&generalHeaderLastBlock, rawData, sizeof(GeneralHeader));
            generalHeaderLastBlock.NextBlockId = generalHeaderNewBlock.BlockId;
            UpdateGeneralHeader(generalHeaderLastBlock.BlockId,generalHeaderLastBlock);
            //cout<< "Last Block: " << generalHeaderLastBlock.PreviousBlockId << "<==" << generalHeaderLastBlock.BlockId << "==>" << generalHeaderLastBlock.NextBlockId << "\n";
        }

        if(systemBlock.LastEmptyBlockId==0)
            systemBlock.FirstEmptyBlockId=generalHeaderNewBlock.BlockId;

        systemBlock.PhysicalBlockCount+=1;
        systemBlock.LastEmptyBlockId = generalHeaderNewBlock.BlockId;
        writeData(0,(char*)&systemBlock,sizeof(SystemBlock));
        return generalHeaderNewBlock.BlockId;
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
		//Obtener id del primer metadatatable y enviarlo
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

    bool TableSpace::CreateFieldOnMetadataExtensionBlock(long blockId, char * metadataField)
    {
        MetadataField field;
        memcpy(&field, metadataField, sizeof(MetadataField));
        TableMetadataExtensionHeader header;
        char* rawData=GetMetadataExtensionHeader(blockId);
        memcpy(&header, rawData, sizeof(TableMetadataExtensionHeader));

        long offset=defaultBlockSize*blockId+sizeof(GeneralHeader)+sizeof(TableMetadataExtensionHeader)+(sizeof(MetadataField)*header.PhysicalColumnsCount);
        header.FreeFields--;
        header.LogicalColumnsCount++;
        header.PhysicalColumnsCount++;
        //cout << "Escribiendo campo en Bloque:"<<blockId<<" Espacio:"<<header.PhysicalColumnsCount<<"\n";
        writeData(offset,metadataField,sizeof(MetadataField));
        UpdateMetadataExtensionHeader(blockId,header);
    }

    bool TableSpace::CreateMetadataField(long blockId, char *metadataField){
        TableMetadataHeader header;
        char* rawData=GetTableMetadataHeader(blockId);
        memcpy(&header, rawData, sizeof(TableMetadataHeader));

        if(header.NextMetadataExtensionBlockId!=0){
            TableMetadataExtensionHeader meh;
            long lastBlockId=header.NextMetadataExtensionBlockId;
            long nextBlockId=header.NextMetadataExtensionBlockId;
            do{
                char* rawData=GetMetadataExtensionHeader(nextBlockId);
                memcpy(&meh, rawData, sizeof(TableMetadataExtensionHeader));
                lastBlockId=nextBlockId;
                nextBlockId=meh.Next;
            }
            while(nextBlockId!=0);
            if(meh.FreeFields==0){
                //Crear nuevo bloque de extension
                long newblockid=getNextFreeBlockAndUseIt();
                GeneralHeader newgh;
                char* rawData=GetGeneralHeader(newblockid);
                memcpy(&newgh, rawData, sizeof(GeneralHeader));
                newgh.BlockType=TableMetadataExtension;
                newgh.NextBlockId=0;
                newgh.PreviousBlockId=lastBlockId;

                meh.Next=newblockid;

                UpdateMetadataExtensionHeader(lastBlockId,meh);
                UpdateGeneralHeader(newblockid,newgh);
                CreateMetadataExtensionHeader(newblockid);
                CreateFieldOnMetadataExtensionBlock(newblockid,metadataField);
            }
            else
            {
                CreateFieldOnMetadataExtensionBlock(lastBlockId,metadataField);
            }
        }
        else if(header.FreeFields==0){
            //cout <<"Creando bloque de extension de metada"<<"\n";
            //cout <<"Creando campo en bloque de extension"<<"\n";
            long newblockid=getNextFreeBlockAndUseIt();
            char* rawData=GetGeneralHeader(newblockid);
            GeneralHeader newgh;
            memcpy(&newgh, rawData, sizeof(GeneralHeader));
            newgh.BlockType=TableMetadataExtension;
            newgh.NextBlockId=0;
            newgh.PreviousBlockId=blockId;
            UpdateGeneralHeader(newblockid,newgh);
            CreateMetadataExtensionHeader(newblockid);

            rawData=GetTableMetadataHeader(blockId);
            TableMetadataHeader lastgh;
            memcpy(&lastgh, rawData, sizeof(TableMetadataHeader));
            lastgh.NextMetadataExtensionBlockId=newblockid;
            UpdateTableMetadataHeader(blockId,lastgh);

            return CreateFieldOnMetadataExtensionBlock(newblockid,metadataField);
        }else{
            //cout <<"Creando campo en bloque principal"<<"\n";
            char* rawData=GetTableMetadataHeader(blockId);
            TableMetadataHeader tm;
            memcpy(&tm, rawData, sizeof(TableMetadataHeader));
            long offset= blockId*defaultBlockSize+sizeof(GeneralHeader)+sizeof(TableMetadataHeader)+(sizeof(MetadataField)*tm.PhysicalColumnsCount);

            tm.FreeFields--;
            tm.LogicalColumnsCount++;
            tm.PhysicalColumnsCount++;
            tm.Identity++;

            writeData(offset,metadataField,sizeof(MetadataField));
            UpdateTableMetadataHeader(blockId,tm);

        }
    }

    char *TableSpace::GetMetadataField(long blockId, long fieldPosition)
    {
        char* rawData=GetTableMetadataHeader(blockId);
        TableMetadataHeader header;
        memcpy(&header, rawData, sizeof(TableMetadataHeader));

        if(fieldPosition<=header.LogicalColumnsCount){
            long counter=0;
            //Buscar el campo dentro del bloque principal
            for(int x=0;x<header.PhysicalColumnsCount;x++){
                long offset=blockId*defaultBlockSize+sizeof(GeneralHeader)+sizeof(TableMetadataHeader)+(x*sizeof(MetadataField));
                rawData=GetData(offset,sizeof(MetadataField));
                MetadataField mf;
                memcpy(&mf,rawData,sizeof(MetadataField));
                if(!mf.IsDeleted){
                    counter++;
                    if(counter==fieldPosition){
                        return rawData;
                    }
                }
            }
        }else
        {
            //Restar las columnas previas
            fieldPosition-=header.LogicalColumnsCount;
            //Buscar el campo entre los bloques de metadata extension

            long counter=0;
            blockId=header.NextMetadataExtensionBlockId;
            long lastBlockId=blockId;
            do{
                rawData=GetMetadataExtensionHeader(blockId);
                TableMetadataExtensionHeader meh;
                memcpy(&meh,rawData,sizeof(TableMetadataExtensionHeader));
                lastBlockId=meh.Next;

                if(fieldPosition<=meh.LogicalColumnsCount){
                    for(int x=0;x<meh.PhysicalColumnsCount;x++){
                        long offset=blockId*defaultBlockSize+sizeof(GeneralHeader)+sizeof(TableMetadataExtensionHeader)+(x*sizeof(MetadataField));
                        rawData=GetData(offset,sizeof(MetadataField));
                        MetadataField mf;
                        memcpy(&mf,rawData,sizeof(MetadataField));
                        if(!mf.IsDeleted){
                            counter++;
                            if(counter==fieldPosition){
                                return rawData;
                            }
                        }
                    }
                }

                if(lastBlockId!=0)
                    blockId=lastBlockId;
                fieldPosition-=meh.LogicalColumnsCount;
            }while (lastBlockId!=0);


        }
    }

    bool TableSpace::CreateMetadataExtensionHeader(long blockId)
    {
        TableMetadataExtensionHeader header;
        header.FreeFields=(defaultBlockSize-sizeof(GeneralHeader)-sizeof(TableMetadataExtensionHeader))/sizeof(MetadataField);
        header.FreeFields= 2;
        header.LogicalColumnsCount=0;
        header.PhysicalColumnsCount=0;
        header.Next=0;
        long offset=defaultBlockSize*blockId+sizeof(GeneralHeader);
        writeData(offset,(char*)&header,sizeof(TableMetadataExtensionHeader));
    }

    bool TableSpace::UpdateMetadataExtensionHeader(long blockId,TableMetadataExtensionHeader header)
    {
        long offset=defaultBlockSize*blockId+sizeof(GeneralHeader);
        writeData(offset,(char*)&header,sizeof(TableMetadataExtensionHeader));
    }

    char *TableSpace::GetMetadataExtensionHeader(long blockId)
    {
        long offset=defaultBlockSize*blockId+sizeof(GeneralHeader);
        char* rawData=GetData(offset,sizeof(TableMetadataExtensionHeader));
        return rawData;
    }

    bool TableSpace::UpdateTableMetadataHeader(long blockId,TableMetadataHeader header)
    {
        long offset=defaultBlockSize*blockId+sizeof(GeneralHeader);
        writeData(offset,(char*)&header,sizeof(TableMetadataHeader));
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
