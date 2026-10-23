#include"GameStart_Texture.h"
#include"json.hpp"
#include"Logger.h"
#include<fstream>
#include<sstream>
using json = nlohmann::json;
using Logger::Log;
namespace GameStart {
	Texture::~Texture() {

	}
	void Texture::Loadin(const std::filesystem::path&, SDL_Renderer*&) {

	 }
	void Texture::Draw(SDL_Renderer*&, TTF_Font*&) {

	}
	void Texture::LoadinJson(const std::filesystem::path& gp) {
		
	}
	void Texture::LoadinPic(const std::filesystem::path&, SDL_Renderer*&) {

	}
}