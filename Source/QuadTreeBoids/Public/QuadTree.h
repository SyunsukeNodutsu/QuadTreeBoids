// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 四分木に格納するポイント
 */
struct FPoint
{
	FPoint() = default;
	FPoint(float x, float y)
	{
		Position.X = x;
		Position.Y = y;
	}
	FPoint(FVector2D position)
	{
		Position = position;
	}

	FVector2D Position = FVector2D(0);

	// ボイドで使用
	FVector2D Velocity = FVector2D(0);
	FVector2D Accel = FVector2D(0);
};

/**
 * 矩形
 */
struct FRectAngle
{
	FRectAngle() = default;
	FRectAngle(float x, float y, float width, float height)
	{
		Center = FVector2D(x, y);
		Width = width;
		Height = height;
	}
	FRectAngle(FVector2D center, float width, float height)
	{
		Center = center;
		Width = width;
		Height = height;
	}
	bool Constains(FVector2D point) const
	{
		return (point.X > Center.X - Width && point.X < Center.X + Width &&
			point.Y > Center.Y - Height && point.Y < Center.Y + Height);
	}
	bool Intersects(FRectAngle range) const
	{
		return !(range.Center.X - range.Width > Center.X + Width ||
			range.Center.X + range.Width < Center.X - Width ||
			range.Center.Y - range.Height > Center.Y + Height ||
			range.Center.Y + range.Height < Center.Y - Height);
	}
	FVector2D Center = FVector2D(0);
	float Width = 0.0f;
	float Height = 0.0f;
};

/**
 * 四分木
 */
class QUADTREEBOIDS_API QuadTree
{
public:
	QuadTree() = default;
	QuadTree(FRectAngle boundary, int level = 0, int capacity = 4);
	QuadTree(const QuadTree& other);
	QuadTree& operator=(const QuadTree& other);
	void Subdivide();
	void Insert(FVector2D point);
	void Clear();
	void Query(FRectAngle range, TArray<FPoint>& found);
	void Query(FVector2D center, float radius, TArray<FPoint>& found);
	void GetPoints(TArray<FPoint>& points) const { points = Points; }
	FRectAngle GetBoundary() const { return Boundary; }
	void Visualize(const UObject* wcObject);

private:
	int Capacity = 4;
	int Level = 0;
	bool IsDivided = false;
	FRectAngle Boundary;
	TArray<FPoint> Points;
	QuadTree* NorthWest = nullptr;
	QuadTree* NorthEast = nullptr;
	QuadTree* SouthWest = nullptr;
	QuadTree* SouthEast = nullptr;
};
