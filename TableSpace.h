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
        bool UpdateSystemBlock(char* newSystemBlock);
        char * GetSystemBlock();
        long getNextFreeBlock();
        long getNextFreeBlockAndUseIt();
        long addNewBlock();
        long addNewBlock(long id);
        long writeTableMetadata();
        long CreateNewTable(TableMetadataHeader tableMetadata);
        long getLastTableMetadataBlockId(long BlockID);
		TableSpace();
		~TableSpace();
	private:
		std::fstream tableSpaceFile;
		char * GetData(long positionInFile, long sizeToRead);
		void CreateDatabaseFile(char * fileName);
        char * GetGeneralHeaderData(long blockId);
		void VerifyTableSpaceFile();
		void CloseDatabaseFile();
		void CreateSystemBlock(char DatabaseName[256]);
		void InitializeDatabaseFile(std::ios_base::openmode openmode);
        void WriteBlock(long dir, char * blockData);
        void writeData(long dir, char * data, int size);
};
