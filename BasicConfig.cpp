#include "BasicConfig.h"
#include"Logger.h"
#include <sstream>
#include <unordered_map>
#include <algorithm>
#include <cctype>
#include <vector>
using Logger::Log;
namespace {

    // 去掉首尾空白
    std::string Trim(const std::string& s) {
        size_t b = s.find_first_not_of(" \t\r\n");
        if (b == std::string::npos) return "";
        size_t e = s.find_last_not_of(" \t\r\n");
        return s.substr(b, e - b + 1);
    }

    std::string ToLower(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(),
            [](unsigned char c) { return std::tolower(c); });
        return s;
    }

    // ---- SDL_WINDOW_* 映射 ----
    const std::unordered_map<std::string, Uint64>& WindowModeMap() {
        static const std::unordered_map<std::string, Uint64> m = {
            { "SDL_WINDOW_FULLSCREEN",         SDL_WINDOW_FULLSCREEN },
            { "SDL_WINDOW_OPENGL",             SDL_WINDOW_OPENGL },
            { "SDL_WINDOW_HIDDEN",             SDL_WINDOW_HIDDEN },
            { "SDL_WINDOW_BORDERLESS",         SDL_WINDOW_BORDERLESS },
            { "SDL_WINDOW_RESIZABLE",          SDL_WINDOW_RESIZABLE },
            { "SDL_WINDOW_MINIMIZED",          SDL_WINDOW_MINIMIZED },
            { "SDL_WINDOW_MAXIMIZED",          SDL_WINDOW_MAXIMIZED },
            { "SDL_WINDOW_HIGH_PIXEL_DENSITY", SDL_WINDOW_HIGH_PIXEL_DENSITY },
        };
        return m;
    }

    // ---- SDL_INIT_* 映射 ----
    const std::unordered_map<std::string, Uint64>& InitFlagMap() {
        static const std::unordered_map<std::string, Uint64> m = {
            { "SDL_INIT_AUDIO",    SDL_INIT_AUDIO },
            { "SDL_INIT_VIDEO",    SDL_INIT_VIDEO },
            { "SDL_INIT_JOYSTICK", SDL_INIT_JOYSTICK },
            { "SDL_INIT_HAPTIC",   SDL_INIT_HAPTIC },
            { "SDL_INIT_GAMEPAD",  SDL_INIT_GAMEPAD },
            { "SDL_INIT_EVENTS",   SDL_INIT_EVENTS },
            { "SDL_INIT_SENSOR",   SDL_INIT_SENSOR },
            { "SDL_INIT_CAMERA",   SDL_INIT_CAMERA },
        };
        return m;
    }

    // 解析 "A | B | C" 形式的表达式
    Uint64 ParseFlagExpr(const std::string& expr,
        const std::unordered_map<std::string, Uint64>& table) {
        Uint64 result = 0;
        std::stringstream ss(expr);
        std::string token;
        while (std::getline(ss, token, '|')) {
            token = Trim(token);
            if (token.empty()) continue;
            auto it = table.find(token);
            if (it != table.end()) result |= it->second;
            // 未识别的 token 可加日志警告
        }
        return result;
    }

} // namespace

void BasicConfig::LoadConfig(const std::filesystem::path& gp) {
    std::ifstream configin(gp /"config"/ "BasicConfig.ini");
    if (!configin.is_open()) {
        Log("ConfigInit", __func__, Logcode::OPENFAILED);
        return;
    }

    std::string line;
    while (std::getline(configin, line)) {
        // 去注释（# 或 ;）
        size_t hash = line.find_first_of("#;");
        if (hash != std::string::npos) line = line.substr(0, hash);

        line = Trim(line);
        if (line.empty()) continue;
        if (line.front() == '[') continue; // 跳过 section 头（可选）

        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;

        std::string key = ToLower(Trim(line.substr(0, eq)));
        std::string value = Trim(line.substr(eq + 1));
        if (key.empty() || value.empty()) continue;

        if (key == "name") {
            GameName = value;
        }
        else if (key == "width") {
            try { WindowW = std::stof(value); }
            catch (...) {}
        }
        else if (key == "height") {
            try { WindowH = std::stof(value); }
            catch (...) {}
        }
        else if (key == "windowmode") {
            WindowMode = ParseFlagExpr(value, WindowModeMap());
        }
        else if (key == "font") {
            FontName = value;
        }
        else if (key == "ptsize") {
            try { FontSize = std::stof(value); }
            catch (...) {}
        }
        else if (key == "gamemode") {
            GameMode = static_cast<Uint32>(ParseFlagExpr(value, InitFlagMap()));
        }
        else if (key == "frametime") {
            FrameTime = static_cast<Uint32>(std::stoi(value));
        }
        else if (key == "gamesaved") {
            try { GameSaved = std::stoi(value); }
            catch (...) {}
        }
    }
}

bool BasicConfig::SetGameSaved(const std::filesystem::path& gp, int value) {
    const std::filesystem::path path = gp / "config" / "BasicConfig.ini";
    std::ifstream in(path);
    if (!in.is_open()) {
        Log("BasicConfig.ini open failed", __func__, Logcode::OPENFAILED);
        return false;
    }

    std::vector<std::string> lines;
    std::string line;
    bool found = false;
    while (std::getline(in, line)) {
        std::string bare = line;
        size_t hash = bare.find_first_of("#;");
        if (hash != std::string::npos) bare = bare.substr(0, hash);

        const std::string trimmed = Trim(bare);
        const size_t eq = trimmed.find('=');
        if (eq != std::string::npos &&
            ToLower(Trim(trimmed.substr(0, eq))) == "gamesaved") {
            lines.push_back("gamesaved=" + std::to_string(value));
            found = true;
        }
        else {
            lines.push_back(line);
        }
    }
    in.close();

    if (!found) {
        lines.push_back("gamesaved=" + std::to_string(value));
    }

    std::error_code ec;
    std::filesystem::create_directories(path.parent_path(), ec);
    std::filesystem::path tmp = path;
    tmp += ".tmp";
    std::ofstream out(tmp, std::ios::trunc);
    if (!out.is_open()) {
        Log("BasicConfig.ini tmp open failed", __func__, Logcode::OPENFAILED);
        return false;
    }
    for (const auto& l : lines) out << l << "\n";
    out.close();

    std::filesystem::remove(path, ec);
    std::filesystem::rename(tmp, path, ec);
    if (ec) {
        Log("BasicConfig.ini rename failed", __func__, Logcode::ERROR);
        std::filesystem::remove(tmp, ec);
        return false;
    }
    GameSaved = value;
    return true;
}