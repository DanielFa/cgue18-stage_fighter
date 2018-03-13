#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <chrono>
#include <memory>

#include <btBulletDynamicsCommon.h>


#include "Window.h"
#include "BulletUniverse.h"
#include "object3d/Triangle.h"
#include "manager/ShaderManager.h"

#include <kaguya/kaguya.hpp>

int main(int argc, char *argv[]) {
	kaguya::State config;
    config.dofile("../config.lua");

    ShaderManager::build("../resources/shader/");

	auto *window = new Window(config["config"]["height"], config["config"]["width"], "Stage Fighter");
    window->setVSync(config["config"]["vsync"]);

	BulletUniverse *world = new BulletUniverse(btVector3(0,-10,0));

	window->registerKeyProcessor([window]{
		if(window->getKey(GLFW_KEY_ESCAPE) == GLFW_PRESS)
			window->close();
	});

	btCollisionShape* groundShape = new btStaticPlaneShape(btVector3(0, 1, 0), 1);
	btCollisionShape* fallShape = new btSphereShape(1);
	btDefaultMotionState* groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, -1, 0)));
	btRigidBody::btRigidBodyConstructionInfo
		groundRigidBodyCI(0, groundMotionState, groundShape, btVector3(0, 0, 0));
	btRigidBody* groundRigidBody = new btRigidBody(groundRigidBodyCI);
	world->addRigidBody(groundRigidBody);


	btDefaultMotionState* fallMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 50, 0)));
	btScalar mass = 1;
	btVector3 fallInertia(0, 0, 0);
	fallShape->calculateLocalInertia(mass, fallInertia);
	btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass, fallMotionState, fallShape, fallInertia);
	btRigidBody* fallRigidBody = new btRigidBody(fallRigidBodyCI);
	world->addRigidBody(fallRigidBody);

    auto triangle = std::make_shared<Triangle>(glm::vec4(0,0,0,0));
    window->addObject3D(triangle);

    auto lastTick = std::chrono::high_resolution_clock::now();
	while (window->isOpen())
	{
		auto curTick = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, std::milli> delta =  curTick - lastTick;
		lastTick = curTick;

		window->render([delta, world, fallRigidBody]{
            btTransform trans;

            world->simulate(delta);

			fallRigidBody->getMotionState()->getWorldTransform(trans);
			std::cout << "sphere height: " << trans.getOrigin().getY() << std::endl;
            std::cout << "tick time: " << delta.count() << std::endl;

            auto error = glGetError();
            if(error)
			    std::cout << "OpenGL Error: " << error << std::endl;
		});
	}

	world->removeRigidBody(fallRigidBody);
	delete fallRigidBody->getMotionState();
	delete fallRigidBody;

	world->removeRigidBody(groundRigidBody);
	delete groundRigidBody->getMotionState();
	delete groundRigidBody;


	delete fallShape;
	delete groundShape;

	delete world;
	delete window;

    ShaderManager::destroy();

	glfwTerminate();
	exit(EXIT_SUCCESS);
}
