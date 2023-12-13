// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

// Check if a mask has a single flag
#define TEST_BIT(Bitmask, Bit) (((Bitmask) & (1 << static_cast<int32>(Bit))) > 0)

// Check if a mask contains another mask
#define TEST_BITS(Bitmask, Bits) (((Bitmask) & Bits) == Bits)

// Set a specific bit
#define SET_BIT(Bitmask, Bit) (Bitmask |= 1 << static_cast<int32>(Bit))

// Clear a specific bit
#define CLEAR_BIT(Bitmask, Bit) (Bitmask &= ~(1 << static_cast<int32>(Bit)))

#include "CoreMinimal.h"
#include "LocalContext.h"
#include "RelativeContext.h"
#include "FGGameplayMath/Constants.h"
#include "FGGameplayMath/State/StateDemonstrator.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ContextHelpers.generated.h"

/**
 * 
 */
UCLASS()
class FGGAMEPLAYMATH_API UContextHelpers : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Context", meta = ( DefaultToSelf = Target ))
	static int32 GetLocalContext(const AActor* Target)
	{
		int32 ReturnContext = 0;

		if(const auto StateDemonstrator = Cast<AStateDemonstrator>(Target))
		{
			const auto Health = StateDemonstrator->Health;
			
			if(Health < GContextDyingThreshold)
				SET_BIT(ReturnContext, ELocalContext::NearDeath);
			else if (Health < GContextHurtThreshold)
				SET_BIT(ReturnContext, ELocalContext::Hurt);
		}
		
		return ReturnContext;
	}
	
	UFUNCTION(BlueprintCallable, Category = "Context", meta = ( DefaultToSelf = Target ))
	static int32 GetRelativeContext(const AActor* Target, const AActor* Other)
	{
		int32 ReturnContext = 0;

		const auto TargetLocation = Target->GetActorLocation();
		const auto OtherLocation = Other->GetActorLocation();
		const auto Direction = OtherLocation - TargetLocation;

		const auto LocalSpaceLocation = Target->GetActorTransform().InverseTransformPosition(OtherLocation);

		if(LocalSpaceLocation.X > 0)
			SET_BIT(ReturnContext, ERelativeContext::InFront);
		else if(LocalSpaceLocation.X < 0)
			SET_BIT(ReturnContext, ERelativeContext::Behind);

		if(LocalSpaceLocation.Y > 0)
			SET_BIT(ReturnContext, ERelativeContext::ToTheRight);
		else if(LocalSpaceLocation.Y < 0)
			SET_BIT(ReturnContext, ERelativeContext::ToTheLeft);
		
		if(LocalSpaceLocation.Z > 0)
			SET_BIT(ReturnContext, ERelativeContext::Above);
		else if(LocalSpaceLocation.Z < 0)
			SET_BIT(ReturnContext, ERelativeContext::Below);
		
		const auto Distance = Direction.Length();
		if(Distance > GContextRange)
			SET_BIT(ReturnContext, ERelativeContext::Far);
		else
			SET_BIT(ReturnContext, ERelativeContext::Close);
		
		const auto Angle = FindAngle(Target->GetActorForwardVector(), Direction.GetSafeNormal());
		const auto HalfGlobalAngle = GContextAngle * .5f;
		if(Angle < HalfGlobalAngle && Angle > -HalfGlobalAngle)
			SET_BIT(ReturnContext, ERelativeContext::Seen);
		else
			SET_BIT(ReturnContext, ERelativeContext::Unseen);

		const auto FacingDot = FVector::DotProduct(Target->GetActorForwardVector(), Other->GetActorForwardVector());
		if(FacingDot > GContextDirectionThreshold)
			SET_BIT(ReturnContext, ERelativeContext::FacingSame);
		else if(FacingDot < -GContextDirectionThreshold)
			SET_BIT(ReturnContext, ERelativeContext::FacingOpposite);
		
		return ReturnContext;
	}

	UFUNCTION(BlueprintCallable, Category = "Context")
	static float FindAngle(const FVector SelfForward, const FVector TargetForward)
	{
		auto Dot = FVector::DotProduct(SelfForward, TargetForward);
		Dot = FMath::Clamp(Dot, -1.f, 1.f);
		return FMath::RadiansToDegrees(FMath::Acos(Dot));
	}

	// Used for checking a group of flags
	UFUNCTION(BlueprintCallable, Category = "Context")
	static bool ContextPredicate(const int32 Test, const int32 Value)
    {
		return TEST_BITS(Test, Value);
    }

	// Used for checking a single flag
	UFUNCTION(BlueprintCallable, Category = "Context")
	static bool FlagPredicate(const int32 Test, const ERelativeContext Value)
	{
		return TEST_BIT(Test, Value);
	}	
};