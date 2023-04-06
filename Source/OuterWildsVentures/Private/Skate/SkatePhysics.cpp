// Fill out your copyright notice in the Description page of Project Settings.


#include "Skate/SkatePhysics.h"

#include "Grindface.h"
#include "Skater.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
ASkatePhysics::ASkatePhysics()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootSphere = CreateDefaultSubobject<UStaticMeshComponent>("PhysicsSphere");
	RootComponent = RootSphere;
	RootSphere->SetSimulatePhysics(true);
	RootSphere->bIgnoreRadialForce = true;
	RootSphere->bIgnoreRadialImpulse = true;
	RootSphere->bApplyImpulseOnDamage = false;
	RootSphere->SetCollisionResponseToChannel(ECC_Visibility,ECR_Ignore);
	RootSphere->SetUseCCD(true);
	RootSphere->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
	RootSphere->SetHiddenInGame(true);
	RootSphere->SetCollisionResponseToChannel(ECC_Camera,ECollisionResponse::ECR_Overlap);

}

// Called when the game starts or when spawned
void ASkatePhysics::BeginPlay()
{
	Super::BeginPlay();

	SkaterRef =  Cast<ASkater>(UGameplayStatics::GetPlayerPawn(GetWorld(),0));
}

// Called every frame
void ASkatePhysics::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TickDelta = DeltaTime;

	switch (CurrentSkateMode)
	{
	case Skate:
		{
			{
				CheckGrinding();
				ClampVelocity();
				StickToGround();

				// Perform grind cooldown
				if (!bGrindCooldownComplete)
				{
					GrindCurrentCooldownTracker += TickDelta;
					if(GrindCurrentCooldownTracker>GrindCooldownTargetSeconds)
					{
						bGrindCooldownComplete = true;
						GrindCurrentCooldownTracker = 0.0;
					}
				}

				// Perform ollie when grinding
				if (bOllieNextFrame)
				{
					this->Ollie();
				}
			}
			break;
		}
		
	case ESkateMode::Grind:
		{
			{
				Grind();
			}
			break;
		}
		
	default: break;
	}

}

void ASkatePhysics::CheckGrinding()
{
	if (bGrindCooldownComplete)
	{
		// Perform a trace towards velocity to find a grind actor
		FHitResult GrindHitResult;
		if (GetWorld()->LineTraceSingleByChannel(GrindHitResult,this->GetActorLocation(),this->GetActorLocation()+RootSphere->GetPhysicsLinearVelocity(),ECC_GameTraceChannel1))
		{
			if (GrindHitResult.bBlockingHit && GrindHitResult.Distance<50.0)
			{
				GrindActor = GrindHitResult.GetActor();
				
				FVector SplineTangent = IGrindface::Execute_FindSplineTangentNearHitLocation(GrindActor,GrindHitResult.ImpactPoint);

				// Check 1 = has enough velocity to grind
				bool bCheck1 = RootSphere->GetPhysicsLinearVelocity().Length() > 250.0f;

				// Check 2 = Velocity direction and spline direction is within 25 degree angle of each other
				bool bCheck2 = (UKismetMathLibrary::Abs( FVector::DotProduct(SplineTangent,RootSphere->GetPhysicsLinearVelocity().GetSafeNormal()))) > (0.9);
				
				if (bCheck1 && bCheck2)
				{
					// Decide if Skater intends to move in direction of grind spline or opposite of it.
					bMovingInSplineDirection = (UKismetMathLibrary::Abs( FVector::DotProduct(SplineTangent,RootSphere->GetPhysicsLinearVelocity().GetSafeNormal()))) > (0.0);
					
					GrindSnapPoint = IGrindface::Execute_GetInitialSnapPoint(GrindActor,GrindHitResult.ImpactPoint);
					GrindSplineLength = IGrindface::Execute_GetSplineLength(GrindActor);
					GrindCurrentDistance = IGrindface::Execute_GetInitialHitDistanceAlongSpline(GrindActor,GrindHitResult.ImpactPoint);

					// Change skate mode to grind
					ChangeSkateMode(ESkateMode::Grind);

					bGrindInitialSnapHappened = false;
				}
			}
		}
	}
}

