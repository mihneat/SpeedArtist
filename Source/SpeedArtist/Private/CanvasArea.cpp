// Fill out your copyright notice in the Description page of Project Settings.


#include "CanvasArea.h"

#include "FrameTypes.h"
#include "Misc/InteractiveProcess.h"

void FStroke::AddPoint(const FPoint& Point)
{
	Points.Add(Point);
}

FString FStroke::Serialize()
{
	FString Xs = "[", Ys = "[";
	for (int i = 0; i < Points.Num(); ++i)
	{
		// Skip over simplified points
		if (Points[i].Fixed == false)
			continue;
		
		if (i > 0)
		{
			Xs.Append(",");
			Ys.Append(",");
		}

		const FPoint& Point = Points[i];
		Xs.Append(FString::Printf(TEXT("%d"), FMath::FloorToInt(Point.Coords.X)));
		Ys.Append(FString::Printf(TEXT("%d"), FMath::FloorToInt(Point.Coords.Y)));
	}

	Xs.Append("]");
	Ys.Append("]");
	
	return FString::Format(TEXT("[{0},{1}]"), { Xs, Ys });
}

// Algorithm used: https://en.wikipedia.org/wiki/Ramer%E2%80%93Douglas%E2%80%93Peucker_algorithm
void FStroke::Simplify(float Eps)
{
	for (FPoint& Point : Points)
		Point.Fixed = false;

	SimplifyRec(Eps, 0, Points.Num() - 1);
}

void FStroke::SimplifyRec(float Eps, int Left, int Right)
{
	// Set the endpoints as fixed
	Points[Left].Fixed = true;
	Points[Right].Fixed = true;

	// Stop if we cannot divide anymore
	if (Left + 1 >= Right)
		return;
	
	// Find the farthest point
	float MaxDist = -1;
	float MaxPos = -1;
	for (int i = Left + 1; i < Right; ++i)
	{
		const float Dist = FMath::PointDistToSegment(Points[i].Coords, Points[Left].Coords, Points[Right].Coords);
		if (Dist > MaxDist)
		{
			MaxDist = Dist;
			MaxPos = i;
		}
	}

	// Stop if we've reached the desired error
	if (MaxDist < Eps)
		return;

	SimplifyRec(Eps, Left, MaxPos);
	SimplifyRec(Eps, MaxPos, Right);
}

void FPainting::AddStroke(const FStroke& Stroke)
{
	// Avoid adding empty strokes
	if (Stroke.Points.Num() == 0)
		return;
	
	Strokes.Add(Stroke);
}

FString FPainting::Serialize()
{
	FString StrokesJson = "[";
	for (int i = 0; i < Strokes.Num(); ++i)
	{
		if (i > 0)
			StrokesJson.Append(",");

		StrokesJson.Append(FString::Format(TEXT("{0}"), { *Strokes[i].Serialize() }));
	}

	StrokesJson.Append("]");
	
	return StrokesJson;
}

void FPainting::Simplify(float Eps)
{
	for (FStroke& Stroke : Strokes)
		Stroke.Simplify(Eps);
}

// Sets default values
ACanvasArea::ACanvasArea()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ACanvasArea::BeginPlay()
{
	Super::BeginPlay();

	World = GetWorld();
	PlayerController = World->GetFirstPlayerController();

	InitializeCanvas(StartWidth, StartHeight);
	InitializeDrawingTools(StartBrushRadius);
}

// Called every frame
void ACanvasArea::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACanvasArea::StartDrawing()
{
	// Initialize a new stroke
	CurrentStroke = FStroke{};
}

