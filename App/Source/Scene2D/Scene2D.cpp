/**
 CScene2D
 @brief A class which manages the 2D game scene
 By: Toh Da Jun
 Date: Mar 2020
 */
#include "Scene2D.h"
#include <iostream>
using namespace std;

// Include Shader Manager
#include "RenderControl\ShaderManager.h"

#include "System\filesystem.h"

/**
 @brief Constructor This constructor has protected access modifier as this class will be a Singleton
 */
CScene2D::CScene2D(void)
	: cMap2D(NULL)
	, cPlayer2D(NULL)
	, cKeyboardController(NULL)
	, cGUI_Scene2D(NULL)
	, cGameManager(NULL)
	, cSoundController(NULL)
{
}

/**
 @brief Destructor
 */
CScene2D::~CScene2D(void)
{
	if (cSoundController)
	{
		// We don't delete this since it was created elsewhere
		cSoundController = NULL;
	}

	if (cKeyboardController)
	{
		// We won't delete this since it was created elsewhere
		cKeyboardController = NULL;
	}

	// Destroy the enemies
	for (auto iter = enemyVector.begin(); iter != enemyVector.end(); ++iter)
	{
		delete (*iter);
		(*iter) = NULL;
	}
	enemyVector.clear();
}

/**
@brief Init Initialise this instance
*/ 
bool CScene2D::Init(void)
{
	CScene2D::time = 0.0;

	// Include Shader Manager
	CShaderManager::GetInstance().Use("Shader2D");
	
	// Load the sounds into CSoundController
	cSoundController = &CSoundController::GetInstance();
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\Sound_Bell.ogg"), 1, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\Sound_Explosion.ogg"), 2, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\Sound_Jump.ogg"), 3, true);

	cKeyboardController = &CKeyboardController::GetInstance();

	// Game Manager
	cGameManager = &CGameManager::GetInstance();
	cGameManager->Init();

	// Create and initialise the Map 2D
	cMap2D = &CMap2D::GetInstance();
	// Set a shader to this class
	cMap2D->SetShader("Shader2D");
	// Initialise the insatnce
	if (cMap2D->Init(1, 24, 32) == false)
	// if (cMap2D->Init(2, CSettings::GetInstance().NUM_TILES_YAXIS,
	//						CSettings::GetInstance().NUM_TILES_XAXIS) == false)
	{
		cout << "Failed to load CMap2D" << endl;
		return false;
	}

	if (cMap2D->LoadMap("Maps/Map_1.csv") == false)
	{
		std::cout << "Failed to load Map_1.csv" << std::endl;
		return false;
	}

	// Load Scene2DColour into ShaderManager
	CShaderManager::GetInstance().Use("Shader2D_Colour");
	// Create and initialise the CPlayer2D
	cPlayer2D = &CPlayer2D::GetInstance();
	// Pass shader to cPlayer2D
	cPlayer2D->SetShader("Shader2D_Colour");
	// Initialise the instance
	if (cPlayer2D->Init() == false)
	{
		cout << "Failed to load CPlayer2D" << endl;
		return false;
	}

	// Create and initialise the CEnemy2D
	enemyVector.clear();
	while (true)
	{
		CEnemy2D* cEnemy2D = new CEnemy2D();
		// Pass shader to cEnemy2D
		cEnemy2D->SetShader("Shader2D_Colour");
		// Initialise the instance
		if (cEnemy2D->Init() == true)
		{
			cEnemy2D->SetEnemyNum((int)enemyVector.size());
			enemyVector.push_back(cEnemy2D);
		}
		else
		{
			// Break out of this loop if the enemy has all been loaded
			break;
		}
	}

	for (auto iter = enemyVector.begin(); iter != enemyVector.end(); ++iter)
		(*iter)->SetEnemyVector(enemyVector);

	//enemyVector[0]->SetFSM("KnockOut");

	cGUI_Scene2D = &CGUI_Scene2D::GetInstance();
	cGUI_Scene2D->SetEnemyVector(enemyVector);
	if (cGUI_Scene2D->Init() == false)
	{
		cout << "Failed to load CGUI_Scene2D" << endl;
		return false;
	}

	return true;
}

