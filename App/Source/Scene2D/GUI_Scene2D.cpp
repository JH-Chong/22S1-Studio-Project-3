/**
 CGUI_Scene2D
 @brief A class which manages the GUI for Scene2D
 By: Toh Da Jun
 Date: May 2021
 */
#include "GUI_Scene2D.h"

#include "../Application.h"

#include <iostream>
using namespace std;

/**
 @brief Constructor This constructor has protected access modifier as this class will be a Singleton
 */
CGUI_Scene2D::CGUI_Scene2D(void)
	: cSettings(NULL)
	, window_flags(0)
	, cPlayerInventoryManager(NULL)
	, cInventoryItem(NULL)
{
}

/**
 @brief Destructor This destructor has protected access modifier as this class will be a Singleton
 */
CGUI_Scene2D::~CGUI_Scene2D(void)
{
	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	// We won't delete this since it was created elsewhere
	cSettings = NULL;
}

/**
  @brief Initialise this instance
  */
bool CGUI_Scene2D::Init(void)
{
	// Get the handler to the CSettings instance
	cSettings = &CSettings::GetInstance();

	// Store the CFPSCounter singleton instance here
	cFPSCounter = &CFPSCounter::GetInstance();

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(CSettings::GetInstance().pWindow, true);
	const char* glsl_version = "#version 330";
	ImGui_ImplOpenGL3_Init(glsl_version);

	// Define the window flags
	window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoBackground;
	window_flags |= ImGuiWindowFlags_NoTitleBar;
	window_flags |= ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoResize;
	window_flags |= ImGuiWindowFlags_NoCollapse;

	// Show the mouse pointer
	glfwSetInputMode(CSettings::GetInstance().pWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	// Player
	cPlayer2D = &CPlayer2D::GetInstance();

	// Initialise the cPlayerInventoryManager
	cPlayerInventoryManager = cPlayer2D->GetInventoryManager();
	cInventoryItem = cPlayerInventoryManager->Add("Player Live", "Image/Scene2D/Live.tga", 5, 0);
	cInventoryItem->vec2Size = glm::vec2(25, 25);
	cInventoryItem = cPlayerInventoryManager->Add("Key", "Image/Scene2D/Key.tga", 1, 0);
	cInventoryItem->vec2Size = glm::vec2(25, 25);
	cInventoryItem = cPlayerInventoryManager->Add("Speed", "Image/Scene2D/Speed.tga", 3, 0);
	cInventoryItem->vec2Size = glm::vec2(25, 25);
	cInventoryItem = cPlayerInventoryManager->Add("Invisibility", "Image/Scene2D/Invisibility.tga", 1, 0);
	cInventoryItem->vec2Size = glm::vec2(25, 25);
	cInventoryItem = cPlayerInventoryManager->Add("Spike", "Image/Scene2D/Spike.tga", 3, 0);
	cInventoryItem->vec2Size = glm::vec2(25, 25);

	for (unsigned i = 0; i < enemyVector->size(); ++i)
	{
		(*enemyVector)[i]->GetInventoryManager()->Add("Enemy " + std::to_string(i) + " Live", "Image/Scene2D/Live.tga", 1, 0);
	}

	// Game Manager
	cGameManager = &CGameManager::GetInstance();

	openInventory = false;

	return true;
}

/**
 @brief Update this instance
 */
void CGUI_Scene2D::Update(const double dElapsedTime, const double& time)
{
	this->time = time;

	// Calculate the relative scale to our default windows width
	const float relativeScale_x = cSettings->iWindowWidth / 800.0f;
	const float relativeScale_y = cSettings->iWindowHeight / 600.0f;

	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	if ((relativeScale_x == 0.0f) || (relativeScale_y == 0.0f))
		return;

	// Create an invisible window which covers the entire OpenGL window
	ImGui::Begin("Invisible window", NULL, window_flags);
	{
		ImGui::SetWindowPos(ImVec2(0.0f, 0.0f));
		ImGui::SetWindowSize(ImVec2((float)cSettings->iWindowWidth, (float)cSettings->iWindowHeight));
		ImGui::SetWindowFontScale(1.5f * relativeScale_y);

		if (!((cGameManager->bPlayerWon) || (cGameManager->bPlayerLost)))
		{
			// Display the FPS
			ImGui::TextColored(ImVec4(0.0f, 0.0f, 0.0f, 1.0f), "FPS: %d", cFPSCounter->GetFrameRate());

			ImGuiWindowFlags statsWinFlags =
				ImGuiWindowFlags_AlwaysAutoResize |
				ImGuiWindowFlags_NoBackground |
				ImGuiWindowFlags_NoTitleBar |
				ImGuiWindowFlags_NoMove |
				ImGuiWindowFlags_NoResize |
				ImGuiWindowFlags_NoCollapse |
				ImGuiWindowFlags_NoScrollbar |
				ImGuiWindowFlags_NoBackground;
			ImGui::Begin("Player Live", NULL, statsWinFlags);
			{
				ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth * 0.9f, 0.0f));
				ImGui::SetWindowSize(ImVec2(100.0f * relativeScale_x, 25.0f * relativeScale_y));
				ImGui::SetWindowFontScale(1.5f * relativeScale_y);
				cInventoryItem = cPlayerInventoryManager->GetItem("Player Live");
				ImGui::Image((void*)(intptr_t)cInventoryItem->GetTextureID(),
					ImVec2(cInventoryItem->vec2Size.x * relativeScale_x,
						cInventoryItem->vec2Size.y * relativeScale_y),
					ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
				ImGui::SameLine();
				ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%d", cPlayer2D->GetLives());
			}
			ImGui::End();
			ImGui::Begin("Speed", NULL, statsWinFlags);
			{
				ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth * 0.9f, cSettings->iWindowHeight * 0.05f));
				ImGui::SetWindowSize(ImVec2(100.0f * relativeScale_x, 25.0f * relativeScale_y));
				ImGui::SetWindowFontScale(1.5f * relativeScale_y);
				cInventoryItem = cPlayerInventoryManager->GetItem("Speed");
				ImGui::Image((void*)(intptr_t)cInventoryItem->GetTextureID(),
					ImVec2(cInventoryItem->vec2Size.x * relativeScale_x,
						cInventoryItem->vec2Size.y * relativeScale_y),
					ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
				ImGui::SameLine();
				ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "%d", cPlayer2D->GetMicroSpeed());
			}
			ImGui::End();
			if (cPlayer2D->GetInvisibility())
			{
				float floatTime = static_cast<float>(time);
				static int invisibleTimer = 5;
				static float currTime = floatTime;
				if (!openInventory)
				{
					if (floatTime >= (currTime + 1.0f))
					{
						--invisibleTimer;
						currTime = floatTime;
					}
				}
				ImGui::Begin("Invisibility", NULL, statsWinFlags);
				{
					ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth * 0.9f, cSettings->iWindowHeight * 0.1f));
					ImGui::SetWindowSize(ImVec2(100.0f * relativeScale_x, 25.0f * relativeScale_y));
					ImGui::SetWindowFontScale(1.5f * relativeScale_y);
					cInventoryItem = cPlayerInventoryManager->GetItem("Invisibility");
					ImGui::Image((void*)(intptr_t)cInventoryItem->GetTextureID(),
						ImVec2(cInventoryItem->vec2Size.x * relativeScale_x,
							cInventoryItem->vec2Size.y * relativeScale_y),
						ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
					ImGui::SameLine();
					ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%d", invisibleTimer);
				}
				ImGui::End();
				if (invisibleTimer < 0)
				{
					invisibleTimer = 5;
					currTime = 0.f;
					cPlayer2D->SetInvisibility(false);
					cPlayer2D->SetInvisibility(false);
				}
			}
			static char* buttonVal = "Use";
			static float currTime = 0.f;
			if (buttonVal == "Invalid!")
			{
				if (static_cast<float>(time) > (currTime + 1.5f))
					buttonVal = "Use";
			}

			ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
			{
				ImGuiWindowFlags inventoryButtonWinFlags =
					ImGuiWindowFlags_AlwaysAutoResize |
					ImGuiWindowFlags_NoTitleBar |
					ImGuiWindowFlags_NoMove |
					ImGuiWindowFlags_NoResize |
					ImGuiWindowFlags_NoCollapse |
					ImGuiWindowFlags_NoScrollbar;
				ImGui::Begin("Inventory Button", NULL, inventoryButtonWinFlags);
				{
					ImGui::SetWindowPos(ImVec2(0.0f, (float)cSettings->iWindowHeight * 0.55f));
					if (ImGui::Button("Inventory", ImVec2(100.0f, 50.0f)))
						openInventory = !openInventory;
				}
				ImGui::End();

				if (openInventory)
				{
					ImGuiWindowFlags inventoryBgWinFlags =
						ImGuiWindowFlags_AlwaysAutoResize |
						ImGuiWindowFlags_NoMove |
						ImGuiWindowFlags_NoResize |
						ImGuiWindowFlags_NoCollapse |
						ImGuiWindowFlags_NoScrollbar |
						ImGuiWindowFlags_NoTitleBar;
					ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.25f, (float)cSettings->iWindowHeight * 0.25f));
					ImGui::SetNextWindowSize(ImVec2(400.0f * relativeScale_x, 300.0f * relativeScale_y));
					ImGui::Begin("Inventory Background", NULL, inventoryBgWinFlags);
					{
						ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.25f, (float)cSettings->iWindowHeight * 0.25f));
						ImGui::SetNextWindowSize(ImVec2(400.0f * relativeScale_x, 25.0f * relativeScale_y));
						ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
						{
							ImGui::Begin("Inventory Title", NULL, inventoryButtonWinFlags);
							{
								ImGui::SameLine((float)cSettings->iWindowWidth * 0.2f);
								ImGui::Text("Inventory");
							}
							ImGui::End();;
						}
						ImGui::PopStyleColor();

						ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.71f, (float)cSettings->iWindowHeight * 0.24f));
						ImGui::Begin("Inv Close Button", NULL, inventoryButtonWinFlags | ImGuiWindowFlags_NoBackground);
						{
							ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
							{
								if (ImGui::Button("X", ImVec2(20.0f, 20.0f)))
									openInventory = false;
							}
							ImGui::PopStyleColor();
						}
						ImGui::End();

						ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.5f, 1.0f, 1.0f));
						{
							ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.2625f, (float)cSettings->iWindowHeight * 0.305f));
							ImGui::SetNextWindowSize(ImVec2(380.0f * relativeScale_x, 255.0f * relativeScale_y));
							ImGui::Begin("Inventory", NULL, inventoryBgWinFlags);
							{
								ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(1.0, 0.8f, 0.87f, 1.0f));
								{
									float widthMul = 0.27f, heightMul = 0.32f;
									for (unsigned i = 0; i < cPlayerInventoryManager->GetNumItems(); i++)
									{
										/*
										// Doesnt work for some reason //
										if (i == 0)
											RenderInventoryItems("Player Live", relativeScale_x, relativeScale_y, widthMul, heightMul);
										else if (i == 1)
											RenderInventoryItems("Key", relativeScale_x, relativeScale_y, widthMul, heightMul);
										else if (i == 2)
											RenderInventoryItems("Speed", relativeScale_x, relativeScale_y, widthMul, heightMul);
										else if (i == 3)
											RenderInventoryItems("Invisibility", relativeScale_x, relativeScale_y, widthMul, heightMul);
										*/

										if (i == 0)
										{
											if (cPlayerInventoryManager->Check("Player Live"))
											{
												cInventoryItem = cPlayerInventoryManager->GetItem("Player Live");
												for (unsigned j = 0; j < cInventoryItem->GetCount(); j++)
												{
													ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * widthMul, (float)cSettings->iWindowHeight * heightMul));


													ImGui::BeginChild("Player Live Child " + j, ImVec2(60.0f * relativeScale_x, 80.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
													{
														ImGui::BeginChild("Player Live Image " + j, ImVec2(60.0f * relativeScale_x, 60.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
														{
															ImGui::Image((void*)(intptr_t)cInventoryItem->GetTextureID(),
																ImVec2(60.0f * relativeScale_x,
																	60.0f * relativeScale_y),
																ImVec2(0, 1), ImVec2(1, 0));
														}
														ImGui::EndChild();

														ImGui::BeginChild("Player Live Function " + j, ImVec2(60.0f * relativeScale_x, 20.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
														{
															ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
															{
																if (ImGui::Button("Use", ImVec2(60.0f, 20.0f)))
																{
																	cInventoryItem->Remove(1);
																	cPlayer2D->AddLives(1);
																}
															}
															ImGui::PopStyleColor();
														}
														ImGui::EndChild();
													}
													ImGui::EndChild();


													widthMul += 0.0925f;
													if (widthMul > 0.64f)
													{
														widthMul = 0.27f;
														heightMul += 0.14f;
													}
												}
											}
										}
										else if (i == 1)
										{
											if (cPlayerInventoryManager->Check("Key"))
											{
												static bool keyEquipped = false;

												cInventoryItem = cPlayerInventoryManager->GetItem("Key");
												for (unsigned j = 0; j < cInventoryItem->GetCount(); j++)
												{
													ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * widthMul, (float)cSettings->iWindowHeight * heightMul));


													ImGui::BeginChild("Key Child " + j, ImVec2(60.0f * relativeScale_x, 80.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
													{
														ImGui::BeginChild("Key Image " + j, ImVec2(60.0f * relativeScale_x, 60.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
														{
															ImGui::Image((void*)(intptr_t)cInventoryItem->GetTextureID(),
																ImVec2(60.0f * relativeScale_x,
																	60.0f * relativeScale_y),
																ImVec2(0, 1), ImVec2(1, 0));
														}
														ImGui::EndChild();

														ImGui::BeginChild("Key Function " + j, ImVec2(60.0f * relativeScale_x, 20.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
														{
															ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
															{
																if (ImGui::Button(((keyEquipped == false) ? ("Equip") : ("Equipped")), ImVec2(60.0f, 20.0f)))
																{
																	keyEquipped = !keyEquipped;
																	cPlayer2D->SetEquippedKey(keyEquipped);
																}
															}
															ImGui::PopStyleColor();
														}
														ImGui::EndChild();
													}
													ImGui::EndChild();


													widthMul += 0.0925f;
													if (widthMul > 0.64f)
													{
														widthMul = 0.27f;
														heightMul += 0.14f;
													}
												}
											}
										}
										else if (i == 2)
										{
											if (cPlayerInventoryManager->Check("Speed"))
											{
												cInventoryItem = cPlayerInventoryManager->GetItem("Speed");
												for (unsigned j = 0; j < cInventoryItem->GetCount(); j++)
												{
													ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * widthMul, (float)cSettings->iWindowHeight * heightMul));


													ImGui::BeginChild("Speed Child " + j, ImVec2(60.0f * relativeScale_x, 80.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
													{
														ImGui::BeginChild("Speed Image " + j, ImVec2(60.0f * relativeScale_x, 60.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
														{
															ImGui::Image((void*)(intptr_t)cInventoryItem->GetTextureID(),
																ImVec2(60.0f * relativeScale_x,
																	60.0f * relativeScale_y),
																ImVec2(0, 1), ImVec2(1, 0));
														}
														ImGui::EndChild();

														ImGui::BeginChild("Speed Function " + j, ImVec2(60.0f * relativeScale_x, 20.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
														{
															ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
															{
																if (ImGui::Button("Use", ImVec2(60.0f, 20.0f)))
																{
																	cInventoryItem->Remove(1);
																	cPlayer2D->AddMicroSpeed(1);
																}
															}
															ImGui::PopStyleColor();
														}
														ImGui::EndChild();
													}
													ImGui::EndChild();


													widthMul += 0.0925f;
													if (widthMul > 0.64f)
													{
														widthMul = 0.27f;
														heightMul += 0.14f;
													}
												}
											}
										}
										else if (i == 3)
										{
											if (cPlayerInventoryManager->Check("Invisibility"))
											{
												cInventoryItem = cPlayerInventoryManager->GetItem("Invisibility");
												for (unsigned j = 0; j < cInventoryItem->GetCount(); j++)
												{
													ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * widthMul, (float)cSettings->iWindowHeight * heightMul));


													ImGui::BeginChild("Invisibility Child " + j, ImVec2(60.0f * relativeScale_x, 80.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
													{
														ImGui::BeginChild("Invisibility Image " + j, ImVec2(60.0f * relativeScale_x, 60.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
														{
															ImGui::Image((void*)(intptr_t)cInventoryItem->GetTextureID(),
																ImVec2(60.0f * relativeScale_x,
																	60.0f * relativeScale_y),
																ImVec2(0, 1), ImVec2(1, 0));
														}
														ImGui::EndChild();

														ImGui::BeginChild("Invisibility Function " + j, ImVec2(60.0f * relativeScale_x, 20.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
														{
															ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
															{
																if (ImGui::Button("Use", ImVec2(60.0f, 20.0f)))
																{
																	cInventoryItem->Remove(1);
																	cPlayer2D->SetInvisibility(true);
																}
															}
															ImGui::PopStyleColor();
														}
														ImGui::EndChild();
													}
													ImGui::EndChild();


													widthMul += 0.0925f;
													if (widthMul > 0.64f)
													{
														widthMul = 0.27f;
														heightMul += 0.14f;
													}
												}
											}
										}
										else if (i == 4)
										{
										if (cPlayerInventoryManager->Check("Spike"))
										{
											cInventoryItem = cPlayerInventoryManager->GetItem("Spike");
											for (unsigned j = 0; j < cInventoryItem->GetCount(); j++)
											{
												ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * widthMul, (float)cSettings->iWindowHeight * heightMul));


												ImGui::BeginChild("Spike Child " + j, ImVec2(60.0f * relativeScale_x, 80.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
												{
													ImGui::BeginChild("Spike Image " + j, ImVec2(60.0f * relativeScale_x, 60.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
													{
														ImGui::Image((void*)(intptr_t)cInventoryItem->GetTextureID(),
															ImVec2(60.0f * relativeScale_x,
																60.0f * relativeScale_y),
															ImVec2(0, 1), ImVec2(1, 0));
													}
													ImGui::EndChild();

													ImGui::BeginChild("Spike Function " + j, ImVec2(60.0f * relativeScale_x, 20.0f * relativeScale_y), false, ImGuiWindowFlags_NoScrollbar);
													{
														ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
														{
															if (ImGui::Button(buttonVal, ImVec2(60.0f, 20.0f)))
															{
																currTime = static_cast<float>(time);
																if (cPlayer2D->PlaceSpike())
																	cInventoryItem->Remove(1);
																else
																	buttonVal = "Invalid!";
															}
														}
														ImGui::PopStyleColor();
													}
													ImGui::EndChild();
												}
												ImGui::EndChild();


												widthMul += 0.0925f;
												if (widthMul > 0.64f)
												{
													widthMul = 0.27f;
													heightMul += 0.14f;
												}
											}
										}
										}
									}
								}
								ImGui::PopStyleColor();
							}
							ImGui::End();
						}
						ImGui::PopStyleColor();
					}
					ImGui::End();
				}
			}
			ImGui::PopStyleColor();
		}
		else
		{
			ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
			{
				ImGuiWindowFlags endScreenWinFlags =
					ImGuiWindowFlags_AlwaysAutoResize |
					ImGuiWindowFlags_NoTitleBar |
					ImGuiWindowFlags_NoMove |
					ImGuiWindowFlags_NoResize |
					ImGuiWindowFlags_NoCollapse |
					ImGuiWindowFlags_NoScrollbar;
				ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
				ImGui::SetNextWindowSize(ImVec2((float)cSettings->iWindowWidth, (float)cSettings->iWindowHeight));
				ImGui::Begin("End Screen", NULL, endScreenWinFlags);
				{
					float widthMul = (cGameManager->bPlayerWon) ? (0.45f) : (0.425f);
					ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * widthMul, (float)cSettings->iWindowHeight * 0.45f));
					ImGui::BeginChild("End Screen Text", ImVec2(200.0f, 25.0f));
					{
						ImGui::SetWindowFontScale(1.5f * relativeScale_y);
						if (cGameManager->bPlayerWon)
							ImGui::Text("You Won!");
						else
							ImGui::Text("You Lost :(");
					}
					ImGui::EndChild();
				}
				ImGui::End();

				ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
				{
					ImGui::SetNextWindowPos(ImVec2((float)cSettings->iWindowWidth * 0.425f, (float)cSettings->iWindowHeight * 0.5f));
					ImGui::Begin("Confirm Button", NULL, endScreenWinFlags);
					{
						ImGui::SetWindowFontScale(1.5f * relativeScale_y);
						if (ImGui::Button("Confirm", ImVec2(100.0f, 25.0f)))
							Application::SetQuit(true);
					}
					ImGui::End();
				}
				ImGui::PopStyleColor();
			}
			ImGui::PopStyleColor();
		}
	}
	ImGui::End();


	ImGui::EndFrame();
}


/**
 @brief Set up the OpenGL display environment before rendering
 */
void CGUI_Scene2D::PreRender(void)
{
}

/**
 @brief Render this instance
 */
void CGUI_Scene2D::Render(void)
{
	// Rendering
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

/**
 @brief PostRender Set up the OpenGL display environment after rendering.
 */
void CGUI_Scene2D::PostRender(void)
{
}

const bool& CGUI_Scene2D::InventoryOpen() const
{
	return openInventory;
}

void CGUI_Scene2D::SetEnemyVector(std::vector<CEnemy2D*>& cEnemyVector)
{
	enemyVector = &cEnemyVector;
}
