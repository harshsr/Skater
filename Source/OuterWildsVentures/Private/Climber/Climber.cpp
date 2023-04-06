// Fill out your copyright notice in the Description page of Project Settings.


#include "Climber/Climber.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

// Sets default values
AClimber::AClimber(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer.SetDefaultSubobjectClass<UClimberCMC>(ACharacter::CharacterMovementComponentName))
{
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	
	GetCharacterMovement()->bOrientRotationToMovement = true;
	
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("Boom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;
	
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	Camera->SetupAttachment(CameraBoom);
	Camera->bUsePawnControlRotation = false;

	ClimberMovementComponent = Cast<UClimberCMC>(GetCharacterMovement());
}

void AClimber::BeginPlay()
{
	Super::BeginPlay();

	// Add input mapping context
	if (const APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if(UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			InputSubsystem->AddMappingContext(ClimberInputMappingContext,0);
		}
	}
}

void AClimber::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AClimber::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if(UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveLongitudinalAction,ETriggerEvent::Triggered,this,&AClimber::MoveLongitudinal);

		EnhancedInputComponent->BindAction(MoveLateralAction,ETriggerEvent::Triggered,this,&AClimber::MoveLateral);

		EnhancedInputComponent->BindAction(CameraPitchAction,ETriggerEvent::Triggered,this,&AClimber::CameraPitch);

		EnhancedInputComponent->BindAction(CameraYawAction,ETriggerEvent::Triggered,this,&AClimber::CameraYaw);

		EnhancedInputComponent->BindAction(JumpAction,ETriggerEvent::Triggered,this,&AClimber::JumpFunc);

		EnhancedInputComponent->BindAction(StartClimbAction,ETriggerEvent::Triggered,this,&AClimber::StartClimb);

		EnhancedInputComponent->BindAction(CancelClimbAction,ETriggerEvent::Triggered,this,&AClimber::CancelClimb);
	}
} 

void AClimber::MoveLongitudinal(const FInputActionValue& Value)
{
	if (Controller == nullptr || Value.Get<float>() == 0.0f)
	{
		return;
	}

	FVector Direction;
	
	if (ClimberMovementComponent->IsClimbing())
	{
		Direction = FVector::CrossProduct(ClimberMovementComponent->GetClimbSurfaceNormal(), -GetActorRightVector());
	}
	else
	{
		Direction = GetControlOrientationMatrix().GetUnitAxis(EAxis::X);
	}
	
	AddMovementInput(Direction, Value.Get<float>());
}

void AClimber::MoveLateral(const FInputActionValue& Value)
{
	if (Controller == nullptr || Value.Get<float>() == 0.0f)
	{
		return;
	}

	FVector Direction;
	if (ClimberMovementComponent->IsClimbing())
	{
		Direction = FVector::CrossProduct(ClimberMovementComponent->GetClimbSurfaceNormal(), GetActorUpVector());
	}
	else
	{
		Direction = GetControlOrientationMatrix().GetUnitAxis(EAxis::Y);
	}
	
	AddMovementInput(Direction, Value.Get<float>());
}

void AClimber::CameraYaw(const FInputActionValue& Value)
{
	AddControllerYawInput(Value.Get<float>() * CameraTurnRate * GetWorld()->GetDeltaSeconds());
}

void AClimber::CameraPitch(const FInputActionValue& Value)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Value.Get<float>() * CameraTurnRate * GetWorld()->GetDeltaSeconds());
}

void AClimber::JumpFunc(const FInputActionValue& Value)
{
	if (ClimberMovementComponent->IsClimbing())
	{
		ClimberMovementComponent->TryClimbDashing();
	}
	else
	{
		Super::Jump();
	}
}

void AClimber::StartClimb(const FInputActionValue& Value)
{
	ClimberMovementComponent->TryClimbing();
}

void AClimber::CancelClimb(const FInputActionValue& Value)
{
	ClimberMovementComponent->CancelClimbing();
}

FRotationMatrix AClimber::GetControlOrientationMatrix() const
{
	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	return FRotationMatrix(YawRotation);
}
