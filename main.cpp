#include <QtCore/QCoreApplication>
#include "TableSpace.h"
#include "stdafx.h"

using namespace std;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    TableSpace tbspace;
    tbspace.CreateTableSpace("prueba1");

    TableMetadataHeader tbmdHeader;

    tbmdHeader.LogicalColumnsCount = 6;

    tbspace.CreateNewTable(tbmdHeader);

    char* systemC=tbspace.GetSystemBlock();

    SystemBlock system;
    memcpy(&system, systemC, sizeof(SystemBlock));

    string name="OtraBaseDeDatosCUIIII2";
    name.copy(system.DatabaseName,sizeof(system.DatabaseName),0);

    bool result=tbspace.UpdateSystemBlock((char*)&system);
    char numbers[]={'0','1','2','3','4','5'};
    for(int x=0; x<5 ; x++){
        char tableName[]={'a','l','u','m','n','o','s',numbers[x]};

        long lastId= tbspace.CreateTable(tableName);

        TableMetadataHeader header;
        char * rawData= tbspace.GetTableMetadataHeader(lastId);
        memcpy(&header, rawData, sizeof(TableMetadataHeader));
        cout << "Table Name: " <<  header.TableName;
        printf("Last table created: %d \n", lastId);
    }

    return a.exec();
}
