#include "stdafx.h"
#include "MinidumpReader.h"
#include <assert.h>
#include "Utility.h"
#include "strconv.h"

CMiniDumpReader* g_pMiniDumpReader = NULL;

// Callback function prototypes

BOOL CALLBACK ReadProcessMemoryProc64(
  HANDLE hProcess,
  DWORD64 lpBaseAddress,
  PVOID lpBuffer,
  DWORD nSize,
  LPDWORD lpNumberOfBytesRead);

PVOID CALLBACK FunctionTableAccessProc64(
  HANDLE hProcess,
  DWORD64 AddrBase);

DWORD64 CALLBACK GetModuleBaseProc64(
  HANDLE hProcess,
  DWORD64 Address);

CMiniDumpReader::CMiniDumpReader()
{
  m_bLoaded = false;
  m_hFileMiniDump = INVALID_HANDLE_VALUE;
  m_hFileMapping = NULL;
  m_pMiniDumpStartPtr = NULL;  
}

CMiniDumpReader::~CMiniDumpReader()
{
  Close();
}

int CMiniDumpReader::Open(CString sFileName, CString sSymSearchPath)
{  
  static DWORD dwProcessID = 0;

  if(m_bLoaded)
  {
    return 1;
  }

  m_hFileMiniDump = CreateFile(
    sFileName, 
    FILE_ALL_ACCESS, 
    0, 
    NULL, 
    OPEN_EXISTING, 
    NULL, 
    NULL);

  if(m_hFileMiniDump==INVALID_HANDLE_VALUE)
  {
    Close();
    return 1;
  }

  m_hFileMapping = CreateFileMapping(
    m_hFileMiniDump, 
    NULL, 
    PAGE_READONLY, 
    0, 
    0, 
    0);

  if(m_hFileMapping==NULL)
  {
    Close();
    return 2;
  }

  m_pMiniDumpStartPtr = MapViewOfFile(
    m_hFileMapping, 
    FILE_MAP_READ, 
    0, 
    0, 
    0);

  if(m_pMiniDumpStartPtr==NULL)
  {    
    Close();
    return 3;
  }
  
  m_DumpData.m_hProcess = (HANDLE)(++dwProcessID);  
  
  strconv_t strconv;
  BOOL bSymInit = SymInitialize(
    m_DumpData.m_hProcess,
    (PSTR)strconv.t2a(sSymSearchPath), 
    FALSE);

  if(!bSymInit)
  {
    m_DumpData.m_hProcess = NULL;
    Close();
    return 4;
  }
  
  DWORD dwOptions = SymGetOptions();
  dwOptions |= ( 
    //SYMOPT_DEFERRED_LOADS | // Symbols are not loaded until a reference is made requiring the symbols be loaded.
    SYMOPT_EXACT_SYMBOLS  | // Do not load an unmatched .pdb file. 
    SYMOPT_FAIL_CRITICAL_ERRORS | // Do not display system dialog boxes when there is a media failure such as no media in a drive.
    SYMOPT_UNDNAME // All symbols are presented in undecorated form. 
    );
  SymSetOptions(dwOptions);

  m_bReadSysInfoStream = !ReadSysInfoStream();
  m_bReadExceptionStream = !ReadExceptionStream();    
  m_bReadModuleListStream = !ReadModuleListStream();
  m_bReadThreadListStream = !ReadThreadListStream();
  m_bReadMemoryListStream = !ReadMemoryListStream();    
  
  m_bLoaded = true;
  return 0;
}

void CMiniDumpReader::Close()
{
  UnmapViewOfFile(m_pMiniDumpStartPtr);

  if(m_hFileMapping!=NULL)
  {
    CloseHandle(m_hFileMapping);
  }

  if(m_hFileMiniDump!=INVALID_HANDLE_VALUE)
  {
    CloseHandle(m_hFileMiniDump);
  }

  m_pMiniDumpStartPtr = NULL;

  if(m_DumpData.m_hProcess!=NULL)
  {
    SymCleanup(m_DumpData.m_hProcess);
  }
}

// Extracts a UNICODE string stored in minidump file by its relative address
CString CMiniDumpReader::GetMinidumpString(LPVOID start_addr, RVA rva)
{
  MINIDUMP_STRING* pms = (MINIDUMP_STRING*)((LPBYTE)start_addr+rva);
  return CString(pms->Buffer, pms->Length);
}

