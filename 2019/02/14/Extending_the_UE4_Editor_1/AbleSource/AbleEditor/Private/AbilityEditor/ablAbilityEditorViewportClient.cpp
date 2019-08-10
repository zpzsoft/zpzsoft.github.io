// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#include "AbilityEditor/ablAbilityEditorViewportClient.h"

#include "AbleEditorPrivate.h"

#include "ablAbility.h"
#include "AbilityEditor/ablAbilityEditor.h"
#include "AbilityEditor/AblAbilityEditorSettings.h"
#include "AbilityEditor/SAbilityViewport.h"

#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SphereReflectionCaptureComponent.h"
#include "Components/StaticMeshComponent.h"

#include "Engine/StaticMesh.h"
#include "Runtime/Engine/Classes/Materials/Material.h"

#include "ImageUtils.h"
#include "ObjectTools.h"

FAbilityEditorPreviewScene::FAbilityEditorPreviewScene(ConstructionValues CVS)
	: FPreviewScene(CVS)
{
	// This is all from the AnimationEditorPreviewScene.cpp

	// set light options 
	DirectionalLight->SetRelativeLocation(FVector(-1024.f, 1024.f, 2048.f));
	DirectionalLight->SetRelativeScale3D(FVector(15.f));
	DirectionalLight->Mobility = EComponentMobility::Movable;
	DirectionalLight->DynamicShadowDistanceStationaryLight = 3000.f;

	SetLightBrightness(4.f);

	DirectionalLight->InvalidateLightingCache();
	DirectionalLight->RecreateRenderState_Concurrent();

	// A background sky sphere
	m_EditorSkyComp = NewObject<UStaticMeshComponent>(GetTransientPackage());
	UStaticMesh * StaticMesh = LoadObject<UStaticMesh>(NULL, TEXT("/Engine/MapTemplates/Sky/SM_SkySphere.SM_SkySphere"), NULL, LOAD_None, NULL);
	check(StaticMesh);
	m_EditorSkyComp->SetStaticMesh(StaticMesh);

	// TODO: Clone this material in case it is ever removed?
	UMaterial* SkyMaterial = LoadObject<UMaterial>(NULL, TEXT("/Engine/EditorMaterials/PersonaSky.PersonaSky"), NULL, LOAD_None, NULL);
	check(SkyMaterial);
	
	m_EditorSkyComp->SetMaterial(0, SkyMaterial);
	
	const float SkySphereScale = 1000.f;
	const FTransform SkyTransform(FRotator(0, 0, 0), FVector(0, 0, 0), FVector(SkySphereScale));
	
	AddComponent(m_EditorSkyComp, SkyTransform);

	// now add height fog component

	m_EditorHeightFogComponent = NewObject<UExponentialHeightFogComponent>(GetTransientPackage());

	m_EditorHeightFogComponent->FogDensity = 0.00075f;
	m_EditorHeightFogComponent->FogInscatteringColor = FLinearColor(3.f, 4.f, 6.f, 0.f)*0.3f;
	m_EditorHeightFogComponent->DirectionalInscatteringExponent = 16.f;
	m_EditorHeightFogComponent->DirectionalInscatteringColor = FLinearColor(1.1f, 0.9f, 0.538427f, 0.f);
	m_EditorHeightFogComponent->FogHeightFalloff = 0.01f;
	m_EditorHeightFogComponent->StartDistance = 30000.f;
	
	const FTransform FogTransform(FRotator(0, 0, 0), FVector(3824.f, 34248.f, 50000.f), FVector(80.f));
	AddComponent(m_EditorHeightFogComponent, FogTransform);

	// add capture component for reflection
	USphereReflectionCaptureComponent* CaptureComponent = NewObject<USphereReflectionCaptureComponent>(GetTransientPackage());

	const FTransform CaptureTransform(FRotator(0, 0, 0), FVector(0.f, 0.f, 100.f), FVector(1.f));
	AddComponent(CaptureComponent, CaptureTransform);
	
	CaptureComponent->MarkDirtyForRecaptureOrUpload();
	CaptureComponent->UpdateReflectionCaptureContents(GetWorld());

	// now add floor
	UStaticMesh* FloorMesh = LoadObject<UStaticMesh>(NULL, TEXT("/Engine/EditorMeshes/PhAT_FloorBox.PhAT_FloorBox"), NULL, LOAD_None, NULL);
	check(FloorMesh);

	m_EditorFloorComp = NewObject<UStaticMeshComponent>(GetTransientPackage());
	m_EditorFloorComp->SetStaticMesh(FloorMesh);

	AddComponent(m_EditorFloorComp, FTransform::Identity);
	
	m_EditorFloorComp->SetRelativeScale3D(FVector(3.f, 3.f, 1.f));

	// TODO: Again, clone since this is marked as Persona specific?
	UMaterial* Material = LoadObject<UMaterial>(NULL, TEXT("/Engine/EditorMaterials/PersonaFloorMat.PersonaFloorMat"), NULL, LOAD_None, NULL);
	check(Material);

	m_EditorFloorComp->SetMaterial(0, Material);
}

