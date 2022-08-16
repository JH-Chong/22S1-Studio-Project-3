/**
 Application
 @brief This file contains the Application class for this project, which initialises and control the OpenGL environment
 By: Toh Da Jun
 Date: Mar 2020
 */
#pragma once

// Include SingletonTemplate
#include "Singleton\SingletonTemplate.h"

#include "TimeControl\StopWatch.h"
#include "Scene2D\Scene2D.h"

// FPS Counter
#include "TimeControl\FPSCounter.h"

// Add your include files here

struct GLFWwindow;

class CSettings;

class Application : public CSingletonTemplate<Application>
{
	friend CSingletonTemplate<Application>;
public:
	// Initialise this class instance
	bool Init(void);
	// Run this class instance
	void Run(void);
	// Destroy this class instance
	void Destroy(void);

	// Get window height
	int GetWindowHeight(void) const;
	// Get window width
	int GetWindowWidth(void) const;

	static void SetQuit(bool value);
	
protected:
	// Declare timer for calculating time per frame
	CStopWatch cStopWatch;

	// The handler to the CSettings instance
	CSettings* cSettings;
	// The handler to the CScene2D instance
	CScene2D* cScene2D;
	// The handler to the CFPSCounter instance
	CFPSCounter* cFPSCounter;

	static bool quit;

	// Constructor
	Application(void);

	// Destructor
	virtual ~Application(void);

	// Update input devices
	void UpdateInputDevices(void);
	void PostUpdateInputDevices(void);
};
