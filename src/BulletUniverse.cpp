//
// Created by raphael on 13.03.18.
//

#include "BulletUniverse.h"
#include "object3d/BulletObject.h"

bool BulletUniverse::debuggingFlag = false;

BulletUniverse::BulletUniverse(const btVector3 &gravity) {
    this->broadphase = new btDbvtBroadphase();
    this->collisionConfiguration = new btDefaultCollisionConfiguration();
    this->dispatcher = new btCollisionDispatcher(collisionConfiguration);
    this->solver = new btSequentialImpulseConstraintSolver;

    this->dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
    this->dynamicsWorld->setGravity(gravity);

    this->debugDrawer = std::make_shared<GLDebugDrawer>();
    this->debug = BulletUniverse::debuggingFlag;

    if (this->debug)
        this->enableDebugging();
}

BulletUniverse::~BulletUniverse() {
    delete dynamicsWorld;
    delete solver;
    delete collisionConfiguration;
    delete dispatcher;
    delete broadphase;
}

void BulletUniverse::addRigidBody(btRigidBody *body) {
    this->dynamicsWorld->addRigidBody(body);
}

void BulletUniverse::simulate(std::chrono::duration<double, std::milli> tick) {
    /*
     * Limit updates to ~ 16,6666 ms to preserve numeric stability and reduce
     * load for more FPS
     *
     * TODO: Why does glitching occur if limited?
     *  (Limit maybe to 500fps or so?)
     */
    internalTick += tick.count();
    if (internalTick < 1.0/60.0)
        return;

    /*
     * Simulate the actual world
     *  (time elapsed in world, max steps, fixed step size ~1/60 fps)
     */
    dynamicsWorld->stepSimulation(static_cast<btScalar>(internalTick / 1000.f), (int)glm::ceil((float)internalTick / (1.0f/60.0f) + 1), 1.0f / 60.0f);
    dynamicsWorld->performDiscreteCollisionDetection();

    internalTick = 0;

    /*
     * Get all the collisions of the world and call the objects to invoke the logic
     */
    const int mainfolds = dynamicsWorld->getDispatcher()->getNumManifolds();
    for (int i=0; i < mainfolds; i++) {
        btPersistentManifold* contactManifold = dynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);
        const btCollisionObject* obA = const_cast<btCollisionObject*>(contactManifold->getBody0());
        const btCollisionObject* obB = const_cast<btCollisionObject*>(contactManifold->getBody1());

        auto bA = reinterpret_cast<BulletObject *>(obA->getUserPointer());
        auto bB = reinterpret_cast<BulletObject *>(obB->getUserPointer());

        if (bA->getKind() == BulletObject::ENVIRONMENT && bB->getKind() == BulletObject::ENVIRONMENT)
            continue;

        bA->collideWith(bB);
        bB->collideWith(bA);
    }
}

void BulletUniverse::removeRigidBody(btRigidBody *body) {
    this->dynamicsWorld->removeRigidBody(body);
}

void BulletUniverse::addCollsionObject(btCollisionObject *body) {
    this->dynamicsWorld->addCollisionObject(body);
}

void BulletUniverse::removeCollsipnObject(btCollisionObject *body) {
    this->dynamicsWorld->removeCollisionObject(body);
}

void BulletUniverse::enableDebugging() {
    this->debugDrawer->setDebugMode(btIDebugDraw::DBG_DrawWireframe);// | btIDebugDraw::DBG_DrawAabb);
    this->dynamicsWorld->setDebugDrawer(debugDrawer.get());
}

void BulletUniverse::drawDebug() {
    this->dynamicsWorld->debugDrawWorld();
}

void BulletUniverse::rayTest(const btVector3 &start, const btVector3 &end,
                             btCollisionWorld::ClosestRayResultCallback &rayCallback) {
    this->dynamicsWorld->rayTest(start, end, rayCallback);
}

