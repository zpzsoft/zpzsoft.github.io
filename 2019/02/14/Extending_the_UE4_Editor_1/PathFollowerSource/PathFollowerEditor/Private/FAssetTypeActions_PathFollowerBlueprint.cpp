// Fill out your copyright notice in the Description page of Project Settings.

#include "FAssetTypeActions_PathFollowerBlueprint.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "BlueprintEditorModule.h"
#include "AssetRegistryModule.h"
#include "Editor/UnrealEd/Classes/ThumbnailRendering/SceneThumbnailInfo.h"
#include "PathFollowerBlueprint.h"
#include "FPathFollowerEditor.h"
#include "Components/SphereComponent.h"
#include "Components/SplineComponent.h"

FAssetTypeActions_PathFollowerBlueprint::FAssetTypeActions_PathFollowerBlueprint(EAssetTypeCategories::Type AssetCategory)
	: m_AssetCategory(AssetCategory)
{

}

FAssetTypeActions_PathFollowerBlueprint::~FAssetTypeActions_PathFollowerBlueprint()
{

}

FText FAssetTypeActions_PathFollowerBlueprint::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_PathFollowerBlueprint", "Path Follower Blueprint");
}

FColor FAssetTypeActions_PathFollowerBlueprint::GetTypeColor() const
{
	return FColor::Emerald;
}

UClass * FAssetTypeActions_PathFollowerBlueprint::GetSupportedClass() const
{
	return UPathFollowerBlueprint::StaticClass();
}

void FAssetTypeActions_PathFollowerBlueprint::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor)
{
	EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (UObject* Object : InObjects)
	{
		if (UBlueprint* Blueprint = Cast<UBlueprint>(Object))
		{
			bool isFirstCreate = Blueprint->bIsNewlyCreated;
			TSharedRef<FPathFollowerEditor> Editor(new FPathFollowerEditor());
			TArray<UBlueprint*> Blueprints;
			Blueprints.Add(Blueprint);

			Editor->InitPathFollowerEditor(Mode, EditWithinLevelEditor, Blueprints, ShouldUseDataOnlyEditor(Blueprint));

			//第一次打开, 初始化部分对象.
			if(isFirstCreate)
			{
				Editor->AddComponent(USphereComponent::StaticClass(), TEXT("Sphere"));
				Editor->AddComponent(USplineComponent::StaticClass(), TEXT("Spline"));
			}
		}
	}
}

UThumbnailInfo * FAssetTypeActions_PathFollowerBlueprint::GetThumbnailInfo(UObject * Asset) const
{
	return nullptr;
}

UFactory * FAssetTypeActions_PathFollowerBlueprint::GetFactoryForBlueprintType(UBlueprint * InBluePrint) const
{
	return nullptr;
}

bool FAssetTypeActions_PathFollowerBlueprint::ShouldUseDataOnlyEditor(const UBlueprint * Blueprint) const
{
	return false;
}
