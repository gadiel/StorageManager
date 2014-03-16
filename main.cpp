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

    return a.exec();
}
