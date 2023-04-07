// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputMappingContext.h"
#include "SkatePhysics.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "Skater.generated.h"

UCLASS()
class ASkater : public APawn, public ISkaterface
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ASkater();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	// Components

	// Character mesh
	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* MaxMesh;

	// Board Mesh
	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* BoardMesh;

	// Rotation Tracker. All rotation changes are applied to this instead of the whole pawn. This allows free movement of character meshes and camera.
	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category = "Components")
	USceneComponent* RotationTracker;

	// Boom
	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category = "Components")
	USpringArmComponent* CameraBoom;

	// Camera
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* MainCamera;

	// Scene root
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* Root;

protected:
	// Input

	// Input mapping context
	UPROPERTY(EditAnywhere, Category="Input")
	UInputMappingContext* SkateInputMappingContext;

	// Pump action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* PumpAction;
	
	// Lean action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* LeanAction;
	
	// Ollie action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* OllieAction;


	// Input Functions

	void PumpActionTriggered(const FInputActionValue& Value);
	void LeanActionTriggered(const FInputActionValue& Value);
	void LeanActionCompleted(const FInputActionValue& Value);
	void OllieActionCompleted(const FInputActionValue& Value);

	
public:
	// Config variables

	// A pump cooldown target to prevent multiple pumps
	UPROPERTY(EditAnywhere, Category = "Config")
	float PumpCooldownTarget = 1.5f;


	// Properties

	// A pump cooldown tracker to prevent multiple pumps
	UPROPERTY(BlueprintReadWrite, Category = "Pump")
	float PumpCooldownTracker;

	// if pumped
	UPROPERTY(BlueprintReadWrite, Category = "Pump")
	bool bPumped;

	// Ground trace hit normal checked on skate physics
	UPROPERTY(BlueprintReadWrite, Category = "Ground Condition")
	FVector GroundTraceHitNormal;

	// Skate Physics actor reference
	UPROPERTY(BlueprintReadWrite)
	ASkatePhysics* SkatePhysics;

	// Tick delta
	UPROPERTY(BlueprintReadOnly)
	float TickDelta;

	// bGrounded
	UPROPERTY(BlueprintReadWrite, Category = "Ground Condition")
	bool bGrounded;

public:
	// Functions

	// Adjust rotations and grounded conditions and flip jumps by calling ReportGroundCondition on Skate Physics.
	UFUNCTION(BlueprintCallable, Category = "Ground Condition")
	void GroundAdjust();

	// Input cooldown. Mainly for pump.
	UFUNCTION(BlueprintCallable, Category = "Input")
	void InputCoolDowns();

	// Move pawn with skate physics. Perform first on tick.
	UFUNCTION(BlueprintCallable)
	void MoveWithSkatePhysics();

	// Camera rotation along with interpolation
	UFUNCTION(BlueprintImplementableEvent, Category = "Rotation")
	void CameraRotation();

	// Mesh rotation along with interpolation
	UFUNCTION(BlueprintImplementableEvent, Category = "Rotation")
	void MeshRotation();

	// Landed procedures
	UFUNCTION(BlueprintCallable, Category = "Ground Condition")
	void JustLanded();

	// Interface Functions

	virtual  void OrientToLanding(FHitResult HitResult, float TimeToHit, FVector ProjectedForwardVector) override;
	virtual FVector GetRotationTrackerForwardVector() override;
	virtual FVector GetRotationTrackerRightVector() override;
	virtual FVector GetRotationTrackerUpVector() override;
	virtual bool GetGrounded() override;

	// Internal Functions

protected:
	//TEMPORARY animation asset refs
	
	UPROPERTY(EditDefaultsOnly, Category="TempAnim")
	UAnimationAsset* GrabAnim;
};
