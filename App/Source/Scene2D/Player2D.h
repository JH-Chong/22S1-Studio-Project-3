/**
 CPlayer2D
 @brief A class representing the player object
 By: Toh Da Jun
 Date: Mar 2020
 */
#pragma once

// Include Singleton template
#include "Singleton\SingletonTemplate.h"

// Include GLEW
#ifndef GLEW_STATIC
#include <GL/glew.h>
#define GLEW_STATIC
#endif

// Include GLM
#include <includes/glm.hpp>
#include <includes/gtc/matrix_transform.hpp>
#include <includes/gtc/type_ptr.hpp>

// Include CEntity2D
#include "Primitives/Entity2D.h"

// Include the Map2D as we will use it to check the player's movements and actions
class CMap2D;

// Include Keyboard controller
#include "Inputs\KeyboardController.h"

// Include AnimatedSprites
#include "Primitives/SpriteAnimation.h"

// Include Physics2D
#include "Physics2D.h"

// Include InventoryManager
#include "InventoryManager.h"

// Include SoundController
#include "..\SoundController\SoundController.h"

class CPlayer2D : public CSingletonTemplate<CPlayer2D>, public CEntity2D
{
	friend CSingletonTemplate<CPlayer2D>;
public:

	// Init
	bool Init(void);

	// Reset
	bool Reset(void);

	// Update
	void Update(const double dElapsedTime, const double& time);

	// PreRender
	void PreRender(void);

	// Render
	void Render(void);

	// PostRender
	void PostRender(void);

	const int& GetLives() const;
	void AddLives(int liveNum);

	void SetEquippedKey(const bool& value);

	const int& GetMicroSpeed() const;
	void AddMicroSpeed(int microSpeedBoost);

	const bool& GetInvisibility() const;
	void SetInvisibility(bool value);

	bool PlaceSpike() const;

	const CPhysics2D::STATUS& GetPhysicsStatus() const;

	CInventoryManager* GetInventoryManager() const;

protected:
	enum DIRECTION
	{
		LEFT = 0,
		RIGHT = 1,
		UP = 2,
		DOWN = 3,
		NUM_DIRECTIONS
	};

	double time;

	int m_lives;
	bool m_equippedKey;
	int m_microSpeedBoost;
	bool m_isInvisible;

	float NUM_STEPS_PER_TILE_XAXIS_PLAYER;
	float NUM_STEPS_PER_TILE_YAXIS_PLAYER;

	float MICRO_STEP_XAXIS_PLAYER;
	float MICRO_STEP_YAXIS_PLAYER;

	glm::vec2 vec2OldIndex;

	CSoundController* cSoundController;

	// Handler to the CPhysics2D instance
	CPhysics2D cPhysics2D;

	// Handler to the CMap2D instance
	CMap2D* cMap2D;

	// Keyboard Controller singleton instance
	CKeyboardController* cKeyboardController;

	//CS: The quadMesh for drawing the tiles
	// CMesh* quadMesh;

	//CS: Animated Sprite
	CSpriteAnimation* animatedSprites;

	// InventoryManager
	CInventoryManager* cInventoryManager;

	// InventoryItem
	CInventoryItem* cInventoryItem;

	// Player's colour
	glm::vec4 runtimeColour;

	// Constructor
	CPlayer2D(void);

	// Destructor
	virtual ~CPlayer2D(void);

	// Constraint the player's position within a boundary
	void Constraint(DIRECTION eDirection = LEFT);

	void SetTileValue() const;

	// Let player interact with the map
	void InteractWithMap(void);

	// Update Climb or Fall
	void UpdateClimbFall(const double dElapsedTime);

	// Checks if position is feasible to move into
	bool CheckPosition(DIRECTION eDirection);

	// Check if the player is in mid-air
	bool IsMidAir(void);
};


