#include "EnginePCH.h"
#include "Component/SceneComponent.h"

#include "GameFramework/Actor.h"
#include "Engine/World.h"

USceneComponent::~USceneComponent()
{
	TArray<USceneComponent*> Children = AttachChildren;
	AttachChildren.Reset();
	for (USceneComponent* Child : Children)
	{
		Child->AttachParent = nullptr;          
		Child->SetupAttachment(AttachParent);   
	}

	if (AActor* OwnerActor = GetOwner())
	{
		if (OwnerActor->GetRootComponent() == this)
			OwnerActor->SetRootComponent(Children.Num() > 0 ? Children[0] : nullptr);
	}

	DetachFromParent();
}

void USceneComponent::SetupAttachment(USceneComponent* InParent)
{
	if (InParent == this || AttachParent == InParent) return;

	// 순환 체크
	for (USceneComponent* Parent = InParent; Parent != nullptr; Parent = Parent->AttachParent)
		if (Parent == this) return;

	DetachFromParent();
	AttachParent = InParent;
	if (AttachParent)
	{
		AttachParent->AttachChildren.Add(this);
	}
}

void USceneComponent::DetachFromParent()
{
	if (!AttachParent) return;

	TArray<USceneComponent*>& Siblings = AttachParent->AttachChildren;
	for (uint32 i = 0;i < Siblings.Num(); ++i)
	{
		if (Siblings[i] == this)
		{
			Siblings.RemoveAt(i, 1);
			break;
		}
	}
	AttachParent = nullptr;
}

FRotator USceneComponent::GetWorldRotation() const
{
	if (AttachParent)
	{
		FQuat ParentQuat = AttachParent->GetWorldRotation().Quaternion();
		FQuat LocalQuat = Transform.GetOrientation();

		return (ParentQuat * LocalQuat).ToFRotator();
	}

	return Transform.Rotation;
}

FVector USceneComponent::GetWorldLocation() const
{
	FMatrix WorldMatrix = GetWorldMatrix();

	return FVector(WorldMatrix[3][0], WorldMatrix[3][1], WorldMatrix[3][2]);
}

FVector USceneComponent::GetWorldScale3D() const
{
	if (AttachParent)
	{
		FVector ParentScale = AttachParent->GetWorldScale3D();

		return FVector(
			Transform.Scale.X * ParentScale.X,
			Transform.Scale.Y * ParentScale.Y,
			Transform.Scale.Z * ParentScale.Z
		);
	}

	return Transform.Scale;
}

FMatrix USceneComponent::GetWorldMatrix() const
{
	FMatrix LocalMatrix = Transform.GetLocalMatrix(); // 부모 컴포넌트 연결 없을 때

	if (AttachParent)
	{
		return LocalMatrix * AttachParent->GetWorldMatrix();
	}

	return LocalMatrix;
}

void USceneComponent::MarkTransformDirty()
{
	OnTransformDirty();         

	for (USceneComponent* Child : AttachChildren)
		Child->MarkTransformDirty();      // 부모가 움직이면 자식의 월드 행렬도 바뀐다
}


bool USceneComponent::MoveComponent(const FVector& Delta, const FQuat& NewRotation, bool bSweep, FHitResult* OutHit)
{
	return MoveComponentImpl(Delta, NewRotation, bSweep, OutHit);
}

bool USceneComponent::MoveComponent(const FVector& Delta, const FRotator& NewRotation, bool bSweep, FHitResult* OutHit)
{
	return MoveComponentImpl(Delta, NewRotation.Quaternion(), bSweep, OutHit);
}

float USceneComponent::GetCollisionRadius() const
{
	FBox Bounds = CalcBounds();
	FVector Extent = Bounds.Max - Bounds.Min;
	Extent *= 0.5f;

	if(Extent.IsZero())
	{
		return 10.0f;
	}

	return std::min({ Extent.X, Extent.Y, Extent.Z });
}

bool USceneComponent::MoveComponentImpl(const FVector& Delta, const FQuat& NewRotation, bool bSweep, FHitResult* OutHit)
{
	FHitResult Temphit;
	FHitResult& HitResult = OutHit ? *OutHit : Temphit;
	if (OutHit)
	{
		*OutHit = FHitResult(1.f); // 초기화
	}
	// 이동 거리가 0인 경우
	if(Delta.IsZero())
	{
		// 회전이 바뀌지 않은 경우
		if(NewRotation.Equals(GetRelativeRotation().Quaternion()))
		{
			return false;
		}
		SetRelativeRotation(NewRotation.ToFRotator());
		return true;
	}
	// bSweep = true일 때 충돌 체크
	if(bSweep)
	{
		// 충돌 체크 로직
		AActor* OwnerActor = GetOwner();
		UWorld* World = OwnerActor->GetWorld();
		if (!World) return false;
		float Radius = GetCollisionRadius();
		FRay Ray = FRay(GetWorldLocation(), Delta.Normalized());

		if(World->SweepSingle(Ray, Delta.Size(), Radius, HitResult, OwnerActor))
		{
			// 충돌 발생 시 Hit에 충돌 정보 저장 및 종료
			// 충돌 지점까지 이동 후 종료
			FVector SafeLocation = GetRelativeLocation() + Delta * HitResult.Time + HitResult.ImpactNormal * 0.1f; // 충돌 지점에서 약간 떨어진 위치로 이동
			SetRelativeLocation(SafeLocation);
			//SetRelativeLocation(GetRelativeLocation() + Delta * HitResult.Time);
			SetRelativeRotation(NewRotation.ToFRotator());

			return false;
		}
	}


	// 충돌 발생하지 않으면 위치 및 회전 업데이트
	SetRelativeLocation(GetRelativeLocation() + Delta);
	SetRelativeRotation(NewRotation.ToFRotator());

	return true;
}