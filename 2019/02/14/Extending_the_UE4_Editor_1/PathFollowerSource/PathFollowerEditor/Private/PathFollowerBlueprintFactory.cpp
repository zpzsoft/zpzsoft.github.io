#include "PathFollowerBlueprintFactory.h"
#include "PathFollowerBlueprint.h"
#include "PathFollowerBlueprintGeneratedClass.h"

#include "BlueprintEditorSettings.h"
#include "ClassViewerModule.h" // This has to be before ClassViewFilter
#include "ClassViewerFilter.h"
#include "Editor.h"
#include "Editor/EditorStyle/Public/EditorStyleSet.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/SClassPickerDialog.h"
#include "Misc/MessageDialog.h"
#include "Preferences/UnrealEdOptions.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Text/STextBlock.h"
#include "GameFramework/Character.h"
#include "Components/SphereComponent.h"
#include "Editor/UnrealEd/Public/Toolkits/AssetEditorManager.h"

#define LOCTEXT_NAMESPACE "UPathFollowerBlueprintFactory"

UPathFollowerBlueprintFactory::UPathFollowerBlueprintFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UPathFollowerBlueprint::StaticClass();
}

UPathFollowerBlueprintFactory::~UPathFollowerBlueprintFactory()
{

}

bool UPathFollowerBlueprintFactory::ConfigureProperties()
{
	return true;
};

UObject* UPathFollowerBlueprintFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	check(Class->IsChildOf(UPathFollowerBlueprint::StaticClass()));
	UPathFollowerBlueprint* NewBP = CastChecked<UPathFollowerBlueprint>(FKismetEditorUtilities::CreateBlueprint(ACharacter::StaticClass(), InParent, Name, EBlueprintType::BPTYPE_Normal, UPathFollowerBlueprint::StaticClass(), UPathFollowerBlueprintGeneratedClass::StaticClass(), CallingContext));

	//默认从另外一个蓝图中Copy过来, 省去需要重新创建蓝图内容.
	FString bpFile = TEXT("/PathFollower/FollowSplinePath");
	UObject* loadedObject = StaticLoadObject(UObject::StaticClass(), nullptr, *bpFile);
	UBlueprint* castedBlueprint = Cast<UBlueprint>(loadedObject);
	UBlueprint* ret = FKismetEditorUtilities::ReplaceBlueprint(NewBP, castedBlueprint);

	if (ret != nullptr)
		return ret;

	return NewBP;
}

UObject* UPathFollowerBlueprintFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return FactoryCreateNew(Class, InParent, Name, Flags, Context, Warn, NAME_None);
}

#undef LOCTEXT_NAMESPACE