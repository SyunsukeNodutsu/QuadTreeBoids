// Fill out your copyright notice in the Description page of Project Settings.


#include "QuadTreeBoidsActor.h"
#include "Kismet/KismetSystemLibrary.h"

#define FAST_BOID_DEBUG_DRAW false

//! コンストラクタ
//*****************************************************************************
AQuadTreeBoidsActor::AQuadTreeBoidsActor()
{
	PrimaryActorTick.bCanEverTick = true;

}

//! デストラクタ
//*****************************************************************************
AQuadTreeBoidsActor::~AQuadTreeBoidsActor()
{
	if (MyQuadTree)
	{
		MyQuadTree->Clear();
		delete MyQuadTree;
		MyQuadTree = nullptr;
	}
}

//! 開始
//*****************************************************************************
void AQuadTreeBoidsActor::BeginPlay()
{
	Super::BeginPlay();

	float width = 100, height = 100;

	MyQuadTree = new QuadTree(FRectAngle(0, 0, width, height));

	for (int i = 0; i < NumBoids; i++)
	{
		float x = FMath::RandRange(-width, width);
		float y = FMath::RandRange(-height, height);

		float cos = FMath::Cos(FMath::RandRange(0.0f, PI * 2));
		float sin = FMath::Sin(FMath::RandRange(0.0f, PI * 2));
		const FVector2D RandomVector = FVector2D(cos, sin);
		const float initVelocity = 24;

		FPoint boid(x, y);
		boid.Velocity = RandomVector * initVelocity;
		Boids.Add(boid);

		MyQuadTree->Insert(FVector2D(x, y));
	}
}

//! 更新
//*****************************************************************************
void AQuadTreeBoidsActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MyQuadTree)
	{
		UpdateBoids(DeltaTime);

		MyQuadTree->Visualize(GetWorld());
	}
}

//! ボイド更新
//*****************************************************************************
void AQuadTreeBoidsActor::UpdateBoids(float DeltaTime)
{
	const float CosAngleThreshold = FMath::Cos(FMath::DegreesToRadians(NeighborAngleThreshold));

#if FAST_BOID_DEBUG_DRAW
	int cnt = 0;
#endif
	for (auto& boid : Boids)
	{
		// 場外への移動を防ぐ
		FVector2D wallAvoidanceForce = CalculateWallAvoidanceForce(FVector2D(
			boid.Position.X, boid.Position.Y), MyQuadTree->GetBoundary(), 10.0f);
		boid.Accel += wallAvoidanceForce;

		// ボイドの挙動
		FVector2D separation = FVector2D(0);
		FVector2D alignment = FVector2D(0);
		FVector2D cohesion = FVector2D(0);

		int numNeighbors = 0;

		// 近傍ボイドの検索
		TArray<FPoint> found;
		MyQuadTree->Query(FRectAngle(boid.Position, NeighborDist, NeighborDist), found);

		FVector2D position = boid.Position;

		for (auto& otherBoid : found)
		{
			FVector2D other = otherBoid.Position;
			FVector2D to = other - position;
			FVector2D dir = to.GetSafeNormal();
			FVector2D fwd = boid.Velocity.GetSafeNormal();
			float prod = FVector2D::DotProduct(fwd, dir);
			float distance = FVector2D::Distance(position, other);
			
			// 角度と距離で近傍ボイドと判定
			if ((prod > CosAngleThreshold) && (distance <= NeighborDist))
			{
				separation += (position - other).GetSafeNormal();
				alignment += otherBoid.Velocity;
				cohesion += other;

				numNeighbors++;

#if FAST_BOID_DEBUG_DRAW
				if (cnt == 0)
				{
					UKismetSystemLibrary::DrawDebugPoint(GetWorld(), FVector(
						otherBoid.Position.X, otherBoid.Position.Y, 1), 5, FColor::Purple);
				}
#endif
			}
		}

		if (numNeighbors > 0)
		{
			// 分離
			separation /= numNeighbors;
			boid.Accel += separation * SeparationWeight;

			// 整列
			alignment /= numNeighbors;
			boid.Accel += (alignment - boid.Velocity) * AlignmentWeight;

			// 結束
			cohesion /= numNeighbors;
			boid.Accel += (cohesion - position) * CohesionWeight;
		}

		boid.Velocity += boid.Accel * DeltaTime;

		float minSpeed = 6;
		if (boid.Velocity.Size() < minSpeed)
		{
			boid.Velocity = boid.Velocity.GetSafeNormal() * minSpeed;
		}

		boid.Position += boid.Velocity * DeltaTime;

		boid.Accel = FVector2D(0);

		// ボイドの描画
#if FAST_BOID_DEBUG_DRAW
		FColor color = (cnt == 0 ? FColor::Red : FColor::Green);
		UKismetSystemLibrary::DrawDebugPoint(GetWorld(), FVector(boid.Position.X, boid.Position.Y, 0), 4, color);
		if (cnt == 0)
		{
			UKismetSystemLibrary::DrawDebugCircle(GetWorld(), FVector(boid.Position.X,
				boid.Position.Y, 0), NeighborDist, 12, FColor::Red, 0, 0, FVector(0, 1, 0), FVector(1, 0, 0));
		}
		cnt++;
#else
		UKismetSystemLibrary::DrawDebugPoint(GetWorld(), FVector(boid.Position.X, boid.Position.Y, 0), 4, FColor::Green);
#endif
	}

	// 四分木更新
	MyQuadTree->Clear();
	for (auto& boid : Boids)
	{
		MyQuadTree->Insert(boid.Position);
	}
}

//! 回避力計算
//*****************************************************************************
FVector2D AQuadTreeBoidsActor::CalculateWallAvoidanceForce(const FVector2D& position, const FRectAngle& boundary, float maxForce)
{
	FVector2D force(0, 0);
	float distanceToLeft = position.X - (boundary.Center.X - boundary.Width);
	float distanceToRight = (boundary.Center.X + boundary.Width) - position.X;
	float distanceToTop = (boundary.Center.Y + boundary.Height) - position.Y;
	float distanceToBottom = position.Y - (boundary.Center.Y - boundary.Height);

	if (distanceToLeft < 10.0f)
	{
		force.X += maxForce * (10.0f - distanceToLeft) / 10.0f;
	}
	if (distanceToRight < 10.0f)
	{
		force.X -= maxForce * (10.0f - distanceToRight) / 10.0f;
	}
	if (distanceToTop < 10.0f)
	{
		force.Y -= maxForce * (10.0f - distanceToTop) / 10.0f;
	}
	if (distanceToBottom < 10.0f)
	{
		force.Y += maxForce * (10.0f - distanceToBottom) / 10.0f;
	}

	return force;
}

