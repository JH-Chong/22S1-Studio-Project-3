/**
 CEnemy2D
 @brief A class which represents the enemy object
 By: Toh Da Jun
 Date: Mar 2020
 */
#include "Enemy2D.h"

#include <iostream>
using namespace std;

// Include Shader Manager
#include "RenderControl\ShaderManager.h"
// Include Mesh Builder
#include "Primitives/MeshBuilder.h"

// Include GLEW
#include <GL/glew.h>

// Include ImageLoader
#include "System\ImageLoader.h"

// Include the Map2D as we will use it to check the player's movements and actions
#include "Map2D.h"
// Include math.h
#include <math.h>

/**
 @brief Constructor This constructor has protected access modifier as this class will be a Singleton
 */
CEnemy2D::CEnemy2D(void)
	: isKnockedOut(false)
	, cMap2D(NULL)
	, cSettings(NULL)
	, cPlayer2D(NULL)
	, sCurrentFSM(FSM::ATTACK)
	, iFSMCounter(0)
	// , quadMesh(NULL)
{
	transform = glm::mat4(1.0f);	// make sure to initialize matrix to identity matrix first

	// Initialise vecIndex
	vec2Index = glm::i32vec2(0);

	// Initialise vecNumMicroSteps
	i32vec2NumMicroSteps = glm::i32vec2(0);

	// Initialise vec2UVCoordinate
	vec2UVCoordinate = glm::vec2(0.0f);

	vec2Destination = glm::i32vec2(0, 0);	// Initialise the iDestination
	vec2Direction = glm::i32vec2(0, 0);		// Initialise the iDirection
}

/**
 @brief Destructor This destructor has protected access modifier as this class will be a Singleton
 */
CEnemy2D::~CEnemy2D(void)
{
	// Delete the quadMesh
	/*
	if (quadMesh)
	{
		delete quadMesh;
		quadMesh = NULL;
	}
	*/

	// We won't delete this since it was created elsewhere
	cPlayer2D = NULL;

	// We won't delete this since it was created elsewhere
	cMap2D = NULL;

	// optional: de-allocate all resources once they've outlived their purpose:
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}

/**
  @brief Initialise this instance
  */
bool CEnemy2D::Init(void)
{
	m_lives = 2;

	// Get the handler to the CSettings instance
	cSettings = &CSettings::GetInstance();

	NUM_STEPS_PER_TILE_XAXIS_ENEMY = (int)cSettings->NUM_STEPS_PER_TILE_XAXIS + 2;
	NUM_STEPS_PER_TILE_YAXIS_ENEMY = (int)cSettings->NUM_STEPS_PER_TILE_YAXIS + 2;

	MICRO_STEP_XAXIS_ENEMY = cSettings->TILE_WIDTH / NUM_STEPS_PER_TILE_XAXIS_ENEMY;
	MICRO_STEP_YAXIS_ENEMY = cSettings->TILE_HEIGHT / NUM_STEPS_PER_TILE_YAXIS_ENEMY;

	// Get the handler to the CMap2D instance
	cMap2D = &CMap2D::GetInstance();
	// Find the indices for the player in arrMapInfo, and assign it to cPlayer2D
	unsigned int uiRow = -1;
	unsigned int uiCol = -1;
	if (cMap2D->FindValue(300, uiRow, uiCol) == false)
		return false;	// Unable to find the start position of the player, so quit this game

	// Erase the value of the enemy in the arrMapInfo
	for (unsigned col = 0; col < cSettings->NUM_TILES_XAXIS; col++)
	{
		if (cMap2D->GetMapInfo(uiRow, col) <= 3)
		{
			cMap2D->SetMapInfo(uiRow, uiCol, cMap2D->GetMapInfo(uiRow, col));
			break;
		}
	}

	cPlayer2D = &CPlayer2D::GetInstance();
	playerVec2OldIndex = cPlayer2D->vec2Index;

	// Set the start position of the Player to iRow and iCol
	vec2Index = glm::i32vec2(uiCol, uiRow);
	// By default, microsteps should be zero
	i32vec2NumMicroSteps = glm::i32vec2(0, 0);

	nearestLive = glm::vec2(1000, 1000);

	supportPos = glm::vec2(-1, -1);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	//CS: Create the Quad Mesh using the mesh builder
	// quadMesh = CMeshBuilder::GenerateQuad(glm::vec4(1, 1, 1, 1), cSettings->TILE_WIDTH, cSettings->TILE_HEIGHT);

	// Load the enemy2D texture
	iTextureID = CImageLoader::GetInstance().LoadTextureGetID("Image/Scene2D/Enemy.png", true);
	if (iTextureID == 0)
	{
		std::cout << "Unable to load Image/Scene2D/Enemy.png" << std::endl;
		return false;
	}

	//CS: Create the animated sprite and setup the animation
	animatedSprites = CMeshBuilder::GenerateSpriteAnimation(2, 2, cSettings->TILE_WIDTH, cSettings->TILE_HEIGHT);
	animatedSprites->AddAnimation("left", 0, 1);
	//CS: Play the "idle" animation as default
	animatedSprites->PlayAnimation("left", -1, 1.0f);

	//CS: Init the color to white
	runtimeColour = glm::vec4(1.0, 1.0, 1.0, 1.0);

	cInventoryManager = new CInventoryManager();

	// If this class is initialised properly, then set the isKnockedOut to false
	isKnockedOut = false;
	gotSupport = false;

	return true;
}

