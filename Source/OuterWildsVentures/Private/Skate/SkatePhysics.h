// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Skaterface.h"
#include "GameFramework/Actor.h"
#include "SkatePhysics.generated.h"

class ASkater;
UENUM(BlueprintType)
enum ESkateMode
{
	Skate UMETA(DisplayName = "Skate"),
	Grind UMETA(DisplayName = "Grind")
};

UCLASS()
class ASkatePhysics : public AActor, public ISkaterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASkatePhysics();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadOnly)
	ASkater* SkaterRef;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	// Components

	// Root static mesh for physics
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Component")
	UStaticMeshComponent* RootSphere;

public:
	// Config properties

	// Force to be applied when pumping over a timeline
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Config")
	float PumpForce = 1750.0f;

	// Force to be applied when leaning
	UPROPERTY(EditAnywhere, Category = "Config")
	float LeanForce = 3500.0f;

	// Impulse to be applied when performing an Ollie
	UPROPERTY(EditAnywhere, Category = "Config")
	float OllieImpulse = 500.0f;

	// Z offset from spline to correctly position when grinding
	UPROPERTY(EditAnywhere, Category = "Config")
	float GrindZOffset = 50.0f;

	// Max velocity cap
	UPROPERTY(EditAnywhere, Category = "Config")
	float MaxVelocity = 2250.0f;

	// When getting off a grind, initiate a cool down for these many seconds before grind can be detected again.
	UPROPERTY(EditAnywhere, Category="Config")
	float GrindCooldownTargetSeconds = 1.0f;

public:
	// Properties

	// Distance tracker to track movement along grind spline
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Grind")
	float GrindCurrentDistance;

	// Initial velocity when grinding starts;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Grind")
	FVector GrindInitialVelocity;

	// Points where skate physics snaps where grinding along spline
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Grind")
	FVector GrindSnapPoint;

	// Grind spline's total length
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Grind")
	float GrindSplineLength;

	// Grind actor
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Grind")
	AActor* GrindActor;

	// When grinding, whether skate physics is moving along or against the grind actor's spline direction.
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Grind")
	bool bMovingInSplineDirection;

	// Used to perform initial grind snap to spline and is then set to true until grind ends.
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Grind")
	bool bGrindInitialSnapHappened;

	// Direction for pumping on input
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Movement")
	FVector PumpDirection;

	// Skate mode. All movement decisions are based on this mode.
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Movement")
	TEnumAsByte<ESkateMode> CurrentSkateMode = Skate;

	//
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Grind")
	bool bGrindCooldownComplete = true;

	// Current grind cooldown tracker
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Grind")
	float GrindCurrentCooldownTracker;

	// Used to perform ollie when grinding because disabling physics takes a frame.
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Grind")
	bool bOllieNextFrame;

	// Angle array to perform ground checks. This is so that ground check won't miss on inclined ground.
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "GroundCheck")
	TArray<int> AngleArrayForGroundCheck = {-90,-75,-50,-25,0,25,50,75,90};

	// Hit normal set during ground condition check
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "GroundCheck")
	FVector GroundTraceHitNormal;

	// Tick delta
	UPROPERTY(BlueprintReadOnly)
	float TickDelta;

public:
	// Functions

	// First thing on tick, perform a check that looks for a grind encounter
	UFUNCTION(BlueprintCallable, Category = "Grind")
	void CheckGrinding();

	// Physics linear velocity cap performed on tick
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void ClampVelocity();

	// Push a little down to stick skate physics to steep ramps
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StickToGround();

	// Change skate mode and perform related operaions
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void ChangeSkateMode(TEnumAsByte<ESkateMode> NewSkateMode);

	// Grind
	UFUNCTION(BlueprintCallable, Category = "Grind")
	void Grind();

	// Get skate mode of SkatePhysics.
	UFUNCTION(Category = "Getter")
	TEnumAsByte<ESkateMode> GetCurrentSkateMode() const;

public:

	// Interface Functions

	virtual  void Ollie() override;
	virtual void Lean(bool bAtRest, float AxisValue) override;
	virtual void AirTrajectoryPrediction() override;
	virtual void FlipJump() override;
	virtual FVector GetSkatePhysicsVelocity() override;
	virtual FHitResult ReportGroundCondition() override;

};
