#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <chrono>
#include <thread>

using namespace geode::prelude;

bool g_isNoclipActive = false;
bool g_isWaitingForDecision = false;

class DeathDecisionDelegate : public FLAlertLayerProtocol {
public:
    void FLAlert_Clicked(FLAlertLayer* layer, bool btn2) override {
        if (btn2) {
            // Игрок выбрал "Yes" -> активируем ноуклип
            g_isNoclipActive = true;
            Notification::create("Saved! Noclip active!", NotificationIcon::Success)->show();

            // Безопасный асинхронный поток для таймера отключения флага
            std::thread([]() {
                std::this_thread::sleep_for(std::chrono::seconds(5));
                geode::Loader::get()->queueInMainThread([]() {
                    g_isNoclipActive = false;
                    Notification::create("Noclip deactivated!", NotificationIcon::Warning)->show();
                });
            }).detach();

        } else {
            // Игрок выбрал "No" -> позволяем ему умереть
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
    void destroyPlayer(PlayerObject* p0, GameObject* p1) {
        if (g_isNoclipActive) return;
        if (g_isWaitingForDecision) return;
        g_isWaitingForDecision = true;

        // Заворачиваем строки в fmt::runtime, чтобы полностью обойти валидатор компилятора Ninja
        auto alert = FLAlertLayer::create(
            &g_deathDelegate, 
            fmt::runtime("Are you sure?"), 
            fmt::runtime("Do you really want to die?"), 
            fmt::runtime("No"), 
            fmt::runtime("Yes"), 
            300.f
        );
        
        if (alert) {
            alert->show();
        } else {
            g_isWaitingForDecision = false;
        }
    }
};
