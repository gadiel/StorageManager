#include "TableSpace.h"
#include "stdafx.h"

using namespace std;

using namespace std;

int main()
{

    TableSpace tbspace;
    /*tbspace.CreateTableSpace("prueba1");
*/
    char* systemC=tbspace.GetSystemBlock();

    SystemBlock system;
    memcpy(&system, systemC, sizeof(SystemBlock));


    string name="OtraBaseDeDatosCUIIII2";
    name.copy(system.DatabaseName,sizeof(system.DatabaseName),0);

    bool result=tbspace.UpdateSystemBlock((char*)&system);

    long next= tbspace.getNextFreeBlock();
    printf("Next: %d",next);
    /*
    next= tbspace.getNextFreeBlockAndUseIt();
    printf("Next->:%d",next);
    next= tbspace.getNextFreeBlockAndUseIt();
    printf("Next->:%d",next);*/

    char tableName[]={'a','l','u','m','n','o','s'};

    long lastId= tbspace.CreateMetadataTable(tableName);

    TableMetadataHeader header;
    char * rawData= tbspace.GetTableMetadataHeader(lastId);
    memcpy(&header, rawData, sizeof(TableMetadataHeader));
    cout << "Table Name: " <<  header.TableName;
    printf("Last table created: %d \n", lastId);

    return 0;
}
