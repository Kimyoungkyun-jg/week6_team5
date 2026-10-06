#include "EnginePCH.h"
#include "FireBallComponent.h"
#include "Asset/AssetManager.h"
#include "Render/RenderCommand.h"
#include "Engine/PrimitiveSceneProxy.h"

void UFireBallComponent::SubmitToRenderQueue(FRenderQueue& RenderQueue, const FViewContext& ViewContext)
{
	const float SafeRadius = std::max(Radius, 0.0f);
	if (SafeRadius <= 0.0f)
		return;

	UStaticMesh* Sphere = UAssetManager::GetAssetByPath<UStaticMesh>("Sphere");
	UMaterial* FireBallMaterial = UAssetManager::GetAssetByPath<UMaterial>("FireBallMaterial");
	if (!Sphere || !FireBallMaterial)
		return;

	const FVector Center = GetWorldLocation();
	FRenderPacket& Packet = RenderQueue.AddDefaulted_GetRef();
	Packet.Mesh = Sphere;
	Packet.Material = FireBallMaterial;
	const FVector ToCamera = ViewContext.CameraPosition - Center;
	Packet.CameraToParticleDistance = ToCamera.Dot(ToCamera);
	MaterialParams.Center = Center;
	MaterialParams.Color = Color;
	MaterialParams.Radius = SafeRadius;
	MaterialParams.RadiusFalloff = RadiusFalloff;
	MaterialParams.Intensity = Intensity;
	MaterialParams.CameraPosition = ViewContext.CameraPosition;
	Packet.MaterialParamData = &MaterialParams;
	Packet.MaterialParamDataSize = sizeof(FFireBallMaterialParams);
	FTransform Transform(Center, GetWorldScale3D() * SafeRadius, GetWorldRotation());
	Packet.Model = RenderQueue.StoreWorldMatrix(Transform.GetLocalMatrix());
}

FBox UFireBallComponent::CalcLocalBounds() const
{
	const float SafeRadius = std::max(Radius, 0.0f);
	return FBox{ FVector(-SafeRadius), FVector(SafeRadius) };
}
