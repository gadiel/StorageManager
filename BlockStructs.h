#include "stdafx.h"
#include "OtherStructs.h"
enum BlockTypes
{
	Blank, Data, TableMetadata, TableMetadataExtension, VariableData
};

struct DataBlock
{
	unsigned int RowCount;
	unsigned int FreeSpace;
};

struct GeneralHeader
{
    unsigned long BlockId;
    BlockTypes BlockType;
    unsigned long NextBlockId;
    unsigned long PreviousBlockId;
    bool TombStone;
};

struct SystemBlock
{
    char DatabaseName[64];
    unsigned int Version;
	unsigned long FirstEmptyBlockId;
    unsigned long LastEmptyBlockId;
    unsigned long PhysicalBlockCount;
    unsigned long FirstTableMetadataBlockId;
};

struct TableMetadataHeader
{
    char TableName[256];
    unsigned int LogicalColumnsCount;
    unsigned long PhysicalColumnsCount;
    unsigned long Identity;
    unsigned long NextMetadataExtensionBlockId;
    unsigned int FreeFields;
    unsigned int ColumnsCount;
};

struct TableMetadataExtensionHeader
{
    unsigned int LogicalColumnsCount;
    unsigned long PhysicalColumnsCount;
    unsigned long Next;
    unsigned int FreeFields;
};

struct MetadataField
{
    char FieldName [20];
    DataType FieldType;
    int Precision;
    int Scale;
    bool IsPrimaryKey;
    bool IsIdentity;
    bool IsDeleted;
    bool IsNull;
    char DefaultValue[256];
};

struct VariableData
{

};



