#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <string>

using namespace geode::prelude;

bool g_isNoclipActive = false;
bool g_isWaitingForDecision = false;

class DeathDecisionDelegate : public FLAlertLayerProtocol {
public:
    void FLAlert_Clicked(FLAlertLayer* layer, bool btn2) override {
        if (btn2) {
            g_isNoclipActive = true;
            Notification::create("Saved! Noclip active!", NotificationIcon::Success)->show();

            std::thread([]() {
                std::this_thread::sleep_for(std::chrono::seconds(5));
                geode::Loader::get()->queueInMainThread([]() {
                    g_isNoclipActive = false;
                    Notification::create("Noclip deactivated!", NotificationIcon::Warning)->show();
                });
            }).detach();

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
    void destroyPlayer(PlayerObject* player, GameObject* obj) {
        if (g_isNoclipActive) return;
        if (g_isWaitingForDecision) return;
        g_isWaitingForDecision = true;

        // Явно создаем объекты std::string, чтобы библиотека fmt не трогала эти строки
        std::string title = "Are you sure?";
        std::string description = "Do you really want to die?";
        std::string btn1 = "No";
        std::string btn2 = "Yes";

        auto alert = FLAlertLayer::create(
            &g_deathDelegate, 
            title.c_str(), 
            description.c_str(), 
            btn1.c_str(), 
            btn2.c_str(), 
            300.f
        );
        alert->show();
    }
};
