//
// Created by Raphael on 28.03.2018.
//

#include "Level.h"

#include <glm/gtc/quaternion.hpp>

#include "../entity/CubeEntity.h"

#include "../manager/FontManager.h"
#include "../manager/TextureManager.h"

#include "../object3d/Light.h"

Level::Level(const std::string &file) : Logger("Level"), world(std::make_shared<BulletUniverse>(btVector3(0,-9.81f,0))) {
    winLoseLabel = std::make_shared<Label>("",
                                           FontManager::get("Lato-64"),
                                           0, 0, 1.2f, glm::vec3(1.0f, 1.0f, 1.0f));

    logger->info("Loading file {}", file);

    // Setup Lua Environment:
    state["vec3"].setClass(
            kaguya::UserdataMetatable<LuaVec3>()
                    .setConstructors<LuaVec3(double, double, double)>()
                    .addFunction("x", &LuaVec3::x)
                    .addFunction("y", &LuaVec3::y)
                    .addFunction("z", &LuaVec3::z)
    );
    state["vec4"].setClass(
            kaguya::UserdataMetatable<LuaVec4>()
                    .setConstructors<LuaVec4(double, double, double, double)>()
                    .addFunction("x", &LuaVec4::x)
                    .addFunction("y", &LuaVec4::y)
                    .addFunction("z", &LuaVec4::z)
                    .addFunction("w", &LuaVec4::w)
    );

    state["StaticObject"].setClass(
            kaguya::UserdataMetatable<LuaStaticObject>()
                .setConstructors<LuaStaticObject(std::string,std::string,LuaVec3,LuaVec4,kaguya::LuaTable)>()
    );

    state["Entity"].setClass(kaguya::UserdataMetatable<LuaEntity>());
    state["CubeEntity"].setClass(
            kaguya::UserdataMetatable<LuaCubeEntity, LuaEntity>()
                .setConstructors<LuaCubeEntity(std::string, LuaVec3)>()
    );
    state["EnemyEntity"].setClass(
            kaguya::UserdataMetatable<LuaEnemyEntity, LuaEntity>()
                    .setConstructors<LuaEnemyEntity(std::string, int, int, LuaVec3, LuaVec4&, std::string, float, LuaBtCollisionShape&)>()
    );

    state["btCollisionShape"].setClass(kaguya::UserdataMetatable<LuaBtCollisionShape>());
    state["SphereShape"].setClass(
            kaguya::UserdataMetatable<LuaBtSphere, LuaBtCollisionShape>()
                    .setConstructors<LuaBtSphere(LuaVec3&, LuaVec4&, double, double)>()
    );
    state["BoxShape"].setClass(
            kaguya::UserdataMetatable<LuaBtBox, LuaBtCollisionShape>()
                    .setConstructors<LuaBtBox(LuaVec3&, LuaVec4&, double, LuaVec3&)>()
    );

    state["PointLight"].setClass(
            kaguya::UserdataMetatable<LuaLight>()
                    .setConstructors<LuaLight(LuaVec3&, LuaVec3&, LuaVec3&, LuaVec3&, float)>()
    );

    // Finally load file
    state.dofile(file);
}

void Level::start(Window *window) {
    /*
     * Manipulation internal state since Camera and Window must not be present while loading the Level stuff
     * (lazy loading or pre-loading)
     */
    this->window = window;
    this->player = std::make_shared<Player>(window->getScene()->getCamera(), window, world);

    /*
     * Enable the Debugging Stuff of bullet if debugging is enabled
     */
    if (this->world->isDebugging())
        window->getScene()->bulletDebugDrawer = world->getDebugDrawer();

    /*
     * Initialize Player in the Level:
     */
    player->entitySpeed = state["player"]["speed"];
    const LuaVec3 *pPosition = state["player"]["position"];
    const LuaVec3 *pLookAt = state["player"]["lookAt"];

    player->setEntityPosition(pPosition->pos, glm::quat(0, 0, 0, 1));
    player->lookAt(pLookAt->pos);

    playerInputCallbackID = window->registerKeyPollingCallback(player->getKeyboardCallback());

    for (auto &entity : state["entities"].map<int, LuaEntity *>())
        this->entities.push_back(entity.second->toEntity3D(this->world));

    for (auto &light : state["lights"].map<int, LuaLight*>())
        this->lights.push_back(light.second->toLight());

    for (auto &obj : state["objects"].map<int, LuaStaticObject *>()) {
        this->statics.push_back(obj.second->toModel());

        auto c = obj.second->collisions();
        this->bullet.insert(bullet.end(), c.begin(), c.end());
    }

    for (auto &bulletObj : this->bullet) {
        auto &o = bulletObj->getTransformation().getOrigin();
        world->addRigidBody(bulletObj->getRigidBody());
    }

    this->logger->info("Loaded {} entities", entities.size());
    this->logger->info("Loaded {} objects", statics.size());
    this->logger->info("Loaded {} collision primitives", bullet.size());
    this->logger->info("Loaded {} light sources", lights.size());

    //this->logger->info("Player.add={}", (void*)player.get());

    this->show();
}

