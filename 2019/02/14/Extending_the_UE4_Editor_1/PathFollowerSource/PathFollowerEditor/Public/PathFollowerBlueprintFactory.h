#pragma once

#include "Engine/Blueprint.h"
#include "Factories/Factory.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"
#include "EngineUtils.h"

#include "pathFollowerBlueprintFactory.generated.h"

/* Factory for Ability Blueprints. */
UCLASS(HideCategories = Object, MinimalAPI)
class UPathFollowerBlueprintFactory : public UFactory
{
	GENERATED_BODY()
public:
	UPathFollowerBlueprintFactory(const FObjectInitializer& ObjectInitializer);
	virtual ~UPathFollowerBlueprintFactory();

	//~ Begin UFactory Interface
	virtual bool ConfigureProperties() override;
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override;
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	//~ Begin UFactory Interface	
};

template <typename ObjClass>
static FORCEINLINE ObjClass* LoadBlueprintFromPath(const FName& Path)
{
	if (Path == NAME_None) return NULL;
	FString fileName = FPaths::GetBaseFilename(Path.ToString());
	FString dirPath = FPaths::GetPath(Path.ToString());
	FString cName = fileName.Append(FString("_C"));
	TArray<UObject*> tempArray;

	if (EngineUtils::FindOrLoadAssetsByPath(*dirPath, tempArray, EngineUtils::ATL_Class))
	{
		for (int i = 0; i < tempArray.Num(); ++i)
		{
			UObject* temp = tempArray[i];
			if (temp == NULL || (!Cast<ObjClass>(temp)) || (temp->GetName().Compare(cName) != 0))
				continue;

			return Cast<ObjClass>(temp);
		}
	}

	return NULL;
}