FAbilityEditorViewportClient::FAbilityEditorViewportClient(FAbilityEditorPreviewScene& InPreviewScene, TWeakPtr<FAblAbilityEditor> InAbilityEditor, const TSharedRef<SAbilityEditorViewport>& InAbilityEditorViewport)
	: FEditorViewportClient(nullptr, &InPreviewScene, StaticCastSharedRef<SEditorViewport>(InAbilityEditorViewport)),
	m_CaptureThumbnail(false)
{
	check(InAbilityEditor.IsValid());
	m_AbilityEditor = InAbilityEditor;

	ViewFOV = FMath::Clamp(m_AbilityEditor.Pin()->GetEditorSettings().m_FOV, 70.0f, 180.0f);

	// DrawHelper set up
	DrawHelper.PerspectiveGridSize = HALF_WORLD_MAX1;
	DrawHelper.AxesLineThickness = 0.0f;
	DrawHelper.bDrawGrid = true;

	EngineShowFlags.Game = 0;
	EngineShowFlags.ScreenSpaceReflections = 1;
	EngineShowFlags.AmbientOcclusion = 1;
	EngineShowFlags.SetSnap(0);

	SetRealtime(true);

	EngineShowFlags.DisableAdvancedFeatures();
	EngineShowFlags.SetSeparateTranslucency(true);
	EngineShowFlags.SetCompositeEditorPrimitives(true);
	EngineShowFlags.SetParticles(true);

	if (UWorld* PreviewWorld = InPreviewScene.GetWorld())
	{
		PreviewWorld->bAllowAudioPlayback = !m_AbilityEditor.Pin()->GetEditorSettings().m_MuteAudio;
	}

}

FAbilityEditorViewportClient::~FAbilityEditorViewportClient()
{

}

void FAbilityEditorViewportClient::Draw(FViewport* InViewport, FCanvas* Canvas)
{
	FEditorViewportClient::Draw(InViewport, Canvas);
	
	if (m_CaptureThumbnail)
	{
		if (UAblAbility* Ability = m_AbilityEditor.Pin()->GetAbility())
		{
			int32 SrcWidth = InViewport->GetSizeXY().X;
			int32 SrcHeight = InViewport->GetSizeXY().Y;

			// Read the contents of the viewport into an array.
			TArray<FColor> OrigBitmap;
			if (InViewport->ReadPixels(OrigBitmap))
			{
				check(OrigBitmap.Num() == SrcWidth * SrcHeight);

				//pin to smallest value
				int32 CropSize = FMath::Min<uint32>(SrcWidth, SrcHeight);
				//pin to max size
				int32 ScaledSize = FMath::Min<uint32>(ThumbnailTools::DefaultThumbnailSize, CropSize);

				//calculations for cropping
				TArray<FColor> CroppedBitmap;
				CroppedBitmap.AddUninitialized(CropSize*CropSize);
				//Crop the image
				int32 CroppedSrcTop = (SrcHeight - CropSize) / 2;
				int32 CroppedSrcLeft = (SrcWidth - CropSize) / 2;
				for (int32 Row = 0; Row < CropSize; ++Row)
				{
					//Row*Side of a row*byte per color
					int32 SrcPixelIndex = (CroppedSrcTop + Row)*SrcWidth + CroppedSrcLeft;
					const void* SrcPtr = &(OrigBitmap[SrcPixelIndex]);
					void* DstPtr = &(CroppedBitmap[Row*CropSize]);
					FMemory::Memcpy(DstPtr, SrcPtr, CropSize * 4);
				}
				
				//Scale image down if needed
				TArray<FColor> ScaledBitmap;
				if (ScaledSize < CropSize)
				{
					FImageUtils::ImageResize(CropSize, CropSize, CroppedBitmap, ScaledSize, ScaledSize, ScaledBitmap, true);
				}
				else
				{
					//just copy the data over. sizes are the same
					ScaledBitmap = CroppedBitmap;
				}

				// Compress.
				FCreateTexture2DParameters Params;
				Params.bDeferCompression = true;
				Ability->ThumbnailImage = FImageUtils::CreateTexture2D(ScaledSize, ScaledSize, ScaledBitmap, Ability, TEXT("ThumbnailTexture"), RF_NoFlags, Params);
				Ability->MarkPackageDirty();
			}
		}

		m_CaptureThumbnail = false;
	}
}

void FAbilityEditorViewportClient::Tick(float DeltaSeconds)
{
	TSharedPtr<FAblAbilityEditor> AbilityEditor = m_AbilityEditor.IsValid() ? m_AbilityEditor.Pin() : nullptr;
	
	if (AbilityEditor.IsValid())
	{
		float FOV = FMath::Clamp(AbilityEditor->GetEditorSettings().m_FOV, 70.0f, 180.0f);

		// See if we need to update our FOV. 
		if (!FMath::IsNearlyEqual(FOV, ViewFOV))
		{
			ViewFOV = FOV;
			Invalidate();
		}

		PreviewScene->GetWorld()->bAllowAudioPlayback = !AbilityEditor->GetEditorSettings().m_MuteAudio;

		FEditorViewportClient::Tick(DeltaSeconds);

		if (!AbilityEditor->IsPaused())
		{
			TickWorld(DeltaSeconds);
		}
	}
	else
	{
		FEditorViewportClient::Tick(DeltaSeconds);
	}
}

void FAbilityEditorViewportClient::CaptureThumbnail()
{
	if (m_AbilityEditor.IsValid())
	{
		m_CaptureThumbnail = true;
	}
}

void FAbilityEditorViewportClient::TickWorld(float DeltaSeconds)
{
	PreviewScene->GetWorld()->Tick(LEVELTICK_All, DeltaSeconds);
}
