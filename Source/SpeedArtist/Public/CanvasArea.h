// Fill out your copyright notice in the Description page of Project Settings.

// Code inspired by: https://www.parallelcube.com/2018/01/08/dynamic-texture-introduction-drawing-canvas/

#pragma once

#include <memory>

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CanvasArea.generated.h"

struct SPEEDARTIST_API FPoint
{
	FVector Coords;
	bool Fixed = true;
};

struct SPEEDARTIST_API FStroke
{
	TArray<FPoint> Points;

	void AddPoint(const FPoint& Point);

	FString Serialize();
	void Simplify(float Eps);
	void SimplifyRec(float Eps, int Left, int Right);
};

struct SPEEDARTIST_API FPainting
{
	TArray<FStroke> Strokes;

	void AddStroke(const FStroke& Stroke);

	FString Serialize();
	void Simplify(float Eps);
};

UCLASS()
class SPEEDARTIST_API ACanvasArea : public AActor
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	ACanvasArea();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Variables)
	UTexture2D* DynamicCanvas;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Variables)
	int32 StartWidth = 1024;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Variables)
	int32 StartHeight = 1024;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Variables)
	int32 StartBrushRadius = 10;
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void StartDrawing();
	void Draw();
	void StopDrawing();
	
	UFUNCTION(BlueprintCallable, Category = DrawingTools)
	void InitializeCanvas(const int32 PixelsH, const int32 PixelsV);
	
	UFUNCTION(BlueprintCallable, Category = DrawingTools)
	void UpdateCanvas();
	
	UFUNCTION(BlueprintCallable, Category = DrawingTools)
	void ClearCanvas();

	UFUNCTION(BlueprintCallable, Category = DrawingTools)
	void InitializeDrawingTools(const int32 BrushRadius);
	
	UFUNCTION(BlueprintCallable, Category = DrawingTools)
	void DrawDot(const int32 PixelCoordX, const int32 PixelCoordY);
	
	UFUNCTION(BlueprintCallable, Category = DrawingTools)
	void SaveTexture();

	FPainting& GetCurrentPainting();

private:

	// References
	UWorld* World = nullptr;
	APlayerController* PlayerController = nullptr;

	// Canvas
	std::unique_ptr<uint8[]> CanvasPixelData;
	int CanvasWidth;
	int CanvasHeight;
	int BytesPerPixel;
	int BufferPitch;
	int BufferSize;
	std::unique_ptr<FUpdateTextureRegion2D> EchoUpdateTextureRegion;

	// Draw brush tool
	std::unique_ptr<uint8[]> CanvasBrushMask;
	int Radius;
	int BrushBufferSize;

	// Model data storage
	FPainting CurrentPainting;
	FStroke CurrentStroke;

	UE::Math::TVector2<float> PrevCoords = UE::Math::TVector2(-1.0f, -1.0f);
	
	void SetPixelColor(uint8*& Pointer, uint8 Red, uint8 Green, uint8 Blue, uint8 Alpha);


};
