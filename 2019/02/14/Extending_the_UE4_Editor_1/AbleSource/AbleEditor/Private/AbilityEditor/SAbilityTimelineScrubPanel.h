// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#pragma once

#include "AbilityEditor/ablAbilityEditor.h"
#include "Fonts/SlateFontInfo.h"
#include "Widgets/SCompoundWidget.h"

/* The Timeline Scrub Panel displays the time of the Ability from 0.0 to Ability Length with optional tick marks defined by the user. */
class SAblAbilityTimelineScrubPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAblAbilityTimelineScrubPanel)
		: _AbilityEditor()
	{}

	SLATE_ARGUMENT(TSharedPtr<FAblAbilityEditor>, AbilityEditor)

	SLATE_END_ARGS()

	SAblAbilityTimelineScrubPanel();

	void Construct(const FArguments& InArgs);

	// Rendering/Size overrides
	virtual FVector2D ComputeDesiredSize(float scale) const override;
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
private:
	/* Pointer our Ability Editor instance. */
	TWeakPtr<FAblAbilityEditor> m_AbilityEditor;

	/* Brush for our widget. */
	const FSlateBrush* m_Brush;

	/* Font information for our widget. */
	FSlateFontInfo m_Font;
};