///////////////////////////////////////////////////////////////////////////////
//
//  Module: MailMsg.h
//
//    Desc: This class encapsulates the MAPI and CMC mail functions.
//
// Copyright (c) 2003 Michael Carruth
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _MAILMSG_H_
#define _MAILMSG_H_

#include "stdafx.h"
#include <xcmc.h>          // CMC function defs
#include <mapi.h>          // MAPI function defs

#include <map>
#include <string>

typedef std::map<std::string, std::string> TStrStrMap;

//
// Define CMC entry points
//
typedef CMC_return_code (FAR PASCAL *LPCMCLOGON) \
   (CMC_string, CMC_string, CMC_string, CMC_object_identifier, \
   CMC_ui_id, CMC_uint16, CMC_flags, CMC_session_id FAR*, \
   CMC_extension FAR*);

typedef CMC_return_code (FAR PASCAL *LPCMCSEND) \
   (CMC_session_id, CMC_message FAR*, CMC_flags, \
   CMC_ui_id, CMC_extension FAR*);

typedef CMC_return_code (FAR PASCAL *LPCMCLOGOFF) \
   (CMC_session_id, CMC_ui_id, CMC_flags, CMC_extension FAR*);

typedef CMC_return_code (FAR PASCAL *LPCMCQUERY) \
   (CMC_session_id, CMC_enum, CMC_buffer, CMC_extension FAR*);


////////////////////////////// Class Definitions /////////////////////////////

// ===========================================================================
// CMailMsg
// 
// See the module comment at top of file.
//
class CMailMsg  
{
public:
	
  // Construction/destruction
  CMailMsg();
	virtual ~CMailMsg();
  
  // Operations
  void SetTo(CString sAddress);
  void SetFrom(CString sAddress);
  void SetSubject(CString sSubject);
  void SetMessage(CString sMessage);
  void AddAttachment(CString sAttachment, CString sTitle = _T(""));

  BOOL MAPIInitialize();
  void MAPIFinalize();
  
  static BOOL DetectMailClient(CString& sMailClientName);
  CString GetEmailClientName();
  BOOL Send();
  BOOL MAPISend();   
  BOOL CMCSend();
  CString GetLastErrorMsg(){ return m_sErrorMsg; }

protected:

   std::string    m_from;                       // From <address,name>
   std::string    m_to;                         // To <address,name>
   TStrStrMap     m_attachments;                // Attachment <file,title>
   std::string    m_sSubject;                   // EMail subject
   std::string    m_sMessage;                   // EMail message

   HMODULE        m_hMapi;                      // Handle to MAPI32.DLL
   LPCMCQUERY     m_lpCmcQueryConfiguration;    // Cmc func pointer
   LPCMCLOGON     m_lpCmcLogon;                 // Cmc func pointer
   LPCMCSEND      m_lpCmcSend;                  // Cmc func pointer
   LPCMCLOGOFF    m_lpCmcLogoff;                // Cmc func pointer
   LPMAPILOGON    m_lpMapiLogon;                // Mapi func pointer
   LPMAPISENDMAIL m_lpMapiSendMail;             // Mapi func pointer
   LPMAPILOGOFF   m_lpMapiLogoff;               // Mapi func pointer
   
   BOOL           m_bReady;                     // MAPI is loaded
   CString        m_sEmailClientName;

   CString        m_sErrorMsg;
};

#endif	// _MAILMSG_H_
