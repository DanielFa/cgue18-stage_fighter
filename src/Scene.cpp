//
// Created by Raphael on 28.04.2018.
//

#include "object3d/Object3D.h"
#include "bullet/GLDebugDrawer.h"

#include "Scene.h"

#include <glm/glm.hpp>

Scene::Scene(Camera &camera) : camera(camera) {}

void Scene::render(std::chrono::duration<double, std::milli> &delta) {
    this->deltaT = delta;
    this->culledObjects = 0;


    if (dirtyLights) {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->lightSSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER, lights.size() * sizeof(Light), lights.data(), GL_STATIC_COPY);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    for (auto &obj : this->objects) {

        if (frustumCulling &&
            isSphereInFrustum(obj->getPosition(), obj->getBoundingSphereRadius()) == Camera::OUTSIDE) {
            this->culledObjects++;
            continue;

            //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            //obj->render(this);
            //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        } else {
            obj->render(this);
        }
    }

    // Draw Bullet Debug Context after all Objects, just in case ...
    if (this->bulletDebugDrawer)
        this->bulletDebugDrawer->render(this);

    // Draw skybox as last element to make use of the depth buffer
    if (this->skybox)
        this->skybox->render(this);

    for (auto &obj : this->particles)
        obj->update(this);

    // Since particles do not use the DEPTH buffer the skybox would
    // overdraw them, so we have to draw them after the skybox
    for (auto &obj : this->particles) {
        if (frustumCulling &&
            isSphereInFrustum(obj->getPosition(), obj->getBoundingSphereRadius()) == Camera::OUTSIDE) {
            this->culledObjects++;
            continue;
        } else {
            obj->render(this);
        }
    }

    dirtyLights = false;
}

Camera::FrustumLocation Scene::isSphereInFrustum(const glm::vec3 &position, float radius) {
    return camera.isInFrustum(position, radius);

    //auto result = Camera::INSIDE;
    //auto pl = camera.getFrustumPlanes();
    //
    //for (int i=0; i<6; i++) {
    //    auto distance = glm::dot(position, glm::vec3(pl[i])) + pl[i].w + radius;
    //
    //    if (distance <= 0)
    //        return Camera::OUTSIDE;
    //}
    //
    //return result;

}

void Scene::addObject(const std::shared_ptr<Object3DAbstract> &object3D) {
    this->objects.push_back(object3D);
}

void Scene::removeObject(const std::shared_ptr<Object3DAbstract> &object3D) {
    this->objects.erase(
            std::remove_if(
                    this->objects.begin(),
                    this->objects.end(),
                    [object3D](std::shared_ptr<Object3DAbstract> current) -> bool {
                        return current == object3D;
                    }
            ),
            this->objects.end()
    );
}

void Scene::addLight(const Light &light) {
    this->dirtyLights = true;
    this->lights.push_back(light);

    if (this->lights.size() > 30)
        throw std::runtime_error("Error MAX_LGIHTS is 30!");
}

void Scene::removeLight(const Light &light) {
    this->dirtyLights = true;
    this->lights.erase(
            std::remove_if(
                    this->lights.begin(),
                    this->lights.end(),
                    [&light](Light &current) -> bool {
                        return current.position == light.position;
                    }
            ),
            this->lights.end()
    );
}

std::vector<Light> &Scene::getLights() {
    return this->lights;
}

Camera &Scene::getCamera() {
    return this->camera;
}

void Scene::clear() {
    this->lights.clear();
    this->objects.clear();
}

void Scene::addParticleSystem(const std::shared_ptr<AbstractParticleSystem> &s) {
    this->particles.push_back(s);
}

void Scene::removeParticleSystem(const std::shared_ptr<AbstractParticleSystem> &s) {
    this->particles.erase(
            std::remove_if(
                    this->particles.begin(),
                    this->particles.end(),
                    [s](std::shared_ptr<AbstractParticleSystem> current) -> bool {
                        return current == s;
                    }
            ),
            this->particles.end()
    );
}

void Scene::initOpenGLContext() {
    glGenBuffers(1, &this->lightSSBO);
}
