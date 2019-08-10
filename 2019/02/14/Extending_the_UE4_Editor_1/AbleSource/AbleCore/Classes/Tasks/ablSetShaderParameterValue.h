// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#pragma once

#include "Engine/Texture.h"
#include "UObject/ObjectMacros.h"

#include "ablSetShaderParameterValue.generated.h"

#define LOCTEXT_NAMESPACE "AblAbilityTask"

/* Base class for all our supported parameter types. */
UCLASS(Abstract)
class UAblSetParameterValue : public UObject
{
	GENERATED_BODY()
public:
	enum Type
	{
		None = 0,
		Scalar,
		Vector,
		Texture,

		TotalTypes
	};

	UAblSetParameterValue(const FObjectInitializer& ObjectInitializer)
		:Super(ObjectInitializer),
		m_Type(None)
	{ }
	virtual ~UAblSetParameterValue() { }

	FORCEINLINE Type GetType() const { return m_Type; }
	virtual const FString ToString() const { return FString(TEXT("Invalid")); }
protected:
	Type m_Type;
};

UCLASS(EditInlineNew, meta = (DisplayName = "Scalar Parameter"))
class UAblSetScalarParameterValue : public UAblSetParameterValue
{
	GENERATED_BODY()
public:
	UAblSetScalarParameterValue(const FObjectInitializer& ObjectInitializer)
		:Super(ObjectInitializer),
		m_Scalar(0.0f)
	{
		m_Type = UAblSetParameterValue::Scalar;
	}
	virtual ~UAblSetScalarParameterValue() { }

	/* Sets the Scalar value. */
	FORCEINLINE void SetScalar(float InScalar) { m_Scalar = InScalar; }
	
	/* Returns the Scalar value. */
	FORCEINLINE float GetScalar() const { return m_Scalar; }

	virtual const FString ToString() const { return FString::Printf(TEXT("%4.3f"), m_Scalar); }
private:
	UPROPERTY(EditInstanceOnly, Category = "Value", meta = (DisplayName = "Scalar"))
	float m_Scalar;
};

UCLASS(EditInlineNew, meta = (DisplayName = "Vector Parameter"))
class UAblSetVectorParameterValue : public UAblSetParameterValue
{
	GENERATED_BODY()
public:
	UAblSetVectorParameterValue(const FObjectInitializer& ObjectInitializer)
		:Super(ObjectInitializer),
		m_Vector()
	{
		m_Type = UAblSetParameterValue::Vector;
	}
	virtual ~UAblSetVectorParameterValue() { }

	/* Sets the Vector value. */
	FORCEINLINE void SetVector(const FVector& InVector) { m_Vector = InVector; }
	
	/* Returns the Vector value. */
	FORCEINLINE const FVector GetVector() const { return FVector(m_Vector); }

	virtual const FString ToString() const { return m_Vector.ToString(); }
private:
	UPROPERTY(EditInstanceOnly, Category = "Value", meta = (DisplayName = "Vector"))
	FLinearColor m_Vector;
};

UCLASS(EditInlineNew, meta = (DisplayName = "Texture Parameter"))
class UAblSetTextureParameterValue : public UAblSetParameterValue
{
	GENERATED_BODY()
public:
	UAblSetTextureParameterValue(const FObjectInitializer& ObjectInitializer)
		:Super(ObjectInitializer),
		m_Texture(nullptr)
	{
		m_Type = UAblSetParameterValue::Texture;
	}
	virtual ~UAblSetTextureParameterValue() { }

	/* Sets the Texture value. */
	FORCEINLINE void SetTexture(/*const*/ UTexture* InTexture) { m_Texture = InTexture; }
	
	/* Returns the Texture value. */
	FORCEINLINE UTexture* GetTexture() const { return m_Texture; }

	virtual const FString ToString() const { return m_Texture ? m_Texture->GetName() : Super::ToString(); }
private:
	UPROPERTY(EditInstanceOnly, Category = "Value", meta = (DisplayName = "Texture"))
	/*const*/ UTexture* m_Texture; // Fix this when the API returns const
};

#undef LOCTEXT_NAMESPACE