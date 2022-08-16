/**
 Player2D
 @brief A class representing the player object
 By: Toh Da Jun
 Date: Mar 2020
 */
#include "Player2D.h"

#include <iostream>
using namespace std;

// Include Shader Manager
#include "RenderControl\ShaderManager.h"

// Include ImageLoader
#include "System\ImageLoader.h"

// Include the Map2D as we will use it to check the player's movements and actions
#include "Map2D.h"
#include "Primitives/MeshBuilder.h"

#include "GameManager.h"

/**
 @brief Constructor This constructor has protected access modifier as this class will be a Singleton
 */
CPlayer2D::CPlayer2D(void)
	: cMap2D(NULL)
	, cKeyboardController(NULL)
	, animatedSprites(NULL)
	, runtimeColour(glm::vec4(1.0f))
{
	transform = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first

	// Initialise vecIndex
	vec2Index = glm::i32vec2(0);

	// Initialise vecNumMicroSteps
	vec2NumMicroSteps = glm::i32vec2(0);

	// Initialise vec2UVCoordinate
	vec2UVCoordinate = glm::vec2(0.0f);
}

/**
 @brief Destructor This destructor has protected access modifier as this class will be a Singleton
 */
CPlayer2D::~CPlayer2D(void)
{
	// Delete the CAnimationSprites
	if (animatedSprites)
	{
		delete animatedSprites;
		animatedSprites = NULL;
	}

	if (cInventoryManager)
	{
		delete cInventoryManager;
		cInventoryManager = NULL;
	}

	// We won't delete this since it was created elsewhere
	cKeyboardController = NULL;

	// We won't delete this since it was created elsewhere
	cMap2D = NULL;

	// optional: de-allocate all resources once they've outlived their purpose:
	glDeleteVertexArrays(1, &VAO);
}

/**
  @brief Initialise this instance
  */
bool CPlayer2D::Init(void)
{
	time = 0.0;

	m_lives = 5;
	m_microSpeedBoost = 0;
	m_isInvisible = m_equippedKey = false;

	cSoundController = &CSoundController::GetInstance();

	// Store the keyboard controller singleton instance here
	cKeyboardController = &CKeyboardController::GetInstance();
	// Reset all keys since we are starting a new game
	cKeyboardController->Reset();

	// Get the handler to the CSettings instance
	cSettings = &CSettings::GetInstance();

	NUM_STEPS_PER_TILE_XAXIS_PLAYER = (int)cSettings->NUM_STEPS_PER_TILE_XAXIS;
	NUM_STEPS_PER_TILE_YAXIS_PLAYER = (int)cSettings->NUM_STEPS_PER_TILE_YAXIS;

	MICRO_STEP_XAXIS_PLAYER = cSettings->TILE_WIDTH / NUM_STEPS_PER_TILE_XAXIS_PLAYER;
	MICRO_STEP_YAXIS_PLAYER = cSettings->TILE_HEIGHT / NUM_STEPS_PER_TILE_YAXIS_PLAYER;

	// Set the Physics to fall status by default
	cPhysics2D.Init();
	cPhysics2D.SetStatus(CPhysics2D::STATUS::FALL);

	// Get the handler to the CMap2D instance
	cMap2D = &CMap2D::GetInstance();
	// Find the indices for the player in arrMapInfo, and assign it to cPlayer2D
	unsigned int uiRow = -1;
	unsigned int uiCol = -1;
	if (cMap2D->FindValue(200, uiRow, uiCol) == false)
		return false;	// Unable to find the start position of the player, so quit this game

	// Erase the value of the player in the arrMapInfo
	cMap2D->SetMapInfo(uiRow, uiCol, 3);

	if (cMap2D->FindValue(200, uiRow, uiCol))
		return false;

	// Set the start position of the Player to iRow and iCol
	vec2Index = glm::i32vec2(uiCol, uiRow);
	// By default, microsteps should be zero
	vec2NumMicroSteps = glm::i32vec2(0, 0);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// Create the quad mesh for the player
	/*
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	quadMesh = CMeshBuilder::GenerateQuad(glm::vec4(1, 1, 1, 1), cSettings->TILE_WIDTH, cSettings->TILE_HEIGHT);
	*/

	// Load the player texture
	iTextureID = CImageLoader::GetInstance().LoadTextureGetID("Image/Scene2D/Player.png", true);
	if (iTextureID == 0)
	{
		std::cout << "Unable to load Image/Scene2D/Player.png" << std::endl;
		return false;
	}

	//CS: Create the animated sprite and setup the animation
	animatedSprites = CMeshBuilder::GenerateSpriteAnimation(2, 2, cSettings->TILE_WIDTH, cSettings->TILE_HEIGHT);
	animatedSprites->AddAnimation("left", 0, 1);
	animatedSprites->AddAnimation("right", 2, 3);
	//CS: Play the "idle" animation as default
	animatedSprites->PlayAnimation("left", -1, 1.0f);

	//CS: Init the color to white
	runtimeColour = glm::vec4(1.0, 1.0, 1.0, 1.0);

	cInventoryManager = new CInventoryManager();
	/*
	// Add a Lives icon as one of the inventory items
	cInventoryItem = cInventoryManager->Add("Lives", "Image/Scene2D_Lives.tga", 3, 0);
	cInventoryItem->vec2Size = glm::vec2(25, 25);

	// Add a Health icon as one of the inventory items
	cInventoryItem = cInventoryManager->Add("Health", "Image/Scene2D_Health.tga", 100, 100);
	cInventoryItem->vec2Size = glm::vec2(25, 25);
	*/

	return true;
}