float Length(glm::vec2 index)
{
	return sqrt((index.x * index.x) + (index.y * index.y));
}

/**
 @brief Update this instance
 */
void CEnemy2D::Update(const double dElapsedTime)
{
	switch (sCurrentFSM)
	{
	case ATTACK:
	{
		// A little bit of optimization
		if (cPlayer2D->vec2Index != playerVec2OldIndex)
		{
			path = cMap2D->PathFind(vec2Index,
									cPlayer2D->vec2Index,
									heuristic::euclidean,
									10);
		}

		UpdatePosition();

		InteractWithPlayer();

		InteractWithMap();

		if (m_lives == 1)
			sCurrentFSM = FSM::FLEE;

		break;
	}
	case FLEE:
	{
		static bool nearestLiveFound = false;

		if (!nearestLiveFound)
		{
			FindNearestLive();

			if (nearestLive != glm::vec2(1000, 1000))
			{
				nearestLiveFound = true;
				path = cMap2D->PathFind(vec2Index,
										nearestLive,
										heuristic::euclidean,
										10);
			}
		}

		UpdatePosition();

		InteractWithPlayer();

		InteractWithMap();

		if ((m_lives == 2) || (m_lives <= 0))
		{
			nearestLiveFound = false;
			nearestLive = glm::vec2(1000, 1000);
			if (m_lives == 2)
			{
				sCurrentFSM = FSM::ATTACK;
			}
			else
			{
				isKnockedOut = true;
				gotSupport = false;
				sCurrentFSM = FSM::KNOCKOUT;
			}
		}

		break;
	}
	case KNOCKOUT:
	{
		runtimeColour = glm::vec4(1.0, 0.0, 0.0, 1.0);

		if (m_lives == 1)
		{
			runtimeColour = glm::vec4(1.0, 1.0, 1.0, 1.0);
			sCurrentFSM = FSM::FLEE;
		}

		break;
	}
	case SUPPORT:
	{
		// Codes heavily NOT optimized (and its very bad)

		static int stage = 1;

		// Find nearest Live
		if (stage == 1)
		{
			FindNearestLive();
			path = cMap2D->PathFind(vec2Index,
									nearestLive,
									heuristic::euclidean,
									10);
			++stage;
		}
		// Move to nearest Live
		else if (stage == 2)
		{
			UpdatePosition();

			InteractWithPlayer();

			InteractWithMap();

			if (vec2Index == nearestLive)
				++stage;
		}
		// Build path to enemy that needs support
		else if (stage == 3)
		{
			path = cMap2D->PathFind(vec2Index,
									supportPos,
									heuristic::euclidean,
									10);
			++stage;
		}
		// Move to support pos
		else if (stage == 4)
		{
			UpdatePosition();

			InteractWithPlayer();

			InteractWithMap();

			if (vec2Index == supportPos)
				++stage;
		}
		// Make changes to current enemy and enemy that needs support
		else // if (stage == 5)
		{
			// Remove a Live from the current enemy's inventory
			cInventoryManager->GetItem("Enemy " + std::to_string(enemyNum) + " Live")->Remove(1);
			// Make changes to the enemy that needs support
			for (auto iter = enemyVector->begin(); iter != enemyVector->end(); ++iter)
			{
				if ((*iter)->vec2Index == supportPos)
				{
					(*iter)->AddLives(1);
					(*iter)->isKnockedOut = false;
					(*iter)->gotSupport = false;
					(*iter)->runtimeColour = glm::vec4(1.0, 1.0, 1.0, 1.0);
					(*iter)->SetFSM("Flee");
					break;
				}
			}
			sCurrentFSM = FSM::ATTACK;
			stage = 1;
			nearestLive = glm::vec2(1000, 1000);
			supportPos = glm::vec2(-1, -1);
		}

		if ((m_lives == 1) || (m_lives <= 0))
		{
			stage = 1;
			nearestLive = glm::vec2(1000, 1000);
			supportPos = glm::vec2(-1, -1);
			if (m_lives == 1)
				sCurrentFSM = FSM::FLEE;
			else // if (m_lives <= 0)
				sCurrentFSM = FSM::KNOCKOUT;
		}

		break;
	}
	default:
		break;
	}

	/*
	case IDLE:
		if (iFSMCounter > iMaxFSMCounter)
		{
			sCurrentFSM = PATROL;
			iFSMCounter = 0;
			// cout << "Switching to Patrol State" << endl;
		}
		iFSMCounter++;
		break;
	case PATROL:
		if (iFSMCounter > iMaxFSMCounter)
		{
			sCurrentFSM = IDLE;
			iFSMCounter = 0;
			// cout << "Switching to Idle State" << endl;
		}
		else if (cPhysics2D.CalculateDistance(vec2Index, cPlayer2D->vec2Index) < 5.0f)
		{
			sCurrentFSM = ATTACK;
			iFSMCounter = 0;
		}
		else
		{
			// Patrol around
			// Update the Enemy2D's position for patrol
			UpdatePosition();
		}
		iFSMCounter++;
		break;
	case ATTACK:
		if (cPhysics2D.CalculateDistance(vec2Index, cPlayer2D->vec2Index) < 5.0f)
		{
			auto path = cMap2D->PathFind(	vec2Index,
											cPlayer2D->vec2Index,
											heuristic::euclidean,
											10);

			// Calculate new destination
			bool bFirstPosition = true;
			for (const auto& coord : path)
			{
				//std::cout << coord.x << ", " << coord.y << "\n";
				if (bFirstPosition == true)
				{
					// Set a destination
					vec2Destination = coord;
					// Calculate the direction between enemy2D and this destination
					vec2Direction = vec2Destination - vec2Index;
					bFirstPosition = false;
				}
				else
				{
					if ((coord - vec2Destination) == vec2Direction)
					{
						// Set a destination
						vec2Destination = coord;
					}
					else
						break;
				}
			}

			// Update the Enemy2D's position for attack
			UpdatePosition();
		}
		else
		{
			if (iFSMCounter > iMaxFSMCounter)
			{
				sCurrentFSM = PATROL;
				iFSMCounter = 0;
				// cout << "ATTACK : Reset counter: " << iFSMCounter << endl;
			}
			iFSMCounter++;
		}
		break;
	default:
		break;
	*/

	//CS: Update the animated sprite
	animatedSprites->Update(dElapsedTime);

	// Update the UV Coordinates
	vec2UVCoordinate.x = cSettings->ConvertIndexToUVSpace(cSettings->x, vec2Index.x, false, i32vec2NumMicroSteps.x*MICRO_STEP_XAXIS_ENEMY);
	vec2UVCoordinate.y = cSettings->ConvertIndexToUVSpace(cSettings->y, vec2Index.y, false, i32vec2NumMicroSteps.y*MICRO_STEP_YAXIS_ENEMY);

	playerVec2OldIndex = cPlayer2D->vec2Index;
}

