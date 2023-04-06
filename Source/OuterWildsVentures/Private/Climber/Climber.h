// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ClimberCMC.h"
#include "InputMappingContext.h"
#include "GameFramework/Character.h"
#include "Climber.generated.h"

UCLASS()
class AClimber : public ACharacter
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* Camera;

	UFUNCTION(BlueprintPure)
	UClimberCMC* GetCustomCharacterMovement() const { return ClimberMovementComponent; }
	
public:

	virtual void Tick(float DeltaSeconds) override;
	
	AClimber(const FObjectInitializer& ObjectInitializer);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Input)
	float CameraTurnRate;

private:

	virtual void BeginPlay() override;
	
	UPROPERTY()
	UClimberCMC* ClimberMovementComponent;

protected:
	
	FRotationMatrix GetControlOrientationMatrix() const;

protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	UCameraComponent* GetCamera() const { return Camera; }

protected:
	// Input

	// Input mapping context
	UPROPERTY(EditDefaultsOnly, Category="Input")
	UInputMappingContext* ClimberInputMappingContext;

	// Forward/ Backward movement action
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveLongitudinalAction;

	// Right/ Left movement action
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveLateralAction;

	// Camera yaw action
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* CameraYawAction;

	// Camera pitch action
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* CameraPitchAction;

	// Jump action
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* JumpAction;

	// Start climb action
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* StartClimbAction;

	// Stop climbing action
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* CancelClimbAction;

	// Input listener functions

	void MoveLongitudinal(const FInputActionValue& Value);
	void MoveLateral(const FInputActionValue& Value);
	void CameraYaw(const FInputActionValue& Value);
	void CameraPitch(const FInputActionValue& Value);
	void JumpFunc(const FInputActionValue& Value);
	void StartClimb(const FInputActionValue& Value);
	void CancelClimb(const FInputActionValue& Value);

};
