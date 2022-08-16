/**
 Factory template
 By: JH Chong
 Date: Aug 2022
 */
#pragma once

template <typename T>
class CFactoryTemplate : public CSingletonTemplate<CFactoryTemplate<T>>
{
public:
	enum TYPE
	{
		TURRET,
		NUM_TYPES,
	};

private:
	std::unordered_map<TYPE, T*> m_prototypes;

public:
	CFactoryTemplate()
	{
		for (unsigned i = 0; i < NUM_TYPES; ++i)
		{
			m_prototypes[(TYPE)i] = new T;
		}
	};
	virtual ~CFactoryTemplate()
	{
		for (unsigned i = 0; i < NUM_TYPES; ++i)
		{
			delete m_prototypes[(TYPE)i];
		}
	}
	virtual T* GetClone(TYPE _TYPE)
	{
		return m_prototypes[_TYPE]->Clone();
	}
};