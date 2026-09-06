#include "Tools/ChalkTraceActor.h"

#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AChalkTraceActor::AChalkTraceActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	MarkMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MarkMesh"));
	RootComponent = MarkMesh;

	MarkMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MarkMesh->SetCastShadow(false);
	MarkMesh->SetRelativeScale3D(FVector(0.35f, 0.35f, 0.02f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMeshFinder(
		TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (PlaneMeshFinder.Succeeded())
	{
		MarkMesh->SetStaticMesh(PlaneMeshFinder.Object);
	}
}

void AChalkTraceActor::BeginPlay()
{
	Super::BeginPlay();

	SetLifeSpan(LifespanSeconds);

	if (MarkMesh)
	{
		DynamicMaterial = MarkMesh->CreateAndSetMaterialInstanceDynamic(0);
		if (DynamicMaterial)
		{
			// Teinte phosphorescente émeraude / cyan M.E.G.
			DynamicMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.1f, 1.0f, 0.6f, 1.0f));
			DynamicMaterial->SetScalarParameterValue(TEXT("Roughness"), 0.2f);
		}
	}
}

void AChalkTraceActor::SetMarkColor(const FLinearColor& InColor)
{
	if (DynamicMaterial)
	{
		DynamicMaterial->SetVectorParameterValue(TEXT("Color"), InColor);
	}
}
