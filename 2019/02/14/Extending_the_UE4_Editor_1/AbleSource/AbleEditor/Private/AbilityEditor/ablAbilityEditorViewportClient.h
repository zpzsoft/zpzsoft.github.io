// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#pragma once

#include "EditorViewportClient.h"
#include "PreviewScene.h"

class FAblAbilityEditor;
class SAbilityEditorViewport;
class UExponentialHeightFogComponent;
class UStaticMeshComponent;

class FAbilityEditorPreviewScene : public FPreviewScene
{
public:
	FAbilityEditorPreviewScene(ConstructionValues CVS);

private:
	/** Editor accessory components **/
	UStaticMeshComponent*				m_EditorFloorComp;
	UStaticMeshComponent*				m_EditorSkyComp;
	UExponentialHeightFogComponent*		m_EditorHeightFogComponent;
};

class FAbilityEditorViewportClient : public FEditorViewportClient
{
public:
	FAbilityEditorViewportClient(FAbilityEditorPreviewScene& InPreviewScene, TWeakPtr<FAblAbilityEditor> InAbilityEditor, const TSharedRef<SAbilityEditorViewport>& InAbilityEditorViewport);
	virtual ~FAbilityEditorViewportClient();

	virtual void Draw(FViewport* InViewport, FCanvas* Canvas) override;
	virtual void Tick(float DeltaSeconds) override;
	
	void CaptureThumbnail();
	void TickWorld(float DeltaSeconds);
private:
	bool m_CaptureThumbnail;
	TWeakPtr<FAblAbilityEditor> m_AbilityEditor;
};