void ACanvasArea::Draw()
{
	if (!World || !PlayerController)
	{
		UE_LOG(LogTemp, Error, TEXT("[ACanvasArea] Unable to find the world or any players within the world"));
		return;
	}

	FVector WorldPosition{0};
	FVector WorldDirection{0};
	PlayerController->DeprojectMousePositionToWorld(WorldPosition, WorldDirection);

	FHitResult HitResult;
	
	World->LineTraceSingleByChannel(HitResult, WorldPosition, WorldPosition + WorldDirection * 1000.0f, ECC_GameTraceChannel2);

	// UE_LOG(LogTemp, Display, TEXT("[ACanvasArea] World pos: %s, World direction: %s"), *WorldPosition.ToString(), *WorldDirection.ToString());
	// UE_LOG(LogTemp, Display, TEXT("[ACanvasArea] Hit canvas: %hs"), HitResult.bBlockingHit ? "true" : "false");
	
	if (!HitResult.bBlockingHit)
	{
		PrevCoords = UE::Math::TVector2<float>(-1, -1);
		return;
	}

	// Find where the canvas was hit
	const UE::Math::TBox<double> HitBox = HitResult.Component->Bounds.GetBox();
	const UE::Math::TVector<double> MaxBound = HitBox.Max;
	const UE::Math::TVector<double> MinBound = HitBox.Min;
	
	// DrawDebugBox(World, HitBox.GetCenter(), HitBox.GetExtent(), FColor::Cyan);
	// DrawDebugLine(World, MinBound, MaxBound, FColor::Green);

	// TODO-WARNING: STRONGLY HARDCODED, sensitive to canvas rotations 
	const FVector& HitPosition = HitResult.Location;
	float NormalizedHeight = 1 - (HitPosition.Z - MinBound.Z) / (MaxBound.Z - MinBound.Z);
	float NormalizedWidth = 1 - (HitPosition.Y - MinBound.Y) / (MaxBound.Y - MinBound.Y);
	
	// DrawDebugSphere(World, HitPosition, 10, 10, FColor::Red);

	// UE_LOG(LogTemp, Display, TEXT("[ACanvasArea] Draw coords: %f, %f"), NormalizedWidth * CanvasWidth, NormalizedHeight * CanvasHeight);

	const UE::Math::TVector2 Coords(floorf(NormalizedWidth * CanvasWidth), floorf(NormalizedHeight * CanvasHeight));

	// Draw a line from the previous point
	if (PrevCoords.X >= 0 && PrevCoords.Y >= 0 && PrevCoords.X < CanvasWidth && PrevCoords.Y < CanvasHeight)
	{
		const float DistanceBetweenPoints = UE::Math::TVector2<float>::Distance(PrevCoords, Coords);
		const float StepCount = floorf(DistanceBetweenPoints / (Radius / 2.0f));

		if (StepCount > 0)
		{
			const float Step = DistanceBetweenPoints / StepCount;
		
			for (float Alpha = 0.0f; Alpha < DistanceBetweenPoints; Alpha += Step)
			{
				const UE::Math::TVector2<float> NewCoords = UE::Geometry::Lerp(PrevCoords, Coords, Alpha / DistanceBetweenPoints);
				DrawDot(NewCoords.X, NewCoords.Y);
			}
		}
	}
	
	DrawDot(Coords.X, Coords.Y);

	// Store the current point
	PrevCoords = Coords;

	// Add a new point to the stroke
	CurrentStroke.AddPoint(FPoint{ FVector{ Coords.X, Coords.Y, 0 }});
}

void ACanvasArea::StopDrawing()
{
	PrevCoords = UE::Math::TVector2<float>(-1, -1);

	// Add the stroke to the painting
	CurrentPainting.AddStroke(CurrentStroke);
}

void ACanvasArea::InitializeCanvas(const int32 PixelsH, const int32 PixelsV)
{
	// Dynamic texture initialization
	CanvasWidth = PixelsH;
	CanvasHeight = PixelsV;
	
	DynamicCanvas = UTexture2D::CreateTransient(CanvasWidth, CanvasHeight);
#if WITH_EDITORONLY_DATA
	DynamicCanvas->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
#endif
	DynamicCanvas->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
	DynamicCanvas->SRGB = 1;
	DynamicCanvas->AddToRoot();
	DynamicCanvas->Filter = TextureFilter::TF_Nearest;
	DynamicCanvas->UpdateResource();
	
	EchoUpdateTextureRegion = std::unique_ptr<FUpdateTextureRegion2D>(new FUpdateTextureRegion2D(0, 0, 0, 0, CanvasWidth, CanvasHeight));
	
	// Buffers initialization
	BytesPerPixel = 4; // r g b a
	BufferPitch = CanvasWidth * BytesPerPixel;
	BufferSize = CanvasWidth * CanvasHeight * BytesPerPixel;
	
	CanvasPixelData = std::unique_ptr<uint8[]>(new uint8[BufferSize]);
	
	ClearCanvas();
}

