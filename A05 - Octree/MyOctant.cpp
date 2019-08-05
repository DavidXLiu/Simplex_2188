#include "MyOctant.h"
using namespace Simplex;

// My Octant
MyOctant::MyOctant(uint a_nMaxLevel, uint a_nIdealEntityCount)
{
	Init();

	// Set max subdivision level
	m_uMaxLevel = a_nMaxLevel;
	// Set ideal entity count
	m_uIdealEntityCount = a_nIdealEntityCount;

	// Find entities in current octant and get size
	float min = 0;
	float max = 0;
	for (int i = 0; i < m_pMyEntityMngr->GetEntityCount(); i++)
	{
		MyEntity* entity = m_pMyEntityMngr->GetEntity(i);
		MyRigidBody* rigidBody = entity->GetRigidBody();
		vector3 center = rigidBody->GetCenterGlobal();

		if (center.x < min)
			min = center.x;
		if (center.y < min)
			min = center.y;
		if (center.z < min)
			min = center.z;

		if (center.x > max)
			max = center.x;
		if (center.y > max)
			max = center.y;
		if (center.z > max)
			max = center.z;

		m_EntityList.push_back(i);
	}

	// Check which is larger and set size
	if (-min > max)
		max = -min;
	m_fSize = max * 2.0f;

	ConstructTree(a_nMaxLevel);
}

MyOctant::MyOctant(vector3 a_v3Center, float a_fSize)
{
	Init();

	m_v3Center = a_v3Center;
	m_fSize = a_fSize;

	float distance = a_fSize / 2.0f;
	m_v3Max = a_v3Center + vector3(distance, distance, distance);
	m_v3Min = a_v3Center - vector3(distance, distance, distance);

	// Find entities in current octant
	for (int i = 0; i < m_pMyEntityMngr->GetEntityCount(); i++)
	{
		if (IsColliding(i))
		{
			m_EntityList.push_back(i);
		}
	}
}

MyOctant::MyOctant(MyOctant const & other)
{
	m_uOctantCount = other.m_uOctantCount;
	m_uMaxLevel = other.m_uMaxLevel;
	m_uIdealEntityCount = other.m_uIdealEntityCount;

	m_uID = other.m_uID;
	m_uLevel = other.m_uLevel;
	m_uChildren = other.m_uChildren;

	m_fSize = other.m_fSize;

	m_pMeshMngr = other.m_pMeshMngr;
	m_pMyEntityMngr = other.m_pMyEntityMngr;

	m_v3Center = other.m_v3Center;
	m_v3Min = other.m_v3Min;
	m_v3Max = other.m_v3Max;

	m_pParent = other.m_pParent;
	for (int i = 0; i < 8; i++)
	{
		m_pChild[i] = other.m_pChild[i];
	}

	m_EntityList = other.m_EntityList;

	m_pRoot = other.m_pRoot;
	m_lChild = other.m_lChild;
}

MyOctant & MyOctant::operator=(MyOctant const & other)
{
	if (this != &other)
	{
		Release();
		Init();
		MyOctant temp(other);
		Swap(temp);
	}
	return *this;
}

MyOctant::~MyOctant(void)
{
	Release();
}

void Simplex::MyOctant::Swap(MyOctant & other)
{
	std::swap(m_uID, other.m_uID);
	std::swap(m_uLevel, other.m_uLevel);
	std::swap(m_uChildren, other.m_uChildren);
	std::swap(m_fSize, other.m_fSize);
	std::swap(m_pMeshMngr, other.m_pMeshMngr);
	std::swap(m_pMyEntityMngr, other.m_pMyEntityMngr);
	std::swap(m_v3Center, other.m_v3Center);
	std::swap(m_v3Min, other.m_v3Min);
	std::swap(m_v3Max, other.m_v3Max);
	std::swap(m_pParent, other.m_pParent);
	std::swap(m_pChild, other.m_pChild);
	std::swap(m_pRoot, other.m_pRoot);
	std::swap(m_lChild, other.m_lChild);
}

float Simplex::MyOctant::GetSize(void) { return m_fSize; }
vector3 Simplex::MyOctant::GetCenterGlobal(void) { return m_v3Center; }
vector3 Simplex::MyOctant::GetMinGlobal(void) { return m_v3Min; }
vector3 Simplex::MyOctant::GetMaxGlobal(void) { return m_v3Max; }

bool Simplex::MyOctant::IsColliding(uint a_uRBIndex)
{
	// Get the entity and find its max and min
	MyEntity* entity = m_pMyEntityMngr->GetEntity(a_uRBIndex);
	MyRigidBody* rigidBody = entity->GetRigidBody();
	vector3 center = rigidBody->GetCenterGlobal();

	if (center.x > m_v3Min.x && center.x < m_v3Max.x &&
		center.y > m_v3Min.y && center.y < m_v3Max.y &&
		center.z > m_v3Min.z && center.z < m_v3Max.z)
		return true;
	else
		return false;
}

void Simplex::MyOctant::Display(uint a_nIndex, vector3 a_v3Color)
{
	// Check if the current octant node is the specified index
	if (a_nIndex == m_uID)
	{
		Display(a_v3Color);
	}
}

void Simplex::MyOctant::Display(vector3 a_v3Color)
{
	//m_pMeshMngr->AddWireCubeToRenderList(glm::translate(GetCenterGlobal()) * glm::scale(vector3(GetSize())), a_v3Color);

	// Display children
	if (!IsLeaf())
	{
		for (uint i = 0; i < 8; i++)
		{
			m_pChild[i]->Display();
		}
	}
	else
		m_pMeshMngr->AddWireCubeToRenderList(glm::translate(GetCenterGlobal()) * glm::scale(vector3(GetSize())), a_v3Color);
}

