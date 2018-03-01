#ifndef __MD_Client_H
#define __MD_Client_H

#include "E15_client.h"
#include "E15_value.h"
#include "E15_map.h"
#include "E15_thread.h"
#include "E15_zip.h"
#include <DiaTypeMap.h>
#include<E15_log.h>
class E15_Timer;
class MD_Client : public E15_Client
{
public:
	MD_Client();
	virtual ~MD_Client();

	virtual void OnLoginOk(E15_ClientInfo * user,E15_String *& json) ;
	virtual int OnLogout(E15_ClientInfo * user);
	virtual int OnLoginFailed(E15_ClientInfo * user,int status,const char * errmsg);

	virtual void OnRequest(E15_ClientInfo * user,E15_ClientMsg * cmd,E15_String *& json);
	virtual void OnResponse(E15_ClientInfo * user,E15_ClientMsg * cmd,E15_String *& json);
	virtual void OnNotify(E15_ClientInfo * user,E15_ClientMsg * cmd,E15_String *& json);

public:
	void Lock();
	void Unlock();

private:

	E15_ValueTable   	m_vt;
	E15_Lock  			m_lock;

	E15_Zip 				m_unzip;
	E15_String			m_unzip_buffer;
    E15_Log m_log;
};

extern MD_Client * g_client;
extern E15_Id   * g_history_srv;

#endif
