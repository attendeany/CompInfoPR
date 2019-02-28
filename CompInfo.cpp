// CompInfo.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include <iostream>
#include <WinSock2.h>
#include <Windows.h>
#include <string>
#include <vector>
#include <fstream>
#pragma comment(lib, "ws2_32.lib")
 
using namespace std;
#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

//returns values to &hostname and &ip_list
bool get_name_and_ips(string &hostname,vector<string> &ip_list)
{
	//getting PC name
	char ac[80];
	//https://docs.microsoft.com/en-us/windows/desktop/api/winsock/nf-winsock-gethostname
	if (gethostname(ac, sizeof(ac)) == SOCKET_ERROR) {
		cerr << "Error " << WSAGetLastError() <<
			" when getting local host name." << endl;
		return 1;
	}
	std::cout << "Host name is " << ac << "." << endl;
	hostname = ac;

	//getting hostent structure (contains ip_addr_list)
	//https://docs.microsoft.com/en-us/windows/desktop/api/wsipv6ok/nf-wsipv6ok-gethostbyname
	struct hostent *phe = gethostbyname(ac);
	if (phe == 0) {
		cerr << "Bad host lookup." << endl;
		return false;
	}

	//parsing ip_addr_list
	for (int i = 0; phe->h_addr_list[i] != 0; ++i) {
		struct in_addr addr;
		memcpy(&addr, phe->h_addr_list[i], sizeof(struct in_addr));
		std::cout << "Address " << i << ": " << inet_ntoa(addr) << endl;
		ip_list.push_back(inet_ntoa(addr));
	}

	return true;
}

struct SysInfo{
	string hostname;
	vector<string>ip_list;
	string win_version;
	string office_version;
	string antivirus;

	void export_to_file()
	{
		string filename = "Info about " + this->hostname + ".txt";
		ofstream fout(filename);

		fout << this->hostname << endl;
		for (const auto &i : this->ip_list) {
			fout << i << endl;
		}
		fout << this->win_version << endl << this->office_version << endl << this->antivirus;

		std::cout << "saved to " << filename << endl;
	}
};

//returns the Windows version, bit depth, service pack and build to &win_version
void get_win_version(string &win_version)
{
	//getting Windows version 
	//https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/content/wdm/nf-wdm-rtlgetversion
	NTSTATUS(WINAPI *RtlGetVersion)(LPOSVERSIONINFOEXW);
	
	//OSVERSIONINFOEXW contains dwMajorVersion, dwMinorVersion, dwBuildNumber, szCSDVersion (SP)
	OSVERSIONINFOEXW osInfo;

	*(FARPROC*)&RtlGetVersion = GetProcAddress(GetModuleHandleA("ntdll"), "RtlGetVersion");

	if (NULL != RtlGetVersion)
	{
		osInfo.dwOSVersionInfoSize = sizeof(osInfo);
		RtlGetVersion(&osInfo);
	}
	
	//check for Service Pack
	std::wstring sp(osInfo.szCSDVersion);
	if (!sp.empty())
		sp.insert(sp.begin(),' ');

	//Windows edition
	string winEdition = "";
	char edition[255];
	DWORD ed_size = sizeof(edition);
	if (RegGetValueA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", "EditionID", RRF_RT_REG_SZ, NULL, edition, &ed_size) == ERROR_SUCCESS) {
		winEdition = edition;
		winEdition += ' ';
	}

	//parse the Windows version
	//The following table summarizes the most recent operating system version numbers:
	//https://docs.microsoft.com/ru-ru/windows/desktop/SysInfo/operating-system-version
	switch (osInfo.dwMajorVersion)
	{
	case 5:
		if (osInfo.dwMinorVersion == 0)//win 5.0
			win_version = "Windows 2000 "+winEdition+"build " + to_string(osInfo.dwBuildNumber);
		else if (osInfo.dwMinorVersion == 1)//win 5.1
				win_version = "Windows XP "+ winEdition + "x32 build " + string(sp.begin(), sp.end()) + ' ' + to_string(osInfo.dwBuildNumber);
			 else //win 5.2
				win_version = "Windows XP " + winEdition + "x64 build " + string(sp.begin(), sp.end()) + ' ' + to_string(osInfo.dwBuildNumber);
			 return;
	case 6:
		if (osInfo.dwMinorVersion == 0)//win 6.0
			win_version = "Windows Vista ";
		else if (osInfo.dwMinorVersion == 1)//win 6.1
			win_version = "Windows 7 ";
		else if (osInfo.dwMinorVersion == 2)//win 6.2
			win_version = "Windows 8 ";
		else //win 6.3
			win_version = "Windows 8.1 ";
		break;
	case 10://win 10.0
		win_version = "Windows 10 ";
			break;
	default:
		cout << "win version detectinon error\n";
		win_version = "error ";
		//system("pause");
		//exit(3);
		break;
	}
	
	//getting Windows bit deph
	_SYSTEM_INFO  sysinf;//contains wProcessorArchitecture
	//https://docs.microsoft.com/en-us/windows/desktop/api/sysinfoapi/nf-sysinfoapi-getnativesysteminfo
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

	win_version += winEdition + bit_deph + string(sp.begin(),sp.end()) + " build "+ to_string(osInfo.dwBuildNumber);
	std::cout << win_version << endl;
}

