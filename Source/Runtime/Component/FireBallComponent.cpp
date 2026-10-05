#include "EnginePCH.h"
#include "FireBallComponent.h"
#include "Asset/AssetManager.h"
#include "Render/RenderCommand.h"
#include "Engine/PrimitiveSceneProxy.h"

UFireBallComponent::UFireBallComponent()
{
}

UFireBallComponent::~UFireBallComponent()
{
}

void UFireBallComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UFireBallComponent::TickComponent(float DeltaTime)
{
	Super::TickComponent(DeltaTime);
}

void UFireBallComponent::SubmitToRenderQueue(FRenderQueue& RenderQueue, const FViewContext& ViewContext)
{
	FRenderPacket& Packet = RenderQueue.AddDefaulted_GetRef();
	Packet.Mesh = UAssetManager::GetAssetByPath<UStaticMesh>("Sphere");
	//Packet.Material = UAssetManager::GetAssetByPath<UMaterial>("FireBallMaterial");
	MaterialParams.Center = GetWorldLocation();
	MaterialParams.Color = Color;
	MaterialParams.Radius = Radius;
	MaterialParams.RadiusFalloff = RadiusFalloff;
	MaterialParams.Intensity = Intensity;
	Packet.MaterialParamData = &MaterialParams;
	Packet.MaterialParamDataSize = sizeof(FFireBallMaterialParams);
	FTransform Transform = FTransform(GetWorldLocation(), GetRelativeScale3D() * Radius, GetRelativeRotation());
	Packet.Model = RenderQueue.StoreWorldMatrix(Transform.GetLocalMatrix());
}