int CMiniDumpReader::ReadSysInfoStream()
{
  LPVOID pStreamStart = NULL;
  ULONG uStreamSize = 0;
  MINIDUMP_DIRECTORY* pmd = NULL;
  BOOL bRead = FALSE;

  bRead = MiniDumpReadDumpStream(m_pMiniDumpStartPtr, SystemInfoStream, 
    &pmd, &pStreamStart, &uStreamSize);
  
  if(bRead)
  {
    MINIDUMP_SYSTEM_INFO* pSysInfo = (MINIDUMP_SYSTEM_INFO*)pStreamStart;
    
    m_DumpData.m_uProcessorArchitecture = pSysInfo->ProcessorArchitecture;
    m_DumpData.m_uchNumberOfProcessors = pSysInfo->NumberOfProcessors;
    m_DumpData.m_uchProductType = pSysInfo->ProductType;
    m_DumpData.m_ulVerMajor = pSysInfo->MajorVersion;
    m_DumpData.m_ulVerMinor = pSysInfo->MinorVersion;
    m_DumpData.m_ulVerBuild = pSysInfo->BuildNumber;
    m_DumpData.m_sCSDVer = GetMinidumpString(m_pMiniDumpStartPtr, pSysInfo->CSDVersionRva);

    // Clean up
    pStreamStart = NULL;
    uStreamSize = 0;    
    pmd = NULL;
  }
  else 
  {
    return 1;    
  }

  return 0;
}

int CMiniDumpReader::ReadExceptionStream()
{
  LPVOID pStreamStart = NULL;
  ULONG uStreamSize = 0;
  MINIDUMP_DIRECTORY* pmd = NULL;
  BOOL bRead = FALSE;

  bRead = MiniDumpReadDumpStream(
    m_pMiniDumpStartPtr, 
    ExceptionStream, 
    &pmd, 
    &pStreamStart, 
    &uStreamSize);

  if(bRead)
  {
    MINIDUMP_EXCEPTION_STREAM* pExceptionStream = (MINIDUMP_EXCEPTION_STREAM*)pStreamStart;
    if(pExceptionStream!=NULL && 
      uStreamSize>=sizeof(MINIDUMP_EXCEPTION_STREAM))
    {
      m_DumpData.m_uExceptionThreadId = pExceptionStream->ThreadId;
      m_DumpData.m_uExceptionCode = pExceptionStream->ExceptionRecord.ExceptionCode;
      m_DumpData.m_uExceptionAddress = pExceptionStream->ExceptionRecord.ExceptionAddress;          
      m_DumpData.m_pExceptionThreadContext = 
        (CONTEXT*)(((LPBYTE)m_pMiniDumpStartPtr)+pExceptionStream->ThreadContext.Rva);      
    }    
  }
  else
  {
    return 1;
  }

  return 0;
}

int CMiniDumpReader::ReadModuleListStream()
{
  LPVOID pStreamStart = NULL;
  ULONG uStreamSize = 0;
  MINIDUMP_DIRECTORY* pmd = NULL;
  BOOL bRead = FALSE;
  strconv_t strconv;

  bRead = MiniDumpReadDumpStream(
    m_pMiniDumpStartPtr, 
    ModuleListStream, 
    &pmd, 
    &pStreamStart, 
    &uStreamSize);

  if(bRead)
  {
    MINIDUMP_MODULE_LIST* pModuleStream = (MINIDUMP_MODULE_LIST*)pStreamStart;
    if(pModuleStream!=NULL)
    {
      ULONG32 uNumberOfModules = pModuleStream->NumberOfModules;
      ULONG32 i;
      for(i=0; i<uNumberOfModules; i++)
      {
        MINIDUMP_MODULE* pModule = 
          (MINIDUMP_MODULE*)((LPBYTE)pModuleStream->Modules+i*sizeof(MINIDUMP_MODULE));

        CString sModuleName = GetMinidumpString(m_pMiniDumpStartPtr, pModule->ModuleNameRva);               
        LPCSTR szModuleName = strconv.t2a(sModuleName);
        DWORD64 dwBaseAddr = pModule->BaseOfImage;
        DWORD64 dwImageSize = pModule->SizeOfImage;

        CString sShortModuleName = sModuleName;
        int pos = -1;
        pos = sModuleName.ReverseFind('\\');
        if(pos>=0)
          sShortModuleName = sShortModuleName.Mid(pos+1);          

        /*DWORD64 dwLoadResult = */SymLoadModuleEx(
          m_DumpData.m_hProcess,
          NULL,
          (PSTR)szModuleName,
          NULL,
          dwBaseAddr,
          (DWORD)dwImageSize,
          NULL,
          0);
        
        IMAGEHLP_MODULE64 modinfo;
        memset(&modinfo, 0, sizeof(IMAGEHLP_MODULE64));
        modinfo.SizeOfStruct = sizeof(IMAGEHLP_MODULE64);
        BOOL bModuleInfo = SymGetModuleInfo64(m_DumpData.m_hProcess,
          dwBaseAddr, 
          &modinfo);
        if(bModuleInfo)
        {
          MdmpModule m;
          m.m_uBaseAddr = modinfo.BaseOfImage;
          m.m_uImageSize = modinfo.ImageSize;        
          m.m_sModuleName = sShortModuleName;
          m.m_sImageName = modinfo.ImageName;
          m.m_sLoadedImageName = modinfo.LoadedImageName;
          m.m_sLoadedPdbName = modinfo.LoadedPdbName;
          m.m_pVersionInfo = &pModule->VersionInfo;

          m_DumpData.m_Modules.push_back(m);
          m_DumpData.m_ModuleIndex[m.m_uBaseAddr] = m_DumpData.m_Modules.size()-1;
        }        
      }
    }
  }
  else
  {
    return 1;
  }

  return 0;
}

