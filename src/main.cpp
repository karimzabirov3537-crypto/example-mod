#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

// Глобальные переменные для контроля логики
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
            g_deathSavedCount++;
            g_noclipActive = true;
            g_noclipTimer = 5.0f; 

            if (m_player) {
                m_player->setOpacity(100);
            }
            
            m_layer->resume();
        } else {
            if (m_layer) {
                int tempCount = g_deathSavedCount;
                g_deathSavedCount = 999; 
                
                m_layer->destroyPlayer(m_player, m_object);
                
                g_deathSavedCount = tempCount; 
            }
        }
    }
};

class $modify(MyPlayLayer, PlayLayer) {
    
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
        // НАДЕЖНАЯ ЗАЩИТА: Обращаемся к состоянию старта уровня через m_fields в Geode 5.7.1
        if (m_fields && !m_fields->m_hasLevelStarted) {
            PlayLayer::destroyPlayer(player, object);
            return;
        }

        if (g_noclipActive) return;
        if (g_isAwaitingDecision) return;

        if (g_deathSavedCount < 3) {
            g_isAwaitingDecision = true;

            this->pauseGame(true);
            
            auto delegate = new MyAlertDelegate(player, object, this);

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

        PlayLayer::destroyPlayer(player, object);
    }
};
