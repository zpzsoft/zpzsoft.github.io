#pragma once

#include "Editor/Kismet/Public/BlueprintEditor.h"

class FPathFollowerEditor : public FBlueprintEditor
{
public:
	void InitPathFollowerEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, const TArray<UBlueprint*>& InAbilityBlueprints, bool ShouldUseDataOnlyEditor);

	void AddComponent(UClass* NewComponentClass, FName ComponentName);
};