/**
 @brief Set up the OpenGL display environment before rendering
 */
void CEnemy2D::PreRender(void)
{
	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);

	// Activate blending mode
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Activate the shader
	CShaderManager::GetInstance().Use(sShaderName);
}

/**
 @brief Render this instance
 */
void CEnemy2D::Render(void)
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
void CEnemy2D::PostRender(void)
{
	// Disable blending
	glDisable(GL_BLEND);
}

/**
@brief Set the indices of the enemy2D
@param iIndex_XAxis A const int variable which stores the index in the x-axis
@param iIndex_YAxis A const int variable which stores the index in the y-axis
*/
void CEnemy2D::Seti32vec2Index(const int iIndex_XAxis, const int iIndex_YAxis)
{
	this->vec2Index.x = iIndex_XAxis;
	this->vec2Index.y = iIndex_YAxis;
}

/**
@brief Set the number of microsteps of the enemy2D
@param iNumMicroSteps_XAxis A const int variable storing the current microsteps in the X-axis
@param iNumMicroSteps_YAxis A const int variable storing the current microsteps in the Y-axis
*/
void CEnemy2D::Seti32vec2NumMicroSteps(const int iNumMicroSteps_XAxis, const int iNumMicroSteps_YAxis)
{
	this->i32vec2NumMicroSteps.x = iNumMicroSteps_XAxis;
	this->i32vec2NumMicroSteps.y = iNumMicroSteps_YAxis;
}

