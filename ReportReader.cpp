// ReportReader.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <Windows.h>
#include <string>

using namespace std;

struct SysInfo {
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

vector<string>readfile(const string &fname)
{
	ifstream fin(fname);
	if (!fin.is_open())
	{
		cout << "could not open the " << fname << endl;
		return vector<string>(5, "error");
	}
	
	vector<string>file;
	while (!fin.eof())
	{
		file.push_back("");
		getline(fin, file.back());
	}

	while (file.size()<=5)
	{
		file.push_back("empty");
	}

	return file;
}

SysInfo get_info_from_file(const vector<string>&file)
{
	SysInfo si;
	for (auto i = file.rbegin(); i != file.rend(); i++)
	{
		if (si.antivirus.empty()){
			si.antivirus = *i;
			continue;}
		if (si.office_version.empty()) {
			si.office_version = *i;
			continue;
		}
		if (si.win_version.empty()) {
			si.win_version = *i;
			continue;
		}
		if (i==file.rend()-1) {
			si.hostname = *i;
				continue;
			}
		si.ip_list.push_back(*i);
	}
	return si;
}

void getfiles(vector<string>&filenames)
{
	WIN32_FIND_DATAA found_file;
	HANDLE f_find = FindFirstFileA("*.txt", &found_file);

	if (f_find != INVALID_HANDLE_VALUE)
	{
		do
		{
			filenames.push_back(found_file.cFileName);
		} while (FindNextFileA(f_find, &found_file) != 0);
	}
}


int main()
{
	setlocale(LC_ALL, "");
	cout << "Поместите программу в папку с отчётами, запустите её и нажмите enter\n";
	{
		string input;
		getline(cin, input);
	}

	vector<string> filenames;
	getfiles(filenames);
	
	vector<SysInfo>comps_info;
	for (auto &file:filenames)
	{
		comps_info.push_back(get_info_from_file(readfile(file)));
	}

    return 0;
}

