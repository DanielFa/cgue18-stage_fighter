//
// Created by Raphael on 28.03.2018.
//

#include "Level.h"
#include "../entity/CubeEntity.h"
#include "../manager/TextureManager.h"

Level::Level(const std::string &file, const std::shared_ptr<BulletUniverse> &world) : Logger("Level"), world(world) {
    logger->info("Loading file {}", file);

    // Setup Lua Environment:
    state["vec3"].setClass(
            kaguya::UserdataMetatable<LuaVec3>()
                    .setConstructors<LuaVec3(double, double, double)>()
    );
    state["vec4"].setClass(
            kaguya::UserdataMetatable<LuaVec4>()
                    .setConstructors<LuaVec4(double, double, double, double)>()
    );

    state["StaticObject"].setClass(
            kaguya::UserdataMetatable<LuaStaticObject>()
                .setConstructors<LuaStaticObject(std::string,std::string,LuaVec3,kaguya::LuaTable)>()
    );

    state["Entity"].setClass(kaguya::UserdataMetatable<LuaEntity>());
    state["CubeEntity"].setClass(
            kaguya::UserdataMetatable<LuaCubeEntity, LuaEntity>()
                .setConstructors<LuaCubeEntity(std::string, LuaVec3)>()
    );

    state["btCollisionShape"].setClass(kaguya::UserdataMetatable<LuaBtCollisionShape>());
    state["SphereShape"].setClass(
            kaguya::UserdataMetatable<LuaBtSphere, LuaBtCollisionShape>()
                    .setConstructors<LuaBtSphere(LuaVec3 const&, LuaVec4 const&, double, double)>()
    );
    state["BoxShape"].setClass(
            kaguya::UserdataMetatable<LuaBtBox, LuaBtCollisionShape>()
                    .setConstructors<LuaBtSphere(LuaVec3 const&, LuaVec4 const&, double, LuaVec3 const&)>()
    );

    // Finally load file
    state.dofile(file);
}

void Level::start(const Camera &camera, Window *window) {
    this->window = window;

    //TODO: Set Camera position
    //TODO: Set Camera rotation


    for (auto &entity : state["entities"].map<int, LuaEntity *>()) {
        this->entities.push_back(entity.second->toEntity3D(this->world));
    }

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

    this->show();
}

void Level::destroy() {
    this->hide();

    this->entities.clear();
    this->statics.clear();
}

void Level::tick(std::chrono::duration<double, std::milli> delta) {
    for (auto &entity : this->entities) {
        entity->think(delta);
    }
}

void Level::resetEnvironment() {
    // TODO: reset positons and stuff
}

void Level::hide() {
    for (auto &entity : this->entities) this->window->removeObject(entity);
    for (auto &obj    : this->statics ) this->window->removeObject(obj);
}

void Level::show() {
    for (auto &entity : this->entities) this->window->addObject3D(entity);
    for (auto &obj    : this->statics ) this->window->addObject3D(obj);
}