int CMiniDumpReader::GetModuleRowIdByBaseAddr(DWORD64 dwBaseAddr)
{
  std::map<DWORD64, size_t>::iterator it = m_DumpData.m_ModuleIndex.find(dwBaseAddr);
  if(it!=m_DumpData.m_ModuleIndex.end())
    return (int)it->second;
  return -1;
}

int CMiniDumpReader::GetModuleRowIdByAddress(DWORD64 dwAddress)
{
  UINT i;
  for(i=0;i<m_DumpData.m_Modules.size();i++)
  {
    if(m_DumpData.m_Modules[i].m_uBaseAddr<=dwAddress && 
      dwAddress<m_DumpData.m_Modules[i].m_uBaseAddr+m_DumpData.m_Modules[i].m_uImageSize)
      return i;
  }

  return -1;
}

int CMiniDumpReader::GetThreadRowIdByThreadId(DWORD dwThreadId)
{
  std::map<DWORD, size_t>::iterator it = m_DumpData.m_ThreadIndex.find(dwThreadId);
  if(it!=m_DumpData.m_ThreadIndex.end())
    return (int)it->second;
  return -1;
}

int CMiniDumpReader::ReadMemoryListStream()
{
  LPVOID pStreamStart = NULL;
  ULONG uStreamSize = 0;
  MINIDUMP_DIRECTORY* pmd = NULL;
  BOOL bRead = FALSE;

  bRead = MiniDumpReadDumpStream(
    m_pMiniDumpStartPtr, 
    MemoryListStream, 
    &pmd, 
    &pStreamStart, 
    &uStreamSize);

  if(bRead)
  {
    MINIDUMP_MEMORY_LIST* pMemStream = (MINIDUMP_MEMORY_LIST*)pStreamStart;
    if(pMemStream!=NULL)
    {
      ULONG32 uNumberOfMemRanges = pMemStream->NumberOfMemoryRanges;
      ULONG i;
      for(i=0; i<uNumberOfMemRanges; i++)
      {
        MINIDUMP_MEMORY_DESCRIPTOR* pMemDesc = (MINIDUMP_MEMORY_DESCRIPTOR*)(&pMemStream->MemoryRanges[i]);
        MdmpMemRange mr;
        mr.m_u64StartOfMemoryRange = pMemDesc->StartOfMemoryRange;
        mr.m_uDataSize = pMemDesc->Memory.DataSize;
        mr.m_pStartPtr = (LPBYTE)m_pMiniDumpStartPtr+pMemDesc->Memory.Rva;

        m_DumpData.m_MemRanges.push_back(mr);
      }
    }
  }
  else    
  {
    return 1;
  }

  return 0;
}
  
int CMiniDumpReader::ReadThreadListStream()
{
  LPVOID pStreamStart = NULL;
  ULONG uStreamSize = 0;
  MINIDUMP_DIRECTORY* pmd = NULL;
  BOOL bRead = FALSE;

  bRead = MiniDumpReadDumpStream(
    m_pMiniDumpStartPtr, 
    ThreadListStream, 
    &pmd, 
    &pStreamStart, 
    &uStreamSize);

  if(bRead)
  {
    MINIDUMP_THREAD_LIST* pThreadList = (MINIDUMP_THREAD_LIST*)pStreamStart;
    if(pThreadList!=NULL && 
      uStreamSize>=sizeof(MINIDUMP_THREAD_LIST))
    {
      ULONG32 uThreadCount = pThreadList->NumberOfThreads;

      ULONG32 i;
      for(i=0; i<uThreadCount; i++)
      {
        MINIDUMP_THREAD* pThread = (MINIDUMP_THREAD*)(&pThreadList->Threads[i]);

        MdmpThread mt;
        mt.m_dwThreadId = pThread->ThreadId;
        mt.m_pThreadContext = (CONTEXT*)(((LPBYTE)m_pMiniDumpStartPtr)+pThread->ThreadContext.Rva);
        
        m_DumpData.m_Threads.push_back(mt);
        m_DumpData.m_ThreadIndex[mt.m_dwThreadId] = m_DumpData.m_Threads.size()-1;        
      }
    }  
  }
  else
  {
    return 1;
  }

  return 0;
}

