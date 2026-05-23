#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

std::string getSaveKey(int levelID) {
    return "platformer_save_level_" + std::to_string(levelID);
}

// Создаем делегат для обработки ответа в окне (ОК/Отмена) по стандартам Geode 5
class CheckpointLoadDelegate : public CCObject, public FLAlertLayerProtocol {
public:
    PlayLayer* m_layer;
    
    void FLAlert_Clicked(FLAlertLayer* alert, bool btn2) override {
        // Если пользователь нажал вторую кнопку ("ОК")
        if (btn2 && m_layer && m_layer->m_player1) {
            int levelID = m_layer->m_level->m_levelID;
            auto savedData = Mod::get()->getSavedValue<matjson::Value>(getSaveKey(levelID));

            // Извлекаем сохраненную позицию кубика
            float posX = static_cast<float>(savedData["player_x"].asDouble().unwrapOrDefault());
            float posY = static_cast<float>(savedData["player_y"].asDouble().unwrapOrDefault());
            
            // Восстанавливаем позицию игрока
            m_layer->m_player1->m_position = ccp(posX, posY);

            // Безопасно обновляем физическое тело персонажа на сцене
            m_layer->m_player1->resetObject();

            // Создаем официальный чекпоинт игры в этой точке
            m_layer->createCheckpoint(); 
            
            log::info("Successfully loaded state at: {}, {}", posX, posY);
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
        m_fields->m_delegate->retain(); // Защищаем от случайного удаления из памяти
        return true;
    }

    // В Geode 5 этот метод гарантирует, что весь UI уровня готов к отрисовке всплывающих окон
    void updateProgressbar() {
        PlayLayer::updateProgressbar();

        if (!m_fields->m_hasCheckedLoad && this->m_level && this->m_level->isPlatformer()) {
            m_fields->m_hasCheckedLoad = true;
            
            int levelID = this->m_level->m_levelID;
            auto savedData = Mod::get()->getSavedValue<matjson::Value>(getSaveKey(levelID));

            // Проверяем наличие файла сохранения
            if (savedData.contains("has_save") && savedData["has_save"].asBool().unwrapOrDefault()) {
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

    // Каждый раз, когда игрок прыгает на чекпоинт, мы мгновенно обновляем конфиг
    void createCheckpoint() {
        PlayLayer::createCheckpoint();

        if (this->m_level && this->m_level->isPlatformer() && this->m_player1) {
            int levelID = this->m_level->m_levelID;
            
            matjson::Value save;
            save["has_save"] = true;
            save["player_x"] = this->m_player1->m_position.x;
            save["player_y"] = this->m_player1->m_position.y;

            Mod::get()->setSavedValue(getSaveKey(levelID), save);
            Mod::get()->saveData();
            
            log::info("Position auto-saved to mod config for level {}", levelID);
        }
    }

    // Освобождаем ресурсы при выходе из уровня
    void onExit() {
        if (m_fields->m_delegate) {
            m_fields->m_delegate->release();
        }
        PlayLayer::onExit();
    }
};

