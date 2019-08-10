// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#include "AbilityEditor/SAbilityTimeline.h"

#include "AbleEditorPrivate.h"

#include "ablAbility.h"
#include "AbilityEditor/ablAbilityEditor.h"
#include "AbilityEditor/ablAbilityEditorCommands.h"
#include "AbilityEditor/SAbilityTimelineScrubPanel.h"
#include "AbilityEditor/SAbilityTimelineStatus.h"
#include "AbilityEditor/SAbilityTimelineTrack.h"
#include "Fonts/FontMeasure.h"
#include "Framework/Application/MenuStack.h"
#include "Framework/Application/SlateApplication.h"
#include "Rendering/SlateRenderer.h"
#include "Tasks/IAblAbilityTask.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SWindow.h"

#define LOCTEXT_NAMESPACE "AblAbilityEditor"

SAblAbilityTimelinePanel::SAblAbilityTimelinePanel()
	: m_AbilityEditor(),
	m_CachedTaskSize(0),
	m_CachedTimelineLength(0.0f)
{

}

void SAblAbilityTimelinePanel::Construct(const FArguments& InArgs)
{
	m_AbilityEditor = InArgs._AbilityEditor;

	m_FontInfo = FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Regular.ttf"), 8);
	m_TaskList = SNew(SAblAbilityTimelineScrollBox).AbilityEditor(InArgs._AbilityEditor);

	ChildSlot
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(0.98f)
				.HAlign(EHorizontalAlignment::HAlign_Fill)
				[

					SNew(SVerticalBox)
					+SVerticalBox::Slot()
						.AutoHeight()
						.VAlign(VAlign_Top)
						[
							SNew(SAblAbilityTimelineStatus)
							.AbilityEditor(InArgs._AbilityEditor)
						]
					+ SVerticalBox::Slot()
						.AutoHeight()
						.VAlign(EVerticalAlignment::VAlign_Top)
						[
							SNew(SAblAbilityTimelineScrubPanel)
							.AbilityEditor(InArgs._AbilityEditor)
						]

					+ SVerticalBox::Slot()
						.VAlign(EVerticalAlignment::VAlign_Fill)
						[
							m_TaskList.ToSharedRef()
						]

				]

			]
		];

	BuildTaskList();

	m_AbilityEditor.Pin()->OnRefresh().AddSP(this, &SAblAbilityTimelinePanel::OnRefresh);
}

void SAblAbilityTimelinePanel::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	if (m_AbilityEditor.IsValid())
	{
		if (const UAblAbility* Ability = m_AbilityEditor.Pin()->GetConstAbility())
		{
			const float CurrentLength = Ability->GetLength();
			const int32 CurrentTaskSize = Ability->GetTasks().Num();
			if (m_CachedTaskSize != CurrentTaskSize || FMath::Abs(m_CachedTimelineLength - CurrentLength) > KINDA_SMALL_NUMBER)
			{
				BuildTaskList();
				m_CachedTimelineLength = CurrentLength;
			}
		}
	}

	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
}

FReply SAblAbilityTimelinePanel::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (!m_AbilityEditor.IsValid())
	{
		return FReply::Unhandled();
	}

	TSharedPtr<FAblAbilityEditor> Editor = m_AbilityEditor.Pin();
	if (InKeyEvent.GetKey() == EKeys::N && InKeyEvent.IsControlDown())
	{
		Editor->AddNewTask();

		return FReply::Handled();
	}
	if (InKeyEvent.GetKey() == EKeys::Delete)
	{
		if (UAblAbilityTask* CurrentTask = m_AbilityEditor.Pin()->GetCurrentlySelectedAbilityTask())
		{
			Editor->RemoveTask(*CurrentTask);

			return FReply::Handled();
		}
	}
	else if (InKeyEvent.GetKey() == EKeys::SpaceBar)
	{
		if (Editor->IsPlayingAbility())
		{
			Editor->PauseAbility();
		}
		else
		{
			Editor->PlayAbility();
		}

		return FReply::Handled();
	}
	else if (InKeyEvent.GetKey() == EKeys::Right || InKeyEvent.GetKey() == EKeys::Left)
	{
		InKeyEvent.GetKey() == EKeys::Right ?  m_AbilityEditor.Pin()->StepAbility() : m_AbilityEditor.Pin()->StepAbilityBackwards();

		return FReply::Handled();
	}

	return FReply::Unhandled();
}

void SAblAbilityTimelinePanel::OnRefresh()
{
	m_CachedTaskSize = 0;
	m_TaskList->ClearChildren();
}

void SAblAbilityTimelinePanel::BuildTaskList()
{
	if (UAblAbility* Ability = m_AbilityEditor.Pin()->GetAbility())
	{
		// Determine the Padding we'll need to fully display our time range.
		const TSharedRef< FSlateFontMeasure > FontMeasureService = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();

		FNumberFormattingOptions FmtOptions;
		FmtOptions.SetMaximumFractionalDigits(2);
		FmtOptions.SetMinimumFractionalDigits(1);

		FText LengthText = FText::AsNumber(Ability->GetLength(), &FmtOptions);
		FVector2D LengthSize = FontMeasureService->Measure(LengthText, m_FontInfo);

		// Our Padding is how much it takes to safely display our largest string.
		const float HPadding = LengthSize.X * 0.5f;

		m_TaskList->ClearChildren();
		const TArray<UAblAbilityTask*>& Tasks = Ability->GetTasks();
		for (UAblAbilityTask* Task : Tasks)
		{
			m_TaskList->AddSlot()
				.VAlign(VAlign_Top)
				.Padding(HPadding, 0.0f)
				[
					SNew(SAblAbilityTimelineTrack)
					.AbilityEditor(m_AbilityEditor)
					.AbilityTask(Task)
				];
		}
		
		m_CachedTaskSize = Tasks.Num();
	}
}

SAblAbilityTimelineScrollBox::SAblAbilityTimelineScrollBox()
{

}

void SAblAbilityTimelineScrollBox::Construct(const FArguments& InArgs)
{
	m_AbilityEditor = InArgs._AbilityEditor;

	SScrollBox::Construct(SScrollBox::FArguments().Orientation(Orient_Vertical).ScrollBarAlwaysVisible(true));
}

FReply SAblAbilityTimelineScrollBox::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		if (m_AbilityEditor.IsValid())
		{
			TSharedRef<SWidget> MenuContents = m_AbilityEditor.Pin()->GenerateTaskContextMenu();
			FWidgetPath WidgetPath = MouseEvent.GetEventPath() != nullptr ? *MouseEvent.GetEventPath() : FWidgetPath();
			FSlateApplication::Get().PushMenu(AsShared(), WidgetPath, MenuContents, MouseEvent.GetScreenSpacePosition(), FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu));
		}
	}

	return SScrollBox::OnMouseButtonUp(MyGeometry, MouseEvent);
}

#undef LOCTEXT_NAMESPACE