int CMiniDumpReader::StackWalk(DWORD dwThreadId)
{ 
  int nThreadIndex = GetThreadRowIdByThreadId(dwThreadId);
  if(m_DumpData.m_Threads[nThreadIndex].m_bStackWalk == TRUE)
    return 0; // Already done
  
  CONTEXT* pThreadContext = NULL;
  
  if(m_DumpData.m_Threads[nThreadIndex].m_dwThreadId==m_DumpData.m_uExceptionThreadId)
    pThreadContext = m_DumpData.m_pExceptionThreadContext;
  else
    pThreadContext = m_DumpData.m_Threads[nThreadIndex].m_pThreadContext;  
    
  if(pThreadContext==NULL)
    return 1;

  // Make modifiable context
  CONTEXT Context;
  memcpy(&Context, pThreadContext, sizeof(CONTEXT));

  g_pMiniDumpReader = this;

  // Init stack frame with correct initial values
  // See this:
  // http://www.codeproject.com/KB/threads/StackWalker.aspx
  //
  // Given a current dbghelp, your code should:
  //  1. Always use StackWalk64
  //  2. Always set AddrPC to the current instruction pointer (Eip on x86, Rip on x64 and StIIP on IA64)
  //  3. Always set AddrStack to the current stack pointer (Esp on x86, Rsp on x64 and IntSp on IA64)
  //  4. Set AddrFrame to the current frame pointer when meaningful. On x86 this is Ebp, on x64 you 
  //     can use Rbp (but is not used by VC2005B2; instead it uses Rdi!) and on IA64 you can use RsBSP. 
  //     StackWalk64 will ignore the value when it isn't needed for unwinding.
  //  5. Set AddrBStore to RsBSP for IA64. 

  STACKFRAME64 sf;
  memset(&sf, 0, sizeof(STACKFRAME64));

  sf.AddrPC.Mode = AddrModeFlat;  
  sf.AddrFrame.Mode = AddrModeFlat;  
  sf.AddrStack.Mode = AddrModeFlat;  
  sf.AddrBStore.Mode = AddrModeFlat;  

  DWORD dwMachineType = 0;
  switch(m_DumpData.m_uProcessorArchitecture)
  {
#ifdef _X86_
  case PROCESSOR_ARCHITECTURE_INTEL: 
    dwMachineType = IMAGE_FILE_MACHINE_I386;
    sf.AddrPC.Offset = pThreadContext->Eip;    
    sf.AddrStack.Offset = pThreadContext->Esp;
    sf.AddrFrame.Offset = pThreadContext->Ebp;
    break;
#endif
#ifdef _AMD64_
  case PROCESSOR_ARCHITECTURE_AMD64:
    dwMachineType = IMAGE_FILE_MACHINE_AMD64;
    sf.AddrPC.Offset = pThreadContext->Rip;    
    sf.AddrStack.Offset = pThreadContext->Rsp;
    sf.AddrFrame.Offset = pThreadContext->Rbp;
    break;
#endif
#ifdef _IA64_
  case PROCESSOR_ARCHITECTURE_AMD64:
    dwMachineType = IMAGE_FILE_MACHINE_IA64;
    sf.AddrPC.Offset = pThreadContext->StIIP;
    sf.AddrStack.Offset = pThreadContext->IntSp;
    sf.AddrFrame.Offset = pThreadContext->RsBSP;    
    sf.AddrBStore.Offset = pThreadContext->RsBSP;
    break;
#endif 
  default:
    {
      assert(0);
      return 1; // Unsupported architecture
    }
  }

  for(;;)
  {    
    BOOL bWalk = ::StackWalk64(
      dwMachineType,               // machine type
      m_DumpData.m_hProcess,       // our process handle
      (HANDLE)dwThreadId,          // thread ID
      &sf,                         // stack frame
      dwMachineType==IMAGE_FILE_MACHINE_I386?NULL:(&Context), // used for non-I386 machines 
      ReadProcessMemoryProc64,     // our routine
      FunctionTableAccessProc64,   // our routine
      GetModuleBaseProc64,         // our routine
      NULL                         // safe to be NULL
      );

    if(!bWalk)
      break;      

    MdmpStackFrame stack_frame;
    stack_frame.m_dwAddrPCOffset = sf.AddrPC.Offset;
  
    // Get module info
    IMAGEHLP_MODULE64 mi;
    memset(&mi, 0, sizeof(IMAGEHLP_MODULE64));
    mi.SizeOfStruct = sizeof(IMAGEHLP_MODULE64);
    BOOL bGetModuleInfo = SymGetModuleInfo64(m_DumpData.m_hProcess, sf.AddrPC.Offset, &mi);     
    if(bGetModuleInfo)
    {
      stack_frame.m_nModuleRowID = GetModuleRowIdByBaseAddr(mi.BaseOfImage);      
    }

    // Get symbol info
    DWORD64 dwDisp64;
    BYTE buffer[4096];
    SYMBOL_INFO* sym_info = (SYMBOL_INFO*)buffer;
    sym_info->SizeOfStruct = sizeof(SYMBOL_INFO);
    sym_info->MaxNameLen = 4096-sizeof(SYMBOL_INFO)-1;
    BOOL bGetSym = SymFromAddr(
      m_DumpData.m_hProcess, 
      sf.AddrPC.Offset, 
      &dwDisp64, 
      sym_info);
    
    if(bGetSym)
    {
      stack_frame.m_sSymbolName = CString(sym_info->Name, sym_info->NameLen);
      stack_frame.m_dw64OffsInSymbol = dwDisp64;
    }

    // Get source filename and line
    DWORD dwDisplacement;
    IMAGEHLP_LINE64 line;
    BOOL bGetLine = SymGetLineFromAddr64(
      m_DumpData.m_hProcess, 
      sf.AddrPC.Offset,
      &dwDisplacement,
      &line);

    if(bGetLine)
    {
      stack_frame.m_sSrcFileName = line.FileName;
      stack_frame.m_nSrcLineNumber = line.LineNumber;
    }

    m_DumpData.m_Threads[nThreadIndex].m_StackTrace.push_back(stack_frame);
  }

  m_DumpData.m_Threads[nThreadIndex].m_bStackWalk = TRUE;

  return 0;
}

