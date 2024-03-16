#include <Geode/Geode.hpp>
#include <Geode/ui/GeodeUI.hpp>
#include <Geode/modify/PauseLayer.hpp>

using namespace geode::prelude;

// enabled toggle!
// this kinda breaks with betterpause because for some reason it forces the settings sprite scale to 0.6f
// but i hope nobody will notice
class $modify(MyPauseLayer, PauseLayer)
{
	void onAutoCheckpointSettings(CCObject * object)
	{
		geode::openSettingsPopup(Mod::get());
	}

	// for some reason every single fucking time i add my button to ANY of the menus it BREAKS
	// the layout so instead i'm going to hardcode the positions!!!!
	void customSetup()
	{
		PauseLayer::customSetup();

		if (!Mod::get()->getSettingValue<bool>("pause-layer-btn")) return;

		auto winSize = CCDirector::sharedDirector()->getWinSize();

		auto ccmenu = CCMenu::create();
		ccmenu->setPosition(CCPoint{ 0, 0 });
		ccmenu->setID("settings-button-menu"_spr);

		auto* settingsButtonSprite = CCSprite::create("logo.png"_spr);
		settingsButtonSprite->setScale(0.25f);

		auto settingsBtn = CCMenuItemSpriteExtra::create(
			settingsButtonSprite,
			this,
			menu_selector(MyPauseLayer::onAutoCheckpointSettings)
		);
		settingsBtn->setID("settings-button"_spr);

		// hardcoded positions!
		auto padding = 20;
		auto btnRadius = settingsBtn->getContentSize().height / 2;
		auto offset = padding + btnRadius;
		settingsBtn->setPosition(CCPoint{ winSize.width - offset, offset });

		ccmenu->addChild(settingsBtn);
		this->addChild(ccmenu);
	}
};
