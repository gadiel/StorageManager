
#include <fstream>
#include "BlockStructs.h"

#define defaultBlockSize 4096
#define DatabaseEngineVersion 1
#define defaultBlockCount 256
#define defaultTableSpaceFileName "tablespace.db"


namespace{
	class TableSpace
	{
	public:
		
		void createTableSpace(char DatabaseName[256]);
		TableSpace();
		~TableSpace();
	private:
		std::fstream tableSpaceFile;

		void getSystemBlock();
		char * getData(long positionInFile, long sizeToRead);
		void CreateDatabaseFile(char * fileName);
		void verifyTableSpaceFile();
		void closeDatabaseFile();
		void createSystemBlock(char DatabaseName[256]);
		void initializeDatabaseFile(std::ios_base::openmode openmode);
		void AllocateBlock(long dir, char * blockData);
	};
}