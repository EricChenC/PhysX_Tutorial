#include "stdafx.h"
#include <iostream>

using namespace physx;

const int WIDTH  = 800;
const int HEIGHT = 600;

static PxPhysics* g_physicsSDK = NULL;
static PxDefaultErrorCallback g_defaultErrorCallback;
static PxDefaultAllocator g_defaultAllocatorCallback;
static PxSimulationFilterShader g_defaultFilterShader = PxDefaultSimulationFilterShader;

PxScene* g_scene = NULL;
PxReal myTimestep = 1.0f / 60.0f;
PxRigidActor *box;

//for mouse dragging
int oldX = 0, oldY = 0;
float rX = 15, rY = 0;
float fps = 0;
int startTime = 0;
int totalFrames = 0;
int state = 1;
float dist = -5;

void SetOrthoForFont(){
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, WIDTH, 0, HEIGHT);
	glScalef(1, -1, 1);
	glTranslatef(0, -HEIGHT, 0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void ResetPerspectiveProjection(){
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

void RenderSpacedBitmapString(int x,int y,int spacing,void *font,char *string){
	char *c;
	int x1 = x;
	for (c = string; *c != '\0'; c++) {
		glRasterPos2i(x1, y);
		glutBitmapCharacter(font, *c);
		x1 = x1 + glutBitmapWidth(font, *c) + spacing;
	}
}

void DrawAxes()
{
	//To prevent the view from disturbed on repaint
	//this push matrix call stores the current matrix state
	//and restores it once we are done with the arrow rendering
	glPushMatrix();
	glColor3f(0, 0, 1);
	glPushMatrix();
	glTranslatef(0, 0, 0.8f);
	glutSolidCone(0.0325, 0.2, 4, 1);
	//Draw label			
	glTranslatef(0, 0.0625, 0.225f);
	RenderSpacedBitmapString(0, 0, 0, GLUT_BITMAP_HELVETICA_10, "Z");
	glPopMatrix();
	glutSolidCone(0.0225, 1, 4, 1);

	glColor3f(1, 0, 0);
	glRotatef(90, 0, 1, 0);
	glPushMatrix();
	glTranslatef(0, 0, 0.8f);
	glutSolidCone(0.0325, 0.2, 4, 1);
	//Draw label
	glTranslatef(0, 0.0625, 0.225f);
	RenderSpacedBitmapString(0, 0, 0, GLUT_BITMAP_HELVETICA_10, "X");
	glPopMatrix();
	glutSolidCone(0.0225, 1, 4, 1);

	glColor3f(0, 1, 0);
	glRotatef(90, -1, 0, 0);
	glPushMatrix();
	glTranslatef(0, 0, 0.8f);
	glutSolidCone(0.0325, 0.2, 4, 1);
	//Draw label
	glTranslatef(0, 0.0625, 0.225f);
	RenderSpacedBitmapString(0, 0, 0, GLUT_BITMAP_HELVETICA_10, "Y");
	glPopMatrix();
	glutSolidCone(0.0225, 1, 4, 1);
	glPopMatrix();
}

void DrawGrid(int GRID_SIZE)
{
	glBegin(GL_LINES);
	glColor3f(0.75f, 0.75f, 0.75f);
	for (int i = -GRID_SIZE; i <= GRID_SIZE; i++)
	{
		glVertex3f((float)i, 0, (float)-GRID_SIZE);
		glVertex3f((float)i, 0, (float)GRID_SIZE);

		glVertex3f((float)-GRID_SIZE, 0, (float)i);
		glVertex3f((float)GRID_SIZE, 0, (float)i);
	}
	glEnd();
}

void InitPhysX() {
	PxFoundation* foundation = PxCreateFoundation(PX_PHYSICS_VERSION, g_defaultAllocatorCallback, g_defaultErrorCallback);
	if (!foundation) std::cerr << "PxCreateFoundation failed!" << std::endl;

	g_physicsSDK = PxCreatePhysics(PX_PHYSICS_VERSION, *foundation, PxTolerancesScale());
	if (g_physicsSDK == NULL) {
		std::cerr << "Error creating PhysX device." << std::endl;
		std::cerr << "Exiting..." << std::endl;
		exit(1);
	}

	if (!PxInitExtensions(*g_physicsSDK)) std::cerr << "PxInitExtensions failed!" << std::endl;

	//Create the scene
	PxSceneDesc sceneDesc(g_physicsSDK->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	if (!sceneDesc.cpuDispatcher) {
		PxDefaultCpuDispatcher* m_cpuDispatcher = PxDefaultCpuDispatcherCreate(1);

		if (!m_cpuDispatcher)std::cerr << "PxDefaultCpuDispatcherCreate failed!" << std::endl;

		sceneDesc.cpuDispatcher = m_cpuDispatcher;
	}

	if (!sceneDesc.filterShader)sceneDesc.filterShader = g_defaultFilterShader;

	g_scene = g_physicsSDK->createScene(sceneDesc);

	if (!g_scene)std::cerr << "createScene failed!" << std::endl;

	g_scene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0);
	g_scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f);

	PxMaterial* m_material = g_physicsSDK->createMaterial(0.5, 0.5, 0.5);

	//Create actors 
	//1) Create ground plane
	PxReal d = 0.0f;
	PxTransform pose = PxTransform(PxVec3(0.0f, 0, 0.0f), PxQuat(PxHalfPi, PxVec3(0.0f, 0.0f, 1.0f))); //Actors' default transform is upright, 
	//so we must rotate 90 degrees to make horizontal plane
	PxRigidStatic* plane = g_physicsSDK->createRigidStatic(pose);

	if (!plane)	std::cerr << "createPlane failed!" << std::endl;

	PxShape* shape = plane->createShape(PxPlaneGeometry(), *m_material);

	if (!shape)	std::cerr << "createShape failed!" << std::endl;

	g_scene->addActor(*plane);

	//2) Create cube  
	PxReal density = 1.0f;
	PxTransform transform(PxVec3(0.0f, 100.0f, 0.0f), PxQuat::createIdentity());
	PxVec3 dimensions(0.5, 0.5, 0.5);
	PxBoxGeometry geometry(dimensions);

	PxRigidDynamic *actor = PxCreateDynamic(*g_physicsSDK, transform, geometry, *m_material, density);
	actor->setAngularDamping(0.75);
	actor->setLinearVelocity(PxVec3(0, 0, 0));

	if (!actor)std::cerr << "create actor failed!" << std::endl;

	g_scene->addActor(*actor);

	box = actor;

	std::cout << "PhysX is Initialized" << std::endl;
}

void TerminatePhysX() {
	g_scene->removeActor(*box);
	g_scene->release();
	box->release();
	g_physicsSDK->release();
	std::cout << "PhysX is Terminated" << std::endl;
}

void StepPhysX()
{
	g_scene->simulate(myTimestep);

	//...perform useful work here using previous frame's state data        
	while (!g_scene->fetchResults())
	{
		// do something useful        
	}
}

void InitGL() {
	glEnable(GL_CULL_FACE);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	GLfloat ambient[4] = { 0.25f, 0.25f, 0.25f, 0.25f };
	GLfloat diffuse[4] = { 1, 1, 1, 1 };
	GLfloat mat_diffuse[4] = { 0.85f, 0, 0, 0 };

	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_diffuse);

	glDisable(GL_LIGHTING);
}

