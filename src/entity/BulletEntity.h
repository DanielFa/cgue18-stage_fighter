//
// Created by raphael on 16.04.18.
//

#ifndef STAGE_FIGHTER_BULLETENTITY_H
#define STAGE_FIGHTER_BULLETENTITY_H


#include "../BulletUniverse.h"
#include "../object3d/BulletObject.h"
#include "../object3d/Model3DObject.h"

#include "Entity.h"

class BulletEntity : public Entity, public BulletObject, public Model3DObject {

private:
    std::shared_ptr<BulletUniverse> world;
    glm::vec3 direction;

    float speed = 5.0f;
    float maxLifeTime = 10000.0f; // 10 sec

    std::shared_ptr<ParticleSystem> smoke;

public:
    BulletEntity(const btVector3 &pos, const btVector3 &target, std::shared_ptr<BulletUniverse> &world);
    ~BulletEntity();

    void think(Level *level, std::chrono::duration<double, std::milli> delta) override;

    void collideWith(BulletObject *other) override;

    void setPosition(const glm::vec3 &vec, const glm::quat &rot) override;

    void think(std::chrono::duration<double, std::milli> delta) override;

    void render(Scene *scene) override;

    float getBoundingSphereRadius() override;

    const glm::vec3 &getPosition() const override;

    const std::shared_ptr<ParticleSystem> &getSmoke() const { return this->smoke; }

    Kind getEntityKind() override;
};


#endif //STAGE_FIGHTER_BULLETENTITY_H
