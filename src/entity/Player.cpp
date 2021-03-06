//
// Created by raphael on 16.04.18.
//

#include <cfloat>
#include <utility>

#include "Player.h"

#include "../manager/FontManager.h"
#include "../manager/ModelManager.h"

#include "ScriptedEntity.h"

#include "../helper/QuatUtils.h"
#include "ScriptedObject.h"
#include "../manager/AudioManager.h"

Player::Player(Camera &camera, Window *window, const std::shared_ptr<BulletUniverse> &world) :
        CameraEntity(camera, world, new btSphereShape(0.7f), 1.0f),
        Logger("Player"),
        hud(std::make_shared<PlayerHud>(FontManager::get("Lato-24"), camera.getViewPort().z, camera.getViewPort().w)),
        weaponHurtBox(btVector3(0,0,0), btQuaternion(0,0,0,1), new btSphereShape(0.8f), 0),
        weaponHitCallback(this, weaponHurtBox.getRigidBody())
        {

    this->window = window;

    this->health = 100;
    this->maxHealth = 100;

    this->shield = 100;
    this->maxShield = 100;

    this->entitySpeed = 5.5f;

    this->shieldModel = std::make_shared<Model3DObject>(
            camera.getPosition(),
            1.0f,
            ModelManager::load("shield"),
            std::vector<std::shared_ptr<Shader>> {ShaderManager::load("standard")}
    );
    this->weaponModel = std::make_shared<Model3DObject>(
            camera.getPosition(),
            1.0f,
            ModelManager::load("Sword"),
            std::vector<std::shared_ptr<Shader>> {ShaderManager::load("standard")}
    );


    //this->shieldModel->enableAnimation(Model3DObject::Animation(
    //    std::string("Cube_CubeAction"), 0.0f, 30 * 1.0f/24.0f, true
    //));

    this->oSpeed = CameraEntity::entitySpeed;
    this->oJumpSpeed = CameraEntity::jumpSpeed;
    this->stickyVelocity = btVector3(0,0,0);

    this->animationMatrix = glm::translate(this->animationMatrix, glm::vec3(0,-0.13f,0));
}

