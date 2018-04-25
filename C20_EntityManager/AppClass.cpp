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
	m_pEntityMngr->AddEntity(PLAYER_MODEL_PATH, PLAYER_UID, Simplex::EntityLayer::Player);

	// set the model matrix and visibility of the player
	m_pEntityMngr->SetAxisVisibility(true, PLAYER_UID);

	// Seed random
	srand(static_cast <unsigned> (time(0)));

#if DEBUG
	m_uNumberObstacles = 100;
#else 
	m_uNumberObstacles = 800;
#endif

	// Generate the coins and obstacles
	m_mObstacles = GenerateObjects(m_sCowUID, m_sCowModelPath, m_uNumberObstacles, m_fObstacleSpacing, Simplex::EntityLayer::Obstacle);
	m_mCoins = GenerateObjects(m_sCoinUID, m_sCoinModelPath, m_uNumberOfCoins, m_fCoinSpacing, Simplex::EntityLayer::Coin);
	// Create the root node to the octree
	m_pRootOctant = new MyOctant(
		m_pMeshMngr,		// Mesh man
		m_pEntityMngr,		// Entity man
		m_uOctantLevels,		// Max octant levels
		5,				// Ideal entity count
		5.0f,			// Size of the octant
		vector3(0.f, 1.f, 0.f)
	);

	m_pRootOctant->AddAllEntites();
	m_pRootOctant->BuildTree();
}

std::map<std::string, vector3> Simplex::Application::GenerateObjects(const std::string a_UID, const std::string a_ModelPath, const uint & a_Amount, float & a_Spacing, Simplex::EntityLayer a_layer)
{
	std::map<std::string, vector3> objects;

	// Generate the coins
	for (size_t i = 0; i < a_Amount; ++i)
	{
		std::string name = a_UID;
		name += std::to_string(i);

		m_pEntityMngr->AddEntity(a_ModelPath, name, a_layer);
		m_pEntityMngr->SetAxisVisibility(true, name);

		vector3 pos(GenerateRandomLaneX(), 0.f, -20.0f - (a_Spacing * i));

		// Add this to the map of coins
		objects[name] = pos;

		m_pEntityMngr->SetModelMatrix(
			glm::translate(pos) * glm::rotate(IDENTITY_M4, 90.0f, AXIS_Y),
			name);
	}

	return objects;
}

void Application::Update(void)
{
	//Update the system so it knows how much time has passed since the last call
	m_pSystem->Update();

	// get delta time
	static float fDeltaTime = 0;	//store the new timer
	static uint uClock = m_pSystem->GenClock(); //generate a new clock for that timer
	fDeltaTime = m_pSystem->GetDeltaTime(uClock); //get the delta time for that timer

	if (m_gameState == GameState::Playing)
	{
		// Move the player
		UpdatePlayer(fDeltaTime);

		// Move the obstacles towards the player
		UpdateObtacles(fDeltaTime);

		// Update the coins
		UpdateCoins(fDeltaTime);
	}

	//Add objects to render list
	m_pEntityMngr->AddEntityToRenderList(-1, true);
	++m_uFrameCount;
}

