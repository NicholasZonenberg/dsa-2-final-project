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
	
	// Seed random
	srand(static_cast <unsigned> (time(0)));

	// Generate the walls here
	for (size_t i = 0; i < m_uNumberObstacles; ++i)
	{
		std::string name = OBSTACLE_UID;
		name += std::to_string(i);

		m_pEntityMngr->AddEntity(OBSTACLE_MODEL_PATH, name);
		m_pEntityMngr->SetAxisVisibility(true, name);
		vector3 pos(GenerateRandomLaneX(), 0.f, -20.0f + (-5.f * i));
		// Add this to the map of obstacles
		m_mObstacles[name] = pos;

		m_pEntityMngr->SetModelMatrix(
			glm::translate(pos) * glm::rotate(IDENTITY_M4, 90.0f, AXIS_Y),
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
	
	//move the player
	UpdatePlayer(fDeltaTime);

	// Move the obstacles towards the player
	UpdateObtacles(fDeltaTime);

	//Update Entity Manager
	m_pEntityMngr->Update();
		
	//Add objects to render list
	m_pEntityMngr->AddEntityToRenderList(-1, true);
}

void Application::UpdatePlayer(float & dt)
{
	for (uint i=0;i<m_uNumberObstacles;i++)
	{
		if (m_pEntityMngr->GetEntity(0)->IsColliding(m_pEntityMngr->GetEntity(i+1)))
		{
			m_v3PlayerPos.z += m_fSpeed*dt;
			if (m_fPlayerRotY < 270)
			{
				m_fPlayerRotY += 20.0f;
			}
			matrix4 mPlayer = glm::translate(m_v3PlayerPos) * glm::rotate(IDENTITY_M4, m_fPlayerRotY, AXIS_Y);
			m_pEntityMngr->SetModelMatrix(mPlayer, PLAYER_UID);

			//if the player is off screen respawn
			if (m_v3PlayerPos.z > 8.0f)
			{
				PlayerRespawn();
			}
		}
		else
		{
			// update player velocity from input
			m_v3PlayerVelo.x = m_fPlayerInputDirection * m_fPlayerHorizSpeed;

			// update player position from velocity
			m_v3PlayerPos += m_v3PlayerVelo * dt;

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
			m_v3PlayerVelo += m_v3Gravity * dt;
		}
	}
	
}

void Application::UpdateObtacles(float & dt)
{
	std::map<String, Simplex::vector3>::iterator it;

	for (it = m_mObstacles.begin(); it != m_mObstacles.end(); ++it)
	{
		// Add to this position
		it->second.z += m_fSpeed * dt;

		// Check if we need to reset this obstacle
		if (it->second.z >= OBSTACLE_Z_MAX)
		{
			it->second.z = OBSTACLE_Z_START;
			// Put the X in a random position inside the lanes
			it->second.x = GenerateRandomLaneX();
		}

		// Calculate new model matrix
		matrix4 mObstacle = glm::translate(it->second) * glm::rotate(IDENTITY_M4, 90.f, AXIS_Y);

		// Set the model matrix
		m_pEntityMngr->SetModelMatrix(mObstacle, it->first);
	}
}

float Simplex::Application::GenerateRandomLaneX()
{
	return LANE_X_MIN + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (LANE_X_MAX - LANE_X_MIN)));
}

void Application::PlayerRespawn(void)
{
	m_v3PlayerPos = vector3(0.0f);
	m_fPlayerRotY = 180;
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