void ASkatePhysics::ClampVelocity()
{
	float NewVelocityMagnitude = FMath::Clamp(RootSphere->GetPhysicsLinearVelocity().Length(),0.0,MaxVelocity);
	RootSphere->SetPhysicsLinearVelocity(RootSphere->GetPhysicsLinearVelocity().GetSafeNormal()*NewVelocityMagnitude);
}

void ASkatePhysics::StickToGround()
{
	if (Cast<ASkater>(UGameplayStatics::GetPlayerPawn(GetWorld(),0))->GetGrounded())
	{
		FVector Force =  SkaterRef->RotationTracker->GetUpVector() * -1000;
		RootSphere->AddForce(Force,NAME_None,true);
	}
}

void ASkatePhysics::ChangeSkateMode(TEnumAsByte<ESkateMode> NewSkateMode)
{
	CurrentSkateMode = NewSkateMode;
	switch (CurrentSkateMode)
	{
	case Skate:
		{
			RootSphere->SetSimulatePhysics(true);
			FVector NewVelocity = SkaterRef->GetRotationTrackerForwardVector()*GrindInitialVelocity.Length();
			RootSphere->SetPhysicsLinearVelocity(NewVelocity);
			break;
		}
	case ESkateMode::Grind:
		{
			GrindInitialVelocity = RootSphere->GetPhysicsLinearVelocity();
			RootSphere->SetSimulatePhysics(false);
			break;
		}
	default: break;
	}
}

void ASkatePhysics::Grind()
{
	if (bGrindInitialSnapHappened)
	{
		if (bMovingInSplineDirection)
		{
			// try to move in positive direction of grind spline
			GrindCurrentDistance +=GrindInitialVelocity.Length()*TickDelta;
			bool bIsGrindOver = GrindCurrentDistance>GrindSplineLength;
			if (bIsGrindOver)
			{
				// Abandon grind
				ChangeSkateMode(Skate);
			}
			else
			{
				// Continue to grind
				FVector SplineSnapPoint = IGrindface::Execute_GetSnapPointAtDistanceAlongSpline(GrindActor,GrindCurrentDistance);

				// apply z offset to snap point
				GrindSnapPoint = FVector{SplineSnapPoint.X, SplineSnapPoint.Y,SplineSnapPoint.Z + GrindZOffset};

				SetActorLocation(GrindSnapPoint,false,nullptr,ETeleportType::TeleportPhysics);
				
				// Set Skater's rotation tracker's rotation in tangential direction
				Cast<ASkater>(UGameplayStatics::GetPlayerPawn(GetWorld(),0))->RotationTracker->SetWorldRotation(UKismetMathLibrary::MakeRotFromX(IGrindface::Execute_GetTangentAtDistanceAlongSpline(GrindActor,GrindCurrentDistance)));  
			}
		}
		else
		{
			// try to move in opposite direction of grind spline
			GrindCurrentDistance -=GrindInitialVelocity.Length()*TickDelta;
			bool bIsGrindOver = GrindCurrentDistance<0.0;
			if (bIsGrindOver)
			{
				// Abandon grind
				ChangeSkateMode(Skate);
			}
			else
			{
				// Continue to grind
				FVector SplineSnapPoint = IGrindface::Execute_GetSnapPointAtDistanceAlongSpline(GrindActor,GrindCurrentDistance);

				// apply z offset to snap point
				GrindSnapPoint = FVector{SplineSnapPoint.X, SplineSnapPoint.Y,SplineSnapPoint.Z + GrindZOffset};

				SetActorLocation(GrindSnapPoint,false,nullptr,ETeleportType::TeleportPhysics);
				// Set Skater's rotation tracker's rotation in tangential direction
				Cast<ASkater>(UGameplayStatics::GetPlayerPawn(GetWorld(),0))->RotationTracker->SetWorldRotation(UKismetMathLibrary::MakeRotFromX(IGrindface::Execute_GetTangentAtDistanceAlongSpline(GrindActor,GrindCurrentDistance)*-1));  
			}
		}
	}
	else
	{
		SetActorLocation(FVector{GrindSnapPoint.X,GrindSnapPoint.Y,GrindSnapPoint.Z+GrindZOffset},false,nullptr,ETeleportType::TeleportPhysics);
		bGrindInitialSnapHappened = true;
	}
}