void Simplex::MyOctant::DisplayLeafs(vector3 a_v3Color)
{
	m_pRoot->Display(a_v3Color);
}

void Simplex::MyOctant::ClearEntityList(void)
{
	m_EntityList.clear();
}

void Simplex::MyOctant::Subdivide(void)
{
	// Set the children
	m_uChildren = 8;

	// Make the child octants
	float distance = GetSize() / 4.0f;
	float size = GetSize() / 2.0f;

	m_pChild[0] = new MyOctant(m_v3Center + vector3(distance, distance, distance), size);
	m_pChild[1] = new MyOctant(m_v3Center + vector3(-distance, distance, distance), size);
	m_pChild[2] = new MyOctant(m_v3Center + vector3(distance, distance, -distance), size);
	m_pChild[3] = new MyOctant(m_v3Center + vector3(-distance, distance, -distance), size);
	m_pChild[4] = new MyOctant(m_v3Center + vector3(distance, -distance, distance), size);
	m_pChild[5] = new MyOctant(m_v3Center + vector3(-distance, -distance, distance), size);
	m_pChild[6] = new MyOctant(m_v3Center + vector3(distance, -distance, -distance), size);
	m_pChild[7] = new MyOctant(m_v3Center + vector3(-distance, -distance, -distance), size);

	// Set data for child octants
	for (int i = 0; i < 8; i++)
	{
		m_pChild[i]->m_uOctantCount = GetOctantCount();
		m_pChild[i]->m_uMaxLevel = m_uMaxLevel;
		m_pChild[i]->m_uIdealEntityCount = m_uIdealEntityCount;

		m_pChild[i]->m_uID = m_uID + (i + 1);
		m_pChild[i]->m_uLevel = m_uLevel + 1;

		m_pChild[i]->m_pMeshMngr = m_pMeshMngr;
		m_pChild[i]->m_pMyEntityMngr = m_pMyEntityMngr;

		m_pChild[i]->m_pParent = this;

		// Special case for root
		if (m_pRoot == nullptr)
			m_pChild[i]->m_pRoot = this;
		else
			m_pChild[i]->m_pRoot = m_pRoot;
	}

	// Continue subdividing until max level is reached
	if (m_uLevel + 1 < m_uMaxLevel)
	{
		for (int i = 0; i < 8; i++)
		{
			// Subdivide only if there are more entities than expected in the octant
			if (m_pChild[i]->ContainsMoreThan(m_uIdealEntityCount))
				m_pChild[i]->Subdivide();
		}
	}
}

MyOctant * Simplex::MyOctant::GetChild(uint a_nChild) { return m_pChild[a_nChild]; }
MyOctant * Simplex::MyOctant::GetParent(void) { return m_pParent; }

bool Simplex::MyOctant::IsLeaf(void)
{
	if (m_uChildren == 0)
		return true;
	else
		return false;
}

bool Simplex::MyOctant::ContainsMoreThan(uint a_nEntities)
{
	if (m_EntityList.size() <= a_nEntities)
		return false;
	else
		return true;
}

void Simplex::MyOctant::KillBranches(void)
{
	// Recursively delete all children
	if (m_uChildren > 0)
	{
		for (int i = 0; i < 8; i++)
		{
			m_pChild[i]->KillBranches();
			delete m_pChild[i];
		}
	}

	// Set children to zero
	m_uChildren = 0;
}

void Simplex::MyOctant::ConstructTree(uint a_nMaxLevel)
{
	// Create the tree
	if (m_uLevel < m_uMaxLevel)
		Subdivide();

	// Set entities to specific octants and put octants in list
	AssignIDtoEntity();
	ConstructList();
}

void Simplex::MyOctant::AssignIDtoEntity(void)
{
	// Check if a leaf has been reached
	if (IsLeaf())
	{
		// Set index of each object for the current octant
		for (int i = 0; i < m_EntityList.size(); i++)
		{
			m_pMyEntityMngr->AddDimension(m_EntityList[i], m_uID);
		}
	}
	// Continue recursively going through the octree
	else if (m_uChildren > 0)
	{
		for (int i = 0; i < 8; i++)
		{
			m_pChild[i]->AssignIDtoEntity();
		}
	}
}

uint Simplex::MyOctant::GetOctantCount(void) { return m_uOctantCount; }

void Simplex::MyOctant::Release(void)
{
	m_pMeshMngr = nullptr;
	m_pMyEntityMngr = nullptr;

	// Find the root
	if (m_pRoot == nullptr)
		KillBranches();
}

void Simplex::MyOctant::Init(void)
{
	m_uID = 0;
	m_uLevel = 0;
	m_uChildren = 0;

	m_fSize = 0.0f;

	m_pMeshMngr = MeshManager::GetInstance();
	m_pMyEntityMngr = MyEntityManager::GetInstance();

	m_v3Center = vector3(0.0f);
	m_v3Min = vector3(0.0f);
	m_v3Max = vector3(0.0f);

	m_pParent = nullptr;

	for (int i = 0; i < 8; i++)
	{
		m_pChild[i] = nullptr;
	}

	m_pRoot = nullptr;
}

void Simplex::MyOctant::ConstructList(void)
{
	// Increase amount of octants
	if (m_pRoot != nullptr)
		m_pRoot->m_uOctantCount++;
	else
		m_uOctantCount = 1;

	// Check if there are any objects in the current octant
	if (m_EntityList.size() != 0)
	{
		// Check if the root is currently being checked
		if (m_pRoot == nullptr)
			m_lChild.push_back(this);
		else
			m_pRoot->m_lChild.push_back(this);
		
		// Recursively go through the octree
		if (m_uChildren > 0)
		{
			for (int i = 0; i < 8; i++)
			{
				m_pChild[i]->ConstructList();
			}
		}
	}
}
