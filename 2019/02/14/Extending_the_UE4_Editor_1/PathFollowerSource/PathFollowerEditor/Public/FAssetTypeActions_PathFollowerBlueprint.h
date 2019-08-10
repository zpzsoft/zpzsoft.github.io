// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AssetTypeActions/AssetTypeActions_Blueprint.h"

/**
 * 
 */
class FAssetTypeActions_PathFollowerBlueprint : public FAssetTypeActions_Blueprint
{
public:
	FAssetTypeActions_PathFollowerBlueprint(EAssetTypeCategories::Type AssetCategory);

	~FAssetTypeActions_PathFollowerBlueprint();

	// IAssetTypeActions Implementation
	virtual FText GetName() const override;
	virtual FColor GetTypeColor() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual void OpenAssetEditor(const TArray<UObject *>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override;
	virtual uint32 GetCategories() override {return m_AssetCategory;}
	virtual UThumbnailInfo* GetThumbnailInfo(UObject* Asset) const override;
	// End IAssetTypeActions Implementation

	// FAssetTypeActions_Blueprint interface
	virtual UFactory* GetFactoryForBlueprintType(UBlueprint* InBluePrint) const override;

private:
	bool ShouldUseDataOnlyEditor(const UBlueprint* Blueprint) const;

	EAssetTypeCategories::Type m_AssetCategory;
};
