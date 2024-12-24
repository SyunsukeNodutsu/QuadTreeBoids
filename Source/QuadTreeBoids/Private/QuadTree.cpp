// Fill out your copyright notice in the Description page of Project Settings.


#include "QuadTree.h"
#include "Kismet/KismetSystemLibrary.h"

//! コンストラクタ
//*****************************************************************************
QuadTree::QuadTree(FRectAngle boundary, int level, int capacity)
{
	Level = level;
	Boundary = boundary;
	Capacity = capacity;
}

//! コンストラクタ
//*****************************************************************************
QuadTree::QuadTree(const QuadTree& other)
{
	Boundary = other.Boundary;
	Level = other.Level;
	Capacity = other.Capacity;
	IsDivided = other.IsDivided;
	Points = other.Points;

	if (other.IsDivided)
	{
		NorthWest = new QuadTree(*other.NorthWest);
		NorthEast = new QuadTree(*other.NorthEast);
		SouthWest = new QuadTree(*other.SouthWest);
		SouthEast = new QuadTree(*other.SouthEast);
	}
	else
	{
		NorthWest = nullptr;
		NorthEast = nullptr;
		SouthWest = nullptr;
		SouthEast = nullptr;
	}
}

//! コンストラクタ
//*****************************************************************************
QuadTree& QuadTree::operator=(const QuadTree& other)
{
	if (this == &other)
	{
		return *this;
	}

	Clear();

	Boundary = other.Boundary;
	Level = other.Level;
	Capacity = other.Capacity;
	IsDivided = other.IsDivided;
	Points = other.Points;

	if (other.IsDivided)
	{
		NorthWest = new QuadTree(*other.NorthWest);
		NorthEast = new QuadTree(*other.NorthEast);
		SouthWest = new QuadTree(*other.SouthWest);
		SouthEast = new QuadTree(*other.SouthEast);
	}
	else
	{
		NorthWest = nullptr;
		NorthEast = nullptr;
		SouthWest = nullptr;
		SouthEast = nullptr;
	}

	return *this;
}

//! 追加
//*****************************************************************************
void QuadTree::Subdivide()
{
	FVector2D center = Boundary.Center;
	float width = Boundary.Width / 2;
	float height = Boundary.Height / 2;

	NorthEast = new QuadTree(FRectAngle(FVector2D(center.X + width, center.Y - height), width, height), Level + 1, Capacity);
	NorthWest = new QuadTree(FRectAngle(FVector2D(center.X - width, center.Y - height), width, height), Level + 1, Capacity);
	SouthEast = new QuadTree(FRectAngle(FVector2D(center.X + width, center.Y + height), width, height), Level + 1, Capacity);
	SouthWest = new QuadTree(FRectAngle(FVector2D(center.X - width, center.Y + height), width, height), Level + 1, Capacity);

	IsDivided = true;
}

//! 追加
//*****************************************************************************
void QuadTree::Insert(FVector2D point)
{
	if (!Boundary.Constains(point))
	{
		return;
	}
	if (Points.Num() < Capacity || Level >= 8)
	{
		Points.Add(FPoint(point));
	}
	else
	{
		if (!IsDivided)
		{
			Subdivide();
		}
		NorthWest->Insert(point);
		NorthEast->Insert(point);
		SouthWest->Insert(point);
		SouthEast->Insert(point);

		for (const auto& oldPoint : Points)
		{
			NorthWest->Insert(FVector2D(oldPoint.Position));
			NorthEast->Insert(FVector2D(oldPoint.Position));
			SouthWest->Insert(FVector2D(oldPoint.Position));
			SouthEast->Insert(FVector2D(oldPoint.Position));
		}
		Points.Empty();
	}
}

//! クリア
//*****************************************************************************
void QuadTree::Clear()
{
	Points.Empty();
	if (IsDivided)
	{
		NorthWest->Clear();
		NorthEast->Clear();
		SouthWest->Clear();
		SouthEast->Clear();

		delete NorthWest;
		delete NorthEast;
		delete SouthWest;
		delete SouthEast;

		NorthWest = nullptr;
		NorthEast = nullptr;
		SouthWest = nullptr;
		SouthEast = nullptr;

		IsDivided = false;
	}
}

//! 範囲内のポイントを取得(ボックス)
//*****************************************************************************
void QuadTree::Query(FRectAngle range, TArray<FPoint>& found)
{
	if (!Boundary.Intersects(range))
	{
		return;
	}
	for (const auto& point : Points)
	{
		if (range.Constains(point.Position))
		{
			found.Add(point);
		}
	}
	if (IsDivided)
	{
		NorthWest->Query(range, found);
		NorthEast->Query(range, found);
		SouthWest->Query(range, found);
		SouthEast->Query(range, found);
	}
}

//! 範囲内のポイントを取得(サークル)
//*****************************************************************************
void QuadTree::Query(FVector2D center, float radius, TArray<FPoint>& found)
{

}

//! ビジュアライズ
//*****************************************************************************
void QuadTree::Visualize(const UObject* wcObject)
{
	FColor color = FColor::White;

	FVector PointA = FVector(Boundary.Center.X - Boundary.Width, Boundary.Center.Y - Boundary.Height, 0);
	FVector PointB = FVector(Boundary.Center.X + Boundary.Width, Boundary.Center.Y - Boundary.Height, 0);
	FVector PointC = FVector(Boundary.Center.X + Boundary.Width, Boundary.Center.Y + Boundary.Height, 0);
	FVector PointD = FVector(Boundary.Center.X - Boundary.Width, Boundary.Center.Y + Boundary.Height, 0);

	UKismetSystemLibrary::DrawDebugLine(wcObject, PointA, PointB, color);
	UKismetSystemLibrary::DrawDebugLine(wcObject, PointB, PointC, color);
	UKismetSystemLibrary::DrawDebugLine(wcObject, PointC, PointD, color);
	UKismetSystemLibrary::DrawDebugLine(wcObject, PointD, PointA, color);

	if (IsDivided)
	{
		NorthWest->Visualize(wcObject);
		NorthEast->Visualize(wcObject);
		SouthWest->Visualize(wcObject);
		SouthEast->Visualize(wcObject);
	}

	/*for (const auto& point : Points)
	{
		UKismetSystemLibrary::DrawDebugString(wcObject, FVector(point.X, point.Y, -1),
			FString::Printf(TEXT("Lv: %d"), Level), nullptr, color);
	}*/
}