/**
@brief Update Update this instance
*/
bool CScene2D::Update(const double dElapsedTime)
{
	time += dElapsedTime;

	if (!cGUI_Scene2D->InventoryOpen())
	{
		// Call the Map2D's update method
		cMap2D->Update(dElapsedTime, time);

		// Call the cPlayer2D's update method before Map2D
		// as we want to capture the inputs before map2D update
		cPlayer2D->Update(dElapsedTime, time);

		// Call all the cEnemy2D's update method before Map2D
		// as we want to capture the updates before map2D update
		for (auto iter = enemyVector.begin(); iter != enemyVector.end(); ++iter)
		{
			(*iter)->Update(dElapsedTime);
			if (((*iter)->isKnockedOut) && (!(*iter)->gotSupport))
			{
				for (auto iter2 = enemyVector.begin(); iter2 != enemyVector.end(); ++iter2)
				{
					if (!(*iter2)->isKnockedOut)
					{
						(*iter)->gotSupport = true;
						(*iter2)->SetFSM("Support", (*iter)->vec2Index);
						break;
					}
				}
			}
		}
	}

	// Call the cGUI_Scene2D's update method
	cGUI_Scene2D->Update(dElapsedTime, time);

	if (cPlayer2D->GetLives() <= 0)
		cGameManager->bPlayerLost = true;

	/*if (cGameManager->bLevelCompleted == true)
	{
		cMap2D->SetCurrentLevel(cMap2D->GetCurrentLevel() + 1);
		cPlayer2D->Reset();
		cGameManager->bLevelCompleted = false;
	}*/

	/*
	if (cKeyboardController->IsKeyDown(GLFW_KEY_F4))
	{
		cMap2D->SetCurrentLevel(0);
		if (cMap2D->LoadMap("Maps/DM2213_Map_Level_01.csv") == false)
			return false;
		if (cPlayer2D->Reset() == false)
			return false;
	}

	if (cKeyboardController->IsKeyDown(GLFW_KEY_F5))
	{
		cMap2D->SetCurrentLevel(1);
		if (cMap2D->LoadMap("Maps/DM2213_Map_Level_02.csv", 1) == false)
			return false;
		if (cPlayer2D->Reset() == false)
			return false;
	}
	*/
	
	// Get keyboard updates
	/*
	if (cKeyboardController->IsKeyDown(GLFW_KEY_F6))
	{
		// Save the current game to a save file
		// Make sure the file is open
		try
		{
			if (cMap2D->SaveMap("Maps/DM2213_Map_Level_01_SAVEGAME.csv") == false)
				throw runtime_error("Unable to save the current game to a file");
		}
		catch (runtime_error e)
		{
			cout << "Runtime error: " << e.what();
			return false;
		}
	}
	*/

	// Check if the game should go to the next level
	if (cGameManager->bLevelCompleted == true)
	{
		cMap2D->SetCurrentLevel(cMap2D->GetCurrentLevel() + 1);
		cGameManager->bLevelCompleted = false;
	}


	// Check if the game has been won by the player
	if (cGameManager->bPlayerWon == true)
	{
		// End the game and switch to Win screen

	}
	// Check if the game should be ended
	else if (cGameManager->bPlayerLost == true)
	{
		cSoundController->PlaySoundByID(2);
		return false;
	}

	return true;
}

/**
 @brief PreRender Set up the OpenGL display environment before rendering
 */
void CScene2D::PreRender(void)
{
	// Reset the OpenGL rendering environment
	glLoadIdentity();

	// Clear the screen and buffer
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// Enable 2D texture rendering
	glEnable(GL_TEXTURE_2D);
}

/**
 @brief Render Render this instance
 */
void CScene2D::Render(void)
{
	// Call the Map2D's PreRender()
	cMap2D->PreRender(); // [If don't call, will not be able to use shader and render things]
	// Call the Map2D's Render()
	cMap2D->Render();
	// Call the Map2D's PostRender()
	cMap2D->PostRender(); // [To unset some OpenGL settings called in CMap2D::PreRender() method]

	for (auto iter = enemyVector.begin(); iter != enemyVector.end(); ++iter)
	{
		// Call the CEnemy2D's PreRender()
		(*iter)->PreRender();
		// Call the CEnemy2D's Render()
		(*iter)->Render();
		// Call the CEnemy2D's PostRender()
		(*iter)->PostRender();
	}

	// Call the CPlayer2D's PreRender()
	cPlayer2D->PreRender();
	// Call the CPlayer2D's Render()
	cPlayer2D->Render();
	// Call the CPlayer2D's PostRender()
	cPlayer2D->PostRender();

	cGUI_Scene2D->PreRender();
	cGUI_Scene2D->Render();
	cGUI_Scene2D->PostRender();
}

/**
 @brief PostRender Set up the OpenGL display environment after rendering.
 */
void CScene2D::PostRender(void)
{
}