#include"Logger.h"
#include<SDL3/SDL.h>
void Logger::init(const std::filesystem::path& gp) {
	Logger::log.open(gp / "Log.txt");
}
void Logger::Log(const std::string& logname, const std::string& func, Logcode err){
	if (!Logger::log.is_open()) {
		Logger::init(SDL_GetBasePath());
	}
	static bool Logallow = true;
	if (!Logallow ) {
		if (func != "Debug") {
			return;
		}
	}
	if (func == "Debug") {
		Logallow = false;
	}
	switch (err) {
	case Logcode::OK:
		Logger::log <<"["<< SDL_GetTicks()<< " Log:" << func << "]:" << logname << std::endl;
		break;
	case Logcode::ERROR:
		Logger::log << "[Error:" << func << "]:" << logname << std::endl;
		break;
	case Logcode::CREATEFAILED:
		Logger::log << "[Fail to create:" << func << "]:" << logname << std::endl;
		break;
	case Logcode::NOTFOUND :
		Logger::log << "[NotFound:" << func << "]:" << logname << std::endl;
		break;
	case Logcode::OPENFAILED :
		Logger::log << "[Fail to open:" << func << "]:" << logname << std::endl;
		break;
	case Logcode::PARSEERROR:
		Logger::log << "[Parse error:" << func << "]:" << logname << std::endl;
		break;
	case Logcode::RENDERFAILED:
		Logger::log << "[Render failed:" << func << "]:" << logname << std::endl;
		break;
	default:
		Logger::log << "[Unexpected error:" << func << "]:" << logname << std::endl;
		break;
	}
}