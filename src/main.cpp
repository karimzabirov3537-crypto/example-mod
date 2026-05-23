#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

// Ключ для сейва конкретного уровня
std::string getSaveKey(int levelID) {
    return "auto_platformer_save_" + std::to_string(levelID);
}

class $modify(MyPlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool usePracticeMode, bool isSfxPreview) {
        // Запускаем оригинальную инициализацию уровня
        if (!PlayLayer::init(level, usePracticeMode, isSfxPreview)) return false;
        
        // Работаем только в платформере
        if (level && level->isPlatformer()) {
            int levelID = level->m_levelID;
            auto savedData = Mod::get()->getSavedValue<matjson::Value>(getSaveKey(levelID));

            // Если нашли автосохранение
            if (savedData.contains("has_save") && savedData["has_save"].asBool().unwrapOrDefault()) {
                
                float posX = static_cast<float>(savedData["player_x"].asDouble().unwrapOrDefault());
                float posY = static_cast<float>(savedData["player_y"].asDouble().unwrapOrDefault());
                
                // Телепортируем игрока на сохраненную точку прямо на старте
                if (this->m_player1) {
                    this->m_player1->m_position = ccp(posX, posY);
                    this->m_player1->resetObject();
                }

                // Ставим невидимый чекпоинт игры, чтобы при смерти возрождаться здесь
                this->createCheckpoint(); 
                
                log::info("[AutoSave] Player automatically teleported to: {}, {}", posX, posY);
            }
        }
        
        return true;
    }

    // Хук на создание чекпоинта (когда игрок прыгает на него)
    void createCheckpoint() {
        PlayLayer::createCheckpoint();

        if (this->m_level && this->m_level->isPlatformer() && this->m_player1) {
            int levelID = this->m_level->m_levelID;
            
            matjson::Value save;
            save["has_save"] = true;
            save["player_x"] = this->m_player1->m_position.x;
            save["player_y"] = this->m_player1->m_position.y;

            // Записываем данные в кэш мода
            Mod::get()->setSavedValue(getSaveKey(levelID), save);
            
            // В Geode 5 метод flushSavedValues() ПРИНУДИТЕЛЬНО и намертво записывает файл на диск прямо сейчас
            Mod::get()->saveData();
            
            log::info("[AutoSave] Saved and Flushed coordinates for level {}", levelID);
        }
    }
};

