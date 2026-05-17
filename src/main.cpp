#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

bool g_isNoclipActive = false;
bool g_isWaitingForDecision = false;

// Безопасный делегат без лишних перегрузок
class DeathDecisionDelegate : public FLAlertLayerProtocol {
public:
    void FLAlert_Clicked(FLAlertLayer* layer, bool btn2) override {
        if (btn2) {
            // Игрок выбрал "Yes" -> даем ноуклип
            g_isNoclipActive = true;
            Notification::create("Saved! Noclip active!", NotificationIcon::Success)->show();

            // Родной таймер Cocos2d-x вместо std::thread — никогда не крашит Android
            auto pl = PlayLayer::get();
            if (pl) {
                auto delay = cocos2d::CCDelayTime::create(5.0f);
                auto callback = cocos2d::CCCallFunc::create(pl, callfunc_selector(DeathDecisionDelegate::disableNoclip));
                pl->runAction(cocos2d::CCSequence::create(delay, callback, nullptr));
            }
        } else {
            // Игрок выбрал "No" -> принудительно убиваем его
            g_isNoclipActive = false;
            g_isWaitingForDecision = false;
            if (auto pl = PlayLayer::get()) {
                pl->destroyPlayer(pl->m_player1, nullptr);
            }
        }
        g_isWaitingForDecision = false;
    }

    // Метод выключения, который вызовется экшеном Cocos2d
    static void disableNoclip() {
        g_isNoclipActive = false;
        Notification::create("Noclip deactivated!", NotificationIcon::Warning)->show();
    }
};

static DeathDecisionDelegate g_deathDelegate;

class $modify(MyPlayLayer, PlayLayer) {
    void destroyPlayer(PlayerObject* player, GameObject* obj) {
        if (g_isNoclipActive) return;
        if (g_isWaitingForDecision) return;
        g_isWaitingForDecision = true;

        // Защищаем макрос FLAlertLayer::create от fmt. 
        // Передаем пустые скобки {}, чтобы компилятор не пытался форматировать текст.
        auto alert = FLAlertLayer::create(
            &g_deathDelegate, 
            "Are you sure?", 
            "Do you really want to die?", 
            "No", 
            "Yes", 
            300.f
        );
        
        if (alert) {
            alert->show();
        } else {
            g_isWaitingForDecision = false;
        }
    }
};
