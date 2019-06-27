#include "AppClass.h"
void Application::InitVariables(void)
{
	//Change this to your name and email
	m_sProgrammer = "Alberto Bobadilla - labigm@rit.edu";
	
	//Set the position and target of the camera
	//(I'm at [0,0,10], looking at [0,0,0] and up is the positive Y axis)
	m_pCameraMngr->SetPositionTargetAndUpward(AXIS_Z * 20.0f, ZERO_V3, AXIS_Y);

	//if the light position is zero move it
	if (m_pLightMngr->GetPosition(1) == ZERO_V3)
		m_pLightMngr->SetPosition(vector3(0.0f, 0.0f, 3.0f));

	//if the background is cornflowerblue change it to black (its easier to see)
	if (vector3(m_v4ClearColor) == C_BLUE_CORNFLOWER)
	{
		m_v4ClearColor = vector4(ZERO_V3, 1.0f);
	}
	
	//if there are no segments create 7
	if(m_uOrbits < 1)
		m_uOrbits = 7;

	float fSize = 1.0f; //initial size of orbits
	float fRadius = 0.95f; // Radius of orbits

	//creating a color using the spectrum 
	uint uColor = 650; //650 is Red
	//prevent division by 0
	float decrements = 250.0f / (m_uOrbits > 1? static_cast<float>(m_uOrbits - 1) : 1.0f); //decrement until you get to 400 (which is violet)
	/*
		This part will create the orbits, it start at 3 because that is the minimum subdivisions a torus can have
	*/
	uint uSides = 3; //start with the minimal 3 sides
	for (uint i = uSides; i < m_uOrbits + uSides; i++)
	{
		vector3 v3Color = WaveLengthToRGB(uColor); //calculate color based on wavelength
		m_shapeList.push_back(m_pMeshMngr->GenerateTorus(fSize, fSize - 0.1f, 3, i, v3Color)); //generate a custom torus and add it to the meshmanager
		uColor -= static_cast<uint>(decrements); //decrease the wavelength

		std::vector<vector3> stopList; // List of all stops on the current shape
		float angleRadians = ((360.0 / i) * (2.0 * PI)) / 360.0; // Calculate the increment angle for the current shape
		for (int j = 1; j <= i; j++)
		{
			// Add the calculated stop
			stopList.push_back(vector3(cos(angleRadians * j) * fRadius, sin(angleRadians * j) * fRadius, 0));
		}
		m_stopShapeList.push_back(stopList); // Add to the stops list for all the shapes

		//Get a timer
		m_fTimers.push_back(0);	//store the new timer
		m_uClocks.push_back(m_pSystem->GenClock()); //generate a new clock for that timer

		// Set path nums
		m_uPathNums.push_back(0); // Start every path num at 0 for each shape

		fSize += 0.5f; //increment the size for the next orbit
		fRadius += 0.5f; // Increment the radius for the next orbit
	}
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

	matrix4 m4View = m_pCameraMngr->GetViewMatrix(); //view Matrix
	matrix4 m4Projection = m_pCameraMngr->GetProjectionMatrix(); //Projection Matrix
	matrix4 m4Offset = IDENTITY_M4; //offset of the orbits, starts as the global coordinate system
	/*
		The following offset will orient the orbits as in the demo, start without it to make your life easier.
	*/
	m4Offset = glm::rotate(IDENTITY_M4, 1.5708f, AXIS_Z);

	// draw a shapes
	for (uint i = 0; i < m_uOrbits; ++i)
	{
		m_pMeshMngr->AddMeshToRenderList(m_shapeList[i], glm::rotate(m4Offset, 1.5708f, AXIS_X));

		m_fTimers[i] += m_pSystem->GetDeltaTime(m_uClocks[i]); //get the delta time for the current shape's timer

		//calculate the current position
		vector3 v3CurrentPos; // The current position
		vector3 v3StartPos; // The starting position of the current path
		vector3 v3EndPos; // The ending position of the current path
		v3StartPos = m_stopShapeList[i][m_uPathNums[i]]; // Set the start position
		v3EndPos = m_stopShapeList[i][(m_uPathNums[i] + 1) % m_stopShapeList[i].size()]; // Set the end position

		// Set the duration for each path
		float fDuration = 1.0f;

		// Calculate the current percentage along the path relevant to time
		float fPercentage = MapValue(m_fTimers[i], 0.0f, fDuration, 0.0f, 1.0f);

		// Use lerp for the current position of the sphere along the path
		v3CurrentPos = glm::lerp(v3StartPos, v3EndPos, fPercentage);

		// Check end of path
		if (fPercentage >= 1.0f)
		{
			// Change to next path
			m_uPathNums[i]++;

			// Restart timer
			m_fTimers[i] = m_pSystem->GetDeltaTime(m_uClocks[i]);

			// Make sure path num does not exceed the number of paths
			m_uPathNums[i] %= m_stopShapeList[i].size();
		}

		//calculate the current position
		//vector3 v3CurrentPos = ZERO_V3;
		matrix4 m4Model = glm::translate(m4Offset, v3CurrentPos);

		//draw spheres
		m_pMeshMngr->AddSphereToRenderList(m4Model * glm::scale(vector3(0.1)), C_WHITE);
	}

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
	//release GUI
	ShutdownGUI();
}