void CEnemy2D::SetTileValue() const
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

/**
 @brief Let enemy2D interact with the player.
 */
bool CEnemy2D::InteractWithPlayer(void)
{
	glm::i32vec2 i32vec2PlayerPos = cPlayer2D->vec2Index;
	
	// Check if the enemy2D is within 1.5 indices of the player2D
	if (((vec2Index.x >= i32vec2PlayerPos.x - 0.5) && 
		(vec2Index.x <= i32vec2PlayerPos.x + 0.5))
		&& 
		((vec2Index.y >= i32vec2PlayerPos.y - 0.5) &&
		(vec2Index.y <= i32vec2PlayerPos.y + 0.5)))
	{
		if (cPlayer2D->GetInvisibility())
			return false;
		cPlayer2D->AddLives(-1);
		return true;
	}

	return false;
}

/**
 @brief Let enemy2D interact with the map.
 */
void CEnemy2D::InteractWithMap(void)
{
	cInventoryItem = nullptr;

	switch (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x))
	{
	case 5:
		cMap2D->GetLiveMap()[make_pair(vec2Index.x, vec2Index.y)] = cMap2D->GetTime();
		if (sCurrentFSM == FSM::FLEE)
		{
			SetTileValue();
			++m_lives;
		}
		else if (sCurrentFSM == FSM::SUPPORT)
		{
			cInventoryItem = cInventoryManager->GetItem("Enemy " + std::to_string(enemyNum) + " Live");
		}
		break;

	case 55:
		SetTileValue();
		--m_lives;
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

/**
@brief Update position.
*/
void CEnemy2D::UpdatePosition(void)
{
	if (path.size() > 0)
	{
		glm::vec2 dir = (path[0] - vec2Index);

		if (dir.x < 0)
		{
			if (vec2Index.x >= 0)
			{
				i32vec2NumMicroSteps.x--;
				if (i32vec2NumMicroSteps.x < 0)
				{
					i32vec2NumMicroSteps.x = (int)NUM_STEPS_PER_TILE_XAXIS_ENEMY;
					vec2Index.x--;
				}
			}
		}
		else if (dir.x > 0)
		{
			if (vec2Index.x < (int)cSettings->NUM_TILES_XAXIS)
			{
				i32vec2NumMicroSteps.x++;
				if (i32vec2NumMicroSteps.x >= (int)NUM_STEPS_PER_TILE_XAXIS_ENEMY)
				{
					i32vec2NumMicroSteps.x = 0;
					vec2Index.x++;
				}
			}
		}

		if (dir.y < 0)
		{
			if (vec2Index.y >= 0)
			{
				i32vec2NumMicroSteps.y--;
				if (i32vec2NumMicroSteps.y < 0)
				{
					i32vec2NumMicroSteps.y = (int)NUM_STEPS_PER_TILE_YAXIS_ENEMY;
					vec2Index.y--;
				}
			}
		}
		else if (dir.y > 0)
		{
			if (vec2Index.y < (int)cSettings->NUM_TILES_YAXIS)
			{
				i32vec2NumMicroSteps.y++;
				if (i32vec2NumMicroSteps.y >= (int)NUM_STEPS_PER_TILE_YAXIS_ENEMY)
				{
					i32vec2NumMicroSteps.y = 0;
					vec2Index.y++;
				}
			}
		}

		if ((round(vec2Index.y) == round(path[0].y)) &&
			(round(vec2Index.x) == round(path[0].x)))
		{
			if ((cMap2D->GetMapInfo(vec2Index.y - 1, vec2Index.x) >= 100) &&
				(cMap2D->GetMapInfo(vec2Index.y - 1, vec2Index.x) <= 120))
			{
				i32vec2NumMicroSteps.y--;
				if (i32vec2NumMicroSteps.y < 0)
				{
					i32vec2NumMicroSteps.y = 0;
					path.erase(path.begin());
				}
			}
			else if ((cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 30) &&
					 (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) <= 36))
			{
				i32vec2NumMicroSteps.x--;
				if (i32vec2NumMicroSteps.x < 0)
				{
					i32vec2NumMicroSteps.x = 0;
					path.erase(path.begin());
				}
			}
			else
			{
				path.erase(path.begin());
			}
		}
	}
}

