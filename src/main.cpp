#include <Geode/Geode.hpp>
#include <Geode/ui/GeodeUI.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <chrono>

using namespace geode::prelude;

int getGamemodeAsInt(PlayerObject* player)
{
	if (player->m_isShip) return 1;
	if (player->m_isBall) return 2;
	if (player->m_isBird) return 3; // ufo
	if (player->m_isDart) return 4; // wave
	if (player->m_isRobot) return 5;
	if (player->m_isSpider) return 6;
	if (player->m_isSwing) return 7;

	return 0; // cube
}

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

class $modify(PlayLayer) {
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
	int gamemode = getGamemodeAsInt(this->m_player1);
	float speed = this->m_player1->m_playerSpeed;
	float size = this->m_player1->m_vehicleSize;

	// for things that have hooks --------------------------------------------------------------------------
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

	// for things that don't have hooks -------------------------------------------------------------------
	void update(float dt)
	{
		GJBaseGameLayer::update(dt);

		if (cmpint(gamemode, getGamemodeAsInt(this->m_player1), "gamemode"))
			placeCheckpointNextFrame = true;

		if (cmpfloat(speed, this->m_player1->m_playerSpeed, "speed"))
			placeCheckpointNextFrame = true;

		if (cmpfloat(size, this->m_player1->m_vehicleSize, "size"))
			placeCheckpointNextFrame = true;

		// update things
		gamemode = getGamemodeAsInt(this->m_player1);
		speed = this->m_player1->m_playerSpeed;
		size = this->m_player1->m_vehicleSize;
	}
};

// enabled toggle!
class $modify(MyPauseLayer, PauseLayer)
{
	void onAutoCheckpointSettings(CCObject* object)
	{
		geode::openSettingsPopup(Mod::get());
	}
	
	void customSetup()
	{
		std::cout << "custmsetup" << std::endl;
		PauseLayer::customSetup();

		auto* lbm = this->getChildByID("left-button-menu");
		if (!lbm)
		{
			// no left button menu this is so sad rip right button menu :(

			std::cout << "no left button menu :(" << std::endl;
			return;
		}

		auto* spr = CCSprite::create("logo.png"_spr);
		auto settingsBtn = CCMenuItemSpriteExtra::create(
			spr,
			this,
			menu_selector(MyPauseLayer::onAutoCheckpointSettings)
		);
		settingsBtn->setID("settings-button"_spr);
		lbm->addChild(settingsBtn);


		// this is so broken and will 100% break with other mods (only tested with click sounds) but oh well :)
		if (auto rawLayout = lbm->getLayout()) {
			if (auto layout = typeinfo_cast<AxisLayout*>(rawLayout)) {
				layout->setCrossAxisOverflow(true);
				layout->setAutoScale(false);
			}
		}

		settingsBtn->setScale(0.3);
		settingsBtn->m_baseScale = 0.3;
		lbm->updateLayout();
	}
};
