// Fill out your copyright notice in the Description page of Project Settings.

#include "PathFollowerBlueprint.h"
#include "PathFollowerBlueprintGeneratedClass.h"

UPathFollowerBlueprint::UPathFollowerBlueprint(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

UClass * UPathFollowerBlueprint::GetBlueprintClass() const
{
	return UPathFollowerBlueprintGeneratedClass::StaticClass();
}

UPathFollowerBlueprint * UPathFollowerBlueprint::FindRootGameplayAbilityBlueprint(UPathFollowerBlueprint * DerivedBlueprint)
{
	UPathFollowerBlueprint* ParentBP = NULL;

	// Determine if there is a ability blueprint in the ancestry of this class
	for (UClass* ParentClass = DerivedBlueprint->ParentClass; ParentClass != UObject::StaticClass(); ParentClass = ParentClass->GetSuperClass())
	{
		if (UPathFollowerBlueprint* TestBP = Cast<UPathFollowerBlueprint>(ParentClass->ClassGeneratedBy))
		{
			ParentBP = TestBP;
		}
	}

	return ParentBP;
}

void UPathFollowerBlueprint::AddComponent(UClass * NewComponentClass, FName ComponentName)
{
	
}