void Player::think(std::chrono::duration<double, std::milli> delta) {
    if (window->getMouseButton(GLFW_MOUSE_BUTTON_RIGHT) && this->shield > 0) {
        if (shieldAnimationTime < 45.0f) {
            shieldAnimationTime = std::min(45.0f, shieldAnimationTime + (float)delta.count() / 10.0f);
        } else {
            isBlocking = true;
            shieldRegTime = 0;

            entitySpeed = this->oSpeed / 2.0f;
            jumpSpeed = 0.0f;

            shieldDamageTime += delta.count();
            if(shieldDamageTime > 250.0f) {
                shieldDamageTime = 0;
                shield = std::max(0, shield-1);
            }
        }
    } else {
        if (shieldAnimationTime > 18.0f) {
            shieldAnimationTime = std::max(18.0f, shieldAnimationTime - (float)delta.count() / 2.0f);
        } else {
            isBlocking = false;
            shieldDamageTime = 0;

            entitySpeed = this->oSpeed;
            jumpSpeed = this->oJumpSpeed;

            shieldRegTime += delta.count();
            if (shieldRegTime > 300.0f) {
                shieldRegTime = 0;
                shield = std::min(100, shield+1);
            }
        }
    }

    CameraEntity::think(delta);
    const float angle = glm::radians(shieldAnimationTime + camera.getYaw());
    const glm::quat rot = glm::quat(-glm::cos(angle/2.0f), 0, glm::sin(angle/2.0f), 0);
    //const glm::vec3 sPos = glm::vec3(camera.getPosition().x, camera.getPosition().y - 0.13f, camera.getPosition().z);

    auto leftMButton = window->getMouseButton(GLFW_MOUSE_BUTTON_LEFT);
    static bool isPlaying = false;
    if (leftMButton == GLFW_PRESS) {
        weaponAngle = std::min(65.0f, weaponAngle + (float)delta.count() / 2.6f);
        if (!isHitting && weaponAngle < 20.0f) {
            isHitting = true;
            if (!isPlaying) {
                AudioManager::audioEngine->play2D("../resources/audio/whoosh_weapon_knife_swing.wav");
                isPlaying = true;
            }
        }
    } else if (leftMButton == GLFW_RELEASE || revertHitAnimation){
        weaponAngle = std::max(-25.0f, weaponAngle - (float)delta.count() / 1.2f);
        if (isHitting && weaponAngle < 20.0f) {
            isHitting = false;
        }
        if (isPlaying) {
            isPlaying = false;
        }

    }

    const float wSwingAngle = glm::radians(weaponAngle);
    const glm::quat sRot = glm::quat(-glm::cos(wSwingAngle/2.0f), 0, 0, glm::sin(wSwingAngle/2.0f));
    const float wAngle = glm::radians(camera.getYaw() - 18.0f);
    const glm::quat wRot = glm::quat(-glm::cos(wAngle/2.0f), 0, glm::sin(wAngle/2.0f), 0);

                              //player position * object offset * frame rotation
    shieldModel->setModelMatrix(modelMatrix * animationMatrix * glm::toMat4(rot));
    weaponModel->setModelMatrix(modelMatrix * animationMatrix * glm::toMat4(wRot * sRot));

    //this->shieldModel->applyAnimation(static_cast<float>(delta.count() / 5000.0f));

    if (isHitting) {
        const auto hurtBoxOffset = camera.getFront() * (0.8f + 0.7f);
        const auto hurtBoxPositon = camera.getPosition() + hurtBoxOffset;
        weaponHurtBox.setOrigin(btVector3(hurtBoxPositon.x, hurtBoxPositon.y, hurtBoxPositon.z), btQuaternion(0,0,0,1));
        world->contactTest(weaponHurtBox.getRigidBody(), weaponHitCallback);
    }

    hud->hitEnemyVisibility = std::max(0.0f, hud->hitEnemyVisibility - (float)delta.count() / 750.0f);
    hud->setHealth(health);
    hud->setShield(shield);
}

void Player::computeEnemyInView(std::vector<std::shared_ptr<Entity>> &entities) {
    const auto width = this->camera.getViewPort().z;
    const auto height = this->camera.getViewPort().w;

    selectedEnemy = nullptr;
    float targetDist = FLT_MAX;

    for (auto &entity : entities) {
        if (entity->getEntityKind() != BulletObject::ENEMY)
            continue;

        const glm::vec3 distV = glm::abs(getPosition() - entity->getPosition());
        const auto distance =  distV.x + distV.y + distV.z;

        if (distance < 25.0f) {
            const auto p = isInView(entity.get());

            if (p.x > width /2 - width/4  && p.x < width /2 + width/4 &&
                p.y > height/2 - height/3 && p.y < height/2 + height/3 &&
                targetDist > distance && p.z < 1.0) {
                selectedEnemy = entity;
            }
        }

    }

    if (selectedEnemy != nullptr) {
        auto name = selectedEnemy->getName();
        hud->setEnemy(name, selectedEnemy->getHealth(), selectedEnemy->getMaxHealth());
    } else {
        hud->showEnemy = false;
        selectedEnemy.reset();
    }
}

void Player::collideWith(BulletObject *other) {
    if (other->getKind() == BulletObject::BULLET) {
        if (!this->isBlocking) {
            this->health = std::max(0, health - 10);
            AudioManager::audioEngine->play2D("../resources/audio/body_hit.wav");
        } else {
            this->shield = std::max(0, shield - 10);
            AudioManager::audioEngine->play2D("../resources/audio/retro_impact_hit_13.wav");
        }
    }
}

BulletObject::Kind Player::getEntityKind() {
    return BulletObject::PLAYER;
}

void Player::render(Scene *scene) {
    this->shieldModel->render(scene);
    this->weaponModel->render(scene);
}

void Player::setCrosshairState(bool show) {
    hud->showCrosshair = show;
}
