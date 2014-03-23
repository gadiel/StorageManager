#include "stdafx.h"
#include "OtherStructs.h"
enum BlockTypes
{
	Blank, Data, TableMetadata, TableMetadataExtension, VariableData
};

struct DataBlockHeader
{
    unsigned int LogicalRowsCount;
    unsigned int PhysicalRowsCount;
    unsigned int TrailSize;
};

struct GeneralHeader
{
    unsigned long BlockId;
    BlockTypes BlockType;
    unsigned long NextBlockId;
    unsigned long PreviousBlockId;    
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
    unsigned long FirstDataBlock;
    unsigned int FreeFields;
};

struct TableMetadataExtBlock
{
    unsigned int LogicalColumnsCount;
    unsigned long PhysicalColumnsCount;
    unsigned long ParentMetadataBlockId;
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

struct RowHeader{
    bool TombStone;
    //time Modified;
};


struct VariableData //VarCharBlocks
{

};



