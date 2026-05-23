#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

std::string getSaveKey(int levelID) {
    return "platformer_save_level_" + std::to_string(levelID);
}

class CheckpointLoadDelegate : public CCObject, public FLAlertLayerProtocol {
public:
    PlayLayer* m_layer;
    
    void FLAlert_Clicked(FLAlertLayer* alert, bool btn2) override {
        if (btn2 && m_layer) {
            int levelID = m_layer->m_level->m_levelID;
            auto savedData = Mod::get()->getSavedValue<matjson::Value>(getSaveKey(levelID));

            float posX = static_cast<float>(savedData["player_x"].as<double>().unwrapOrDefault());
            float posY = static_cast<float>(savedData["player_y"].as<double>().unwrapOrDefault());
            
            if (m_layer->m_player1) {
                m_layer->m_player1->m_position = ccp(posX, posY);
                // Принудительно обновляем физику объекта, чтобы он не провалился в текстуры
                m_layer->m_player1->resetObject();
            }

            // Создаем чекпоинт в игре на этих координатах
            m_layer->createCheckpoint(); 
            
            // Логируем в консоль Geode для проверки работы
            log::info("Successfully loaded checkpoint from file at: {}, {}", posX, posY);
        }
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

    // Юзаем метод обновления интерфейса — он гарантирует, что уровень уже полностью загрузился
    void updateProgressbar() {
        PlayLayer::updateProgressbar();

        if (!m_fields->m_hasCheckedLoad && this->m_level && this->m_level->isPlatformer()) {
            m_fields->m_hasCheckedLoad = true;
            
            int levelID = this->m_level->m_levelID;
            auto savedData = Mod::get()->getSavedValue<matjson::Value>(getSaveKey(levelID));

            if (savedData.contains("has_save") && savedData["has_save"].as<bool>().unwrapOrDefault()) {
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

    // ХУК: Сохраняем данные СРАЗУ, как только ставится чекпоинт
    void createCheckpoint() {
        PlayLayer::createCheckpoint();

        // Проверяем, что это платформер и игрок существует
        if (this->m_level && this->m_level->isPlatformer() && this->m_player1) {
            int levelID = this->m_level->m_levelID;
            
            matjson::Value save;
            save["has_save"] = true;
            save["player_x"] = this->m_player1->m_position.x;
            save["player_y"] = this->m_player1->m_position.y;

            // Моментально пишем в локальный файл конфигурации мода
            Mod::get()->setSavedValue(getSaveKey(levelID), save);
            Mod::get()->saveData();
            
            log::info("Checkpoint autosaved to config for level {}", levelID);
        }
    }

    // Очищаем память
    void onDestroy() {
        if (m_fields->m_delegate) {
            m_fields->m_delegate->release();
        }
        PlayLayer::onDestroy();
    }
};
