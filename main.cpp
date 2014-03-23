#include "TableSpace.h"
#include "stdafx.h"

using namespace std;
static void printSystemBlockInfo(TableSpace &tbspace);

static void printSystemBlockInfo(TableSpace &tbspace){
    char* systemC=tbspace.GetSystemBlock();
    SystemBlock system;
    memcpy(&system, systemC, sizeof(SystemBlock));
    std::cout << "DatabaseName:" << system.DatabaseName;
    std::cout << "\nVersion:" << system.Version;
    std::cout << "\nFirstEmptyBlockId:" << system.FirstEmptyBlockId;
    std::cout << "\nLastEmptyBlockId:" << system.LastEmptyBlockId;
    std::cout << "\nPhysicalBlockCount:" << system.PhysicalBlockCount;
    std::cout << "\nFirstTableMetadataBlockId:" << system.FirstTableMetadataBlockId;
}


int main()
{

    cout <<"__________________________________________\n";
    cout <<"Size of:\n";
    cout <<"System Block:"<<sizeof(SystemBlock)<<"\n";
    cout <<"General Header:"<<sizeof(GeneralHeader)<<"\n";
    cout <<"Metadata Table:"<<sizeof(TableMetadata)<<"\n";
    cout <<"Metadata Field:"<<sizeof(MetadataField)<<"\n";
    cout <<"__________________________________________\n";

    TableSpace tbspace;
    tbspace.CreateTableSpace("prueba1");

    char* systemC=tbspace.GetSystemBlock();
    SystemBlock system;
    memcpy(&system, systemC, sizeof(SystemBlock));

    printSystemBlockInfo(tbspace);


    string name="OtraBaseDeDatosCUIIII2";
    name.copy(system.DatabaseName,sizeof(system.DatabaseName),0);

    bool result=tbspace.UpdateSystemBlock((char*)&system);
    if(result)
    {
        cout << "\nSystem Block Updated!";
        printSystemBlockInfo(tbspace);
    }

    /* Just for test this methods, this code can be deleted later
    long next= tbspace.getNextFreeBlock();
    printf("\nNext not used: %d \n",next);

    next= tbspace.getNextFreeBlockAndUseIt();
    printf("\nActual used:%d \n",next);

    next= tbspace.getNextFreeBlockAndUseIt();
    printf("\nActual used:%d \n",next);
    */

    char tableName[]={'a','l','u','m','n','o','s'};

    long lastId= tbspace.CreateMetadataTable(tableName);

    TableMetadataHeader header;
    char * rawData= tbspace.GetTableMetadataHeader(lastId);
    memcpy(&header, rawData, sizeof(TableMetadataHeader));
    cout << "Table Name: " <<  header.TableName;
    printf("Last table created: %d \n", lastId);

    for(int i=0;i<25;i++){
        tbspace.AddNewRecord(1,(char*)"1 Hello WORLD MYMYMY BABY ",300);
    }

    return 0;
}