void OnReshape(int nw, int nh) {
	glViewport(0, 0, nw, nh);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, (GLfloat)nw / (GLfloat)nh, 0.1f, 100.0f);
	glMatrixMode(GL_MODELVIEW);
}

void OnShutdown() {
	TerminatePhysX();
}

void Mouse(int button, int s, int x, int y)
{
	if (s == GLUT_DOWN)
	{
		oldX = x;
		oldY = y;
	}

	if (button == GLUT_MIDDLE_BUTTON)
		state = 0;
	else
		state = 1;
}

void Motion(int x, int y)
{
	if (state == 0)
		dist *= (1 + (y - oldY) / 60.0f);
	else
	{
		rY += (x - oldX) / 5.0f;
		rX += (y - oldY) / 5.0f;
	}
	oldX = x;
	oldY = y;

	glutPostRedisplay();
}

void OnIdle() {
	glutPostRedisplay();
}

void getColumnMajor(PxMat33 m, PxVec3 t, float* mat)
{
	mat[0] = m.column0[0];
	mat[1] = m.column0[1];
	mat[2] = m.column0[2];
	mat[3] = 0;

	mat[4] = m.column1[0];
	mat[5] = m.column1[1];
	mat[6] = m.column1[2];
	mat[7] = 0;

	mat[8] = m.column2[0];
	mat[9] = m.column2[1];
	mat[10] = m.column2[2];
	mat[11] = 0;

	mat[12] = t[0];
	mat[13] = t[1];
	mat[14] = t[2];
	mat[15] = 1;
}

