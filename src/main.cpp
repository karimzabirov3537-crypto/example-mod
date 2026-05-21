#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

// Глобальные переменные для контроля ноуклипа
bool g_noclipActive = false;
float g_noclipTimer = 0.0f;
int g_deathSavedCount = 0;      
bool g_isAwaitingDecision = false; 

class MyAlertDelegate : public FLAlertLayerProtocol {
public:
    PlayerObject* m_player;
    GameObject* m_object;
    PlayLayer* m_layer;

    MyAlertDelegate(PlayerObject* p, GameObject* o, PlayLayer* l) 
        : m_player(p), m_object(o), m_layer(l) {}

    void FLAlert_Clicked(FLAlertLayer* alert, bool selectedSecondButton) override {
        g_isAwaitingDecision = false; 

        if (selectedSecondButton) {
            // Нажали "NO" -> включаем временный ноуклип
            g_deathSavedCount++;
            g_noclipActive = true;
            g_noclipTimer = 5.0f; 

            if (m_player) {
                m_player->setOpacity(100);
            }
            
            m_layer->resume();
        } else {
            // Нажали "YES" -> убиваем по-честному
            if (m_layer) {
                int tempCount = g_deathSavedCount;
                g_deathSavedCount = 999; 
                m_layer->destroyPlayer(m_player, m_object);
                g_deathSavedCount = tempCount; 
            }
        }
    }
};

// Регистрируем кастомное поле структуры Geode
class $modify(MyPlayLayer, PlayLayer) {
    struct Fields {
        bool m_playerHasMoved = false;
    };
    
    bool init(GJGameLevel* level, bool useIndex, bool dontUseIndex) {
        if (!PlayLayer::init(level, useIndex, dontUseIndex)) return false;
        g_deathSavedCount = 0;
        g_noclipActive = false;
        g_noclipTimer = 0.0f;
        g_isAwaitingDecision = false;
        m_fields->m_playerHasMoved = false; // При старте игрок еще стоит
        return true;
    }

    void update(float dt) {
        PlayLayer::update(dt);

        // Если куб начал движение по оси X — активируем маркер старта уровня
        if (m_player1 && m_player1->m_position.x > 10.0f) {
            m_fields->m_playerHasMoved = true;
        }

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
        // Если игрок еще не проехал вперед ни на один пиксель — полностью игнорируем фейковую смерть
        if (!m_fields->m_playerHasMoved) {
            PlayLayer::destroyPlayer(player, object);
            return;
        }

        if (g_noclipActive) return;
        if (g_isAwaitingDecision) return;

        if (g_deathSavedCount < 3) {
            g_isAwaitingDecision = true;
            this->pauseGame(true);
            
            auto delegate = new MyAlertDelegate(player, object, this);
            auto alert = FLAlertLayer::create(delegate, "WASTED?", "Are you sure you want to die?", "Yes...", "No!", 300.f);
            alert->show();
            return;
        }

        PlayLayer::destroyPlayer(player, object);
    }
};
