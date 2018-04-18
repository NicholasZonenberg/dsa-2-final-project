#include "AppClass.h"
using namespace Simplex;
void Application::InitVariables(void)
{
	// Initialize camera
	m_pCameraMngr->SetPositionTargetAndUp(
		CAMERA_POS, //Position
		CAMERA_TARGET,	//Target
		AXIS_Y); //Up
	
	// Set pitch
	m_pCameraMngr->ChangePitch(CAMERA_PITCH);

	m_pLightMngr->SetPosition(vector3(0.0f, 3.0f, 13.0f), 1); //set the position of first light (0 is reserved for ambient light)
	
	//Entity Manager
	m_pEntityMngr = MyEntityManager::GetInstance();
	
	// add the player
	m_pEntityMngr->AddEntity(PLAYER_MODEL_PATH, PLAYER_UID);

	// set the model matrix and visibility of the player
	m_pEntityMngr->SetAxisVisibility(true, PLAYER_UID);
	
	
	// Generate the walls here
	for (size_t i = 0; i < m_uNumberObstacles; ++i)
	{
		std::string name = OBSTACLE_UID;
		name += std::to_string(i);
		m_vObstactleNames.push_back(name);

		m_pEntityMngr->AddEntity(OBSTACLE_MODEL_PATH, name);
		m_pEntityMngr->SetAxisVisibility(true, name);
		m_pEntityMngr->SetModelMatrix(
			glm::translate(vector3(3.f, 0.f, -5.f * i)) * glm::rotate(IDENTITY_M4, 90.0f, AXIS_Y),
			name);
	}


}
void Application::Update(void)
{
	//Update the system so it knows how much time has passed since the last call
	m_pSystem->Update();

	//Is the ArcBall active?
	//ArcBall();

	//Is the first person camera active?
	//CameraRotation();

	// get delta time
	static float fDeltaTime = 0;	//store the new timer
	static uint uClock = m_pSystem->GenClock(); //generate a new clock for that timer
	fDeltaTime = m_pSystem->GetDeltaTime(uClock); //get the delta time for that timer
	
	// update player velocity from input
	m_v3PlayerVelo.x = m_fPlayerInputDirection * m_fPlayerHorizSpeed;

	// update player position from velocity
	m_v3PlayerPos += m_v3PlayerVelo * fDeltaTime;

	// clamp player to ground
	if (m_v3PlayerPos.y < 0)
		m_v3PlayerPos.y = 0;

	// check if player is on ground
	m_bIsPlayerOnGround = m_v3PlayerPos.y == 0;
	
	// clamp player x to lane bounds
	if (m_v3PlayerPos.x > LANE_X_MAX)
	{
		m_v3PlayerPos.x = LANE_X_MAX;
		m_fPlayerInputDirection = 0.0f;
	}
	else if (m_v3PlayerPos.x < LANE_X_MIN) 
	{
		m_v3PlayerPos.x = LANE_X_MIN;
		m_fPlayerInputDirection = 0.0f;
	}
	else
	{
		m_fPlayerInputDirection *= m_fPlayerInputDampening;
	}

	// set the player's position and rotation
	matrix4 mPlayer = glm::translate(m_v3PlayerPos) * glm::rotate(IDENTITY_M4, m_fPlayerRotY, AXIS_Y);
	m_pEntityMngr->SetModelMatrix(mPlayer, PLAYER_UID);

	// apply gravity to player velo
	m_v3PlayerVelo += m_v3Gravity * fDeltaTime;

	// Move the obstacles towards the player
	UpdateObtacles();

	//Update Entity Manager
	m_pEntityMngr->Update();
		
	//Add objects to render list
	m_pEntityMngr->AddEntityToRenderList(-1, true);
}
void Application::UpdateObtacles(void)
{
	// Loop through all the walls
	// Update the position of each obstacle
	for (std::vector<String>::iterator it = m_vObstactleNames.begin(); it != m_vObstactleNames.end(); ++it)
	{
		matrix4 mObstacle;

		// Set the model matrix
		m_pEntityMngr->SetModelMatrix(mObstacle, *it);
	}

	// If wall position is past a certain amount
		// Reset the wall
		// Put back to obctascle start Z
		// Set the X to a random value between the lane max and mins
}
void Application::Display(void)
{
	// Clear the screen
	ClearScreen();
	
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
	//release the entity manager
	m_pEntityMngr->ReleaseInstance();

	//release GUI
	ShutdownGUI();
}