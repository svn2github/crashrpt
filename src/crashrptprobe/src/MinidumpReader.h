#pragma once

#include "stdafx.h"
#include "dbghelp.h"
#include <map>
#include <vector>

// Describes a loaded module
struct MdmpModule
{
  ULONG64 m_uBaseAddr;   // Base address
  ULONG64 m_uImageSize;  // Size of module
  CString m_sModuleName; // Module name  
  CString m_sImageName;  // The image name. The name may or may not contain a full path. 
  CString m_sLoadedImageName; // The full path and file name of the file from which symbols were loaded. 
  CString m_sLoadedPdbName;   // The full path and file name of the .pdb file.     
  VS_FIXEDFILEINFO* m_pVersionInfo; // Version info for module.
};

// Describes a stack frame
struct MdmpStackFrame
{
  MdmpStackFrame()
  {
    m_nModuleRowID = -1;
    m_dw64OffsInSymbol = 0;
    m_nSrcLineNumber = -1;
  }
  
  DWORD64 m_dwAddrPCOffset; 
  int m_nModuleRowID;         // ROWID of the record in CPR_MDMP_MODULES table.
  CString m_sSymbolName;      // Name of symbol
  DWORD64 m_dw64OffsInSymbol; // Offset in symbol
  CString m_sSrcFileName;     // Name of source file
  int m_nSrcLineNumber;       // Line number in the source file
};

// Describes a thread
struct MdmpThread
{
  MdmpThread()
  {
    m_dwThreadId = 0;
    m_pThreadContext = NULL;
    m_bStackWalk = FALSE;
  }

  DWORD m_dwThreadId;        // Thread ID.
  CONTEXT* m_pThreadContext; // Thread context
  BOOL m_bStackWalk;         // Was stack trace retrieved for this thread?
  std::vector<MdmpStackFrame> m_StackTrace; // Stack trace for this thread.
};

// Describes a memory range
struct MdmpMemRange
{
  ULONG64 m_u64StartOfMemoryRange; // Starting address
  ULONG32 m_uDataSize;             // Size of data
  LPVOID m_pStartPtr;              // Pointer to the memrange data stored in minidump
};

// Minidump data
struct MdmpData
{   
  MdmpData()
  {
    m_hProcess = INVALID_HANDLE_VALUE;
    //m_pExceptionContext = NULL;
  }

  HANDLE m_hProcess; // Process ID
    
  USHORT m_uProcessorArchitecture; // CPU architecture
  UCHAR  m_uchNumberOfProcessors;  // Number of processors
  UCHAR  m_uchProductType;         // Type of machine (workstation, server, ...)
  ULONG  m_ulVerMajor;             // OS major version number
  ULONG  m_ulVerMinor;             // OS minor version number
  ULONG  m_ulVerBuild;             // OS build number
  CString m_sCSDVer;               // The latest service pack installed
    
  ULONG32 m_uExceptionCode;        // Structured exception's code
  ULONG64 m_uExceptionAddress;     // Exception address
  ULONG32 m_uExceptionThreadId;    // Exceptions thread ID 
  CONTEXT* m_pExceptionThreadContext; // Thread context
  
  std::vector<MdmpThread> m_Threads;       // The list of threads.
  std::map<DWORD, size_t> m_ThreadIndex;   // <thread_id, thread_entry_index> pairs
  std::vector<MdmpModule> m_Modules;       // The list of loaded modules.
  std::map<DWORD64, size_t> m_ModuleIndex; // <base_addr, module_entry_index> pairs
  std::vector<MdmpMemRange> m_MemRanges;   // The list of memory ranges.  
};

// Class for opening minidumps
class CMiniDumpReader
{
public:
  
  /* Construction/destruction */
  CMiniDumpReader();
  ~CMiniDumpReader();

  /* Operations */

  // Opens a minidump (DMP) file
  int Open(CString sFileName, CString sSymSearchPath);

  // Retreives stack trace for specified thread ID
  int StackWalk(DWORD dwThreadId);  

  // Closes the opened minidump file
  void Close();

  int GetModuleRowIdByBaseAddr(DWORD64 dwBaseAddr);
  int GetModuleRowIdByAddress(DWORD64 dwAddress);
  int GetThreadRowIdByThreadId(DWORD dwThreadId);

  MdmpData m_DumpData; // Minidump data

  BOOL m_bLoaded;               // Is minidump loaded?
  BOOL m_bReadSysInfoStream;    // Was system info stream read?
  BOOL m_bReadExceptionStream;  // Was exception stream read?
  BOOL m_bReadModuleListStream; // Was module list stream read?
  BOOL m_bReadMemoryListStream; // Was memory list stream read?
  BOOL m_bReadThreadListStream; // Was thread list stream read?  

private:

  /* Internally used member functions */
  
  // Helper function which extracts a UNICODE string from the minidump
  CString GetMinidumpString(LPVOID pStartAddr, RVA rva);
  
  // Reads MINIDUMP_SYSTEM_INFO stream
  int ReadSysInfoStream();

  // Reads MINIDUMP_EXCEPTION_STREAM stream
  int ReadExceptionStream();

  // Reads MINIDUMP_MODULE_LIST stream
  int ReadModuleListStream();

  // Reads MINIDUMP_MEMORY_LIST stream
  int ReadMemoryListStream();

  // Reads MINIDUMP_THREAD_LIST stream
  int ReadThreadListStream();

  /* Member variables */
  
  HANDLE m_hFileMiniDump; // Handle to opened .DMP file
  HANDLE m_hFileMapping;  // Handle to memory mapping object
  LPVOID m_pMiniDumpStartPtr; // Pointer to the biginning of memory-mapped minidump  

};

