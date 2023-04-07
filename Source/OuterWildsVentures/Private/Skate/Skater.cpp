// Fill out your copyright notice in the Description page of Project Settings.


#include "Skate/Skater.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
ASkater::ASkater()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>("Root");
	MaxMesh = CreateDefaultSubobject<USkeletalMeshComponent>("Max");
	BoardMesh = CreateDefaultSubobject<USkeletalMeshComponent>("Board");
	RotationTracker = CreateDefaultSubobject<USceneComponent>("RotationTracker");
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>("Boom");
	MainCamera = CreateDefaultSubobject<UCameraComponent>("Camera");

	RootComponent = Root;
	MaxMesh->SetupAttachment(Root);
	BoardMesh->SetupAttachment(MaxMesh);

	RotationTracker->SetupAttachment(Root);

	CameraBoom->SetupAttachment(Root);
	MainCamera->SetupAttachment(CameraBoom);
	

	

}

// Called when the game starts or when spawned
void ASkater::BeginPlay()
{
	Super::BeginPlay();

	// Skate physics reference set
	SkatePhysics = Cast<ASkatePhysics>(UGameplayStatics::GetActorOfClass(GetWorld(),ASkatePhysics::StaticClass()));
	if (SkatePhysics == nullptr)
	{
		GEngine->AddOnScreenDebugMessage(1,25,FColor::Red,FString("Missing Physics"),false);
	}

	// Add input mapping context
	if (const APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if(UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			InputSubsystem->AddMappingContext(SkateInputMappingContext,0);
		}
	}
}

// Called every frame
void ASkater::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TickDelta = DeltaTime;

	// Move pawn with skate physics.
	MoveWithSkatePhysics();

	// Input Cooldown
	InputCoolDowns();

	// Adjust rotations and grounded conditions and flip jumps
	GroundAdjust();

	// Rotate camera to rotation tracker
	CameraRotation();

	// Rotate mesh to rotation tracker
	MeshRotation();

}

// Called to bind functionality to input
void ASkater::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if(UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Pump
		EnhancedInputComponent->BindAction(PumpAction,ETriggerEvent::Triggered,this,&ASkater::PumpActionTriggered);

		// Lean
		EnhancedInputComponent->BindAction(LeanAction,ETriggerEvent::Triggered,this,&ASkater::LeanActionTriggered);
		EnhancedInputComponent->BindAction(LeanAction,ETriggerEvent::Completed,this,&ASkater::LeanActionCompleted);

		// Ollie
		EnhancedInputComponent->BindAction(OllieAction,ETriggerEvent::Completed,this,&ASkater::OllieActionCompleted);
	}

}

void ASkater::PumpActionTriggered(const FInputActionValue& Value)
{
	if (SkatePhysics && !bPumped && bGrounded)
	{
		if (SkatePhysics->GetSkatePhysicsVelocity().Length()>100.0)
		{
			ISkaterface::Execute_Pump(SkatePhysics);
			bPumped = true;
		}
		else
		{
			ISkaterface::Execute_Pump(SkatePhysics);
			bPumped = true;
		}
	}
}

void ASkater::LeanActionTriggered(const FInputActionValue& Value)
{
	if(bGrounded)
	{
		if(SkatePhysics)
		{
			float AxisValue = Value.Get<float>();
			//GEngine->AddOnScreenDebugMessage(1,TickDelta,FColor::Black,FString::SanitizeFloat(AxisValue));
			if(SkatePhysics->GetSkatePhysicsVelocity().Length()>50.0)
			{
				SkatePhysics->Lean(false,AxisValue);
				FRotator BoardRotationTarget = UKismetMathLibrary::MakeRotator(MaxMesh->GetComponentRotation().Roll,MaxMesh->GetComponentRotation().Pitch,(MaxMesh->GetComponentRotation().Yaw)+(AxisValue*25));
				BoardMesh->SetWorldRotation(UKismetMathLibrary::RInterpTo(BoardMesh->GetComponentRotation(),BoardRotationTarget,TickDelta,5.0));
			}
			else
			{
				RotationTracker->AddWorldRotation(FRotator(0.0,AxisValue*2,0),false,nullptr,ETeleportType::TeleportPhysics);
				SkatePhysics->Lean(true, AxisValue);
			}
		}
	}
}

void ASkater::LeanActionCompleted(const FInputActionValue& Value)
{
	if (SkatePhysics)
	{
		SkatePhysics->Lean(false,0.0);
		BoardMesh->SetWorldRotation(MaxMesh->GetComponentRotation(),false,nullptr,ETeleportType::TeleportPhysics);
	}
}

void ASkater::OllieActionCompleted(const FInputActionValue& Value)
{
	if (SkatePhysics)
	{
		if (bGrounded)
		{
			SkatePhysics->Ollie();
		}
	}
}

