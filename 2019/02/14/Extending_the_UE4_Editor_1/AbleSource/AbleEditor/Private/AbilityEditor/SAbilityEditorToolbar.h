// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#pragma once

#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Textures/SlateIcon.h"

class FAblAbilityEditor;

/* Ability Editor Toolbar */
class FAblAbilityEditorToolbar : public TSharedFromThis<FAblAbilityEditorToolbar>
{
public:
	/* Populates the Toolbar with basic asset commands. */
	void SetupToolbar(TSharedPtr<FExtender> Extender, TSharedPtr<FAblAbilityEditor> InAbilityEditor);

	/* Populates the Toolbar with Timeline specific commands. */
	void AddTimelineToolbar(TSharedPtr<FExtender> Extender, TSharedPtr<FAblAbilityEditor> InAbilityEditor);
private:
	/* Helper methods for the Toolbar. */
	void FillAbilityEditorModeToolbar(FToolBarBuilder& ToolbarBuilder);
	void FillTimelineModeToolbar(FToolBarBuilder& ToolbarBuilder);

	/* Returns the Icon based on our Ability status (Play/Stop). */
	FSlateIcon GetPlayIcon() const;

	/* Returns the proper Text based on our Ability status. */
	FText GetPlayText() const;

	/* Returns the proper Tooltip based on our AAbility status. */
	FText GetPlayToolTip() const;
private:
	/* Pointer back to the Ability Editor that owns us. */
	TWeakPtr<FAblAbilityEditor> m_AbilityEditor;

	/* Play Icon */
	FSlateIcon m_PlayIcon;

	/* Pause Icon */
	FSlateIcon m_PauseIcon;
};


