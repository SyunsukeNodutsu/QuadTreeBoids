// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "QuadTree.h"
#include "GameFramework/Actor.h"
#include "QuadTreeBoidsActor.generated.h"

/**
 * ボイド用データ
 */
struct FBoidData
{
	FVector2D Velocity = FVector2D(0);
	FVector2D Accel = FVector2D(0);
};

/**
 * 四分木を使用したボイドシミュレーション
 */
UCLASS()
class QUADTREEBOIDS_API AQuadTreeBoidsActor : public AActor
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category = "Control")
	int NumBoids = 100;

	UPROPERTY(EditAnywhere, Category = "Control")
	float SeparationWeight = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Control")
	float AlignmentWeight = 0.34f;

	UPROPERTY(EditAnywhere, Category = "Control")
	float CohesionWeight = 0.16f;

	UPROPERTY(EditAnywhere, Category = "Control")
	float NeighborDist = 24.0f;

	UPROPERTY(EditAnywhere, Category = "Control")
	float NeighborAngleThreshold = 90.0f;

public:
	AQuadTreeBoidsActor();
	~AQuadTreeBoidsActor();
	void BeginPlay() override;
	void Tick(float DeltaTime) override;
	void UpdateBoids(float DeltaTime);
	FVector2D CalculateWallAvoidanceForce(const FVector2D& position, const FRectAngle& boundary, float maxForce);

private:
	QuadTree* MyQuadTree;
	TArray<FPoint> Boids;

};