void ASkater::GroundAdjust()
{
	if(SkatePhysics)
	{
		FHitResult GroundHitResult = SkatePhysics->ReportGroundCondition();
		if(GroundHitResult.bBlockingHit)
		{
			// Skater is on ground
			GroundTraceHitNormal = GroundHitResult.ImpactNormal;
			if (!bGrounded)
			{
				// Skater was previously in air and just hit a grounded check.
				JustLanded();
				bGrounded = true;
			}
			FVector PhysicsVelocity = SkatePhysics->GetSkatePhysicsVelocity();
			if (PhysicsVelocity.Length()>50.0f)
			{
				// Align rotation tracker with SkatePhysics's velocity.
				FRotator TargetRotation = UKismetMathLibrary::MakeRotFromXZ(PhysicsVelocity.GetSafeNormal(),GroundTraceHitNormal);
				RotationTracker->SetWorldRotation(TargetRotation,false,nullptr,ETeleportType::TeleportPhysics);
			}

		}
		else
		{
			if (bGrounded)
			{
				// Just left ground
				bGrounded = false;
			}

			// If SkatePhysics has positive Z velocity and ground trace hit normal is nearly horizontal, Skater should perform a flip jump back onto the ramp.
			// For this, we perform two checks
			
			FVector PhysicsVelocity = SkatePhysics->GetSkatePhysicsVelocity();
			bool bFlipJumpCheck1 = PhysicsVelocity.Z>0.0;

			// Project ground hit normal on the horizontal plane and get length
			FVector GroundHitProjectionOnHorizontal = UKismetMathLibrary::ProjectPointOnToPlane(GroundTraceHitNormal,FVector{1.0,1.0,0.0},FVector{0.0,0.0,1.0});
			float ProjectedLength = GroundHitProjectionOnHorizontal.Length();
			bool bFlipJumpCheck2 = ProjectedLength>0.95;
			
			if (bFlipJumpCheck1 && bFlipJumpCheck2)
			{
				// Perform flip jump and tell SkatePhysics to adjust velocity accordingly.
				SkatePhysics->FlipJump();

				// TODO Temp Anim
				if (GrabAnim)
				{
					MaxMesh->PlayAnimation(GrabAnim,false);
				}
			}
			else
			{
				// Run trajectory prediction
				SkatePhysics->AirTrajectoryPrediction();
			}
			
		}
	}
}

void ASkater::InputCoolDowns()
{
	if (bPumped)
	{
		PumpCooldownTracker += TickDelta;
		if (PumpCooldownTracker > PumpCooldownTarget)
		{
			PumpCooldownTracker = 0.0f;
			bPumped = false;
		}
	}
}

void ASkater::MoveWithSkatePhysics()
{
	if(SkatePhysics)
	{
		FVector PhysicsLocation = SkatePhysics->GetActorLocation();
		SetActorLocation((FMath::VInterpTo(GetActorLocation(),PhysicsLocation,TickDelta,1000.0)),false,nullptr,ETeleportType::TeleportPhysics);
	}
}

/*
void ASkater::CameraRotation()
{
	if(bGrounded)
	{
		FRotator TargetRotation = UKismetMathLibrary::MakeRotator(0.0,0.0,RotationTracker->GetComponentRotation().Yaw);
		FRotator CurrentRotation = UKismetMathLibrary::MakeRotator(0.0,0.0,CameraBoom->GetComponentRotation().Yaw);
		//CameraBoom->SetWorldRotation(UKismetMathLibrary::RInterpTo(CurrentRotation,TargetRotation,TickDelta,10.0));
		CameraBoom->SetWorldRotation(TargetRotation);
	}
	else
	{
		FRotator TargetRotation = UKismetMathLibrary::MakeRotator(0.0,0.0,RotationTracker->GetComponentRotation().Yaw);
		FRotator CurrentRotation = UKismetMathLibrary::MakeRotator(0.0,0.0,CameraBoom->GetComponentRotation().Yaw);
		//CameraBoom->SetWorldRotation(UKismetMathLibrary::RInterpTo(CurrentRotation,TargetRotation,TickDelta,1.0));
		CameraBoom->SetWorldRotation(TargetRotation);
	}
}
*/

/*
void ASkater::MeshRotation()
{
	if(bGrounded)
	{
		//CameraBoom->SetWorldRotation(UKismetMathLibrary::RInterpTo(MaxMesh->GetComponentRotation(),RotationTracker->GetComponentRotation(),TickDelta,10.0));
		MaxMesh->SetWorldRotation(RotationTracker->GetComponentRotation());
	}
	else
	{
		//CameraBoom->SetWorldRotation(UKismetMathLibrary::RInterpTo(MaxMesh->GetComponentRotation(),RotationTracker->GetComponentRotation(),TickDelta,1.0));
		MaxMesh->SetWorldRotation(RotationTracker->GetComponentRotation());
	}
}
*/

void ASkater::JustLanded()
{
	
}

void ASkater::OrientToLanding(FHitResult HitResult, float TimeToHit, FVector ProjectedForwardVector)
{
	if (SkatePhysics)
	{
		switch (Cast<ASkatePhysics>(SkatePhysics)->GetCurrentSkateMode())
		{
		case Skate:
			{
				if (HitResult.bBlockingHit)
				{
					float DotProduct = FVector::DotProduct(HitResult.Normal,FVector{0.0,0.0,1.0});

					// This check ensures that skater doesn't try to orient to 'near vertical' walls
					if (!(DotProduct<0.1 && DotProduct>-0.1))
					{
						FRotator TargetRotation = UKismetMathLibrary::MakeRotFromXZ(ProjectedForwardVector,HitResult.Normal);
						RotationTracker->SetWorldRotation(TargetRotation,false,nullptr,ETeleportType::TeleportPhysics);
					}
				}
				break;
			}
		case Grind:
			{
				break;
			}
		default: break;
		}
		
	}
}

FVector ASkater::GetRotationTrackerForwardVector()
{
	return RotationTracker->GetForwardVector();
}

FVector ASkater::GetRotationTrackerRightVector()
{
	return RotationTracker->GetRightVector();
}

FVector ASkater::GetRotationTrackerUpVector()
{
	return RotationTracker->GetUpVector();
}

bool ASkater::GetGrounded()
{
	return bGrounded;
}