/**
 @brief Reset this instance
 */
bool CPlayer2D::Reset()
{
	unsigned int uiRow = -1;
	unsigned int uiCol = -1;
	if (cMap2D->FindValue(200, uiRow, uiCol) == false)
		return false;	// Unable to find the start position of the player, so quit this game

	// Erase the value of the player in the arrMapInfo
	cMap2D->SetMapInfo(uiRow, uiCol, 0);

	// Set the start position of the Player to iRow and iCol
	vec2Index = glm::i32vec2(uiCol, uiRow);
	// By default, microsteps should be zero
	vec2NumMicroSteps = glm::i32vec2(0, 0);

	//CS: Init the color to white
	runtimeColour = glm::vec4(1.0, 1.0, 1.0, 1.0);

	return true;
}

/**
 @brief Update this instance
 */
void CPlayer2D::Update(const double dElapsedTime, const double& time)
{
	this->time = time;

	NUM_STEPS_PER_TILE_XAXIS_PLAYER = (int)cSettings->NUM_STEPS_PER_TILE_XAXIS - m_microSpeedBoost;
	NUM_STEPS_PER_TILE_YAXIS_PLAYER = (int)cSettings->NUM_STEPS_PER_TILE_YAXIS - m_microSpeedBoost;

	MICRO_STEP_XAXIS_PLAYER = cSettings->TILE_WIDTH / NUM_STEPS_PER_TILE_XAXIS_PLAYER;
	MICRO_STEP_YAXIS_PLAYER = cSettings->TILE_HEIGHT / NUM_STEPS_PER_TILE_YAXIS_PLAYER;

	// Store the old position
	vec2OldIndex = vec2Index;

	// Get keyboard updates
	if (cKeyboardController->IsKeyDown(GLFW_KEY_A))
	{
		// Calculate the new position to the left
		if (vec2Index.x >= 0)
		{
			vec2NumMicroSteps.x--;
			if (vec2NumMicroSteps.x < 0)
			{
				vec2NumMicroSteps.x = (int)NUM_STEPS_PER_TILE_XAXIS_PLAYER - 1;
				vec2Index.x--;
			}
		}

		// If the new position is not feasible, then revert to old position
		if (CheckPosition(LEFT) == false)
		{
			vec2Index = vec2OldIndex;
			vec2NumMicroSteps.x = 0;
		}

		// Check if player is in mid-air, such as walking off a platform
		if (IsMidAir() == true)
		{
			if (cPhysics2D.GetStatus() != CPhysics2D::STATUS::CLIMB)
				cPhysics2D.SetStatus(CPhysics2D::STATUS::FALL);
		}

		Constraint(LEFT);

		//CS: Play the "left" animation
		animatedSprites->PlayAnimation("left", -1, 1.0f);
	}
	else if (cKeyboardController->IsKeyDown(GLFW_KEY_D))
	{
		if (vec2Index.x < (int)cSettings->NUM_TILES_XAXIS)
		{
			vec2NumMicroSteps.x++;
			if (vec2NumMicroSteps.x >= (int)NUM_STEPS_PER_TILE_XAXIS_PLAYER)
			{
				vec2NumMicroSteps.x = 0;
				// Bug occurred here. When the character was standing between the very edge (right side) of the ladder and the cloud
				// It would be teleported into the cloud when S was pressed first then D was pressed after
				if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + 1) < 100) // For safety in case of any bugs (A and S dont have)
					vec2Index.x++;
			}
		}

		// If the new position is not feasible, then revert to old position
		if (CheckPosition(RIGHT) == false)
			vec2NumMicroSteps.x = 0;

		// Check if player is in mid-air, such as walking off a platform
		if (IsMidAir() == true)
		{
			if (cPhysics2D.GetStatus() != CPhysics2D::STATUS::CLIMB)
				cPhysics2D.SetStatus(CPhysics2D::STATUS::FALL);
		}

		Constraint(RIGHT);

		//CS: Play the "right" animation
		animatedSprites->PlayAnimation("right", -1, 1.0f);
	}

	if (cKeyboardController->IsKeyDown(GLFW_KEY_W))
	{
		if ((cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) == 36)
			|| (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) == 35)
			|| (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) == 30))
		{
			// Calculate the new position up
			if (vec2Index.y < (int)cSettings->NUM_TILES_YAXIS)
			{
				vec2NumMicroSteps.y++;
				if (vec2NumMicroSteps.y > (int)NUM_STEPS_PER_TILE_YAXIS_PLAYER)
				{
					vec2NumMicroSteps.y = 0;
					if (cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x) < 100) // For safety in case of any bugs (A and S dont have)
						vec2Index.y++;
					cPhysics2D.SetStatus(CPhysics2D::STATUS::CLIMB);
				}
			}

			if (CheckPosition(UP) == false)
				vec2NumMicroSteps.y = (int)NUM_STEPS_PER_TILE_YAXIS_PLAYER - 1;

			Constraint(UP);
		}
	}
	else if (cKeyboardController->IsKeyDown(GLFW_KEY_S))
	{
		if (cPhysics2D.GetStatus() != CPhysics2D::STATUS::FALL)
		{
			if ((cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) == 36)
				|| (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) == 35)
				|| (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) == 30)
				|| (cMap2D->GetMapInfo(vec2Index.y - 1, vec2Index.x) == 30))
			{
				// Calculate the new position down
				if (vec2Index.y >= 0)
				{
					vec2NumMicroSteps.y--;
					if (vec2NumMicroSteps.y < 0)
					{
						if (cMap2D->GetMapInfo(vec2Index.y - 1, vec2Index.x) < 100)
						{
							vec2NumMicroSteps.y = (int)NUM_STEPS_PER_TILE_YAXIS_PLAYER - 1;
							vec2Index.y--;
						}
						else
						{
							vec2NumMicroSteps.y = 0;
							cPhysics2D.SetStatus(CPhysics2D::STATUS::FALL);
						}
					}
				}

				// Prevent character from going through clouds when going down a ladder
				if ((vec2NumMicroSteps.y != 0)
					&& (vec2NumMicroSteps.x != 0)) // Adding this because of reset when D key is pressed
				{
					if ((cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) == 30)
						&& (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + 1) >= 100))
					{
						vec2NumMicroSteps.y = (int)NUM_STEPS_PER_TILE_YAXIS_PLAYER;
					}
				}

				Constraint(DOWN);
			}
		}
		else
		{
			// This is implemented to fix a bug where the character falls through the ladder very fast
			// due to both FALL physics & S key pressed codes updating the y index
			if (vec2NumMicroSteps.y == 0)
			{
				if (cMap2D->GetMapInfo(vec2Index.y - 1, vec2Index.x) == 30)
				{
					cPhysics2D.SetStatus(CPhysics2D::STATUS::CLIMB);
				}
			}
		}
	}

	// Interact with the Map
	InteractWithMap();

	// Update Climb or Fall
	UpdateClimbFall(dElapsedTime);

	//CS: Update the animated sprite
	animatedSprites->Update(dElapsedTime);

	// Update the UV Coordinates
	vec2UVCoordinate.x = cSettings->ConvertIndexToUVSpace(cSettings->x, vec2Index.x, false, vec2NumMicroSteps.x*MICRO_STEP_XAXIS_PLAYER);
	vec2UVCoordinate.y = cSettings->ConvertIndexToUVSpace(cSettings->y, vec2Index.y, false, vec2NumMicroSteps.y*MICRO_STEP_YAXIS_PLAYER);
}