void ACanvasArea::UpdateCanvas()
{
	if (EchoUpdateTextureRegion)
	{
		DynamicCanvas->UpdateTextureRegions((int32)0, (uint32)1, EchoUpdateTextureRegion.get(), (uint32)BufferPitch, (uint32)BytesPerPixel, CanvasPixelData.get());
	}
}

void ACanvasArea::ClearCanvas()
{
	uint8* canvasPixelPtr = CanvasPixelData.get();
	for (int i = 0; i < CanvasWidth * CanvasHeight; ++i)
	{
		SetPixelColor(canvasPixelPtr, 255, 255, 255, 0); // White
		canvasPixelPtr += BytesPerPixel;
	}
	
	UpdateCanvas();

	CurrentPainting = FPainting{};
}

void ACanvasArea::InitializeDrawingTools(const int32 BrushRadius)
{
	Radius = BrushRadius;
	BrushBufferSize = Radius * Radius * 4 * BytesPerPixel; //2r*2r * bpp
	CanvasBrushMask = std::unique_ptr<uint8[]>(new uint8[BrushBufferSize]);
	uint8* canvasBrushPixelPtr = CanvasBrushMask.get();
	for (int px = -Radius; px < Radius; ++px)
	{
		for (int py = -Radius; py < Radius; ++py)
		{
			int32 tx = px + Radius;
			int32 ty = py + Radius;
			canvasBrushPixelPtr = CanvasBrushMask.get() + (tx +  + ty * 2 * Radius) * BytesPerPixel;
			if (px*px + py*py < Radius*Radius)
			{
				SetPixelColor(canvasBrushPixelPtr, 0, 0, 0, 255); //black alpha 255 - bgra
			}
			else
			{
				SetPixelColor(canvasBrushPixelPtr, 0, 0, 0, 0); // alpha 0
			}
		}
	}
}

void ACanvasArea::DrawDot(const int32 PixelCoordX, const int32 PixelCoordY)
{
	uint8* canvasPixelPtr = CanvasPixelData.get();
	const uint8* canvasBrushPixelPtr = CanvasBrushMask.get();
	for (int px = -Radius; px < Radius; ++px)
	{
		for (int py = -Radius; py < Radius; ++py) 
		{ 
			int32 tbx = px + Radius; 
			int32 tby = py + Radius; 
			canvasBrushPixelPtr = CanvasBrushMask.get() + (tbx + tby * 2* Radius) * BytesPerPixel; 
			if (*(canvasBrushPixelPtr + 3) == 255) // check the alpha value of the pixel of the brush mask 
			{ 
				int32 tx = PixelCoordX + px; 
				int32 ty = PixelCoordY + py; 
				if (tx >= 0 && tx < CanvasWidth && ty >= 0 && ty < CanvasHeight)
				{
					canvasPixelPtr = CanvasPixelData.get() + (tx + ty * CanvasWidth) * BytesPerPixel;
					SetPixelColor(canvasPixelPtr, *(canvasBrushPixelPtr + 2), *(canvasBrushPixelPtr + 1), *(canvasBrushPixelPtr), *(canvasBrushPixelPtr + 3));
				}
			}
		}
	}
	UpdateCanvas();
}

void ACanvasArea::SaveTexture()
{
	if (!IsValid(DynamicCanvas))
		return;

	auto TextureResource = DynamicCanvas->CreateResource();

	FArchive Archive;
	DynamicCanvas->Serialize(Archive);

	// Archive.

	// TODO: More likely, save the strokes and points in a data structure (create a new class for it) and save this info on the disk
}

FPainting& ACanvasArea::GetCurrentPainting()
{
	return CurrentPainting;
}

void ACanvasArea::SetPixelColor(uint8*& Pointer, uint8 Red, uint8 Green, uint8 Blue, uint8 Alpha)
{
	*Pointer = Blue;			// b
	*(Pointer + 1) = Green;		// g
	*(Pointer + 2) = Red;		// r
	*(Pointer + 3) = Alpha;		// a
}

