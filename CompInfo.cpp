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

	void export_to_file()
	{
		string filename = "Info about " + this->hostname + ".txt";
		ofstream fout(filename);

		fout << this->hostname << endl;
		for (const auto &i : this->ip_list) {
			fout << i << endl;
		}
		fout << this->win_version;

		cout << "saved to " << filename << endl;
	}
};

void get_win_version(string &win_version)
{
	//Version
	DWORD encodedVersion = GetVersion(),
		dwmajorVersion,
		dwminorVersion,
		dwBuild;

	dwmajorVersion = (DWORD)(LOBYTE(LOWORD(encodedVersion)));
	dwminorVersion = (DWORD)(HIBYTE(LOWORD(encodedVersion)));

	if (encodedVersion < 0x80000000)
		dwBuild = (DWORD)(HIWORD(encodedVersion));
	
	switch (dwmajorVersion)
	{
	case 5:
		if (dwminorVersion == 0)
			win_version = "Windows 2000 build " + to_string(dwBuild);
		else if (dwminorVersion == 1)
				win_version = "Windows XP x32 build " + to_string(dwBuild);
			 else
				win_version = "Windows XP x64 build " + to_string(dwBuild);
			 return;
	case 6:
		if (dwminorVersion == 0)
			win_version = "Windows Vista ";
		else if (dwminorVersion == 1)
			win_version = "Windows 7 ";
		else if (dwminorVersion == 2)
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

	win_version += bit_deph + " build "+ to_string(dwBuild);
	cout << win_version << endl;
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

	info.export_to_file();

	system("pause");
	return 0;
}

