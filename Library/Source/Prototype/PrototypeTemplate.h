/**
 Prototype template
 By: JH Chong
 Date: Aug 2022
 */
#pragma once

template <typename T>
class CPrototypeTemplate
{
public:
	virtual T* Clone() const = 0;
};