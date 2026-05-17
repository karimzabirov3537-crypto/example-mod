#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

bool g_isNoclipActive = false;
bool g_isWaitingForDecision = false;

class $modify(MyPlayLayer, PlayLayer) {
    void disableNoclip(float dt) {
        g_isNoclipActive = false;
        Notification::create("Noclip deactivated!", NotificationIcon::Warning)->show();
    }

    void onYesClicked(cocos2d::CCObject* sender) {
        g_isNoclipActive = true;
        Notification::create("Saved! Noclip active!", NotificationIcon::Success)->show();

        if (auto layer = cocos2d::CCDirector::sharedDirector()->getRunningScene()->getChildByTag(9999)) {
            layer->removeFromParentAndCleanup(true);
        }

        auto scheduler = cocos2d::CCDirector::sharedDirector()->getScheduler();
        scheduler->scheduleSelector(
            schedule_selector(MyPlayLayer::disableNoclip), 
            this, 
            0.0f, 0, 5.0f, false
        );

        g_isWaitingForDecision = false;
    }

    void onNoClicked(cocos2d::CCObject* sender) {
        if (auto layer = cocos2d::CCDirector::sharedDirector()->getRunningScene()->getChildByTag(9999)) {
            layer->removeFromParentAndCleanup(true);
        }
        g_isNoclipActive = false;
        g_isWaitingForDecision = false;
        this->destroyPlayer(this->m_player1, nullptr);
    }

    void destroyPlayer(PlayerObject* p0, GameObject* p1) {
        if (g_isNoclipActive) return;
        if (g_isWaitingForDecision) return;
        g_isWaitingForDecision = true;

        auto scene = cocos2d::CCDirector::sharedDirector()->getRunningScene();
        if (!scene) return;

        // Создаем чистое полупрозрачное окно Cocos2d-x поверх игры (0% зависимости от Geode/fmt!)
        auto customAlert = cocos2d::CCLayerColor::create(cocos2d::ccc4(0, 0, 0, 180));
        customAlert->setTag(9999);

        // Добавляем текст вопроса
        auto label = cocos2d::CCLabelBMFont::create("Are you sure you want to die?", "goldFont.fnt");
        label->setPosition(cocos2d::CCDirector::sharedDirector()->getWinSize() / 2 + cocos2d::CCSize(0, 40));
        customAlert->addChild(label);

        // Создаем кнопку "Yes" по стандартам селекторов Android
        auto yesLabel = cocos2d::CCLabelBMFont::create("Yes", "bigFont.fnt");
        auto yesBtn = cocos2d::CCMenuItemLabel::create(yesLabel, this, menu_selector(MyPlayLayer::onYesClicked));
        yesBtn->setPosition(cocos2d::CCPoint(-60, -30));

        // Создаем кнопку "No" по стандартам селекторов Android
        auto noLabel = cocos2d::CCLabelBMFont::create("No", "bigFont.fnt");
        auto noBtn = cocos2d::CCMenuItemLabel::create(noLabel, this, menu_selector(MyPlayLayer::onNoClicked));
        noBtn->setPosition(cocos2d::CCPoint(60, -30));

        // Собираем меню кнопок
        auto menu = cocos2d::CCMenu::create(yesBtn, noBtn, nullptr);
        customAlert->addChild(menu);

        // Показываем на экране
        scene->addChild(customAlert, 100);
    }
};
