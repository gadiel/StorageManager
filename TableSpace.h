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

        //Alex
        long CreateMetadataTable(char name[256]);
        bool CreateMetadataField(long blockId, char *metadataField);
        char* GetMetadataField(long blockId,long fieldPosition);

        bool CreateMetadataExtensionHeader(long blockId);
        bool UpdateMetadataExtensionHeader(long blockId,TableMetadataExtensionHeader header);
        char* GetMetadataExtensionHeader(long blockId);
        bool UpdateTableMetadataHeader(long blockId,TableMetadataHeader header);

        //#gadiel

        char * getRecord(long blockId, long recordCount, long recordSize);

        //Alex

        //Wendy
        bool UpdateSystemBlock(char* newSystemBlock);
        char* GetTableMetadataHeader(long blockId);
        bool UpdateMetadataField(long blockId, long fieldId,char* newMetadataField);
        bool AddNewRecord(long blockId, char *record, int recordSize);

		TableSpace();
		~TableSpace();
	private:
        std::fstream tableSpaceFile;
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

        //Wendy
        char* GetData(long positionInFile, long sizeToRead);
        char* GetDataBlockHeader(long blockId);
        bool InsertDataBlockHeader(long blockId, char* dataBlockHeader);
        long GetLastBlockId(long blockId);
        long CreateNewDataBlock(long lastDataBlockId, int trailSize);//Retorna el id del nuevo bloque creado
        //char* ValidateMetadataFieldTypes(long blockId,char* newRecord, int recordSize);

};
