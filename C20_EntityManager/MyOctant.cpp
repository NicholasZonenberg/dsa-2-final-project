#include "MyOctant.h"

// Define the static variables. If this isnt done then we get resolve errors
typename Simplex::uint Simplex::MyOctant::m_uOctantCount = 0;


Simplex::MyOctant::MyOctant()
{
	m_pMeshManager = nullptr;
	m_pEntityManager = nullptr;
	m_pParent = nullptr;
	m_uMaxDepth = 0;
	m_uIdealObjectCount = 0;

	// Setup the ID of this
	m_uID = m_uOctantCount;
	++m_uOctantCount;

	if (m_bPrintDebug)
		PrintDebug("Octant (Default) created! ID is: " + std::to_string(m_uID));
}

/// Constructor with custom things.
/// This should really only be used for the root node
Simplex::MyOctant::MyOctant(MeshManager* a_meshMan, MyEntityManager* a_entityMan, uint aLevels, uint aIdealCount, float a_size, vector3 a_center, float a_minSize)
{
	m_pMeshManager = a_meshMan;
	m_pEntityManager = a_entityMan;
	m_uMaxDepth = aLevels;
	m_uIdealObjectCount = aIdealCount;
	m_fMinSize = a_minSize;
	size = a_size;
	m_v3Center = a_center;

	// Calclate the initial values of these
	right = a_center.x + (size / 2.f);
	left = a_center.x - (size / 2.f);

	top = a_center.y + (size / 2.f);
	bottom = a_center.y - (size / 2.f);

	front = a_center.z + (size / 2.f);
	back = a_center.z - (size / 2.f);

	// Setup the ID of this
	m_uID = Simplex::MyOctant::m_uOctantCount;
	++Simplex::MyOctant::m_uOctantCount;

	if (m_bPrintDebug)
		PrintDebug("Octant (Custom) created! ID is: " + std::to_string(m_uID));
}

Simplex::MyOctant::~MyOctant()
{
	// For all of my children, call delete
	for (std::vector<MyOctant*>::iterator itr = m_lChildOctants.begin(); itr != m_lChildOctants.end(); ++itr)
	{
		(*itr)->DeleteTree();
	}
	// Then delete myself
	DeleteTree();

	// TODO: Implement destructor
	if (m_bPrintDebug)
		PrintDebug("Octant " + std::to_string(m_uID) + " created! ID is: ");
}

/// A recursive method that we can use to build a tree. 
/// This will create 8 children and set them as such on this octant
/// After calculating the values of those children, we will call this 
/// build tree method on them as well
void Simplex::MyOctant::BuildTree()
{
	// If we have no objects in the entity manager, then return
	if (m_pEntityManager->GetEntityCount() == 0)
	{
		return;
	}

	// If this is the max level of octants, then return
	if (m_uCurrentDepth >= m_uMaxDepth)
	{
		if (m_bPrintDebug)
			PrintDebug("(BuildTree) Octant " + std::to_string(m_uID) + " is at the max depth! ");
		return;
	}

	// Check our min size and make sure that we are within the bounds
	float newSize = (right - left) / 2.0f;
	if (newSize <= m_fMinSize)
	{
		return;
	}

	// Calculate the middle
	float midX = left + ((right - left) / 2.0f);
	float midY = bottom + ((top - bottom) / 2.0f);
	float midZ = back + ((front - back) / 2.0f);

	// Create the children with the new dimensions
	m_lChildOctants.push_back(BuildOctant(left, midX, top, midY, front, midZ, newSize));
	m_lChildOctants.push_back(BuildOctant(midX, right, top, midY, front, midZ, newSize));

	m_lChildOctants.push_back(BuildOctant(left, midX, midY, bottom, front, midZ, newSize));
	m_lChildOctants.push_back(BuildOctant(midX, right, midY, bottom, front, midZ, newSize));

	m_lChildOctants.push_back(BuildOctant(left, midX, top, midY, midZ, back, newSize));
	m_lChildOctants.push_back(BuildOctant(midX, right, top, midY, midZ, back, newSize));

	m_lChildOctants.push_back(BuildOctant(left, midX, midY, bottom, midZ, back, newSize));
	m_lChildOctants.push_back(BuildOctant(midX, right, midY, bottom, midZ, back, newSize));

	// For each of those children
	for (std::vector<MyOctant*>::iterator itr = m_lChildOctants.begin(); itr != m_lChildOctants.end(); ++itr)
	{
		// If this child octant has more objects than our Ideal count then
		// Build another layer on it
		if ((*itr)->m_EntityList.size() > m_uIdealObjectCount)
		{
			(*itr)->BuildTree();
		}
	}
}

void Simplex::MyOctant::Set_DrawDebug(bool a_drawDebug)
{
	// For all of my children, call delete
	for (std::vector<MyOctant*>::iterator itr = m_lChildOctants.begin(); itr != m_lChildOctants.end(); ++itr)
	{
		(*itr)->Set_DrawDebug(a_drawDebug);
	}
	// Then delete myself
	m_bDrawOutline = a_drawDebug;
}

void Simplex::MyOctant::AddAllEntites(void)
{
	// Initialize the root node with all of the entities
	for (Simplex::uint i = 0; i < m_pEntityManager->GetEntityCount(); ++i)
	{
		AddEntity(i);
	}
}