//returns the Microsoft Office version and its bitness + Kaspersky antivirus info
void get_office_and_kaspersky_version(string &office_version, string &kaspersky_info){
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall"), 0, KEY_READ, &hKey) != ERROR_SUCCESS)
	{
		cout << "Ошибка чтения реестра\n";
		office_version = "Ошибка office_version";
		kaspersky_info = "Ошибка kaspersky_info";
		return;
	}
	
	DWORD    cSubKeys = 0;               // number of subkeys 
	
	DWORD retCode;
	// Get the class name and the value count. 
	retCode = RegQueryInfoKey(
		hKey,                    // key handle 
		NULL,                // buffer for class name 
		NULL,           // size of class string 
		NULL,                    // reserved 
		&cSubKeys,               // number of subkeys 
		NULL,            // longest subkey size 
		NULL,            // longest class string 
		NULL,                // number of values for this key 
		NULL,            // longest value name 
		NULL,         // longest value data 
		NULL,   // security descriptor 
		NULL);       // last write time 

	string first_office="";

	if (cSubKeys)
	{
		bool is_office_found = false;
		bool is_kasper_found = false;
		for (DWORD i = 0; i < cSubKeys; i++)
		{
			char achKey[MAX_KEY_LENGTH];   // buffer for subkey name
			DWORD cbName = sizeof(achKey);

			if (RegEnumKeyExA(hKey, i, achKey, &cbName, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
			{
				char achValue[MAX_KEY_LENGTH];
				DWORD cchValue = MAX_VALUE_NAME;

				if (RegGetValueA(hKey, achKey, "DisplayName", RRF_RT_REG_SZ, NULL, achValue, &cchValue) == ERROR_SUCCESS)
				{
					string progName(achValue);
					if (!is_office_found&&progName.find("Microsoft Office") != std::string::npos)
					{
						first_office = achValue;

						if (RegGetValueA(hKey, achKey, "InstallLocation", RRF_RT_REG_SZ, NULL, achValue, &cchValue) == ERROR_SUCCESS) {
							string progPath(achValue);
							if (progPath.find("(x86)") != std::string::npos)
								first_office += " x32";
							else
								first_office += " x64";
						}
						string products[3] = { "рофессиональный","ля дома","тандартный" };
						
						for (int i = 0; i < 3; i++)
						{
							if (progName.find(products[i]) != std::string::npos)
							{
								office_version = first_office;
								is_office_found = true;
								break;
							}
						}
						continue;						
					}
					if (!is_kasper_found&&progName.find("Kaspersky") != std::string::npos)
					{
						string products[7] = {"Endpoint Security","Small Office Security",
						"Embedded Systems Security","Total Security","Anti-Virus",
						"Internet Security","Free"};
						for (int i = 0; i < 7; i++)
						{
							if (progName.find(products[i]) != std::string::npos)
							{
								kaspersky_info = achValue;
								
								if (RegGetValueA(hKey, achKey, "DisplayVersion", RRF_RT_REG_SZ, NULL, achValue, &cchValue) == ERROR_SUCCESS)
									kaspersky_info += ' '+string(achValue);
								
								is_kasper_found = true;
								break;
							}
						}											
					}
				}		
			}
		}

		if (!office_version.empty())
			cout << office_version << endl;
		else{
			if (!first_office.empty())
				office_version = first_office;
			else
				office_version = "Microsoft Office отсутствует";
			cout << office_version << endl;
		}
		if (!kaspersky_info.empty())
			cout << kaspersky_info << endl;
		else {
			kaspersky_info = "Kaspersky Antivirus отсутствует";
			cout << kaspersky_info << endl;
		}
	}
	RegCloseKey(hKey);
}

int main()
{	
	setlocale(LC_ALL, "");
	
	WSAData wsaData;
	//WinSock initialization
	if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0) {
		int err = WSAGetLastError();
		std::cout << "socket_error " << err;
		return err;
	}

	SysInfo info;
	if (!get_name_and_ips(info.hostname,info.ip_list))
		return 1;

	WSACleanup();//WinSock terminate

	get_win_version(info.win_version);

	get_office_and_kaspersky_version(info.office_version, info.antivirus);

	info.export_to_file();

	system("pause");
	return 0;
}

