// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#include "AbilityEditor/SAbilityEditorToolbar.h"

#include "AbleEditorPrivate.h"

#include "AbilityEditor/ablAbilityEditor.h"
#include "AbilityEditor/ablAbilityEditorCommands.h"
#include "AbilityEditor/ablAbilityEditorModes.h"

#include "AbleStyle.h"

#include "WorkflowOrientedApp/SModeWidget.h"
#include "WorkflowOrientedApp/SContentReference.h"

#include "IDocumentation.h"

#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSpacer.h"

#define LOCTEXT_NAMESPACE "AblAbilityEditor"

class SModeSeparator : public SBorder
{
public:
	SLATE_BEGIN_ARGS(SModeSeparator) {}
	SLATE_END_ARGS()

		void Construct(const FArguments& InArg)
	{
		SBorder::Construct(
			SBorder::FArguments()
			.BorderImage(FAbleStyle::GetBrush("AbilityEditor.ModeSeparator"))
			.Padding(0.0f)
		);
	}

	// SWidget interface
	virtual FVector2D ComputeDesiredSize(float) const override
	{
		const float Height = 24.0f;
		const float Thickness = 16.0f;
		return FVector2D(Thickness, Height);
	}
	// End of SWidget interface
};

void FAblAbilityEditorToolbar::SetupToolbar(TSharedPtr<FExtender> Extender, TSharedPtr<FAblAbilityEditor> InAbilityEditor)
{
	m_AbilityEditor = InAbilityEditor;

	Extender->AddToolBarExtension(
		"Asset",
		EExtensionHook::After,
		m_AbilityEditor.Pin()->GetToolkitCommands(),
		FToolBarExtensionDelegate::CreateSP(this, &FAblAbilityEditorToolbar::FillAbilityEditorModeToolbar));

	
	m_PlayIcon = FSlateIcon(FAbleStyle::GetStyleSetName(), FName(TEXT("AblAbilityEditor.m_PlayAbility")));
	m_PauseIcon = FSlateIcon(FAbleStyle::GetStyleSetName(), FName(TEXT("AblAbilityEditor.m_PauseAbility")));
}

void FAblAbilityEditorToolbar::FillAbilityEditorModeToolbar(FToolBarBuilder& ToolbarBuilder)
{
	TSharedPtr<FAblAbilityEditor> AbilityEditor = m_AbilityEditor.Pin();
	UBlueprint* BlueprintObj = AbilityEditor->GetBlueprintObj();

	const float ContentRefWidth = 80.0f;

	FToolBarBuilder Builder(ToolbarBuilder.GetTopCommandList(), ToolbarBuilder.GetCustomization());

	{
		TAttribute<FName> GetActiveMode(AbilityEditor.ToSharedRef(), &FAblAbilityEditor::GetCurrentMode);
		FOnModeChangeRequested SetActiveMode = FOnModeChangeRequested::CreateSP(AbilityEditor.ToSharedRef(), &FAblAbilityEditor::SetCurrentMode);

		// Left side padding
		AbilityEditor->AddToolbarWidget(SNew(SSpacer).Size(FVector2D(4.0f, 1.0f)));

		AbilityEditor->AddToolbarWidget(
			SNew(SModeWidget, FAblAbilityEditorModes::GetLocalizedMode(FAblAbilityEditorModes::AbilityTimelineMode), FAblAbilityEditorModes::AbilityTimelineMode)
			.OnGetActiveMode(GetActiveMode)
			.OnSetActiveMode(SetActiveMode)
			.IconImage(FAbleStyle::GetBrush("AblAbilityEditor.TimelineMode"))
			.SmallIconImage(FAbleStyle::GetBrush("AblAbilityEditor.TimelineMode.Small"))
			.DirtyMarkerBrush(AbilityEditor.Get(), &FAblAbilityEditor::GetDirtyImageForMode, FAblAbilityEditorModes::AbilityTimelineMode)
			.ToolTip(IDocumentation::Get()->CreateToolTip(
				LOCTEXT("AbilityTimelineButtonTooltip", "Switch to timeline editing mode"),
				NULL,
				TEXT("Shared/Plugin/Able"),
				TEXT("TimelineMode")))
			.AddMetaData<FTagMetaData>(TEXT("Able.Timeline"))
			.ShortContents()
			[
				SNew(SContentReference)
				.AssetReference(AbilityEditor.ToSharedRef(), &FAblAbilityEditor::GetAbilityBlueprintAsObject)
			.AllowSelectingNewAsset(false)
			.AllowClearingReference(false)
			.WidthOverride(ContentRefWidth)
			]
		);

		AbilityEditor->AddToolbarWidget(SNew(SModeSeparator));

		AbilityEditor->AddToolbarWidget(
			SNew(SModeWidget, FAblAbilityEditorModes::GetLocalizedMode(FAblAbilityEditorModes::AbilityBlueprintMode), FAblAbilityEditorModes::AbilityBlueprintMode)
			.OnGetActiveMode(GetActiveMode)
			.OnSetActiveMode(SetActiveMode)
			.IconImage(FAbleStyle::GetBrush("AblAbilityEditor.GraphMode"))
			.SmallIconImage(FAbleStyle::GetBrush("AblAbilityEditor.GraphMode.Small"))
			.DirtyMarkerBrush(AbilityEditor.Get(), &FAblAbilityEditor::GetDirtyImageForMode, FAblAbilityEditorModes::AbilityBlueprintMode)
			.ToolTip(IDocumentation::Get()->CreateToolTip(
				LOCTEXT("AbilityBlueprintButtonTooltip", "Switch to blueprint editing mode"),
				NULL,
				TEXT("Shared/Plugin/Able"),
				TEXT("BlueprintMode")))
			.AddMetaData<FTagMetaData>(TEXT("Able.Blueprint"))
			.ShortContents()
			[
				SNew(SContentReference)
				.AssetReference(AbilityEditor.ToSharedRef(), &FAblAbilityEditor::GetAbilityBlueprintAsObject)
			.AllowSelectingNewAsset(false)
			.AllowClearingReference(false)
			.WidthOverride(ContentRefWidth)
			]
		);

		// Right side padding
		AbilityEditor->AddToolbarWidget(SNew(SSpacer).Size(FVector2D(4.0f, 1.0f)));
	}
}