/// Create a new octant and set the dimensions of it
/// Set the Parent object of this octant to `this`
/// Set other properties to be the same as this octant
Simplex::MyOctant * Simplex::MyOctant::BuildOctant(float a_left, float a_right, float a_top, float a_bottom, float a_front, float a_back, float a_size)
{
	MyOctant* oct = new MyOctant();

	oct->left = a_left;
	oct->right = a_right;
	oct->top = a_top;
	oct->bottom = a_bottom;
	oct->front = a_front;
	oct->back = a_back;

	oct->size = a_size;

	oct->m_v3Center = vector3(
		oct->left + ((oct->right - oct->left) / 2.0f),
		oct->bottom + ((oct->top - oct->bottom) / 2.0f),
		oct->back + ((oct->front - oct->back) / 2.0f)
	);

	oct->m_pParent = this;
	oct->m_pRoot = m_pRoot;

	oct->m_bDrawOutline = m_bDrawOutline;

	oct->m_pMeshManager = m_pMeshManager;
	oct->m_pEntityManager = m_pEntityManager;
	oct->m_uMaxDepth = m_uMaxDepth;
	oct->m_uIdealObjectCount = m_uIdealObjectCount;
	oct->m_fMinSize = m_fMinSize;

	oct->m_uCurrentDepth = m_uCurrentDepth + 1;

	// Check if there are any objects that should be in this octant
	for (uint i = 0; i < m_EntityList.size(); ++i)
	{
		// Get the global position of this entity
		vector3 entGlobal = m_pEntityManager->GetEntity(m_EntityList[i])->GetRigidBody()->GetCenterGlobal();
		// IF This entity is within the bounds of this octant...
		if (abs(entGlobal.x - oct->m_v3Center.x) <= (oct->size / 2.0) &&
			abs(entGlobal.y - oct->m_v3Center.y) <= (oct->size / 2.0) &&
			abs(entGlobal.z - oct->m_v3Center.z) <= (oct->size / 2.0))
		{
			// Add it to our entity list
			oct->m_EntityList.push_back(m_EntityList[i]);
		}

		/*if( (entGlobal.x <= oct->left)    && (entGlobal.x >= oct->right) &&
		(entGlobal.y >= oct->bottom)  && (entGlobal.y <= oct->top) &&
		(entGlobal.z <= oct->back)    && (entGlobal.z >= oct->front))
		{
		// Add it to our entity list
		oct->m_EntityList.push_back(m_EntityList[i]);
		}*/
	}

	if (m_bPrintDebug)
	{
		PrintDebug("Octant " + std::to_string(oct->m_uID) + "\n     Has " + std::to_string(oct->m_EntityList.size()) + " objects in it.\n"
			+ "     Center: " + std::to_string(oct->m_v3Center.x) + " , " + std::to_string(oct->m_v3Center.y) + " , " + std::to_string(oct->m_v3Center.z)
			+ "\n     Size: " + std::to_string(oct->size) + "\n     On Depth: " + std::to_string(oct->m_uCurrentDepth));
	}

	return oct;
}

/// Check the collisions of every entity in this octant
/// If we have the debug flag set, then tell the mesh 
/// manager to draw a wireframe cube
void Simplex::MyOctant::UpdateOctant(void)
{
	// If we want to, draw our debug cube
	if (m_bDrawOutline)
	{
		m_pMeshManager->AddWireCubeToRenderList(glm::translate(m_v3Center) * glm::scale(vector3(size)), C_YELLOW);
		//m_pMeshManager->AddWireCubeToRenderList(glm::translate(m_v3Center) * glm::scale(vector3(1.0f)), C_BLUE);
	}
	// No need to check collisions on this or children if there are no objects in here. 
	//if (m_EntityList.size() == 0) return;

	// I only want to check this if I am the octant at the bottom of the octree (I have no children)
	if (m_lChildOctants.size() == 0)
	{
		CheckCollisions();
		return;
	}

	// Update all of the children octants of this octant
	for (std::vector<MyOctant*>::iterator itr = m_lChildOctants.begin(); itr != m_lChildOctants.end(); ++itr)
	{
		(*itr)->UpdateOctant();
	}
}

/// Add an enetity ID to this list
void Simplex::MyOctant::AddEntity(uint & a_uEntityID)
{
	m_EntityList.push_back(a_uEntityID);
}

/// Custom debug method that will print stuff out with a class ID in front of it
void Simplex::MyOctant::PrintDebug(std::string message)
{
	if (m_bPrintDebug)
	{
		std::cout << "[MyOctant] " << message << std::endl;
	}
}

/// Iterate through the vector of entity ID's and check collisions on each of them
void Simplex::MyOctant::CheckCollisions(void)
{
	for (std::vector<uint>::iterator outer_itr = m_EntityList.begin(); outer_itr != m_EntityList.end(); ++outer_itr)
	{
		for (std::vector<uint>::iterator inner_itr = m_EntityList.begin() + 1; inner_itr != m_EntityList.end(); ++inner_itr)
		{
			// If this object, is colliding with the other object
			if (m_pEntityManager->GetEntity((*inner_itr))->IsColliding(m_pEntityManager->GetEntity((*outer_itr))))
			{
				m_pEntityManager->GetEntity((*inner_itr))->GetRigidBody()->SetColorColliding(C_RED);
				m_pEntityManager->GetEntity((*outer_itr))->GetRigidBody()->SetColorColliding(C_RED);
			}
			else
			{
				m_pEntityManager->GetEntity((*inner_itr))->GetRigidBody()->SetColorColliding(C_RED);
			}
		}
	}
}

void Simplex::MyOctant::DeleteTree(void)
{
	m_pMeshManager = nullptr;
	m_pEntityManager = nullptr;
	m_pRoot = nullptr;
	m_pParent = nullptr;
	--Simplex::MyOctant::m_uOctantCount;
}
