/**
 Singleton template
 By: JH Chong
 Date: Aug 2022
 */
#pragma once

template <typename T>
class CSingletonTemplate
{
protected:
	CSingletonTemplate() {};
	virtual ~CSingletonTemplate() {};

public:
	// Get the Singleton instance
	static T& GetInstance()
	{
		static T instance;
		return instance;
	}

	// Disable cloning
	CSingletonTemplate(T& other) = delete;
	// Disable assigning
	void operator=(const T& other) = delete;
};