// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#pragma once

#include "Fonts/SlateFontInfo.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SCompoundWidget.h"

class FAblAbilityEditor;
class SAblAbilityTimelineTrack;

/* Timeline Panel is a widget that displays all the Tasks in our Ability. When a Task is added or removed, the widget will update itself appropriately. */
class SAblAbilityTimelinePanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAblAbilityTimelinePanel)
		: _AbilityEditor()
	{}

	SLATE_ARGUMENT(TSharedPtr<FAblAbilityEditor>, AbilityEditor)

	SLATE_END_ARGS()

	SAblAbilityTimelinePanel();

	void Construct(const FArguments& InArgs);

	// Tick Override 
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	// Input Override
	virtual bool SupportsKeyboardFocus() const override { return true; }
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
private:
	/* Callback for when the Editor is refreshed. */
	void OnRefresh();

	/* Builds our list of Task widgets and adds them to our task container. */
	void BuildTaskList();

	/* Pointer to our Ability Editor instance. */
	TWeakPtr<FAblAbilityEditor> m_AbilityEditor;

	/* Our Task Container */
	TSharedPtr<SScrollBox> m_TaskList;

	/* Font information for our Tasks. */
	FSlateFontInfo m_FontInfo;

	/* The size of the Tasks last time the widget updated itself. */
	int32 m_CachedTaskSize;

	/* The length of the Ability last time the widget updated itself. */
	float m_CachedTimelineLength;
};

/* Thin wrapper around SScrollbox to allow for custom mouse commands. */
class SAblAbilityTimelineScrollBox : public SScrollBox
{
public:
	SLATE_BEGIN_ARGS(SAblAbilityTimelineScrollBox)
		: _AbilityEditor()
	{}

	SLATE_ARGUMENT(TSharedPtr<FAblAbilityEditor>, AbilityEditor)

	SLATE_END_ARGS()

	SAblAbilityTimelineScrollBox();

	void Construct(const FArguments& InArgs);

	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
private:
	/* Pointer to our Ability Editor instance. */
	TWeakPtr<FAblAbilityEditor> m_AbilityEditor;
};
