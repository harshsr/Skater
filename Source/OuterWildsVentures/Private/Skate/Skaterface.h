// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Skaterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class USkaterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ISkaterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	// Called on skater physics to add forward force for acceleration
	UFUNCTION(BlueprintImplementableEvent, Category = "Input")
	void Pump();

	// Called on skater physics to add lateral force as if Skater is leaning
	UFUNCTION(Category="Input")
	virtual void Lean(bool bAtRest, float AxisValue);

	// Called on skater physics to add a vertical force to perform an ollie
	UFUNCTION(Category="Input")
	virtual void Ollie();

	// Get Skater's internal rotation tracker's forward vector
	UFUNCTION( Category = "Getter")
	virtual FVector GetRotationTrackerForwardVector();

	// Get Skater's internal rotation tracker's right vector
	UFUNCTION( Category = "Getter")
	virtual FVector GetRotationTrackerRightVector();

	// Get Skater's internal rotation tracker's up vector
	UFUNCTION( Category = "Getter")
	virtual FVector GetRotationTrackerUpVector();

	// Called on skater physics from skater to perform ground checks and report that to skater make grounded and rotation decisions
	UFUNCTION( Category = "Helper")
	virtual FHitResult ReportGroundCondition();

	// Called on skater physics to get physics linear velocity
	UFUNCTION( Category = "Getter")
	virtual FVector GetSkatePhysicsVelocity();

	// Called on skater physics to tell that this is a "Flip Back" jump so skater physics adjusts velocity to avoid overshoot
	UFUNCTION(Category = "InAir")
	virtual void FlipJump();

	// When in air, this is run on skater physics through skater to predict where skater will land and adjust mid air rotation accordingly.
	UFUNCTION(Category = "InAir")
	virtual void AirTrajectoryPrediction();

	// After AirTrajectoryPrediction, call this to orient skater for a smooth landing
	UFUNCTION(Category = "InAir")
	virtual void OrientToLanding(FHitResult HitResult,float TimeToHit,FVector ProjectedForwardVector);

	UFUNCTION()
	virtual bool GetGrounded();
};
