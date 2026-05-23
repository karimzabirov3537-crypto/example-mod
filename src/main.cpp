#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

std::string getSaveKey(int levelID) {
    return "platformer_save_level_" + std::to_string(levelID);
}

class CheckpointLoadDelegate : public CCObject {
public:
    PlayLayer* m_layer;
    
    void onConfirm(CCObject* sender) {
        if (!m_layer) return;

        int levelID = m_layer->m_level->m_levelID;
        auto savedData = Mod::get()->getSavedValue<matjson::Value>(getSaveKey(levelID));

        float posX = static_cast<float>(savedData["player_x"].as<double>());
        float posY = static_cast<float>(savedData["player_y"].as<double>());
        
        if (m_layer->m_player1) {
            m_layer->m_player1->m_position = ccp(posX, posY);
        }

        m_layer->createCheckpoint(); 
        log::info("Successfully loaded checkpoint at: {}, {}", posX, posY);
    }

    static CheckpointLoadDelegate* create(PlayLayer* layer) {
        auto ret = new CheckpointLoadDelegate();
        ret->m_layer = layer;
        ret->autorelease();
        return ret;
    }
};

class $modify(MyPlayLayer, PlayLayer) {
    struct Fields {
        bool m_hasCheckedLoad = false;
        CheckpointLoadDelegate* m_delegate = nullptr;
    };

    bool init(GJGameLevel* level, bool usePracticeMode, bool isSfxPreview) {
        if (!PlayLayer::init(level, usePracticeMode, isSfxPreview)) return false;
        
        m_fields->m_hasCheckedLoad = false;
        m_fields->m_delegate = CheckpointLoadDelegate::create(this);
        m_fields->m_delegate->retain(); 
        return true;
    }

    void showNewBest(bool p0, int p1, int p2, bool p3, bool p4, bool p5) {
        PlayLayer::showNewBest(p0, p1, p2, p3, p4, p5);

        if (!m_fields->m_hasCheckedLoad && this->m_level->isPlatformer()) {
            m_fields->m_hasCheckedLoad = true;
            
            int levelID = this->m_level->m_levelID;
            auto savedData = Mod::get()->getSavedValue<matjson::Value>(getSaveKey(levelID));

            if (savedData.contains("has_save") && savedData["has_save"].as<bool>()) {
                
                // Текст переведен на английский, чтобы избежать краша шрифтов игры
                auto alert = FLAlertLayer::create(
                    m_fields->m_delegate,
                    "Load Save",
                    "A checkpoint save was found from your previous session. Do you want to <cg>continue</c>?",
                    "Cancel", "OK",
                    320.f 
                );
                
                alert->show();
            }
        }
    }

    void onQuit() {
        if (this->m_level && this->m_level->isPlatformer() && this->m_checkpointArray && this->m_checkpointArray->count() > 0) {
            int levelID = this->m_level->m_levelID;
            
            matjson::Value save;
            save["has_save"] = true;
            
            if (this->m_player1) {
                save["player_x"] = this->m_player1->m_position.x;
                save["player_y"] = this->m_player1->m_position.y;
            }

            Mod::get()->setSavedValue(getSaveKey(levelID), save);
            Mod::get()->saveData();
        }

        if (m_fields->m_delegate) {
            m_fields->m_delegate->release();
        }

        PlayLayer::onQuit();
    }
};