// This callback function is used by StackWalk64. It provides access to 
// ranges of memory stored in minidump file
BOOL CALLBACK ReadProcessMemoryProc64(
  HANDLE hProcess,
  DWORD64 lpBaseAddress,
  PVOID lpBuffer,
  DWORD nSize,
  LPDWORD lpNumberOfBytesRead)
{
  *lpNumberOfBytesRead = 0;

  // Validate input parameters
  if(hProcess!=g_pMiniDumpReader->m_DumpData.m_hProcess ||
     lpBaseAddress==NULL ||
     lpBuffer==NULL ||
     nSize==0)
  {
    // Invalid parameter
    return FALSE;
  }

  ULONG i;
  for(i=0; i<g_pMiniDumpReader->m_DumpData.m_MemRanges.size(); i++)
  {
    MdmpMemRange& mr = g_pMiniDumpReader->m_DumpData.m_MemRanges[i];
    if(lpBaseAddress>=mr.m_u64StartOfMemoryRange &&
      lpBaseAddress<mr.m_u64StartOfMemoryRange+mr.m_uDataSize)
    {
      DWORD64 dwOffs = lpBaseAddress-mr.m_u64StartOfMemoryRange;
      
      LONG64 lBytesRead = 0;
      
      if(mr.m_uDataSize-dwOffs>nSize)
        lBytesRead = nSize;
      else
        lBytesRead = mr.m_uDataSize-dwOffs;

      if(lBytesRead<=0 || nSize<lBytesRead)
        return FALSE;

      *lpNumberOfBytesRead = (DWORD)lBytesRead;
      memcpy(lpBuffer, (LPBYTE)mr.m_pStartPtr+dwOffs, (size_t)lBytesRead);
     
      return TRUE;
    }
  }

  return FALSE;
}

// This callback function is used by StackWalk64. It provides access to 
// function table stored in minidump file
PVOID CALLBACK FunctionTableAccessProc64(
  HANDLE hProcess,
  DWORD64 AddrBase)
{   
  return SymFunctionTableAccess64(hProcess, AddrBase);
}

// This callback function is used by StackWalk64. It provides access to 
// module list stored in minidump file
DWORD64 CALLBACK GetModuleBaseProc64(
  HANDLE hProcess,
  DWORD64 Address)
{  
  return SymGetModuleBase64(hProcess, Address);
}