void DrawBox(PxShape* pShape) {
	PxTransform pT = PxShapeExt::getGlobalPose(*pShape, *box);
	PxBoxGeometry bg;
	pShape->getBoxGeometry(bg);
	PxMat33 m = PxMat33(pT.q);
	float mat[16];
	getColumnMajor(m, pT.p, mat);
	glPushMatrix();
	glMultMatrixf(mat);
	glutSolidCube(bg.halfExtents.x * 2);
	glPopMatrix();
}

void DrawShape(PxShape* shape)
{
	PxGeometryType::Enum type = shape->getGeometryType();
	switch (type)
	{
	case PxGeometryType::eBOX:
		DrawBox(shape);
		break;
	}
}

void DrawActor(PxRigidActor* actor)
{
	PxU32 nShapes = actor->getNbShapes();
	PxShape** shapes = new PxShape*[nShapes];

	actor->getShapes(shapes, nShapes);
	while (nShapes--)
	{
		DrawShape(shapes[nShapes]);
	}
	delete[] shapes;
}

void RenderActors()
{
	// Render all the actors in the scene 
	DrawActor(box);
}

char buffer[MAX_PATH];
void OnRender() {
	//Calculate fps
	totalFrames++;
	int current = glutGet(GLUT_ELAPSED_TIME);
	if ((current - startTime)>1000)
	{
		float elapsedTime = float(current - startTime);
		fps = ((totalFrames * 1000.0f) / elapsedTime);
		startTime = current;
		totalFrames = 0;
	}

	sprintf_s(buffer, "Time: %8.8fms (%3.2f FPS)", 1 / fps, fps);

	//Update PhysX	
	if (g_scene)
	{
		StepPhysX();
	}


	glClear(GL_COLOR_BUFFER_BIT);
	glLoadIdentity();
	glTranslatef(0, 0, dist);
	glRotatef(rX, 1, 0, 0);
	glRotatef(rY, 0, 1, 0);

	//Draw the grid and axes
	DrawAxes();
	DrawGrid(100);

	glEnable(GL_LIGHTING);
	RenderActors();
	glDisable(GL_LIGHTING);

	SetOrthoForFont();
	glColor3f(1, 1, 1);
	//Show the fps
	RenderSpacedBitmapString(20, 20, 0, GLUT_BITMAP_HELVETICA_12, buffer);

	ResetPerspectiveProjection();

	glutSwapBuffers();
}

void Display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	//setup the view transformation using 
	//gluLookAt(...);

	//Update PhysX 
	if (g_scene)
	{
		StepPhysX();
	}

	RenderActors();

	glutSwapBuffers();
}

int main(int argc, char** argv)
{
	atexit(OnShutdown);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("GLUT PhysX3 Demo - Simple Box");

	glutDisplayFunc(OnRender);
	glutIdleFunc(OnIdle);
	glutReshapeFunc(OnReshape);

	glutMouseFunc(Mouse);
	glutMotionFunc(Motion);
	InitGL();
	InitPhysX();

	glutMainLoop();

	TerminatePhysX();
	return 0;
}

