// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#include "AbilityEditor/SAbilityTimelineScrubPanel.h"

#include "AbleEditorPrivate.h"

#include "ablAbility.h"
#include "AbilityEditor/ablAbilityEditor.h"
#include "AbilityEditor/AblAbilityEditorSettings.h"
#include "AbleStyle.h"
#include "Framework/Application/MenuStack.h"
#include "Framework/Application/SlateApplication.h"
#include "Fonts/FontMeasure.h"
#include "Rendering/DrawElements.h"
#include "Rendering/SlateLayoutTransform.h"
#include "Rendering/SlateRenderer.h"

#define LOCTEXT_NAMESPACE "AblAbilityEditor"

SAblAbilityTimelineScrubPanel::SAblAbilityTimelineScrubPanel()
{

}

void SAblAbilityTimelineScrubPanel::Construct(const FArguments& InArgs)
{
	m_AbilityEditor = InArgs._AbilityEditor;

	m_Brush = FAbleStyle::GetBrush("Able.AbilityEditor.Timeline");

	m_Font = FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Regular.ttf"), 8);
}

FVector2D SAblAbilityTimelineScrubPanel::ComputeDesiredSize(float scale) const
{
	// Width is assumed to be stretched to fill, so only height really matters here.
	return FVector2D(100.0f, 26.0f);
}

int32 SAblAbilityTimelineScrubPanel::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	FVector2D TimeRange(0.0f, 1.0f);
	const TSharedRef< FSlateFontMeasure > FontMeasureService = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
	const UAblAbilityEditorSettings* EditorSettings = GetDefault<UAblAbilityEditorSettings>();

	if (m_AbilityEditor.IsValid())
	{
		TimeRange = m_AbilityEditor.Pin()->GetAbilityTimeRange();
	}

	FNumberFormattingOptions FmtOptions;
	FmtOptions.SetMaximumFractionalDigits(2);
	FmtOptions.SetMinimumFractionalDigits(1);

	FText MinText = FText::AsNumber(TimeRange.X, &FmtOptions);
	FText MaxText = FText::AsNumber(TimeRange.Y, &FmtOptions);
	FVector2D MinSize = FontMeasureService->Measure(MinText, m_Font);
	FVector2D MaxSize = FontMeasureService->Measure(MaxText, m_Font);

	const FVector2D& WidgetSize = AllottedGeometry.GetLocalSize();

	// Our Padding is how much it takes to safely display our largest string.
	const float Padding = MaxSize.X * 0.5f;
	FVector2D TextOffset(0.0f, 0.0f);

	const float ScrollbarWidth = 12.0f;
	
	int32 CurrentLayer = LayerId;

	FSlateDrawElement::MakeBox(OutDrawElements,
		CurrentLayer++,
		AllottedGeometry.ToPaintGeometry(),
		m_Brush);

	FSlateLayoutTransform Transform;

	if (!EditorSettings->m_ShowGuidelineLabels) // Our Guideline will always get the lower value.
	{
		// Left side (Min Time Label)
		TextOffset.X = Padding + MinSize.X * 0.5f;
		TextOffset.Y = 0.0f;

		Transform = FSlateLayoutTransform(TextOffset);

		FSlateDrawElement::MakeText(OutDrawElements,
			CurrentLayer++,
			AllottedGeometry.ToPaintGeometry(Transform),
			MinText,
			m_Font);
	}

	// Right side (Max Time Label)
	TextOffset.X = WidgetSize.X - MaxSize.X * 0.5f - Padding - ScrollbarWidth;
	
	Transform = FSlateLayoutTransform(TextOffset);
	
	FSlateDrawElement::MakeText(OutDrawElements,
		CurrentLayer++,
		AllottedGeometry.ToPaintGeometry(Transform),
		MaxText,
		m_Font);

	// Lines
	float MajorLineSize = WidgetSize.Y * 0.5f;
	float MinorLineSize = WidgetSize.Y * 0.25f;

	TArray<FVector2D> Line; // Start/End points.
	Line.SetNum(2);

	float MajorFreq = EditorSettings->m_MajorLineFrequency;
	float MinorFreq = EditorSettings->m_MinorLineFrequency;

	// Major
	float StepTime = 0.0f;
	FText MajorLabel;

	Line[0].Y = WidgetSize.Y - 1.0f; // Bottom, minus a bit of padding.
	Line[1].Y = WidgetSize.Y - MajorLineSize;


	const float WidgetInteriorSize = WidgetSize.X - Padding * 2.0f - ScrollbarWidth; // We have padding on both ends.

	for (; StepTime < TimeRange.Y; StepTime += MajorFreq)
	{
		Line[0].X =  Padding + WidgetInteriorSize * (StepTime / TimeRange.Y);
		Line[1].X = Line[0].X;

		FSlateDrawElement::MakeLines(OutDrawElements,
			CurrentLayer++,
			AllottedGeometry.ToPaintGeometry(),
			Line);

		if (EditorSettings->m_ShowGuidelineLabels)
		{
			MajorLabel = FText::AsNumber(StepTime, &FmtOptions);

			TextOffset.X = Line[0].X - FontMeasureService->Measure(MajorLabel, m_Font).X * 0.5f;
			Transform = FSlateLayoutTransform(TextOffset);

			FSlateDrawElement::MakeText(OutDrawElements,
				CurrentLayer++,
				AllottedGeometry.ToPaintGeometry(Transform),
				MajorLabel,
				m_Font);
		}
	}

	// Minor
	for (StepTime = 0.0f; StepTime < TimeRange.Y; StepTime += MinorFreq)
	{
		Line[0].X = Padding + WidgetInteriorSize * (StepTime / TimeRange.Y);
		Line[1].X = Line[0].X;

		FSlateDrawElement::MakeLines(OutDrawElements,
			CurrentLayer++,
			AllottedGeometry.ToPaintGeometry(),
			Line);
	}

	// Far Right Line
	Line[0].X = WidgetSize.X - Padding - ScrollbarWidth;
	Line[1].X = Line[0].X;

	FSlateDrawElement::MakeLines(OutDrawElements,
		CurrentLayer++,
		AllottedGeometry.ToPaintGeometry(),
		Line);

	return SCompoundWidget::OnPaint(Args, AllottedGeometry, MyClippingRect, OutDrawElements, CurrentLayer, InWidgetStyle, bParentEnabled);
}

#undef LOCTEXT_NAMESPACE