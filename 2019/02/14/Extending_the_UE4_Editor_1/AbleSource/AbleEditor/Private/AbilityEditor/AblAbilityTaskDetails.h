// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#pragma once

#include "IDetailCustomization.h"
#include "PropertyEditorModule.h"

class IDetailCategoryBuilder;

/* Property Customizer for UAblAbilityTask. */
class FAblAbilityTaskDetails : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	// IDetailCustomization interface
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
	// End of IDetailCustomization interface
private:
	class UAblAbilityTask* GetCurrentlyViewedTask(IDetailLayoutBuilder& DetailBuilder) const;

protected:
	// End Time
	virtual void GenerateReadOnlyEndTimeRow(class FDetailWidgetRow& OutRow, IDetailLayoutBuilder& DetailBuilder) const;
	virtual FText GetEndTimeText(IDetailLayoutBuilder* DetailBuilder) const;

	// Task Realm
	virtual void GenerateReadOnlyTaskRealmRow(class FDetailWidgetRow& OutRow, IDetailLayoutBuilder& DetailBuilder) const;
	virtual FText GetTaskRealmText(IDetailLayoutBuilder* DetailBuilder) const;
};