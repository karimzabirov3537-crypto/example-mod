#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

// Глобальные переменные для контроля логики
bool g_noclipActive = false;
float g_noclipTimer = 0.0f;
int g_deathSavedCount = 0;      // Счетчик: сколько раз мы спаслись (максимум 3)
bool g_isAwaitingDecision = false; // Блокиратор, чтобы окно не спамилось каждую микросекунду

// Создаем кастомный делегат, который будет слушать, какую кнопку нажал игрок в окне
class MyAlertDelegate : public FLAlertLayerProtocol {
public:
    PlayerObject* m_player;
    GameObject* m_object;
    PlayLayer* m_layer;

    MyAlertDelegate(PlayerObject* p, GameObject* o, PlayLayer* l) 
        : m_player(p), m_object(o), m_layer(l) {}

    // Метод срабатывает, когда игрок нажимает на кнопку в окне
    void FLAlert_Clicked(FLAlertLayer* alert, bool selectedSecondButton) override {
        g_isAwaitingDecision = false; // Снимаем блокировку

        if (selectedSecondButton) {
            // Игрок нажал "NO" (Вторая кнопка) -> Спасаем его!
            g_deathSavedCount++;
            g_noclipActive = true;
            g_noclipTimer = 5.0f; // 5 секунд бессмертия

            // Делаем иконки прозрачными, чтобы было видно бессмертие
            if (m_player) {
                m_player->setOpacity(100);
            }
            
            // Снимаем игру с паузы и возвращаем в экшен
            m_layer->resume();
        } else {
            // Игрок нажал "YES" (Первая кнопка) -> Убиваем по-честному
            if (m_layer) {
                // Чтобы не уйти в бесконечный цикл, временно ставим счетчик на максимум
                int tempCount = g_deathSavedCount;
                g_deathSavedCount = 999; 
                
                m_layer->destroyPlayer(m_player, m_object);
                
                g_deathSavedCount = tempCount; // Возвращаем счетчик обратно
            }
        }
    }
};

class $modify(MyPlayLayer, PlayLayer) {
    
    // Обнуляем счетчик спасенных жизней при каждом старте или рестарте уровня
    bool init(GJGameLevel* level, bool useIndex, bool dontUseIndex) {
        if (!PlayLayer::init(level, useIndex, dontUseIndex)) return false;
        g_deathSavedCount = 0;
        g_noclipActive = false;
        g_noclipTimer = 0.0f;
        g_isAwaitingDecision = false;
        return true;
    }

    void update(float dt) {
        PlayLayer::update(dt);

        // Таймер отсчета ноуклипа
        if (g_noclipActive) {
            g_noclipTimer -= dt;
            if (g_noclipTimer <= 0.0f) {
                g_noclipActive = false;
                if (m_player1) m_player1->setOpacity(255);
                if (m_player2) m_player2->setOpacity(255);
            }
        }
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        // Если ноуклип активен — игнорируем шипы
        if (g_noclipActive) return;

        // Если окно уже открыто и ждет ответа — ничего не делаем
        if (g_isAwaitingDecision) return;

        // Проверяем: если мы умерли меньше 3 раз, то выкатываем окно с вопросом
        if (g_deathSavedCount < 3) {
            g_isAwaitingDecision = true;

            // Надежный и официальный способ поставить уровень на паузу в Geode SDK 5.7.1
            this->pauseGame(true);
            
            // Создаем обработчик кнопок
            auto delegate = new MyAlertDelegate(player, object, this);

            // Создаем встроенное окно Geometry Dash (FLAlertLayer) на английском языке
            auto alert = FLAlertLayer::create(
                delegate, 
                "WASTED?", 
                "Are you sure you want to die?", 
                "Yes...", 
                "No!", 
                300.f
            );
            
            alert->show();
            return;
        }

        // Если 3 попытки уже потрачены — обычная смерть без вопросов
        PlayLayer::destroyPlayer(player, object);
    }
};