void Level::destroy() {
    this->hide();

    this->entities.clear();
    this->statics.clear();
    window->removeKeyPollingCallback(playerInputCallbackID);
}

void Level::tick(std::chrono::duration<double, std::milli> delta) {
    if (levelState != PLAYING)
        return;

    /*
     * Simulate all the Bullet stuff, Collisions and so on will happen here
     * If debugging is enabled then we also want to recompute the Bullet debugging
     * context (all these lines ...)
     */
    world->simulate(delta);
    if (world->isDebugging())
        world->drawDebug();

    // Add and Remove Entities:
    {
        for (auto &entity : this->oldEntities)
            this->entities.erase(
                    std::remove_if(
                            this->entities.begin(),
                            this->entities.end(),
                            [entity, this](std::shared_ptr<Entity> current) -> bool {
                                bool r = current.get() == entity;
                                if (r)
                                    window->getScene()->removeObject(current);

                                return r;
                            }
                    ),
                    this->entities.end()
            );

        for(auto &n : newEntities)
            entities.push_back(n);


        newEntities.clear();
        oldEntities.clear();
    }

    int enemyH = 0;
    for (auto &entity : this->entities) {
        if (entity->mustBeKilled)
            enemyH += entity->getHealth();

        entity->think(this, delta);
    }

    player->think(delta);
    player->computeEnemyInView(entities);

    if (enemyH <= 0) {
        pause();
        levelState = WON;
        this->setLabel("You won!");
        logger->info("You won!");
    }

    if (player->getHealth() < 0) {
        pause();
        levelState = LOST;
        this->setLabel("You lost");
        logger->info("You lost!");
    }
}

void Level::resetEnvironment() {
    this->hide();

    // Clear generated Level
    this->entities.clear();
    this->oldEntities.clear();
    this->bullet.clear();
    this->lights.clear();

    this->window->removeKeyPollingCallback(playerInputCallbackID);

    // Re-Load from Lua Environment
    this->start(this->window);
}

void Level::hide() {
    pause();
    for (auto &entity : this->entities) this->window->getScene()->removeObject(entity);
    for (auto &obj    : this->statics ) this->window->getScene()->removeObject(obj);
    for (auto &light  : this->lights  ) this->window->getScene()->removeLight(light);

    this->window->removeWidget(player->getHud());
    this->window->removeWidget(winLoseLabel);
}

void Level::show() {
    resume();
    for (auto &entity : this->entities) this->window->getScene()->addObject(entity);
    for (auto &obj    : this->statics ) this->window->getScene()->addObject(obj);
    for (auto &light  : this->lights  ) this->window->getScene()->addLight(light);

    this->window->addWidget(player->getHud());
    if (levelState != PLAYING) {
        this->window->addWidget(winLoseLabel);
    }
}

void Level::pause() {
    player->disable();
    levelState = PAUSED;
    setLabel("Paused");
}

void Level::resume() {
    player->enable();
    levelState = PLAYING;
    window->removeWidget(winLoseLabel);
}

void Level::spawn(std::shared_ptr<Entity> entity) {
    this->newEntities.push_back(entity);
    this->window->getScene()->addObject(entity);
}

void Level::despawn(Entity *entity) {
    this->oldEntities.push_back(entity);
}

void Level::setLabel(const std::string text) {
    winLoseLabel->setText(text);
    winLoseLabel->setPosition(
            window->getWidth()  / 2.0f - winLoseLabel->getWidth() / 2.0f,
            window->getHeight() / 4.0f - 64.0f/2.0f
    );

    if (levelState != PLAYING)
        window->addWidget(winLoseLabel);
}

Level::~Level() {
    this->player->disable();
    this->hide();
    this->window->removeKeyPollingCallback(playerInputCallbackID);
}
