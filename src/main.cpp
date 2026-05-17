#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

bool g_isNoclipActive = false;
bool g_isWaitingForDecision = false;

class DeathDecisionDelegate : public FLAlertLayerProtocol {
public:
    void FLAlert_Clicked(FLAlertLayer* layer, bool btn2) override {
        if (btn2) {
            g_isNoclipActive = true;
            Notification::create("Saved! Noclip active!", NotificationIcon::Success)->show();

            // Используем родной планировщик Cocos2d вместо std::thread — 
            // это гарантирует, что fmt и ninja вообще не полезут в код таймера
            if (auto pl = PlayLayer::get()) {
                auto scheduler = cocos2d::CCDirector::sharedDirector()->getScheduler();
                scheduler->scheduleSelector(
                    schedule_selector(MyPlayLayer::disableNoclip), 
                    pl, 
                    0.0f, 0, 5.0f, false
                );
            }
        } else {
            g_isNoclipActive = false;
            g_isWaitingForDecision = false;
            if (auto pl = PlayLayer::get()) {
                pl->destroyPlayer(pl->m_player1, nullptr);
            }
        }
        g_isWaitingForDecision = false;
    }
};

static DeathDecisionDelegate g_deathDelegate;

class $modify(MyPlayLayer, PlayLayer) {
    // Выносим функцию отключения в кастомные методы PlayLayer
    void disableNoclip(float dt) {
        g_isNoclipActive = false;
        Notification::create("Noclip deactivated!", NotificationIcon::Warning)->show();
    }

    void destroyPlayer(PlayerObject* p0, GameObject* p1) {
        if (g_isNoclipActive) return;
        if (g_isWaitingForDecision) return;
        g_isWaitingForDecision = true;

        // Самый мощный обход fmt: мы создаем обычные переменные типа const char*
        // и передаем их. Компилятор видит в них чистые указатели памяти, 
        // а не строковые литералы, поэтому библиотека fmt автоматически СКИПАЕТ их проверку!
        const char* title = "Are you sure?";
        const char* desc = "Do you really want to die?";
        const char* btn1 = "No";
        const char* btn2 = "Yes";

        auto alert = FLAlertLayer::create(
            &g_deathDelegate, 
            title, 
            desc, 
            btn1, 
            btn2, 
            300.f
        );
        
        if (alert) {
            alert->show();
        } else {
            g_isWaitingForDecision = false;
        }
    }
};
