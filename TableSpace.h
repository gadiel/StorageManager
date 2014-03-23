#include <fstream>
#include "BlockStructs.h"

#define defaultBlockSize 4096
#define DatabaseEngineVersion 1
#define defaultBlockCount 256
#define defaultTableSpaceFileName "tablespace.bin"

class TableSpace
	{
	public:
		
		void CreateTableSpace(char DatabaseName[256]);        
        char * GetSystemBlock();
        long getNextFreeBlock();
        long getNextFreeBlockAndUseIt();
        long addNewBlock();
        long addNewBlock(long id);
        long getLastTableMetadataBlockId();

        //Alex
        long CreateMetadataTable(char name[256]);
        bool CreateMetadataField(long blockId, char *metadataField);
        char* GetMetadataField(long BlockId);

        bool CreateMetadataExtensionHeader(long blockId);
        bool UpdateMetadataExtensionHeader(long blockId,TableMetadataExtensionHeader header);
        char* GetMetadataExtensionHeader(long blockId);
        bool UpdateTableMetadataHeader(long blockId,TableMetadataHeader header);

        //Alex

        //Wendy
        bool UpdateSystemBlock(char* newSystemBlock);
        char* GetTableMetadataHeader(long blockId);
        bool UpdateMetadataField(long blockId, long fieldId,char* newMetadataField);

		TableSpace();
		~TableSpace();
	private:
		std::fstream tableSpaceFile;
		char * GetData(long positionInFile, long sizeToRead);
		void CreateDatabaseFile(char * fileName);
        char * GetGeneralHeader(long blockId);
		void VerifyTableSpaceFile();
		void CloseDatabaseFile();
		void CreateSystemBlock(char DatabaseName[256]);
		void InitializeDatabaseFile(std::ios_base::openmode openmode);
        void WriteBlock(long dir, char * blockData);
        void writeData(long dir, char * data, int size);
        //Alex
        void CreateMetadataTableHeader(long BlockId,TableMetadataHeader header);
        void UpdateGeneralHeader(long blockId,GeneralHeader header);
        bool CreateFieldOnMetadataExtensionBlock(long blockId,char*MetadataField);
        //Alex
};