/**
 @brief Set up the OpenGL display environment before rendering
 */
void CPlayer2D::PreRender(void)
{
	// Activate blending mode
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Activate the shader
	CShaderManager::GetInstance().Use(sShaderName);
}

/**
 @brief Render this instance
 */
void CPlayer2D::Render(void)
{
	glBindVertexArray(VAO);
	// get matrix's uniform location and set matrix
	unsigned int transformLoc = glGetUniformLocation(CShaderManager::GetInstance().activeShader->ID, "transform");
	unsigned int colorLoc = glGetUniformLocation(CShaderManager::GetInstance().activeShader->ID, "runtimeColour");
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

	transform = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
	transform = glm::translate(transform, glm::vec3(vec2UVCoordinate.x,
													vec2UVCoordinate.y,
													0.0f));
	// Update the shaders with the latest transform
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
	glUniform4fv(colorLoc, 1, glm::value_ptr(runtimeColour));

	glBindTexture(GL_TEXTURE_2D, iTextureID); // Get the texture to be rendered
	glBindVertexArray(VAO);
	animatedSprites->Render(); // CS: Render the animated sprite
	glBindVertexArray(0);
}

/**
 @brief PostRender Set up the OpenGL display environment after rendering.
 */
void CPlayer2D::PostRender(void)
{
	// Disable blending
	glDisable(GL_BLEND);
}

