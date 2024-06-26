#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include "./PauseButton.cpp"
#include <chrono>

using namespace geode::prelude;

bool cmpint(int first, int second, std::string lockSetting)
{
	return ((first != second) && Mod::get()->getSettingValue<bool>(lockSetting));
}

bool cmpfloat(float first, float second, std::string lockSetting)
{
	return ((first != second) && Mod::get()->getSettingValue<bool>(lockSetting));
}

bool placeCheckpointNextFrame = false;

auto lastChk = std::chrono::high_resolution_clock::now();

class $modify(PlayLayer)
{
	// this needs to be placed here so that there's a reference to a PlayLayer
	void postUpdate(float dt)
	{
		PlayLayer::postUpdate(dt);
		if (!this->m_isPracticeMode) return;
		if (!Mod::get()->getSettingValue<bool>("enabled")) return;

		// see if it's been some amount of seconds
		auto now = std::chrono::high_resolution_clock::now();
		// difference between now and last checkpoint time (seconds)
		auto diff = std::chrono::duration_cast<std::chrono::seconds>(now - lastChk).count();

		if (placeCheckpointNextFrame && diff >= Mod::get()->getSettingValue<double>("min-delay") && !this->m_player1->m_isDashing)
		{
			lastChk = std::chrono::high_resolution_clock::now();
			PlayLayer::markCheckpoint();
		}

		if (diff >= Mod::get()->getSettingValue<double>("max-delay") && !this->m_player1->m_isDashing)
		{
			log::info("been too long! ({})", std::to_string(diff));
			lastChk = std::chrono::high_resolution_clock::now();
			PlayLayer::markCheckpoint();
		}

		placeCheckpointNextFrame = false;
	}
};

class $modify(GJBaseGameLayer)
{
	void toggleDualMode(GameObject * p0, bool p1, PlayerObject * p2, bool p3)
	{
		if (Mod::get()->getSettingValue<bool>("dual"))
			placeCheckpointNextFrame = true;

		GJBaseGameLayer::toggleDualMode(p0, p1, p2, p3);
	}
};

class $modify(PlayerObject)
{
	void switchedToMode(GameObjectType p0)
	{
		PlayerObject::switchedToMode(p0);
		if (Mod::get()->getSettingValue<bool>("gamemode"))
			placeCheckpointNextFrame = true;
	}

	void reversePlayer(EffectGameObject* p0)
	{
		PlayerObject::reversePlayer(p0);
		if (Mod::get()->getSettingValue<bool>("direction"))
			placeCheckpointNextFrame = true;
	}

	//void rotateGameplay(int p0, int p1, bool p2, float p3, float p4, bool p5, bool p6)
	//{
	//	PlayerObject::rotateGameplay(p0, p1, p2, p3, p4, p5, p6);
	//	if (Mod::get()->getSettingValue<bool>("rotate"))
	//		placeCheckpointNextFrame = true;
	//}

	void flipGravity(bool upsideDown, bool somethingThatsAlwaysFalse)
	{
		PlayerObject::flipGravity(upsideDown, somethingThatsAlwaysFalse);
		if (Mod::get()->getSettingValue<bool>("gravity"))
			placeCheckpointNextFrame = true;
	}

	void update(float dt)
	{
		// shit before the update
		float speed = this->m_playerSpeed;
		float size = this->m_vehicleSize;
		
		PlayerObject::update(dt);

		if (cmpfloat(speed, this->m_playerSpeed, "speed"))
			placeCheckpointNextFrame = true;

		if (cmpfloat(size, this->m_vehicleSize, "size"))
			placeCheckpointNextFrame = true;
	}
};
