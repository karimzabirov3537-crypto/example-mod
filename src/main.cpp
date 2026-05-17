#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <chrono>
#include <thread>

using namespace geode::prelude;

bool g_isNoclipActive = false;

class $modify(MyPlayLayer, PlayLayer) {
    void destroyPlayer(PlayerObject* player, GameObject* obj) {
        // Если ноуклип активен — игнорируем смерть
        if (g_isNoclipActive) return; 

        // Включаем ноуклип спасения
        g_isNoclipActive = true;
        
        // Показываем уведомление на экране
        Notification::create("Saved! Noclip active for 5 seconds!", NotificationIcon::Success)->show();

        // Запускаем независимый поток-таймер на 5 секунд для отключения
        std::thread([]() {
            std::this_thread::sleep_for(std::chrono::seconds(5));
            geode::Loader::get()->queueInMainThread([]() {
                g_isNoclipActive = false;
                Notification::create("Noclip deactivated!", NotificationIcon::Warning)->show();
            });
        }).detach();
    }
};
