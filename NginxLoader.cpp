/*

编译方法:
	cl NginxLoader.cpp advapi32.lib user32.lib
*/



#if _MSC_VER > 1000
#pragma once
#endif

#include <windows.h>

#include "stdio.h"
#include "tchar.h"

//定义全局函数变量
BOOL IsInstalled();
BOOL Install();
BOOL Uninstall();
BOOL Nginx(TCHAR *);

void LogEvent(LPCTSTR pszFormat, ...);
void WINAPI ServiceMain();
void WINAPI ServiceStrl(DWORD dwOpcode);

TCHAR szServiceName[] = _T("Nginx");
SERVICE_STATUS_HANDLE hServiceStatus;
SERVICE_STATUS status;
DWORD dwThreadID;

int APIENTRY WinMain(HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPSTR	lpCmdLine,
					int	nCmdShow) {

	hServiceStatus = NULL;
	status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	status.dwCurrentState = SERVICE_STOPPED;
	status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	status.dwWin32ExitCode = 0;
	status.dwServiceSpecificExitCode = 0;
	status.dwCheckPoint = 0;
	status.dwWaitHint = 0;

	dwThreadID = ::GetCurrentThreadId();

	SERVICE_TABLE_ENTRY st[] = {
		{ szServiceName, (LPSERVICE_MAIN_FUNCTION)ServiceMain },
		{ NULL, NULL }
	};

	if (stricmp(lpCmdLine, "/install") == 0) {
		Install();
	} else if (stricmp(lpCmdLine, "/uninstall") == 0) {
		Uninstall();
	} else {
		if (!::StartServiceCtrlDispatcher(st)) {
			LogEvent(_T("Register Service Main Function Error!"));
		}
	}

	return 0;
}


void WINAPI ServiceMain()
{
	// Register the control request handler
	status.dwCurrentState = SERVICE_START_PENDING;
	status.dwControlsAccepted = SERVICE_ACCEPT_STOP;

	//注册服务控制
	hServiceStatus = RegisterServiceCtrlHandler(szServiceName, ServiceStrl);
	if (hServiceStatus == NULL)
	{
		LogEvent(_T("Handler not installed"));
		return;
	}

	//设置服务状态
	SetServiceStatus(hServiceStatus, &status);

	status.dwWin32ExitCode = S_OK;
	status.dwCheckPoint = 0;
	status.dwWaitHint = 0;
	status.dwCurrentState = SERVICE_RUNNING;
	SetServiceStatus(hServiceStatus, &status);

	if(Nginx("")){//启动Nginx
		while (status.dwCurrentState == SERVICE_RUNNING){
			Sleep(1000);
		}
	}

	status.dwCurrentState = SERVICE_STOPPED;
	SetServiceStatus(hServiceStatus, &status);
	LogEvent(_T("Service stopped"));
}

void WINAPI ServiceStrl(DWORD dwOpcode)
{
	switch (dwOpcode)
	{
		case SERVICE_CONTROL_STOP:
			Nginx(" -s stop");

			status.dwCurrentState = SERVICE_STOP_PENDING;
			SetServiceStatus(hServiceStatus, &status);

			PostThreadMessage(dwThreadID, WM_CLOSE, 0, 0);
			break;
		case SERVICE_CONTROL_PAUSE:
			break;
		case SERVICE_CONTROL_CONTINUE:
			break;
		case SERVICE_CONTROL_INTERROGATE:
			break;
		case SERVICE_CONTROL_SHUTDOWN:
			break;
		default:
			LogEvent(_T("Bad service request"));
	}
}

BOOL IsInstalled()
{
	BOOL bResult = FALSE;

	//打开服务控制管理器
	SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if (hSCM != NULL)
	{
		//打开服务
		SC_HANDLE hService = ::OpenService(hSCM, szServiceName, SERVICE_QUERY_CONFIG);
		if (hService != NULL)
		{
			bResult = TRUE;
			::CloseServiceHandle(hService);
		}
		::CloseServiceHandle(hSCM);
	}
	return bResult;
}


BOOL Install()
{
	if (IsInstalled()){
		return TRUE;
	}

	//打开服务控制管理器
	SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hSCM == NULL)
	{
		MessageBox(NULL, _T("Couldn't open service manager"), szServiceName, MB_OK);
		return FALSE;
	}

	// Get the executable file path
	TCHAR szFilePath[MAX_PATH];
	::GetModuleFileName(NULL, szFilePath, MAX_PATH);

	//创建服务
	SC_HANDLE hService = ::CreateService(
		hSCM, szServiceName, szServiceName,
		SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
		SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL,
		szFilePath, NULL, NULL, _T(""), NULL, NULL);

	if (hService == NULL)
	{
		::CloseServiceHandle(hSCM);
		MessageBox(NULL, _T("Couldn't create service"), szServiceName, MB_OK);
		return FALSE;
	}

	::CloseServiceHandle(hService);
	::CloseServiceHandle(hSCM);
	return TRUE;
}

BOOL Uninstall()
{
	if (!IsInstalled()){
		return TRUE;
	}

	SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if (hSCM == NULL)
	{
		MessageBox(NULL, _T("Couldn't open service manager"), szServiceName, MB_OK);
		return FALSE;
	}

	SC_HANDLE hService = ::OpenService(hSCM, szServiceName, SERVICE_STOP | DELETE);

	if (hService == NULL)
	{
		::CloseServiceHandle(hSCM);
		MessageBox(NULL, _T("Couldn't open service"), szServiceName, MB_OK);
		return FALSE;
	}
	SERVICE_STATUS status;
	::ControlService(hService, SERVICE_CONTROL_STOP, &status);

	//删除服务
	BOOL bDelete = ::DeleteService(hService);
	::CloseServiceHandle(hService);
	::CloseServiceHandle(hSCM);

	if (bDelete)
		return TRUE;

	LogEvent(_T("Service could not be deleted"));
	return FALSE;
}


void LogEvent(LPCTSTR pFormat, ...)
{
	TCHAR	chMsg[256];
	HANDLE  hEventSource;
	LPTSTR  lpszStrings[1];
	va_list pArg;

	va_start(pArg, pFormat);
	_vstprintf(chMsg, pFormat, pArg);
	va_end(pArg);

	lpszStrings[0] = chMsg;

	hEventSource = RegisterEventSource(NULL, szServiceName);
	if (hEventSource != NULL)
	{
		ReportEvent(hEventSource, EVENTLOG_INFORMATION_TYPE, 0, 0, NULL, 1, 0, (LPCTSTR*) &lpszStrings[0], NULL);
		DeregisterEventSource(hEventSource);
	}
}


BOOL Nginx(TCHAR *cmd){

	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	TCHAR szFilePath[MAX_PATH], NginxBinPath[MAX_PATH] = {0};
	::GetModuleFileName(NULL, szFilePath, MAX_PATH);

	(_tcsrchr(szFilePath, _T('\\')))[1] = 0;

	sprintf(NginxBinPath, "%snginx.exe", szFilePath);

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	return CreateProcess(NginxBinPath,   // No module name (use command line)
		cmd,		// Command line  //TEXT("X:\\xx.exe"),
		NULL,		   // Process handle not inheritable
		NULL,		   // Thread handle not inheritable
		FALSE,		  // Set handle inheritance to FALSE
		0,			  // No creation flags
		NULL,		   // Use parent's environment block
		szFilePath,		   // Use parent's starting directory
		&si,			// Pointer to STARTUPINFO structure
		&pi);		   // Pointer to PROCESS_INFORMATION structure
}