void ASkatePhysics::Ollie()
{
	switch (CurrentSkateMode)
	{
	case ESkateMode::Skate:
		{
			FVector ForwardVec =SkaterRef->RotationTracker->GetForwardVector();
			FVector UpVec = SkaterRef->RotationTracker->GetUpVector();
			FVector Impulse = (ForwardVec*0.05 + UpVec)*OllieImpulse;
			RootSphere->AddImpulse(Impulse,NAME_None,true);
			bOllieNextFrame = false;
			
			break;
		}
	case ESkateMode::Grind:
		{
			ChangeSkateMode(Skate);
			bOllieNextFrame = true;
			bGrindCooldownComplete = false;
			
			break;
		}
	default: break;
	}
}

void ASkatePhysics::Lean(bool bAtRest, float AxisValue)
{
	if(!bAtRest && CurrentSkateMode == Skate)
	{
		FVector LeanDirection = FVector::CrossProduct(FVector{0.0,0.0,1.0},RootSphere->GetPhysicsLinearVelocity().GetSafeNormal()).GetSafeNormal();
		float LeanMagnitude = FMath::Clamp(RootSphere->GetPhysicsLinearVelocity().Length()/MaxVelocity,0.25,2.0) * AxisValue * LeanForce;

		RootSphere->AddForce(LeanDirection*LeanMagnitude,NAME_None,true);
	}
}

void ASkatePhysics::AirTrajectoryPrediction()
{
	// Populate a time step array
	TArray<float> TimeStepArray;
	TimeStepArray.SetNum(100);
	for(int i=0;i<100;i++)
	{
		if (i==0)
		{
			TimeStepArray[0] = 0.0;
		}
		else
		{
			TimeStepArray[i] = TimeStepArray[i-1] + 0.05;
		}
	}

	// We use this time step array and current physics velocity to 'predict' where the skater will be after each step.
	// This way, we can easily align the skater properly before hitting ground.
	// We use the velocity and acceleration (gravitational acceleration) equations to do these predictions.

	FVector InitialVelocity = RootSphere->GetPhysicsLinearVelocity();

	for (auto TimeStep : TimeStepArray)
	{
		FVector Gravity = FVector{0.0,0.0,-980.0};
		float t = TimeStep - 0.1;
		FVector Displacement = InitialVelocity*t + (0.5 * Gravity * t * t);
		FVector FinalVelocity = InitialVelocity + Gravity*t;

		// Predicted trace
		FHitResult PredictionTraceResult;
		FVector TraceStart = GetActorLocation() + Displacement;
		FVector TraceEnd = TraceStart + (FinalVelocity.Length()*0.05 + (0.5*0.05*0.05*980))*FinalVelocity.GetSafeNormal();  // This is TraceStart + s { = ((u * t) + (1/2 * a * t^2) }.
		//Here, t is 0.05 as we are only considering the displacement during that small time step.

		if(GetWorld()->LineTraceSingleByChannel(PredictionTraceResult,TraceStart,TraceEnd,ECC_Visibility))
		{
			DrawDebugDirectionalArrow(GetWorld(),TraceStart,TraceEnd,10,FColor::Emerald,true);
			
			//DrawDebugLine(GetWorld(),TraceStart,TraceEnd,FColor::Silver,true,-1);
			
			if(PredictionTraceResult.bBlockingHit)
			{
				//GEngine->AddOnScreenDebugMessage(-1,5.0,FColor::Blue,PredictionTraceResult.GetActor()->GetName());

				// following is the direction that the skater will move in just after landing
				FVector ProjectedVelocityDirectionOnLanding = UKismetMathLibrary::ProjectVectorOnToPlane(FinalVelocity,PredictionTraceResult.Normal).GetSafeNormal();

				// Finally tell rotation tracker to use this information to rotate mid air for smooth landing.
				Cast<ASkater>(UGameplayStatics::GetPlayerPawn(GetWorld(),0))->OrientToLanding(PredictionTraceResult,0.0,ProjectedVelocityDirectionOnLanding);

				// Successful so break the loop
				break;
			}
		}
	}
	
}

