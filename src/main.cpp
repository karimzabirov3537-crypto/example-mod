#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

bool g_isNoclipActive = false;
bool g_isWaitingForDecision = false;

// Класс-делегат для обработки нажатий кнопок в окне
class DeathDecisionDelegate : public FLAlertLayerProtocol {
public:
    void FLAlert_Clicked(FLAlertLayer* layer, bool btn2) override {
        if (btn2) {
            // Игрок нажал "Да" -> включаем ноуклип
            g_isNoclipActive = true;
            Notification::create("Засейвлен! 5 секунд ноуклипа!", NotificationIcon::Success)->show();

            // Создаем безопасную задержку на 5 секунд прямо через экшен Cocos2d
            if (auto pl = PlayLayer::get()) {
                auto delay = cocos2d::CCDelayTime::create(5.0f);
                auto callback = cocos2d::CCCallFunc::create(pl, callfunc_selector(DeathDecisionDelegate::onTimerEnd));
                
                // Запускаем последовательность действий на объекте PlayLayer
                pl->runAction(cocos2d::CCSequence::create(delay, callback, nullptr));
            }
        } else {
            // Игрок нажал "Нет" -> сбрасываем флаги и убиваем
            g_isNoclipActive = false;
            g_isWaitingForDecision = false;
            if (auto pl = PlayLayer::get()) {
                pl->destroyPlayer(pl->m_player1, nullptr);
            }
        }
        g_isWaitingForDecision = false;
    }

    // Функция, которая вызовется через 5 секунд действия экшена
    void onTimerEnd() {
        g_isNoclipActive = false;
        Notification::create("Ноуклип кончился, аккуратно!", NotificationIcon::Warning)->show();
    }
};

static DeathDecisionDelegate g_deathDelegate;

class $modify(MyPlayLayer, PlayLayer) {
    void destroyPlayer(PlayerObject* player, GameObject* obj) {
        // Если ноуклип активен — игнорируем урон
        if (g_isNoclipActive) return;

        // Если окно выбора уже открыто — ничего не делаем
        if (g_isWaitingForDecision) return;
        g_isWaitingForDecision = true;

        // Показываем кастомное окно Geode / Geometry Dash
        auto alert = FLAlertLayer::create(
            &g_deathDelegate, 
            "Вы точно?", 
            "Вы уверены, что хотите <cr>умереть</c>?", 
            "Нет", // Лямбда вернет btn2 = false
            "Да",  // Лямбда вернет btn2 = true
            300.f
        );
        alert->show();
    }
};
