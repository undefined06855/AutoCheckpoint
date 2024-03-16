#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include "./PauseButton.cpp"
#include <chrono>

using namespace geode::prelude;

void debLog(std::string str)
{
	//if (Mod::get()->getSettingValue<bool>("DEBUG"))
		std::cout << str << std::endl;
}

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
	void postUpdate(float dt)
	{
		PlayLayer::postUpdate(dt);
		// skip everythin if not in practice mode
		if (!PlayLayer::m_isPracticeMode) return;

		// skip everything if the mod isn't enabled
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
		else if (placeCheckpointNextFrame && this->m_player1->m_isDashing) debLog("player is dashing - cant place");
		else if (placeCheckpointNextFrame) debLog("hasnt been long enough (" + std::to_string(diff) + ")");

		if (diff >= Mod::get()->getSettingValue<double>("max-delay") && !this->m_player1->m_isDashing)
		{
			debLog("been too long! (" + std::to_string(diff) + ")");
			lastChk = std::chrono::high_resolution_clock::now();
			PlayLayer::markCheckpoint();
		}
		else if (diff >= Mod::get()->getSettingValue<double>("max-delay")) debLog("been too long BUT PLAYER IS DASHING :despair:");

		placeCheckpointNextFrame = false;
	}
};

class $modify(GJBaseGameLayer)
{
	float speed = this->m_player1->m_playerSpeed;
	float size = this->m_player1->m_vehicleSize;

	void toggleDualMode(GameObject * p0, bool p1, PlayerObject * p2, bool p3)
	{
		if (Mod::get()->getSettingValue<bool>("dual"))
			placeCheckpointNextFrame = true;

		GJBaseGameLayer::toggleDualMode(p0, p1, p2, p3);
	}

	void flipGravity(PlayerObject* p0, bool upsideDown, bool somethingThatsAlwaysFalse)
	{
		if (Mod::get()->getSettingValue<bool>("gravity"))
			placeCheckpointNextFrame = true;

		GJBaseGameLayer::flipGravity(p0, upsideDown, somethingThatsAlwaysFalse);
	}

	void reverseDirection(EffectGameObject* p0)
	{
		if (Mod::get()->getSettingValue<bool>("direction"))
			placeCheckpointNextFrame = true;
		GJBaseGameLayer::reverseDirection(p0);
	}

	void rotateGameplay(RotateGameplayGameObject* p0)
	{
		if (Mod::get()->getSettingValue<bool>("rotate"))
			placeCheckpointNextFrame = true;
		GJBaseGameLayer::rotateGameplay(p0);
	}

	void update(float dt)
	{
		GJBaseGameLayer::update(dt);

		if (cmpfloat(speed, this->m_player1->m_playerSpeed, "speed"))
			placeCheckpointNextFrame = true;

		if (cmpfloat(size, this->m_player1->m_vehicleSize, "size"))
			placeCheckpointNextFrame = true;

		// update things
		speed = this->m_player1->m_playerSpeed;
		size = this->m_player1->m_vehicleSize;
	}
};


