// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#include "ablAbilityDebug.h"

#include "Engine/World.h"

#include "DrawDebugHelpers.h"

#if !UE_BUILD_SHIPPING

static TAutoConsoleVariable<int32> CVarDrawDebugQueryShapes(TEXT("Able.DrawQueryDebug"), 0, TEXT("1 to enable drawing of debug query volumes for abilities."));
static TAutoConsoleVariable<float> CVarDrawDebugQueryShapesLifetime(TEXT("Able.QueryDebugShapeLifetime"), 0.25f, TEXT("How long, in seconds, a query debug shape should be visible."));
bool FAblAbilityDebug::ForceDrawQueries = false;

void FAblAbilityDebug::EnableDrawQueries(bool Enable)
{
	ForceDrawQueries = Enable;
}

bool FAblAbilityDebug::ShouldDrawQueries()
{
	return ForceDrawQueries || CVarDrawDebugQueryShapes.GetValueOnAnyThread() != 0;
}

float FAblAbilityDebug::GetDebugQueryLifetime()
{
	return CVarDrawDebugQueryShapesLifetime.GetValueOnAnyThread();
}

void FAblAbilityDebug::DrawSphereQuery(const UWorld* World, const FTransform& QueryTransform, float Radius)
{
	DrawDebugSphere(World, QueryTransform.GetLocation(), Radius, 32, FColor::Red, false, GetDebugQueryLifetime());
}

void FAblAbilityDebug::DrawBoxQuery(const UWorld* World, const FTransform& QueryTransform, const FVector& HalfExtents)
{
	DrawDebugBox(World, QueryTransform.GetLocation(), HalfExtents, FQuat::Identity, FColor::Red, false, GetDebugQueryLifetime());
}

void FAblAbilityDebug::Draw2DConeQuery(const UWorld* World, const FTransform& QueryTransform, float FOV, float Length)
{
	const FVector Forward = QueryTransform.GetRotation().GetForwardVector();
	const FVector Up = QueryTransform.GetRotation().GetUpVector();

	int32 Segments = FOV > 180.0f ? 32 : 16; // Double our segments if we're doing a larger circle.
	const float FOVRadians = FMath::DegreesToRadians(FOV);
	const float RadianStep = FOVRadians / (float)Segments;
	float CurrentRadians = -FOVRadians * 0.5f;

	// First and last lines are special and close our shape.
	DrawDebugLine(World, QueryTransform.GetLocation(), QueryTransform.GetLocation() + Forward.RotateAngleAxis(FMath::RadiansToDegrees(CurrentRadians), Up) * Length, FColor::Red, false, GetDebugQueryLifetime());
	FVector LineStart, LineEnd;
	for (int32 i = 0; i < Segments; ++i)
	{
		LineStart = QueryTransform.GetLocation() + Forward.RotateAngleAxis(FMath::RadiansToDegrees(CurrentRadians), Up) * Length;
		LineEnd = QueryTransform.GetLocation() + Forward.RotateAngleAxis(FMath::RadiansToDegrees(CurrentRadians + RadianStep), Up) * Length;
		DrawDebugLine(World, LineStart, LineEnd, FColor::Red, false, GetDebugQueryLifetime());
		CurrentRadians += RadianStep;
	}
	DrawDebugLine(World, QueryTransform.GetLocation(), QueryTransform.GetLocation() + Forward.RotateAngleAxis(FMath::RadiansToDegrees(CurrentRadians), Up) * Length, FColor::Red, false, GetDebugQueryLifetime());
}

void FAblAbilityDebug::DrawConeQuery(const UWorld* World, const FTransform& QueryTransform, float FOV, float Length, float Height)
{
	float HeightAngle = FMath::Atan2(Height, Length);
	if (FOV < 180.0f)
	{
		DrawDebugAltCone(World, QueryTransform.GetLocation(), FRotator(QueryTransform.GetRotation()), Length, FOV, HeightAngle, FColor::Red, false, GetDebugQueryLifetime());
	}
	else
	{
		const float Radius = FMath::Max(Height, Length);
		DrawDebugSphere(World, QueryTransform.GetLocation(), Radius, 32, FColor::Red, false, GetDebugQueryLifetime());
		FTransform ReversedForward(-QueryTransform.GetRotation().GetForwardVector(), QueryTransform.GetRotation().GetRightVector(), QueryTransform.GetRotation().GetUpVector(), QueryTransform.GetLocation());
		DrawDebugAltCone(World, QueryTransform.GetLocation(), FRotator(ReversedForward.GetRotation()), Length, 360.0f - FOV, HeightAngle, FColor::Black, false, GetDebugQueryLifetime());
	}
}

void FAblAbilityDebug::DrawRaycastQuery(const UWorld* World, const FTransform& QueryTransform, float Length)
{
	const FVector LineStart = QueryTransform.GetLocation();
	const FVector LineEnd = LineStart + QueryTransform.GetRotation().GetForwardVector() * Length;
	DrawDebugLine(World, LineStart, LineEnd, FColor::Red, false, GetDebugQueryLifetime());
}

void FAblAbilityDebug::DrawCapsuleQuery(const UWorld* World, const FTransform& QueryTransform, float Radius, float Height)
{
	const float HalfHeight = Height * 0.5f;
	DrawDebugCapsule(World, QueryTransform.GetLocation(), HalfHeight, Radius, QueryTransform.GetRotation(), FColor::Red, false, GetDebugQueryLifetime());
}

void FAblAbilityDebug::DrawBoxSweep(const UWorld* World, const FTransform& Start, const FTransform& End, const FVector& HalfExtents)
{
	DrawDebugBox(World, Start.GetLocation(), HalfExtents, FQuat::Identity, FColor::Red, false, GetDebugQueryLifetime());
	DrawDebugDirectionalArrow(World, Start.GetLocation(), End.GetLocation(), 50.0f, FColor::Red, false, GetDebugQueryLifetime());
	DrawDebugBox(World, End.GetLocation(), HalfExtents, FQuat::Identity, FColor::Red, false, GetDebugQueryLifetime());
}

void FAblAbilityDebug::DrawSphereSweep(const UWorld* World, const FTransform& Start, const FTransform& End, float Radius)
{
	DrawDebugSphere(World, Start.GetLocation(), Radius, 32, FColor::Red, false, GetDebugQueryLifetime());
	DrawDebugDirectionalArrow(World, Start.GetLocation(), End.GetLocation(), 50.0f, FColor::Red, false, GetDebugQueryLifetime());
	DrawDebugSphere(World, End.GetLocation(), Radius, 32, FColor::Red, false, GetDebugQueryLifetime());
}

void FAblAbilityDebug::DrawCapsuleSweep(const UWorld* World, const FTransform& Start, const FTransform& End, float Radius, float Height)
{
	DrawDebugCapsule(World, Start.GetLocation(), Height * 0.5f, Radius, Start.GetRotation(), FColor::Red, false, GetDebugQueryLifetime());
	DrawDebugDirectionalArrow(World, Start.GetLocation(), End.GetLocation(), 50.0f, FColor::Red, false, GetDebugQueryLifetime());
	DrawDebugCapsule(World, End.GetLocation(), Height * 0.5f, Radius, End.GetRotation(), FColor::Red, false, GetDebugQueryLifetime());
}

#endif