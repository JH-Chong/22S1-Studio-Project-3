/**
 CEnemy2D
 @brief A class which represents the enemy object
 By: Toh Da Jun
 Date: Mar 2020
 */
#pragma once

// Include shader
#include "RenderControl\shader.h"

// Include GLM
#include <includes/glm.hpp>
#include <includes/gtc/matrix_transform.hpp>
#include <includes/gtc/type_ptr.hpp>

// Include CEntity2D
#include "Primitives/Entity2D.h"

// Include the Map2D as we will use it to check the player's movements and actions
class CMap2D;

// Include Settings
#include "GameControl\Settings.h"

// Include Player2D
#include "Player2D.h"

#include <math.h>

class CEnemy2D : public CEntity2D
{
public:
	// Constructor
	CEnemy2D(void);

	// Destructor
	virtual ~CEnemy2D(void);

	// Init
	bool Init(void);

	// Update
	void Update(const double dElapsedTime);

	// PreRender
	void PreRender(void);

	// Render
	void Render(void);

	// PostRender
	void PostRender(void);

	// Set the indices of the enemy2D
	void Seti32vec2Index(const int iIndex_XAxis, const int iIndex_YAxis);

	// Set the number of microsteps of the enemy2D
	void Seti32vec2NumMicroSteps(const int iNumMicroSteps_XAxis, const int iNumMicroSteps_YAxis);

	// Set the UV coordinates of the enemy2D
	void Setvec2UVCoordinates(const float fUVCoordinate_XAxis, const float fUVCoordinate_YAxis);

	// Get the indices of the enemy2D
	glm::vec2 Geti32vec2Index(void) const;

	// Get the number of microsteps of the enemy2D
	glm::vec2 Geti32vec2NumMicroSteps(void) const;

	// Set the UV coordinates of the enemy2D
	glm::vec2 Getvec2UVCoordinates(void) const;

	CInventoryManager* GetInventoryManager() const;

	const int& GetLives() const;
	void AddLives(int liveNum);

	void SetEnemyNum(int num);

	void SetFSM(char* fsm, glm::vec2 supportPos = glm::vec2(-1, -1));

	void SetEnemyVector(std::vector<CEnemy2D*>& cEnemyVector);

	// boolean flag to indicate if this enemy is knocked out
	bool isKnockedOut;
	bool gotSupport;

protected:
	enum DIRECTION
	{
		LEFT = 0,
		RIGHT = 1,
		UP = 2,
		DOWN = 3,
		NUM_DIRECTIONS
	};

	enum FSM
	{
		ATTACK = 0,
		FLEE = 1,
		KNOCKOUT = 2,
		SUPPORT = 3,
		NUM_FSM
	};

	// Give the enemy its number in enemyVector (scene2D)
	int enemyNum;

	int m_lives;

	float NUM_STEPS_PER_TILE_XAXIS_ENEMY;
	float NUM_STEPS_PER_TILE_YAXIS_ENEMY;

	float MICRO_STEP_XAXIS_ENEMY;
	float MICRO_STEP_YAXIS_ENEMY;

	glm::vec2 playerVec2OldIndex;

	//CS: The quadMesh for drawing the tiles
	// CMesh* quadMesh;

	// Handler to the CMap2D instance
	CMap2D* cMap2D;

	// A transformation matrix for controlling where to render the entities
	glm::mat4 transform;

	// The vec2 which stores the indices of the enemy2D in the Map2D
	glm::vec2 i32vec2Index;

	// The vec2 variable which stores The number of microsteps from the tile indices for the enemy2D. 
	// A tile's width or height is in multiples of these microsteps
	glm::vec2 i32vec2NumMicroSteps;

	// The vec2 variable which stores the UV coordinates to render the enemy2D
	glm::vec2 vec2UVCoordinate;

	// The vec2 which stores the indices of the destination for enemy2D in the Map2D
	glm::vec2 vec2Destination;
	// The vec2 which stores the direction for enemy2D movement in the Map2D
	glm::vec2 vec2Direction;

	glm::vec2 nearestLive;

	glm::vec2 supportPos;

	// Settings
	CSettings* cSettings;

	// Current color
	glm::vec4 runtimeColour;

	// Handle to the CPlayer2D
	CPlayer2D* cPlayer2D;

	std::vector<CEnemy2D*>* enemyVector;

	//CS: Animated Sprite
	CSpriteAnimation* animatedSprites;

	// InventoryManager
	CInventoryManager* cInventoryManager;

	// InventoryItem
	CInventoryItem* cInventoryItem;

	// Current FSM
	FSM sCurrentFSM;

	// FSM counter - count how many frames it has been in this FSM
	int iFSMCounter;

	// Max count in a state
	const int iMaxFSMCounter = 60;

	std::vector<glm::vec2> path;

	void SetTileValue() const;

	// Let enemy2D interact with the player
	bool InteractWithPlayer(void);

	// Let enemy2D interact with the map
	void InteractWithMap(void);

	// Update position
	void UpdatePosition(void);

	void FindNearestLive();
};