/**
 @brief Constraint the player's position within a boundary
 @param eDirection A DIRECTION enumerated data type which indicates the direction to check
 */
void CPlayer2D::Constraint(DIRECTION eDirection)
{
	runtimeColour = glm::vec4(1.0, 1.0, 1.0, 1.0);

	if (eDirection == LEFT)
	{
		if (vec2Index.x < 0)
		{
			vec2Index.x = 0;
			vec2NumMicroSteps.x = 0;
		}
	}
	else if (eDirection == RIGHT)
	{
		if (vec2Index.x >= (int)cSettings->NUM_TILES_XAXIS - 1)
		{
			vec2Index.x = ((int)cSettings->NUM_TILES_XAXIS) - 1;
			vec2NumMicroSteps.x = 0;
		}
	}
	else if (eDirection == UP)
	{
		if (vec2Index.y >= (int)cSettings->NUM_TILES_YAXIS - 1)
		{
			vec2Index.y = ((int)cSettings->NUM_TILES_YAXIS) - 1;
			vec2NumMicroSteps.y = 0;
		}
	}
	else if (eDirection == DOWN)
	{
		if (vec2Index.y < 0)
		{
			vec2Index.y = 0;
			vec2NumMicroSteps.y = 0;
		}
	}
	else
	{
		cout << "CPlayer2D::Constraint: Unknown direction." << endl;
	}

	if ((vec2Index.x <= 0)
		|| (vec2Index.x >= (int)cSettings->NUM_TILES_XAXIS - 1)
		|| (vec2Index.y >= (int)cSettings->NUM_TILES_YAXIS - 1)
		|| (vec2Index.y < 0))
		runtimeColour = glm::vec4(1.0, 0.0, 0.0, 1.0);
}

void CPlayer2D::SetTileValue() const
{
	for (unsigned col = 0; col < cSettings->NUM_TILES_XAXIS; col++)
	{
		int tileValue = cMap2D->GetMapInfo(vec2Index.y, col);
		if (tileValue <= 3)
		{
			cMap2D->SetMapInfo(vec2Index.y, vec2Index.x, tileValue);
			return;
		}
	}
}

void CPlayer2D::InteractWithMap(void)
{
	cInventoryItem = nullptr;

	switch (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x))
	{
	case 5:
		cMap2D->GetLiveMap()[make_pair(vec2Index.x, vec2Index.y)] = cMap2D->GetTime();
		cInventoryItem = cInventoryManager->GetItem("Player Live");
		break;

	case 10:
		cInventoryItem = cInventoryManager->GetItem("Key");
		break;

	case 15:
		cInventoryItem = cInventoryManager->GetItem("Speed");
		break;

	case 20:
		cInventoryItem = cInventoryManager->GetItem("Invisibility");
		break;

	case 25:
		if (m_equippedKey)
			CGameManager::GetInstance().bPlayerWon = true;
		break;

	case 60:
		cInventoryItem = cInventoryManager->GetItem("Spike");
		break;

	default:
		break;
	}

	if (cInventoryItem != nullptr)
	{
		if (cInventoryItem->iItemCount < cInventoryItem->iItemMaxCount)
		{
			SetTileValue();
			cInventoryItem->Add(1);
		}
	}
}