void FAblAbilityEditorToolbar::AddTimelineToolbar(TSharedPtr<FExtender> Extender, TSharedPtr<FAblAbilityEditor> InAbilityEditor)
{
	m_AbilityEditor = InAbilityEditor;

	Extender->AddToolBarExtension(
		"Asset",
		EExtensionHook::After,
		m_AbilityEditor.Pin()->GetToolkitCommands(),
		FToolBarExtensionDelegate::CreateSP(this, &FAblAbilityEditorToolbar::FillTimelineModeToolbar));
}

void FAblAbilityEditorToolbar::FillTimelineModeToolbar(FToolBarBuilder& ToolbarBuilder)
{
		const FAblAbilityEditorCommands& Commands = FAblAbilityEditorCommands::Get();
		
		ToolbarBuilder.BeginSection("Thumbnail");
		ToolbarBuilder.AddToolBarButton(Commands.m_CaptureThumbnail);
		ToolbarBuilder.EndSection();

		ToolbarBuilder.BeginSection("Timeline");
		ToolbarBuilder.AddToolBarButton(Commands.m_AddTask);
		ToolbarBuilder.AddToolBarButton(Commands.m_StepAbilityBackwards);
		ToolbarBuilder.AddToolBarButton(Commands.m_PlayAbility,
			NAME_None,
			TAttribute<FText>(this, &FAblAbilityEditorToolbar::GetPlayText),
			TAttribute<FText>(this, &FAblAbilityEditorToolbar::GetPlayToolTip),
			TAttribute<FSlateIcon>(this, &FAblAbilityEditorToolbar::GetPlayIcon));
		ToolbarBuilder.AddToolBarButton(Commands.m_StepAbility);
		ToolbarBuilder.AddToolBarButton(Commands.m_StopAbility);
		ToolbarBuilder.AddToolBarButton(Commands.m_Resize);
		ToolbarBuilder.AddToolBarButton(Commands.m_SetPreviewAsset);
		ToolbarBuilder.AddToolBarButton(Commands.m_ResetPreviewAsset);
		ToolbarBuilder.AddToolBarButton(Commands.m_Validate);
		ToolbarBuilder.AddToolBarButton(Commands.m_ToggleCost);
		ToolbarBuilder.EndSection();

}

FSlateIcon FAblAbilityEditorToolbar::GetPlayIcon() const
{
	if (m_AbilityEditor.Pin()->IsPlayingAbility() && !m_AbilityEditor.Pin()->IsPaused())
	{
		return m_PauseIcon;
	}

	return m_PlayIcon;
}

FText FAblAbilityEditorToolbar::GetPlayText() const
{
	if (m_AbilityEditor.Pin()->IsPlayingAbility() && !m_AbilityEditor.Pin()->IsPaused())
	{
		return LOCTEXT("PauseCommand", "Pause");
	}

	return LOCTEXT("PlayCommand", "Play");
}

FText FAblAbilityEditorToolbar::GetPlayToolTip() const
{
	if (m_AbilityEditor.Pin()->IsPlayingAbility() && !m_AbilityEditor.Pin()->IsPaused())
	{
		return LOCTEXT("PauseCommandToolTip", "Pause the currently playing Ability.");
	}

	return LOCTEXT("PlayCommandToolTip", "Play the current Ability.");
}

#undef LOCTEXT_NAMESPACE