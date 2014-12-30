#include "stdafx.h"
#include <iostream>

using namespace physx;

const int WIDTH  = 800;
const int HEIGHT = 600;

static PxPhysics* g_physicsSDK = NULL;
static PxDefaultErrorCallback g_defaultErrorCallback;
static PxDefaultAllocator g_defaultAllocatorCallback;

void InitPhysX() {
	PxFoundation* foundation = PxCreateFoundation(PX_PHYSICS_VERSION, g_defaultAllocatorCallback, g_defaultErrorCallback);
	if (!foundation) std::cerr << "PxCreateFoundation failed!" << std::endl;

	g_physicsSDK = PxCreatePhysics(PX_PHYSICS_VERSION, *foundation, PxTolerancesScale());
	if (g_physicsSDK == NULL) {
		std::cerr << "Error creating PhysX device." << std::endl;
		std::cerr << "Exiting..." << std::endl;
		exit(1);
	}
	std::cout << "PhysX is Initialized" << std::endl;
}

void TerminatePhysX() {
	g_physicsSDK->release();
	std::cout << "PhysX is Terminated" << std::endl;
}

int main(int argc, char** argv)
{
	InitPhysX();
	

	TerminatePhysX();
	return 0;
}