bool CPlayer2D::CheckPosition(DIRECTION eDirection)
{
	if (eDirection == LEFT)
	{
		// If the new position is fully within a row, then check this row only
		if (vec2NumMicroSteps.y == 0)
		{
			// If the grid is no accessible, then return false
			if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 100)
			{
				return false;
			}
		}
		// If the new position is between 2 rows, then check both rows as well
		else if (vec2NumMicroSteps.y != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 100)
				|| (cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x) >= 100))
			{
				return false;
			}
		}
	}
	else if (eDirection == RIGHT)
	{
		// If the new position is at the top row, then return true
		if (vec2Index.x >= cSettings->NUM_TILES_XAXIS - 1)
		{
			vec2NumMicroSteps.x = 0;
			return true;
		}

		// If the new posiiton is fully within a row, then check this row only
		if (vec2NumMicroSteps.y == 0)
		{
			if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + 1) >= 100)
			{
				return false;
			}
		}
		// If the new position is between 2 rows, then check both rows as well
		else if (vec2NumMicroSteps.y != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + 1) >= 100) ||
				(cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x + 1) >= 100))
			{
				return false;
			}
		}
	}
	else if (eDirection == UP)
	{
		// If the new position is at the top row, then return true
		if (vec2Index.y >= cSettings->NUM_TILES_YAXIS - 1)
		{
			vec2NumMicroSteps.y = 0;
			return true;
		}
		// If the new position is fully within a column, then check this column only
		if (vec2NumMicroSteps.x == 0)
		{
			// If the grid is not accessible, then return false
			if (cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x) >= 100)
			{
				return false;
			}
		}
		// If the new position is between 2 columns, then check both columns as well
		else if (vec2NumMicroSteps.x != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x) >= 100) ||
				(cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x + 1) >= 100))
			{
				return false;
			}
		}
	}
	else if (eDirection == DOWN)
	{
		// If the new position is fully within a column, then check this column only
		if (vec2NumMicroSteps.x == 0)
		{
			// If the grid is not accessible, then return false
			if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 100)
			{
				return false;
			}
		}
		// If the new position is between 2 columns, then check both columns as well
		else if (vec2NumMicroSteps.x != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 100) ||
				(cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + 1) >= 100))
			{
				return false;
			}
		}
	}
	else
	{
		cout << "CPlayer2D::CheckPosition: Unknown direction." << endl;
	}

	return true;
}

