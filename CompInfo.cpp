// CompInfo.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include <iostream>
#include <WinSock2.h>
#include <Windows.h>
#include <string>
#include <vector>
#include <fstream>
//#include <winsock.h>
//#pragma comment(lib, "Wsock32.lib")
#pragma comment(lib, "ws2_32.lib")
 
using namespace std;

string get_comp_name()
{
	char boof[16];
	DWORD nsize = sizeof(boof);
	if (!GetComputerNameA(boof, &nsize)) {
		cout << "func. get_comp_name: error at GetComputerNameA()";
		return string("error");
	}
	return string(boof);
}

bool get_name_and_ips(string &hostname,vector<string> &ip_list)
{
	char ac[80];
	if (gethostname(ac, sizeof(ac)) == SOCKET_ERROR) {
		cerr << "Error " << WSAGetLastError() <<
			" when getting local host name." << endl;
		return 1;
	}
	cout << "Host name is " << ac << "." << endl;
	hostname = ac;

	struct hostent *phe = gethostbyname(ac);
	if (phe == 0) {
		cerr << "Bad host lookup." << endl;
		return false;
	}
	string a = phe->h_name;
	for (int i = 0; phe->h_addr_list[i] != 0; ++i) {
		struct in_addr addr;
		memcpy(&addr, phe->h_addr_list[i], sizeof(struct in_addr));
		cout << "Address " << i << ": " << inet_ntoa(addr) << endl;
		ip_list.push_back(inet_ntoa(addr));
	}

	return true;
}

struct SysInfo{
	string hostname;
	vector<string>ip_list;
	string win_version;
	string office_version;

	void export_to_file()
	{
		string filename = "Info about " + this->hostname + ".txt";
		ofstream fout(filename);

		fout << this->hostname << endl;
		for (const auto &i : this->ip_list) {
			fout << i << endl;
		}
		fout << this->win_version << endl << this->office_version;

		cout << "saved to " << filename << endl;
	}
};

void get_win_version(string &win_version)
{
	//Version
	NTSTATUS(WINAPI *RtlGetVersion)(LPOSVERSIONINFOEXW);
	OSVERSIONINFOEXW osInfo;

	*(FARPROC*)&RtlGetVersion = GetProcAddress(GetModuleHandleA("ntdll"), "RtlGetVersion");

	if (NULL != RtlGetVersion)
	{
		osInfo.dwOSVersionInfoSize = sizeof(osInfo);
		RtlGetVersion(&osInfo);
	}
	std::wstring sp(osInfo.szCSDVersion);
	if (!sp.empty())
		sp.insert(sp.begin(),' ');

	switch (osInfo.dwMajorVersion)
	{
	case 5:
		if (osInfo.dwMinorVersion == 0)
			win_version = "Windows 2000 build " + to_string(osInfo.dwBuildNumber);
		else if (osInfo.dwMinorVersion == 1)
				win_version = "Windows XP x32 build " + string(sp.begin(), sp.end()) + ' ' + to_string(osInfo.dwBuildNumber);
			 else
				win_version = "Windows XP x64 build " + string(sp.begin(), sp.end()) + ' ' + to_string(osInfo.dwBuildNumber);
			 return;
	case 6:
		if (osInfo.dwMinorVersion == 0)
			win_version = "Windows Vista ";
		else if (osInfo.dwMinorVersion == 1)
			win_version = "Windows 7 ";
		else if (osInfo.dwMinorVersion == 2)
			win_version = "Windows 8 ";
		else
			win_version = "Windows 8.1 ";
		break;
	case 10:
		win_version = "Windows 10 ";
			break;
	default:
		cout << "win version detectinon error\n";
		win_version = "error ";
		//system("pause");
		//exit(3);
		break;
	}
	
	//bit_deph
	_SYSTEM_INFO  sysinf;
	GetNativeSystemInfo(&sysinf);
	string bit_deph;
	switch (sysinf.wProcessorArchitecture)
	{
	case PROCESSOR_ARCHITECTURE_AMD64:
		bit_deph = "x64";
		break;
	case PROCESSOR_ARCHITECTURE_ARM:
		bit_deph = "ARM";
		break;
	case 12:
		bit_deph = "ARM64";
		break;
	case PROCESSOR_ARCHITECTURE_IA64:
		bit_deph = "Intel Itanium-based";
		break;
	case PROCESSOR_ARCHITECTURE_INTEL:
		bit_deph = "x86";
		break;
	default:
		bit_deph = "Unknown architecture";
		break;
	}
	win_version += bit_deph + string(sp.begin(),sp.end()) + " build "+ to_string(osInfo.dwBuildNumber);
	cout << win_version << endl;
}

void get_office_version(string &office_version)
{
	enum OfficeVer {
		Office97 = 7, Office98 = 8, Office2000 = 9, OfficeXP = 10,
		Office2003 = 11, Office2007 = 12, Office2010 = 14, Office2013 = 15, Office2016 = 16};
	try {	
		HKEY rKey;
		if (RegOpenKeyA(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Office", &rKey) != ERROR_SUCCESS)
			throw(string("Не удалось открыть ключ реестра"));

		char name[512];
		DWORD boofsize = sizeof(name);
		int index = 0;
		while (RegEnumKeyExA(rKey, index++, name, &boofsize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
		{
			//string path = "Software\\Microsoft\\Office\\" + string(name) + "\\Outlook";
			//HKEY Office_rKey;
			//if (RegOpenKeyA(HKEY_LOCAL_MACHINE, path.c_str(), &Office_rKey) != ERROR_SUCCESS)
			//	continue;
			
			/*byte bitness;
			DWORD boofbitness = sizeof(bitness);
			if (RegQueryValueExA(Office_rKey, "Bitness",NULL,NULL,&bitness,&boofbitness) != ERROR_SUCCESS) {
				RegCloseKey(Office_rKey);
				continue;}*/
			switch (atoi(name))
			{
			case Office2016:
				office_version = "Microsoft Office 2016";
				break;
			case Office2013:
				office_version = "Microsoft Office 2013";
				break;
			case Office2010:
				office_version = "Microsoft Office 2010";
				break;
			case Office2007:
				office_version = "Microsoft Office 2007";
				break;
			case Office2003:
				office_version = "Microsoft Office 2003";
				break;
			case OfficeXP:
				office_version = "Microsoft Office XP";
				break;
			case Office2000:
				office_version = "Microsoft Office 2000";
				break;
			case Office98:
				office_version = "Microsoft Office 98";
				break;
			case Office97:
				office_version = "Microsoft Office 97";
				break;
			default:
				break;
			}
			if (!office_version.empty()) {
				RegCloseKey(rKey);
				break;}
		}
	}
	catch (string err)
	{
		office_version = "Ошибка чтения реестра";
		cout << err;
		return;
	}
	if (office_version.empty())
		office_version = "Microsoft Office отсутствует";
	cout << office_version << endl;
}

int main()
{	
	setlocale(LC_ALL, "");
	WSAData wsaData;
	if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0) {
		int err = WSAGetLastError();
		cout << "socket_error " << err;
		return err;
	}

	SysInfo info;
	if (!get_name_and_ips(info.hostname,info.ip_list))
		return 1;

	WSACleanup();

	get_win_version(info.win_version);

	get_office_version(info.office_version);

	info.export_to_file();

	system("pause");
	return 0;
}