void Application::UpdatePlayer(float & dt)
{
	bool colliding = false;

	// See which optimization that we want to use
	if (m_bRunOptimialCollision)
	{
		colliding = OptimizedCollisionDetection();
	}
	else
	{
		colliding = BruteForceCollisionDetection();
	}
	
	if (colliding)
	{
		m_v3PlayerPos.z += m_fSpeed * dt;
		if (m_fPlayerRotY < 270)
		{
			m_fPlayerRotY += 20.0f;
		}

		//if the player is off screen, game over!
		if (m_v3PlayerPos.z > 8.0f)
		{
			SetGameState(GameState::GameOver);
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
	}

	// set the player's position and rotation
	matrix4 mPlayer = glm::translate(m_v3PlayerPos) * glm::rotate(IDENTITY_M4, m_fPlayerRotY, AXIS_Y);
	m_pEntityMngr->SetModelMatrix(mPlayer, PLAYER_UID);

	// apply gravity to player velo
	m_v3PlayerVelo += m_v3Gravity * dt;
}

bool Simplex::Application::BruteForceCollisionDetection()
{
	bool isColliding = false;

	//check collisions. This is brute force and checking every object in the scene.
	for (uint i = 0; i < m_pEntityMngr->GetEntityCount() - 1; ++i)
	{
		for (uint j = i + 1; j < m_pEntityMngr->GetEntityCount(); ++j)
		{
			// If what we just checked was the player, then set isColldiing to true.
			if (m_pEntityMngr->GetEntity(i)->IsColliding(m_pEntityMngr->GetEntity(j)))
			{
				// Check that what we just hit was an obstcle
				if ((m_pEntityMngr->GetEntity(i)->GetEntityLayer() == EntityLayer::Obstacle ||
					m_pEntityMngr->GetEntity(j)->GetEntityLayer() == EntityLayer::Obstacle) &&
					(i == 0 || j == 0))
				{
					isColliding = true;
				}
			}
		}
	}

	return isColliding;
}

bool Simplex::Application::OptimizedCollisionDetection()
{
	bool isColliding = false;
	// this is the octree stuff, which isnt great for this because we need
	// to rebuild the tree so often. But, this was a project requirement so 
	// I implmented it anyway. 
	/*if(m_uFrameCount % 5 ==0)
		m_pRootOctant->BuildTree();

	// Update the octree collisions. Updating root will recusively call the children
	m_pRootOctant->UpdateOctant();
	*/

	// We only need to check against the player in this case, nothing else
	MyEntity * player = m_pEntityMngr->GetEntity(0);

	for (uint i = 0; i < m_pEntityMngr->GetEntityCount(); ++i)
	{		
		if (m_pEntityMngr->GetEntity(i)->GetRigidBody()->GetCenterGlobal().z > 5.f) continue;
		// If what we just checked was the player, then set isColldiing to true.
		if (player->IsColliding(m_pEntityMngr->GetEntity(i)))
		{
			// Check that what we just hit was an obstcle
			if (m_pEntityMngr->GetEntity(i)->GetEntityLayer() == EntityLayer::Obstacle)
			{
				isColliding = true;
			}
		}
	}
	player = nullptr;

	return isColliding;
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

void Simplex::Application::UpdateCoins(float & dt)
{
	// No need to check if the coins are emtpy
	//if (m_mCoins.size() <= 0) return;

	std::map<String, Simplex::vector3>::iterator it;

	for (it = m_mCoins.begin(); it != m_mCoins.end(); ++it)
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
	m_fPlayerInputDirection = 0.0f;
	m_v3PlayerVelo = vector3(0.0f);

	m_v3PlayerPos = vector3(0.0f);
	m_fPlayerRotY = 180;

	matrix4 mPlayer = glm::translate(m_v3PlayerPos) * glm::rotate(IDENTITY_M4, m_fPlayerRotY, AXIS_Y);
	m_pEntityMngr->SetModelMatrix(mPlayer, PLAYER_UID);
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

void Application::SetGameState(const GameState a_gameState)
{
	switch (a_gameState)
	{
	case GameState::Playing:
		ResetObstaclesAndCoins();
		PlayerRespawn();
		// TODO: reset score here
		break;
	case GameState::GameOver:
		break;
	}

	m_gameState = a_gameState;
}

void Application::ResetObstaclesAndCoins(void)
{
	std::map<String, Simplex::vector3>::iterator it;
	int i = 0;

	for (it = m_mCoins.begin(); it != m_mCoins.end(); ++it)
	{
		vector3 pos(GenerateRandomLaneX(), 0.f, -20.0f - (m_fCoinSpacing * i));
		it->second = pos;
		i++;
	}

	i = 0;
	for (it = m_mObstacles.begin(); it != m_mObstacles.end(); ++it)
	{
		vector3 pos(GenerateRandomLaneX(), 0.f, -20.0f - (m_fObstacleSpacing * i));
		it->second = pos;
		i++;
	}
}
