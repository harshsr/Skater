// Fill out your copyright notice in the Description page of Project Settings.


#include "Skate/Skaterface.h"

// Add default functionality here for any ISkaterface functions that are not pure virtual.

void ISkaterface::Lean(bool bAtRest, float AxisValue)
{
}

void ISkaterface::Ollie()
{
}

FVector ISkaterface::GetRotationTrackerForwardVector()
{
	return FVector::ZeroVector;
}

FVector ISkaterface::GetRotationTrackerRightVector()
{
	return FVector::ZeroVector;
}

FVector ISkaterface::GetRotationTrackerUpVector()
{
	return FVector::ZeroVector;
}

FHitResult ISkaterface::ReportGroundCondition()
{
	FHitResult HitResult;
	return HitResult;
}

FVector ISkaterface::GetSkatePhysicsVelocity()
{
	return FVector::ZeroVector;
}

void ISkaterface::FlipJump()
{
}

void ISkaterface::AirTrajectoryPrediction()
{
}

void ISkaterface::OrientToLanding(FHitResult HitResult, float TimeToHit, FVector ProjectedForwardVector)
{
}

bool ISkaterface::GetGrounded()
{
	return false;
}
