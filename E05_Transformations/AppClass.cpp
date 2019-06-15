#include "AppClass.h"
void Application::InitVariables(void)
{
	//init the meshes
	for (int i = 0; i < m_iCount; i++)
	{
		m_pMeshes[i] = new MyMesh();
		m_pMeshes[i]->GenerateCube(1.0f, C_BLACK);
	}

	// Make all the positions of the cubes
	m_v3Positions[0] = vector3(-3.0, 4.0, 0.0);
	m_v3Positions[1] = vector3(3.0, 4.0, 0.0);
	m_v3Positions[2] = vector3(-2.0, 3.0, 0.0);
	m_v3Positions[3] = vector3(2.0, 3.0, 0.0);

	// Row
	float tempNum = -3.0;
	for (int i = 4; i < 11; i++)
	{
		m_v3Positions[i] = vector3(tempNum, 2.0, 0.0);
		tempNum++;
	}

	m_v3Positions[11] = vector3(-4.0, 1.0, 0.0);
	m_v3Positions[12] = vector3(-3.0, 1.0, 0.0);
	m_v3Positions[13] = vector3(-1.0, 1.0, 0.0);
	m_v3Positions[14] = vector3(0.0, 1.0, 0.0);
	m_v3Positions[15] = vector3(1.0, 1.0, 0.0);
	m_v3Positions[16] = vector3(3.0, 1.0, 0.0);
	m_v3Positions[17] = vector3(4.0, 1.0, 0.0);

	// Row
	tempNum = -5.0;
	for (int i = 18; i < 29; i++)
	{
		m_v3Positions[i] = vector3(tempNum, 0.0, 0.0);
		tempNum++;
	}

	m_v3Positions[29] = vector3(-5.0, -1.0, 0.0);

	// Row
	tempNum = -3.0;
	for (int i = 30; i < 37; i++)
	{
		m_v3Positions[i] = vector3(tempNum, -1.0, 0.0);
		tempNum++;
	}

	m_v3Positions[37] = vector3(5.0, -1.0, 0.0);
	m_v3Positions[38] = vector3(-5.0, -2.0, 0.0);
	m_v3Positions[39] = vector3(-3.0, -2.0, 0.0);
	m_v3Positions[40] = vector3(3.0, -2.0, 0.0);
	m_v3Positions[41] = vector3(5.0, -2.0, 0.0);
	m_v3Positions[42] = vector3(-2.0, -3.0, 0.0);
	m_v3Positions[43] = vector3(-1.0, -3.0, 0.0);
	m_v3Positions[44] = vector3(1.0, -3.0, 0.0);
	m_v3Positions[45] = vector3(2.0, -3.0, 0.0);

	//m_pMesh = new MyMesh();
	//m_pMesh->GenerateCube(1.0f, C_BLACK);
	//m_pMesh->GenerateSphere(1.0f, 5, C_WHITE);
}
void Application::Update(void)
{
	//Update the system so it knows how much time has passed since the last call
	m_pSystem->Update();

	//Is the arcball active?
	ArcBall();

	//Is the first person camera active?
	CameraRotation();
}
void Application::Display(void)
{
	// Clear the screen
	ClearScreen();

	matrix4 m4View = m_pCameraMngr->GetViewMatrix();
	matrix4 m4Projection = m_pCameraMngr->GetProjectionMatrix();
	
	// Set scale and translation variable
	matrix4 m4Scale = glm::scale(IDENTITY_M4, vector3(2.0f,2.0f,2.0f));
	static vector3 value = vector3(0);
	value += vector3(0.01, 0.0, 0.0);

	// Render each mesh
	for (int i = 0; i < m_iCount; i++)
	{
		// Set translate for each cube
		matrix4 m4Translate = glm::translate(IDENTITY_M4, m_v3Positions[i] + value);
		matrix4 m4Model = m4Scale * m4Translate;

		m_pMeshes[i]->Render(m4Projection, m4View, m4Model);
	}
	
	// draw a skybox
	m_pMeshMngr->AddSkyboxToRenderList();
	
	//render list call
	m_uRenderCallCount = m_pMeshMngr->Render();

	//clear the render list
	m_pMeshMngr->ClearRenderList();
	
	//draw gui
	DrawGUI();
	
	//end the current frame (internally swaps the front and back buffers)
	m_pWindow->display();
}
void Application::Release(void)
{
	// Delete all values in array
	for (int i = 0; i < m_iCount; i++)
	{
		SafeDelete(m_pMeshes[i]);
	}

	// Delete array
	delete[] m_pMeshes;

	//release GUI
	ShutdownGUI();
}