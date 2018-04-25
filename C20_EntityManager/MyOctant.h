/*----------------------------------------------
Programmer: Ben Hoffman (bah8892@rit.edu)
Date: 4/22/2018
----------------------------------------------*/
#pragma once

#include "MyEntityManager.h"

namespace Simplex
{
	//Adding Application to the Simplex namespace

	class MyOctant
	{
		static uint m_uOctantCount;	// Store the current number of octants instatiated
		uint m_uMaxDepth;	// Store the max level an octant can go
		uint m_uIdealObjectCount;	// Ideal number of entities this object will containt
		float m_fMinSize;

		uint m_uID = 0;	// ID of this octant
		uint m_uCurrentDepth = 0;	// Current level of the octant
		uint m_uChildren = 0;	// Number of children this ocant (0 or 8)

		bool m_bDrawOutline = true;	// Will add a wireframe cube if we want to
		bool m_bPrintDebug = false;	// If true then we will print out debug statements

		MeshManager * m_pMeshManager = nullptr;
		MyEntityManager * m_pEntityManager = nullptr;

		float left;
		float right;
		float top;
		float bottom;
		float front;
		float back;

		float size;
		vector3 m_v3Center;


		MyOctant * m_pParent = nullptr;	// The parent of the current octant
		std::vector<MyOctant*> m_lChildOctants; // The child octants of this octant (Can be improved by make this a pointer array)

		std::vector<uint> m_EntityList;	// A list of entitties under this octant 

		MyOctant * m_pRoot = nullptr;	// The root octant

	public:
		/*
		USAGE: Default constructor for the MyOctant object
		ARGUMENTS: ---
		OUTPUT: new MyOctant
		*/
		MyOctant();

		/*
		USAGE: Builds the this octant
		ARGUMENTS:
		MeshManager* a_meshMan -> Pointer to the mesh manager so we can draw out outlines
		MyEntityManager* a_entityMan -> Pointer to the entity manager so we check what objects are inside of this
		uint aLevels -> The MAX number of levels for this octant to have
		uint aIdealCount -> Ideal number of objects in this octant
		OUTPUT: New MyOctantObject
		*/
		MyOctant(MeshManager* a_meshMan, MyEntityManager* a_entityMan, uint aLevels, uint aIdealCount = 5, float a_size = 80.0f, vector3 a_center = vector3(0.0f), float a_minSize = 1.0f);

		/*
		USAGE: Destructor for the MyOctant object
		ARGUMENTS: ---
		OUTPUT: ---
		*/
		~MyOctant();

		/*
		USAGE: Builds the this octant
		ARGUMENTS: ---
		OUTPUT: ---
		*/
		void BuildTree();

		/*
		USAGE:
		Checks collisions between the objects inside this octant.
		Add our debug cube to mesh manager if we have to
		ARGUMENTS: ---
		OUTPUT: ---
		*/
		void UpdateOctant(void);

		/*
		USAGE: Adds an entity to this octants entity list
		ARGUMENTS: uint & a_uEntityID -> The entity to add to the list
		OUTPUT: ---
		*/
		void AddEntity(uint & a_uEntityID);

		/*
		USAGE: Toggle using the draw debug
		ARGUMENTS: ---
		OUTPUT: ---
		*/
		void Set_DrawDebug(bool a_drawDebug);

		/*
		USAGE: Adds all entities that are in the entity manager to this node.
		This is for the root node, or if we need to reset.
		ARGUMENTS: ---
		OUTPUT: ---
		*/
		void AddAllEntites(void);

	private:

		MyOctant * BuildOctant(float left, float right, float top, float bottom, float front, float back, float size);

		/*
		USAGE:
		Prints a debug message in this classes format
		ARGUMENTS: std::string message -> The message we want to print
		OUTPUT: ---
		*/
		void PrintDebug(std::string message);	// Can be improved by using a reference, this is just for debugging

												/*
												USAGE: Checks the collisions of everything in this octant
												ARGUMENTS: ---
												OUTPUT: ---
												*/
		void CheckCollisions(void);

		/*
		USAGE: Recursively deletes the tree
		ARGUMENTS: ---
		OUTPUT: ---
		*/
		void DeleteTree(void);

	};	// End of MyOctant

}	// namespace Simplex


	/*
	USAGE:
	ARGUMENTS: ---
	OUTPUT: ---
	*/