CInventoryManager* CEnemy2D::GetInventoryManager() const
{
	return cInventoryManager;
}

const int& CEnemy2D::GetLives() const
{
	return m_lives;
}

void CEnemy2D::AddLives(int liveNum)
{
	m_lives += liveNum;
	if (m_lives > 2)
		m_lives = 2;
}

void CEnemy2D::SetEnemyNum(int num)
{
	enemyNum = num;
}

void CEnemy2D::FindNearestLive()
{
	for (auto iter = cMap2D->GetLiveMap().begin(); iter != cMap2D->GetLiveMap().end(); ++iter)
	{
		// If the current live is available
		if (iter->second == -10.0)
		{
			// Flip y to change to screen coords
			glm::vec2 currIndex = glm::vec2(iter->first.first, (int)cSettings->NUM_TILES_YAXIS - iter->first.second - 1);
			if (glm::length(currIndex - vec2Index) < glm::length(nearestLive - vec2Index))
			{
				nearestLive = currIndex;
			}
		}
	}
}

void CEnemy2D::SetFSM(char* fsm, glm::vec2 supportPos)
{
	if (fsm == "Attack")
	{
		sCurrentFSM = FSM::ATTACK;
	}
	else if (fsm == "Flee")
	{
		sCurrentFSM = FSM::FLEE;
	}
	else if (fsm == "KnockOut")
	{
		isKnockedOut = true;
		sCurrentFSM = FSM::KNOCKOUT;
	}
	else // if (fsm == "Support")
	{
		sCurrentFSM = FSM::SUPPORT;
	}

	this->supportPos = supportPos;
}

void CEnemy2D::SetEnemyVector(std::vector<CEnemy2D*>& cEnemyVector)
{
	enemyVector = &cEnemyVector;
}