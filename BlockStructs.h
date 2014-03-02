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

struct GeneralBlock
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

struct TableMetadataBlock
{
	char TableName[256];
	unsigned int FieldCount;
	unsigned long MetadataExtensionBlockId;
	unsigned int FreeSpace;
};

struct TableMetadataExtBlock
{
	char TableName[256];
	unsigned int FieldCount;
	unsigned long ParentMetadataBlockId;
	unsigned int FreeSpace;
};

struct VariableData
{

};



