// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#include "AbilityEditor/AnimGraphNode_AbilityAnimPlayer.h"

#include "AbleEditorPrivate.h"

#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintActionFilter.h"
#include "BlueprintNodeSpawner.h"
#include "Kismet2/CompilerResultsLog.h"
#include "EdGraphSchema_K2_Actions.h"
#include "GraphEditorActions.h"
#include "AssetRegistryModule.h"
#include "AnimationGraphSchema.h"
#include "EditorCategoryUtils.h"

#define LOCTEXT_NAMESPACE "AbilityAnimNode"

// Action to add a ability animation player node to the graph
struct FNewAbilityAnimPlayerAction : public FEdGraphSchemaAction_K2NewNode
{
protected:
	FAssetData AssetInfo;
public:
	FNewAbilityAnimPlayerAction(const FAssetData& InAssetInfo, FText Title)
		: FEdGraphSchemaAction_K2NewNode(LOCTEXT("AbleCategory", "Able"), Title, FText::FromString(TEXT("Evaluates a sequence set by the Play Animation task to produce a pose.")), 0, FText::FromName(InAssetInfo.ObjectPath))
	{
		AssetInfo = InAssetInfo;

		UAnimGraphNode_AbilityAnimPlayer* Template = NewObject<UAnimGraphNode_AbilityAnimPlayer>();
		NodeTemplate = Template;
	}

	virtual UEdGraphNode* PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override
	{
		UAnimGraphNode_AbilityAnimPlayer* SpawnedNode = CastChecked<UAnimGraphNode_AbilityAnimPlayer>(FEdGraphSchemaAction_K2NewNode::PerformAction(ParentGraph, FromPin, Location, bSelectNewNode));

		return SpawnedNode;
	}
};


UAnimGraphNode_AbilityAnimPlayer::UAnimGraphNode_AbilityAnimPlayer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

FLinearColor UAnimGraphNode_AbilityAnimPlayer::GetNodeTitleColor() const
{
	return FLinearColor::Yellow;
}

FText UAnimGraphNode_AbilityAnimPlayer::GetTooltipText() const
{
	return LOCTEXT("AbilityAnimPlayerTooltip", "Allows for the dynamic playing of animations using the Play Animation task.");
}

FText UAnimGraphNode_AbilityAnimPlayer::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	FText Title = LOCTEXT("AbilityAnimPlayerTitle", "Play Ability Animation");
	if (SyncGroup.GroupName != NAME_None)
	{
		FText SyncGroupTitle = LOCTEXT("SyncGroupTitle", "Sync Group");
		FText SyncGroupName = FText::FromName(SyncGroup.GroupName);

		TArray<FStringFormatArg> FormatArgs;
		FormatArgs.Add(FStringFormatArg(Title.ToString()));
		FormatArgs.Add(FStringFormatArg(SyncGroupTitle.ToString()));
		FormatArgs.Add(FStringFormatArg(SyncGroupName.ToString()));
		return FText::FromString(FString::Format(TEXT("{0}/n{1}: {2}"), FormatArgs));
	}

	return Title;
}

FText UAnimGraphNode_AbilityAnimPlayer::GetMenuCategory() const
{
	// We don't actually populate a menu context, but if we need to, change this in the future.
	return FEditorCategoryUtils::GetCommonCategory(FCommonEditorCategory::Animation);
}

void UAnimGraphNode_AbilityAnimPlayer::BakeDataDuringCompilation(class FCompilerResultsLog& MessageLog)
{
	UAnimBlueprint* AnimBlueprint = GetAnimBlueprint();
	Node.GroupIndex = AnimBlueprint->FindOrAddGroup(SyncGroup.GroupName);
	Node.GroupRole = SyncGroup.GroupRole;
}

bool UAnimGraphNode_AbilityAnimPlayer::DoesSupportTimeForTransitionGetter() const
{
	return false;
}

void UAnimGraphNode_AbilityAnimPlayer::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	ActionRegistrar.AddBlueprintAction(UBlueprintNodeSpawner::Create(GetClass()));
}

bool UAnimGraphNode_AbilityAnimPlayer::IsActionFilteredOut(FBlueprintActionFilter const& Filter)
{
	for (UBlueprint* Blueprint : Filter.Context.Blueprints)
	{
		UAnimBlueprint* AnimBlueprint = Cast<UAnimBlueprint>(Blueprint);
		if (!AnimBlueprint)
		{
			// We only work on Animation Blueprints.
			return true;
		}
	}

	return false;
}


void UAnimGraphNode_AbilityAnimPlayer::GetContextMenuActions(const FGraphNodeContextMenuBuilder& Context) const
{
	// Nothing special
}

#undef LOCTEXT_NAMESPACE