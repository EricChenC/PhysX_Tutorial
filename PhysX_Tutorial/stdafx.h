#include<iostream>
#include<GL/freeglut.h>
#include<PxPhysicsAPI.h>
#include<extensions/PxExtensionsAPI.h>
#include<extensions/PxDefaultErrorCallback.h>
#include<extensions/PxDefaultAllocator.h>
#include<extensions/PxDefaultSimulationFilterShader.h>
#include<extensions/PxDefaultCpuDispatcher.h>
#include<extensions/PxShapeExt.h>
#include<extensions/PxSimpleFactory.h>
#include<foundation/PxMat33.h>
#include <cmath>






#ifdef _DEBUG
#pragma comment(lib, "PhysX3DEBUG_x64.lib")
#pragma comment(lib, "PhysX3CommonDEBUG_x64.lib")
#pragma comment(lib, "PhysX3ExtensionsDEBUG.lib")
#else
#pragma comment(lib, "PhysX3_x64.lib")
#pragma comment(lib, "PhysX3Common_x64.lib")
#pragma comment(lib, "PhysX3Extensions.lib")
#endif