void ASkatePhysics::FlipJump()
{
	// When performing this move, we remove the part of velocity that that pushes into the ramp so that skater will land back on the ramp

	// Direction of unnecessary velocity points inward of the ramp
	FVector DirectionOfUnnecessaryVelocity = FVector::CrossProduct(FVector::CrossProduct(GroundTraceHitNormal,FVector{0.0,0.0,1.0}).GetSafeNormal(),
		FVector{0.0,0.0,1.0});

	// Magnitude of unnecessary velocity approachs zero as the angle between actual and unnecessary velocity approaches 90 degrees
	float MagnitudeOfUnnecessaryVelocity = FVector::DotProduct(RootSphere->GetPhysicsLinearVelocity(),DirectionOfUnnecessaryVelocity);

	FVector UnnecessaryVelocity = DirectionOfUnnecessaryVelocity * MagnitudeOfUnnecessaryVelocity;
	FVector NewVel = RootSphere->GetPhysicsLinearVelocity() - UnnecessaryVelocity;

	RootSphere->SetPhysicsLinearVelocity(NewVel);
}

FVector ASkatePhysics::GetSkatePhysicsVelocity()
{
	return  RootSphere->GetPhysicsLinearVelocity();
}

FHitResult ASkatePhysics::ReportGroundCondition()
{
	// To check for grounded, we perform 9 traces in the XZ plane and 9 traces in the YZ plane to make sure that ground trace is not missed on inclined surfaces

	FHitResult HitResult;

	// First, XZ plane from -90 degree to 0 to 90 degree in the downward direction from the center
	for (auto Angle : AngleArrayForGroundCheck)
	{
		FVector TraceStart = GetActorLocation();

		// Finding trace end locations based on angles
		FVector TraceEnd = ((GetActorUpVector() * (-1) * UKismetMathLibrary::DegCos(Angle)) +
			(SkaterRef->CameraBoom->GetForwardVector()* UKismetMathLibrary::DegSin(Angle))) * 65 + TraceStart;

		if(GetWorld()->LineTraceSingleByChannel(HitResult,TraceStart,TraceEnd,ECC_Visibility))
		{
			if(HitResult.bBlockingHit)
			{
				return HitResult;
			}
		}
	}

	// if no hit yet, YZ plane from -90 degree to 0 to 90 degree in the downward direction from the center

	for (auto Angle : AngleArrayForGroundCheck)
	{
		FVector TraceStart = GetActorLocation();

		// Finding trace end locations based on angles
		FVector TraceEnd = ((GetActorUpVector() * (-1) * UKismetMathLibrary::DegCos(Angle)) +
			(SkaterRef->CameraBoom->GetRightVector()* UKismetMathLibrary::DegSin(Angle))) * 65 + TraceStart;

		if(GetWorld()->LineTraceSingleByChannel(HitResult,TraceStart,TraceEnd,ECC_Visibility))
		{
			if(HitResult.bBlockingHit)
			{
				return HitResult;
			}
		}
	}

	return HitResult;
}

TEnumAsByte<ESkateMode> ASkatePhysics::GetCurrentSkateMode() const
{
	return  CurrentSkateMode;
}

