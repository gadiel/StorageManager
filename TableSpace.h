
#include <fstream>
#include "BlockStructs.h"

#define defaultBlockSize 4096
#define DatabaseEngineVersion 1
#define defaultBlockCount 256
#define defaultTableSpaceFileName "tablespace.db"



	class TableSpace
	{
	public:
		
		void CreateTableSpace(char DatabaseName[256]);
        bool UpdateSystemBlock(char* newSystemBlock);
        char* GetSystemBlock();
        char* GetTableMetadataHeader(long blockId);
        char* GetTableMetadataBlock(long blockId);
        bool UpdateMetadataField(long blockId, long fieldId,char* newMetadataField);
		TableSpace();
		~TableSpace();
	private:
		std::fstream tableSpaceFile;
		char * GetData(long positionInFile, long sizeToRead);
		void CreateDatabaseFile(char * fileName);
		void VerifyTableSpaceFile();
		void CloseDatabaseFile();
		void CreateSystemBlock(char DatabaseName[256]);
		void InitializeDatabaseFile(std::ios_base::openmode openmode);
		void AllocateBlock(long dir, char * blockData);

    };
