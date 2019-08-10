// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#include "Channeling/ablChannelingBase.h"

UAblChannelingBase::UAblChannelingBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	m_Negate(false)
{

}

UAblChannelingBase::~UAblChannelingBase()
{

}

bool UAblChannelingBase::GetConditionResult(UAblAbilityContext& Context) const
{
	bool Result = CheckConditional(Context);

	return m_Negate ? !Result : Result;
}
