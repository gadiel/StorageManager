#include "stdafx.h"

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
	char DatabaseName[256];
	unsigned int Version;
	char UserName[50];
	char Password[50];
	unsigned long FirstEmptyBlockId;
	unsigned long FirstMetadataBlockId;
};

struct TableMetadataHeader
{
	char TableName[256];	
    unsigned int LogicalColumnsCount;
    unsigned long PhysicalColumnsCount;
    unsigned long Identity;
	unsigned long MetadataExtensionBlockId;
	unsigned int FreeSpace;
};

struct TableMetadataExtBlock
{
	char TableName[256];
    unsigned int LogicalColumnsCount;
    unsigned long PhysicalColumnsCount;
	unsigned long ParentMetadataBlockId;
	unsigned int FreeSpace;
};

struct VariableData
{

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

struct MetadataExtension
{
    MetadataField Fields[];
};





