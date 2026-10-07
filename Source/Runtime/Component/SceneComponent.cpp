#include "EnginePCH.h"
#include "Component/SceneComponent.h"

#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include <cmath>

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



void USceneComponent::OnPropertyChanged(const FString& PropertyName)
{
	Super::OnPropertyChanged(PropertyName);
	if (PropertyName == "Transform") MarkTransformDirty();
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
    FHitResult LocalHit(1.0f);
    FHitResult& Hit = OutHit ? *OutHit : LocalHit;
    Hit = FHitResult(1.0f);

    FTransform Relative = GetTransform();
    const USceneComponent* Parent = GetAttachParent();
    FMatrix ParentInverse = FMatrix::Identity;
    if (Parent)
    {
        const FMatrix ParentWorld = Parent->GetWorldMatrix();
        const float Determinant = ParentWorld.Determinant();
        if (!std::isfinite(Determinant) || Determinant == 0.0f) return false;
        ParentInverse = ParentWorld.Inverse();
        Relative.Rotation = (Parent->GetWorldRotation().Quaternion().Inverse() * NewRotation).ToFRotator();
    }
    else Relative.Rotation = NewRotation.ToFRotator();

    if (Delta.IsZero())
    {
        if (NewRotation.Equals(GetWorldRotation().Quaternion())) return false;
        SetTransform(Relative);
        return true;
    }

    const FVector Start = GetWorldLocation();
    FVector Destination = Start + Delta;
    bool bBlocked = false;
    if (bSweep && (!GetOwner() || GetOwner()->GetActorEnableCollision()))
    {
        AActor* OwnerActor = GetOwner();
        UWorld* World = OwnerActor ? OwnerActor->GetWorld() : nullptr;
        if (!World) return false;
        const FRay Ray(Start, Delta.Normalized());
        bBlocked = World->SweepSingle(Ray, Delta.Size(), GetCollisionRadius(), Hit, OwnerActor);
        if (bBlocked)
            Destination = Start + Delta * Hit.Time + Hit.ImpactNormal * 0.1f;
    }
    Relative.Location = Parent ? ParentInverse.TransformPosition(Destination) : Destination;
    SetTransform(Relative); // Use the same property/dirty notification as editor changes.
    return !bBlocked;
}

void USceneComponent::SetVisibility(bool bNewVisibility, EVisibilityPropagation Propagation)
{
    const bool bChanged = bVisible != bNewVisibility;
    if (bChanged)
    {
        SetVisibleFlag(bNewVisibility);
        OnVisibilityChanged();
    }
    if (Propagation == EVisibilityPropagation::NoPropagation ||
        (Propagation == EVisibilityPropagation::DirtyOnly && !bChanged)) return;

    TArray<USceneComponent*> Pending;
    for (USceneComponent* Child : AttachChildren) if (Child) Pending.Add(Child);
    while (Pending.Num() > 0)
    {
        USceneComponent* Child = Pending[Pending.Num() - 1];
        Pending.RemoveLast();
        if (Propagation == EVisibilityPropagation::Propagate)
            Child->SetVisibility(bNewVisibility, EVisibilityPropagation::NoPropagation);
        // DirtyOnly preserves the descendant's own visibility flag.
        Child->MarkRenderStateDirty();
        for (USceneComponent* Descendant : Child->GetAttachChildren())
            if (Descendant) Pending.Add(Descendant);
    }
}

void USceneComponent::SetHiddenInGame(bool bNewHidden, EVisibilityPropagation Propagation)
{
    const bool bChanged = bHiddenInGame != bNewHidden;
    if (bChanged)
    {
        bHiddenInGame = bNewHidden;
        OnPropertyChanged("bHiddenInGame");
        OnHiddenInGameChanged();
    }
    if (Propagation == EVisibilityPropagation::NoPropagation ||
        (Propagation == EVisibilityPropagation::DirtyOnly && !bChanged)) return;

    TArray<USceneComponent*> Pending;
    for (USceneComponent* Child : AttachChildren) if (Child) Pending.Add(Child);
    while (Pending.Num() > 0)
    {
        USceneComponent* Child = Pending[Pending.Num() - 1];
        Pending.RemoveLast();
        if (Propagation == EVisibilityPropagation::Propagate)
            Child->SetHiddenInGame(bNewHidden, EVisibilityPropagation::NoPropagation);
        Child->MarkRenderStateDirty();
        for (USceneComponent* Descendant : Child->GetAttachChildren())
            if (Descendant) Pending.Add(Descendant);
    }
}

void USceneComponent::SetVisibleFlag(bool bNewVisible)
{
    if (bVisible == bNewVisible) return;
    bVisible = bNewVisible;
    OnPropertyChanged("bVisible");
}
