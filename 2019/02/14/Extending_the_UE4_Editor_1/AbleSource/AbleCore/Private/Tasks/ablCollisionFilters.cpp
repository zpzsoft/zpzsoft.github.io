// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#include "Tasks/ablCollisionFilters.h"

#include "ablAbilityContext.h"
#include "ablAbilityUtilities.h"
#include "ablSettings.h"

#include "Async/Future.h"
#include "Async/Async.h"

UAblCollisionFilter::UAblCollisionFilter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

UAblCollisionFilter::~UAblCollisionFilter()
{

}

void UAblCollisionFilter::Filter(const TWeakObjectPtr<const UAblAbilityContext>& Context, TArray<FAblQueryResult>& InOutArray) const
{
	verifyf(false, TEXT("This method should never be called. Did you forget to override it in your child class?"));
}

UAblCollisionFilterSelf::UAblCollisionFilterSelf(const FObjectInitializer& ObjectInitializer)
	: Super (ObjectInitializer)
{

}

UAblCollisionFilterSelf::~UAblCollisionFilterSelf()
{

}

void UAblCollisionFilterSelf::Filter(const TWeakObjectPtr<const UAblAbilityContext>& Context, TArray<FAblQueryResult>& InOutArray) const
{
	InOutArray.RemoveAll([&](const FAblQueryResult& LHS) { return LHS.Actor == Context->GetSelfActor(); });
}

UAblCollisionFilterOwner::UAblCollisionFilterOwner(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

UAblCollisionFilterOwner::~UAblCollisionFilterOwner()
{

}

void UAblCollisionFilterOwner::Filter(const TWeakObjectPtr<const UAblAbilityContext>& Context, TArray<FAblQueryResult>& InOutArray) const
{
	InOutArray.RemoveAll([&](const FAblQueryResult& LHS) { return LHS.Actor == Context->GetOwner(); });
}

UAblCollisionFilterInstigator::UAblCollisionFilterInstigator(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

UAblCollisionFilterInstigator::~UAblCollisionFilterInstigator()
{

}

void UAblCollisionFilterInstigator::Filter(const TWeakObjectPtr<const UAblAbilityContext>& Context, TArray<FAblQueryResult>& InOutArray) const
{
	InOutArray.RemoveAll([&](const FAblQueryResult& LHS) { return LHS.Actor == Context->GetInstigator(); });
}

UAblCollisionFilterByClass::UAblCollisionFilterByClass(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

UAblCollisionFilterByClass::~UAblCollisionFilterByClass()
{

}

void UAblCollisionFilterByClass::Filter(const TWeakObjectPtr<const UAblAbilityContext>& Context, TArray<FAblQueryResult>& InOutArray) const
{
	bool Remove = false;
	for (int i = 0; i < InOutArray.Num(); )
	{
		if (AActor* Actor = InOutArray[i].Actor.Get())
		{
			Remove = Actor->GetClass()->IsChildOf(m_Class);
			Remove = m_Negate ? !Remove : Remove;
		}
		else
		{
			Remove = true;
		}

		if (Remove)
		{
			InOutArray.RemoveAt(i, 1, false);
		}
		else
		{
			++i;
		}
	}

	InOutArray.Shrink();
}

UAblCollisionFilterSortByDistance::UAblCollisionFilterSortByDistance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

UAblCollisionFilterSortByDistance::~UAblCollisionFilterSortByDistance()
{

}

void UAblCollisionFilterSortByDistance::Filter(const TWeakObjectPtr<const UAblAbilityContext>& Context, TArray<FAblQueryResult>& InOutArray) const
{
	FTransform SourceTransform;
	m_Location.GetTransform(*Context.Get(), SourceTransform);
	InOutArray.Sort(FAblAbilityResultSortByDistance(SourceTransform.GetLocation(), m_Use2DDistance, m_SortDirection == EAblCollisionFilterSort::AblFitlerSort_Ascending));
}

UAblCollisionFilterMaxResults::UAblCollisionFilterMaxResults(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

UAblCollisionFilterMaxResults::~UAblCollisionFilterMaxResults()
{

}

void UAblCollisionFilterMaxResults::Filter(const TWeakObjectPtr<const UAblAbilityContext>& Context, TArray<FAblQueryResult>& InOutArray) const
{
	if (InOutArray.Num() > m_MaxEntities)
	{
		int32 NumberToTrim = InOutArray.Num() - m_MaxEntities;
		if (NumberToTrim > 0 && NumberToTrim < InOutArray.Num())
		{
			InOutArray.RemoveAt(InOutArray.Num() - NumberToTrim, NumberToTrim);
		}
	}
}

UAblCollisionFilterCustom::UAblCollisionFilterCustom(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	m_EventName(NAME_None),
	m_UseAsync(false)
{

}

UAblCollisionFilterCustom::~UAblCollisionFilterCustom()
{

}

void UAblCollisionFilterCustom::Filter(const TWeakObjectPtr<const UAblAbilityContext>& Context, TArray<FAblQueryResult>& InOutArray) const
{
	check(Context.IsValid());
	const UAblAbility* Ability = Context->GetAbility();
	check(Ability);

	if (m_UseAsync && UAbleSettings::IsAsyncEnabled())
	{
		TArray<TFuture<bool>> FutureResults;
		FutureResults.Reserve(InOutArray.Num());

		FName EventName = m_EventName;
		for (FAblQueryResult& Result : InOutArray)
		{
			FutureResults.Add(Async<bool>(EAsyncExecution::TaskGraph, [&Ability, &Context, EventName, &Result]
			{
				return Ability->CustomFilterCondition(Context.Get(), EventName, Result.Actor.Get());
			}));
		}

		check(FutureResults.Num() == InOutArray.Num());
		int32 InOutIndex = 0;
		for (TFuture<bool>& Future : FutureResults)
		{
			if (!Future.IsReady())
			{
				static const FTimespan OneMillisecond = FTimespan::FromMilliseconds(1.0);
				Future.WaitFor(OneMillisecond);
			}

			if (!Future.Get())
			{
				// Target passed filtering, moving on.
				InOutArray.RemoveAt(InOutIndex, 1, false);
			}
			else
			{
				++InOutIndex;
			}
		}

		InOutArray.Shrink();
	}
	else
	{
		for (int i = 0; i < InOutArray.Num(); )
		{
			if (!Ability->CustomFilterCondition(Context.Get(), m_EventName, InOutArray[i].Actor.Get()))
			{
				InOutArray.RemoveAt(i, 1, false);
			}
			else
			{
				++i;
			}
		}

		InOutArray.Shrink();
	}
}