// Update Climb or Fall
void CPlayer2D::UpdateClimbFall(const double dElapsedTime)
{
	if (cPhysics2D.GetStatus() == CPhysics2D::STATUS::CLIMB)
	{
		if ((cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) != 36)
			&& (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) != 35)
			&& (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) != 30)
			&& (cMap2D->GetMapInfo(vec2Index.y - 1, vec2Index.x) != 30))  // We dont want to change the physics if the character is standing on top of a ladder
		{
			// Update the elapsed time to the physics engine
			cPhysics2D.SetTime((float)dElapsedTime);
			// Call the physics engine update method to calculate the final velocity and displacement
			cPhysics2D.Update();
			// Get the displacement from the physics engine
			glm::vec2 v2Displacement = cPhysics2D.GetDisplacement();

			// Store the current vec2Index.y
			int iIndex_YAxis_OLD = vec2Index.y;

			int iDisplacement_MicroSteps = (int)(v2Displacement.y / MICRO_STEP_YAXIS_PLAYER);
			if (vec2Index.y < (int)cSettings->NUM_TILES_YAXIS)
			{
				vec2NumMicroSteps.y += iDisplacement_MicroSteps;
				if (vec2NumMicroSteps.y > NUM_STEPS_PER_TILE_YAXIS_PLAYER)
				{
					vec2NumMicroSteps.y -= NUM_STEPS_PER_TILE_YAXIS_PLAYER;
					if (vec2NumMicroSteps.y < 0)
						vec2NumMicroSteps.y = 0;
					vec2Index.y++;
				}
			}

			// Constraint the player's position within the screen boundary
			Constraint(UP);

			// Iterate through all rows until the proposed row
			// Check if the player will hit a tile, stop jump if so.
			int iIndex_YAxis_Proposed = vec2Index.y;
			for (int i = iIndex_YAxis_OLD; i <= iIndex_YAxis_Proposed; i++)
			{
				// Change the player's index to the current i value
				vec2Index.y = i;
				// If the new position is not feasible, then revert to old position
				if (CheckPosition(UP) == false)
				{
					// Align with the row
					vec2NumMicroSteps.y = 0;
					// Set the Physics to fall status
					cPhysics2D.SetStatus(CPhysics2D::STATUS::FALL);
					break;
				}
			}

			// If the player is still jumping and the initial velocity has reached zero or below zeros,
			// then it has reach the peak of its jump
			if ((cPhysics2D.GetStatus() == CPhysics2D::STATUS::CLIMB) && (cPhysics2D.GetDisplacement().y <= 0.0f))
			{
				// Set status to fall
				cPhysics2D.SetStatus(CPhysics2D::STATUS::FALL);
			}
		}
	}
	else if (cPhysics2D.GetStatus() == CPhysics2D::STATUS::FALL)
	{
		if ((cMap2D->GetMapInfo(vec2Index.y - 1, vec2Index.x) != 30)
			&& (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) != 50)
			&& (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) != 45)
			&& (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) != 40))
		{
			// Update the elapsed time to the physics engine
			cPhysics2D.SetTime((float)dElapsedTime);
			// Call the physics engine update method to calculate the final velocity and displacement
			cPhysics2D.Update();
			// Get the displacement from the physics engine
			glm::vec2 v2Displacement = cPhysics2D.GetDisplacement();

			// Store the current vec2Index.y
			int iIndex_YAxis_OLD = vec2Index.y;

			// Translate the displacement from pixels to indices
			int iDisplaccement_MicroSteps = (int)(v2Displacement.y / MICRO_STEP_YAXIS_PLAYER);

			if (vec2Index.y >= 0)
			{
				vec2NumMicroSteps.y -= fabs(iDisplaccement_MicroSteps);
				if (vec2NumMicroSteps.y < 0)
				{
					vec2NumMicroSteps.y = (int)NUM_STEPS_PER_TILE_YAXIS_PLAYER - 1;
					vec2Index.y--;
				}
			}

			// Constraint the player's position within the screen boundary
			Constraint(DOWN);

			// Iterate through all rows until the proposed row
			// Check if th eplayer will hit a tile; stop fall is so.
			int iIndex_YAxis_Proposed = vec2Index.y;
			for (int i = iIndex_YAxis_OLD; i >= iIndex_YAxis_Proposed; i--)
			{
				// Change the player's index to the current i value
				vec2Index.y = i;
				// If the new position is not feasible, then revert to old position
				if (CheckPosition(DOWN) == false)
				{
					// Revert to the previous position
					if (i != iIndex_YAxis_OLD)
						vec2Index.y = i + 1;
					// Set the Physics to idle status
					cPhysics2D.SetStatus(CPhysics2D::STATUS::IDLE);
					vec2NumMicroSteps.y = 0;
					break;
				}
			}
		}
		else
		{
			vec2NumMicroSteps.y = 0;
		}
	}
}

// Check if the player is in mid-air
bool CPlayer2D::IsMidAir(void)
{
	// if the player is at the bottom row, then he is not in mid-air for sure
	if (vec2Index.y == 0)
		return false;

	// Check if the tile below the player's current position is empty
	if ((vec2NumMicroSteps.x == 0)
		&& (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) <= 3))
	{
		return true;
	}

	return false;
}

const int& CPlayer2D::GetLives() const
{
	return m_lives;
}

void CPlayer2D::AddLives(int liveNum)
{
	if (liveNum < 0)
	{
		static const double COOLDOWN_TIME = 3.0;
		static double currTime = 0.0;

		if (time < (currTime + COOLDOWN_TIME))
			return;
		else
			currTime = time;
	}

	m_lives += liveNum;
	if (m_lives > 5)
		m_lives = 5;
}

void CPlayer2D::SetEquippedKey(const bool& value)
{
	m_equippedKey = value;
}

const int& CPlayer2D::GetMicroSpeed() const
{
	return m_microSpeedBoost;
}

void CPlayer2D::AddMicroSpeed(int microSpeedBoost)
{
	m_microSpeedBoost += microSpeedBoost;
}

const bool& CPlayer2D::GetInvisibility() const
{
	return m_isInvisible;
}

void CPlayer2D::SetInvisibility(bool value)
{
	m_isInvisible = value;
}

bool CPlayer2D::PlaceSpike() const
{
	if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) > 3)
		return false;

	cMap2D->SetMapInfo(vec2Index.y, vec2Index.x, 55);
	return true;
}

const CPhysics2D::STATUS& CPlayer2D::GetPhysicsStatus() const
{
	return cPhysics2D.GetStatus();
}

CInventoryManager* CPlayer2D::GetInventoryManager() const
{
	return cInventoryManager;
}