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

    void destroyPlayer(PlayerObject* p0, GameObject* p1) {
        if (g_isNoclipActive) return;
        if (g_isWaitingForDecision) return;
        g_isWaitingForDecision = true;

        // Самый стабильный метод Geode v3. Никаких fmt, никаких капризных макросов!
        geode::createQuickPopup(
            "Are you sure?",            // Заголовок
            "Do you really want to die?", // Текст
            "No", "Yes",                // Кнопки
            [this](auto, bool btn2) {
                if (btn2) {
                    g_isNoclipActive = true;
                    Notification::create("Saved! Noclip active!", NotificationIcon::Success)->show();

                    auto scheduler = cocos2d::CCDirector::sharedDirector()->getScheduler();
                    scheduler->scheduleSelector(
                        schedule_selector(MyPlayLayer::disableNoclip), 
                        this, 
                        0.0f, 0, 5.0f, false
                    );
                } else {
                    g_isNoclipActive = false;
                    g_isWaitingForDecision = false;
                    this->destroyPlayer(this->m_player1, nullptr);
                }
                g_isWaitingForDecision = false;
            }
        );
    }
};
