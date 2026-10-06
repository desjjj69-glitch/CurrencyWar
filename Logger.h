#pragma once
#include<fstream>
#include<filesystem>
enum class Logcode:int {
	OK,
	ERROR,
	OPENFAILED,
	NOTFOUND,
	CREATEFAILED,
	PARSEERROR,
	RENDERFAILED
};
namespace Logger {
	inline std::ofstream log;
	void init(const std::filesystem::path&);
	void Log(const std::string&, const std::string&, Logcode err=Logcode::OK);//Logname,func,err
};
