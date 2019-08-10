// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#include "AbleStyle.h"

#include "AbleEditorPrivate.h"
#include "Brushes/SlateImageBrush.h"
#include "Brushes/SlateBoxBrush.h"
#include "Misc/Paths.h"
#include "Styling/SlateStyleRegistry.h"
#include "Interfaces/IPluginManager.h"
#include "ClassIconFinder.h"
#include "Styling/ISlateStyle.h"
#include "Styling/SlateStyle.h"

#define IMAGE_PLUGIN_BRUSH( RelativePath, ... ) FSlateImageBrush( FAbleStyle::InContent( RelativePath, ".png" ), __VA_ARGS__ )
#define BOX_PLUGIN_BRUSH( RelativePath, ... ) FSlateBoxBrush( FAbleStyle::InContent( RelativePath, ".png" ), __VA_ARGS__ )

TSharedPtr< FSlateStyleSet > FAbleStyle::m_StyleSet = nullptr;
FName FAbleStyle::m_StyleName(TEXT("Able"));

void FAbleStyle::Initialize()
{
	// Const icon sizes
	const FVector2D Icon8x8(8.0f, 8.0f);
	const FVector2D Icon12x12(12.0f, 12.0f);
	const FVector2D Icon16x16(16.0f, 16.0f);
	const FVector2D Icon20x20(20.0f, 20.0f);
	const FVector2D Icon40x40(40.0f, 40.0f);

	// Only register once
	if (m_StyleSet.IsValid())
	{
		return;
	}

	m_StyleSet = MakeShareable(new FSlateStyleSet(GetStyleSetName()));
	m_StyleSet->SetContentRoot(FPaths::EngineContentDir() / TEXT("Editor/Slate"));
	m_StyleSet->SetCoreContentRoot(FPaths::EngineContentDir() / TEXT("Slate"));

	m_StyleSet->Set("AbilityEditor.ModeSeparator", new BOX_PLUGIN_BRUSH("Modes/ModeSeparator", FMargin(15.0f / 16.0f, 22.0f / 24.0f, 1.0f / 16.0f, 1.0f / 24.0f), FLinearColor(1, 1, 1, 0.5f)));
	m_StyleSet->Set("Able.AbilityEditor.Timeline", new BOX_PLUGIN_BRUSH("Brushes/TrackBackground", FMargin(4.0f / 16.0f)));
	m_StyleSet->Set("Able.AbilityEditor.TimelineStatus", new BOX_PLUGIN_BRUSH("Brushes/StatusBackground", FMargin(4.0f / 16.0f)));
	m_StyleSet->Set("Able.AbilityEditor.Node", new BOX_PLUGIN_BRUSH("Brushes/RoundedNodeBackground", FMargin(4.0f / 16.0f)));
	m_StyleSet->Set("Able.AbilityEditor.NodeHighlight", new BOX_PLUGIN_BRUSH("Brushes/RoundedNodeBackgroundHi", FMargin(4.0f / 16.0f)));
	m_StyleSet->Set("Able.AbilityEditor.NodeText", new BOX_PLUGIN_BRUSH("Brushes/NodeText", FMargin(4.0f / 16.0f)));
	m_StyleSet->Set("Able.AbilityEditor.TimelimeStatus.Marker", new IMAGE_PLUGIN_BRUSH(TEXT("Icons/StatusBarMarker"), Icon12x12));
	m_StyleSet->Set("Able.AbilityEditor.Timeline.Dependency", new IMAGE_PLUGIN_BRUSH(TEXT("Icons/Dependency_12"), Icon12x12));

	m_StyleSet->Set("ClassIcon.AblAbility", new IMAGE_PLUGIN_BRUSH(TEXT("Icons/AbilityIcon_40"), Icon40x40));
	m_StyleSet->Set("ClassIcon.AblAbility.Small", new IMAGE_PLUGIN_BRUSH(TEXT("Icons/AbilityIcon_16"), Icon16x16));
	m_StyleSet->Set("ClassIcon.AblAbilityBlueprint", new IMAGE_PLUGIN_BRUSH(TEXT("Icons/AbilityIcon_40"), Icon40x40));
	m_StyleSet->Set("ClassIcon.AblAbilityBlueprint.Small", new IMAGE_PLUGIN_BRUSH(TEXT("Icons/AbilityIcon_16"), Icon16x16));
	m_StyleSet->Set("ClassIcon.AblCustomTask", new IMAGE_PLUGIN_BRUSH(TEXT("Icons/CustomTask_40"), Icon40x40));
	m_StyleSet->Set("ClassIcon.AblCustomTask.Small", new IMAGE_PLUGIN_BRUSH(TEXT("Icons/CustomTask_16"), Icon16x16));
	m_StyleSet->Set("ClassIcon.AblCustomTaskScratchPad", new IMAGE_PLUGIN_BRUSH(TEXT("Icons/CustomScratchPad_40"), Icon40x40));
	m_StyleSet->Set("ClassIcon.AblCustomTaskScratchPad.Small", new IMAGE_PLUGIN_BRUSH(TEXT("Icons/CustomScratchPad_16"), Icon16x16));
	m_StyleSet->Set("ClassThumbnail.AblAbility", new IMAGE_PLUGIN_BRUSH(TEXT("Icons/AbilityIcon_40"), Icon40x40));
	m_StyleSet->Set("ClassThumbnail.AblAbilityBlueprint", new IMAGE_PLUGIN_BRUSH(TEXT("Icons/AbilityIcon_40"), Icon40x40));
	m_StyleSet->Set("ClassThumbnail.AblCustomTask", new IMAGE_PLUGIN_BRUSH(TEXT("Icons/CustomTask_40"), Icon40x40));
	m_StyleSet->Set("ClassThumbnail.AblCustomTaskScratchPad", new IMAGE_PLUGIN_BRUSH(TEXT("Icons/CustomScratchPad_40"), Icon40x40));

	// Commands
	m_StyleSet->Set("AblAbilityEditor.m_AddTask", new IMAGE_PLUGIN_BRUSH(TEXT("Icons/AddTask_40"), Icon40x40));
	m_StyleSet->Set("AblAbilityEditor.m_AddTask.Small", new IMAGE_PLUGIN_BRUSH(TEXT("Icons/AddTask_40"), Icon20x20));
	m_StyleSet->Set("AblAbilityEditor.m_RemoveTask", new IMAGE_PLUGIN_BRUSH(TEXT("Icons/RemoveTask_40"), Icon40x40));
	m_StyleSet->Set("AblAbilityEditor.m_RemoveTask.Small", new IMAGE_PLUGIN_BRUSH(TEXT("Icons/RemoveTask_40"), Icon20x20));
	m_StyleSet->Set("AblAbilityEditor.m_Validate", new IMAGE_PLUGIN_BRUSH(TEXT("Icons/Validate_40"), Icon40x40));
	m_StyleSet->Set("AblAbilityEditor.m_Validate.Small", new IMAGE_PLUGIN_BRUSH(TEXT("Icons/Validate_40"), Icon20x20));
	m_StyleSet->Set("AblAbilityEditor.m_SetPreviewAsset", new IMAGE_PLUGIN_BRUSH(TEXT("Icons/SetPreviewAsset_40"), Icon40x40));
	m_StyleSet->Set("AblAbilityEditor.m_SetPreviewAsset.Small", new IMAGE_PLUGIN_BRUSH(TEXT("Icons/SetPreviewAsset_40"), Icon20x20));
	m_StyleSet->Set("AblAbilityEditor.m_PlayAbility", new IMAGE_PLUGIN_BRUSH(TEXT("Icons/PlayAbility_40"), Icon40x40));
	m_StyleSet->Set("AblAbilityEditor.m_PlayAbility.Small", new IMAGE_PLUGIN_BRUSH(TEXT("Icons/PlayAbility_40"), Icon20x20));
	m_StyleSet->Set("AblAbilityEditor.m_StopAbility", new IMAGE_PLUGIN_BRUSH(TEXT("Icons/StopAbility_40"), Icon40x40));
	m_StyleSet->Set("AblAbilityEditor.m_StopAbility.Small", new IMAGE_PLUGIN_BRUSH(TEXT("Icons/StopAbility_40"), Icon20x20));
	m_StyleSet->Set("AblAbilityEditor.m_PauseAbility", new IMAGE_PLUGIN_BRUSH(TEXT("Icons/PauseAbility_40"), Icon40x40));
	m_StyleSet->Set("AblAbilityEditor.m_PauseAbility.Small", new IMAGE_PLUGIN_BRUSH(TEXT("Icons/PauseAbility_40"), Icon20x20));
	m_StyleSet->Set("AblAbilityEditor.m_StepAbility", new IMAGE_PLUGIN_BRUSH(TEXT("Icons/StepAbility_40"), Icon40x40));
	m_StyleSet->Set("AblAbilityEditor.m_StepAbility.Small", new IMAGE_PLUGIN_BRUSH(TEXT("Icons/StepAbility_40"), Icon20x20));
	m_StyleSet->Set("AblAbilityEditor.m_StepAbilityBackwards", new IMAGE_PLUGIN_BRUSH(TEXT("Icons/StepAbilityBackwards_40"), Icon40x40));
	m_StyleSet->Set("AblAbilityEditor.m_StepAbilityBackwards.Small", new IMAGE_PLUGIN_BRUSH(TEXT("Icons/StepAbilityBackwards_40"), Icon20x20));
	m_StyleSet->Set("AblAbilityEditor.m_ResetPreviewAsset", new IMAGE_PLUGIN_BRUSH(TEXT("Icons/ResetPreviewAsset_40"), Icon40x40));
	m_StyleSet->Set("AblAbilityEditor.m_ResetPreviewAsset.Small", new IMAGE_PLUGIN_BRUSH(TEXT("Icons/ResetPreviewAsset_40"), Icon20x20));
	m_StyleSet->Set("AblAbilityEditor.m_ToggleCost", new IMAGE_PLUGIN_BRUSH(TEXT("Icons/ToggleCost_40"), Icon40x40));
	m_StyleSet->Set("AblAbilityEditor.m_ToggleCost.Small", new IMAGE_PLUGIN_BRUSH(TEXT("Icons/ToggleCost_40"), Icon20x20));
	m_StyleSet->Set("AblAbilityEditor.m_Resize", new IMAGE_PLUGIN_BRUSH(TEXT("Icons/Resize_40"), Icon40x40));
	m_StyleSet->Set("AblAbilityEditor.m_Resize.Small", new IMAGE_PLUGIN_BRUSH(TEXT("Icons/Resize_40"), Icon20x20));
	m_StyleSet->Set("AblAbilityEditor.m_CaptureThumbnail", new IMAGE_PLUGIN_BRUSH(TEXT("Icons/CaptureThumbnail_40"), Icon40x40));
	m_StyleSet->Set("AblAbilityEditor.m_CaptureThumbnail.Small", new IMAGE_PLUGIN_BRUSH(TEXT("Icons/CaptureThumbnail_40"), Icon20x20));

	// Tab Styles
	m_StyleSet->Set("Able.Tabs.AbilityTimeline", new IMAGE_PLUGIN_BRUSH("Icons/AbilityIcon_16", Icon16x16));
	m_StyleSet->Set("Able.Tabs.AbilityEditorSettings", new IMAGE_PLUGIN_BRUSH("Icons/EditorSettings_16", Icon16x16));
	m_StyleSet->Set("Able.Tabs.AbilityAssetDetails", new IMAGE_PLUGIN_BRUSH("Icons/AbilityProperties_16", Icon16x16));
	m_StyleSet->Set("Able.Tabs.AbilityTaskAssetDetails", new IMAGE_PLUGIN_BRUSH("Icons/TaskProperties_16", Icon16x16));

	// Mode Icons
	m_StyleSet->Set("AblAbilityEditor.TimelineMode", new IMAGE_PLUGIN_BRUSH(TEXT("Icons/TimelineMode_40"), Icon40x40));
	m_StyleSet->Set("AblAbilityEditor.TimelineMode.Small", new IMAGE_PLUGIN_BRUSH(TEXT("Icons/TimelineMode_16"), Icon16x16));
	m_StyleSet->Set("AblAbilityEditor.GraphMode", new IMAGE_PLUGIN_BRUSH(TEXT("Icons/GraphMode_40"), Icon40x40));
	m_StyleSet->Set("AblAbilityEditor.GraphMode.Small", new IMAGE_PLUGIN_BRUSH(TEXT("Icons/GraphMode_16"), Icon16x16));

	// Images
	m_StyleSet->Set("AblAbilityEditor.PreviewAssetImage", new IMAGE_PLUGIN_BRUSH(TEXT("Images/PreviewActor"), FVector2D(256.0f, 256.0f)));

	FSlateStyleRegistry::RegisterSlateStyle(*m_StyleSet.Get());
}

void FAbleStyle::Shutdown()
{
	if (m_StyleSet.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*m_StyleSet.Get());
		ensure(m_StyleSet.IsUnique());
		m_StyleSet.Reset();
	}
}

TSharedPtr< class ISlateStyle > FAbleStyle::Get()
{
	return m_StyleSet;
}

const FSlateBrush* FAbleStyle::GetBrush(FName BrushName)
{
	if (m_StyleSet.IsValid())
	{
		return m_StyleSet->GetBrush(BrushName);
	}

	return nullptr;
}

FString FAbleStyle::InContent(const FString& RelativePath, const ANSICHAR* Extension)
{
	static FString ContentDir = IPluginManager::Get().FindPlugin(TEXT("Able"))->GetContentDir();
	return (ContentDir / RelativePath) + Extension;
}

#undef IMAGE_PLUGIN_BRUSH