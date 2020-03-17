abstract_class IBSPPack
{
public:
	virtual void LoadBSPFile( IFileSystem *pFileSystem, char *filename ) = 0;
	virtual void WriteBSPFile( char *filename ) = 0;
	virtual void ClearPackFile( void ) = 0;
	virtual void AddFileToPack( const char *relativename, const char *fullpath ) = 0;
	virtual void AddBufferToPack( const char *relativename, void *data, int length, bool bTextMode ) = 0;
	virtual void SetHDRMode( bool bHDR ) = 0;
	virtual bool SwapBSPFile( IFileSystem *pFileSystem, const char *filename, const char *swapFilename, bool bSwapOnLoad, VTFConvertFunc_t pVTFConvertFunc, VHVFixupFunc_t pVHVFixupFunc, CompressFunc_t pCompressFunc ) = 0;

	// used to get/set the pak file from a BSP
	virtual bool GetPakFileLump( IFileSystem *pFileSystem, const char *pBSPFilename, void **pPakData, int *pPakSize ) = 0;
	virtual bool SetPakFileLump( IFileSystem *pFileSystem, const char *pBSPFilename, const char *pNewFilename, void *pPakData, int pakSize ) = 0;

	// populates list of files that bsp owns, i.e. world/cubmap materials, statis props, etc
	virtual bool GetBSPDependants( IFileSystem *pFileSystem, const char *pBSPFilename, CUtlVector< CUtlString > *pList ) = 0;

	// This is here to avoid having things like the runtime engine.dll have to statically link to LZMA for the encoder!!!
	virtual void AddBufferToPackAndLZMACompress( const char *relativename, void *data, int length ) = 0;
	virtual void RemoveFileFromPack( char const *relativename ) = 0;
};

#define IBSPPACK_VERSION_STRING "IBSPPACK003" 