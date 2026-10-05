#include "EnginePCH.h"
#include "Editor/Viewports/EditorViewportClient.h"
#include "Component/PrimitiveComponent.h"
#include "Editor/Gizmo/Gizmo.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Input/InputSystem.h"
#include "Math/EngineMath.h"
#include "Render/RenderCommand.h"

FEditorViewportClient::FEditorViewportClient() {}

void FEditorViewportClient::Draw(FViewport *Viewport) {}

void FEditorViewportClient::Tick(float DeltaTime) {}

void FEditorViewportClient::TickInput(float DeltaTime, bool bCaptured,
                                      bool bHovered, int32 WheelDelta,
                                      float MoveSpeed, float MouseSensitivity) {

  if (bIsPerspective) {
    if (bCaptured) {
      // 마우스 회전 처리
      const float DeltaX = static_cast<float>(FInputSystem::GetMouseDeltaX());
      const float DeltaY = static_cast<float>(FInputSystem::GetMouseDeltaY());
      ViewRotation.Yaw += DeltaX * MouseSensitivity;
      ViewRotation.Pitch = FMath::Clamp(
          ViewRotation.Pitch + DeltaY * MouseSensitivity, -89.0f, 89.0f);

      // 키보드 이동 처리
      const FQuat RotationQuat = ViewRotation.Quaternion();
      const FVector Forward = RotationQuat.GetForwardVector();
      const FVector Right = RotationQuat.GetRightVector();
      const FVector Up = RotationQuat.GetUpVector();

      FVector MoveDir = FVector::ZeroVector;
      if (FInputSystem::IsKeyDown(EKeyCode::W))
        MoveDir += Forward;
      if (FInputSystem::IsKeyDown(EKeyCode::S))
        MoveDir -= Forward;
      if (FInputSystem::IsKeyDown(EKeyCode::D))
        MoveDir += Right;
      if (FInputSystem::IsKeyDown(EKeyCode::A))
        MoveDir -= Right;
      if (FInputSystem::IsKeyDown(EKeyCode::E))
        MoveDir += Up;
      if (FInputSystem::IsKeyDown(EKeyCode::Q))
        MoveDir -= Up;

      if (MoveDir.Size() > 0.0001f) {
        MoveDir = MoveDir.Normalized();
        ViewLocation += MoveDir * (MoveSpeed * DeltaTime);
      }
    }

    // 휠 줌 처리
    if ((bCaptured || bHovered) && WheelDelta != 0) {
      const FQuat RotationQuat = ViewRotation.Quaternion();
      const FVector Forward = RotationQuat.GetForwardVector();
      ViewLocation +=
          Forward * (static_cast<float>(WheelDelta) * 0.01f * MoveSpeed);
    }
  } else {
    const FQuat RotationQuat = ViewRotation.Quaternion();
    const FVector Right = RotationQuat.GetRightVector();
    const FVector Up = RotationQuat.GetUpVector();

    if (bCaptured) {
      // 직교 평면 드래그
      const float ViewWidth = Width > 0 ? static_cast<float>(Width) : 800.0f;
      const float WorldUnitsPerPixel = OrthoWidth / ViewWidth;
      const float DeltaX = static_cast<float>(FInputSystem::GetMouseDeltaX());
      const float DeltaY = static_cast<float>(FInputSystem::GetMouseDeltaY());

      const FVector PanOffset = (Right * (-DeltaX * WorldUnitsPerPixel)) +
                                (Up * (DeltaY * WorldUnitsPerPixel));
      ViewLocation += PanOffset;

      // 키보드 평면 이동
      FVector MoveDir = FVector::ZeroVector;
      if (FInputSystem::IsKeyDown(EKeyCode::W))
        MoveDir += Up;
      if (FInputSystem::IsKeyDown(EKeyCode::S))
        MoveDir -= Up;
      if (FInputSystem::IsKeyDown(EKeyCode::D))
        MoveDir += Right;
      if (FInputSystem::IsKeyDown(EKeyCode::A))
        MoveDir -= Right;

      if (MoveDir.Size() > 0.0001f) {
        MoveDir = MoveDir.Normalized();
        ViewLocation += MoveDir * (MoveSpeed * DeltaTime);
      }
    }

    // 직교 휠 줌
    if ((bCaptured || bHovered) && WheelDelta != 0) {
      const float ZoomFactor = WheelDelta > 0 ? 0.9f : 1.1f;
      OrthoWidth = FMath::Clamp(OrthoWidth * ZoomFactor, 0.1f, 100000.0f);
    }
  }
}

// 기즈모 조작 및 액터 피킹 갱신
void FEditorViewportClient::UpdateGizmoAndPicking(
    FGizmo *InGizmo, const FVector2 &LocalMousePosition,
    const std::function<void(AActor *)> &OnSelectActor) {
  if (!bActive || !InGizmo) {
    return;
  }

  const FVector2 ViewLocalMouse(LocalMousePosition.X - Rect.X,
                                LocalMousePosition.Y - Rect.Y);
  FRay Ray = DeprojectScreenToWorld(ViewLocalMouse);
  const FMatrix ViewProjection = GetViewProjectionMatrix();
  const bool bMouseDown = FInputSystem::IsMouseDown(EMouseButton::Left);

  InGizmo->Update(Ray, ViewLocalMouse, ViewProjection,
                  static_cast<int>(Rect.Width), static_cast<int>(Rect.Height),
                  bMouseDown, GetViewLocation(), !IsPerspective());

  if (FInputSystem::IsMousePressed(EMouseButton::Left) && !InGizmo->IsUsing() &&
      InGizmo->GetHoveredAxis() < 0) {
    FHitResult Hit;
    if (World && World->LineTraceSingle(Ray, Hit)) {
      if (Hit.HitComponent && Hit.HitComponent->GetOwner()) {
        if (OnSelectActor) {
          OnSelectActor(Hit.HitComponent->GetOwner());
        }
      }
    } else {
      if (OnSelectActor) {
        OnSelectActor(nullptr);
      }
    }
  }
}

void FEditorViewportClient::MouseMove(FViewport *Viewport, int32 X, int32 Y) {}

bool FEditorViewportClient::InputKey(FViewport *Viewport, int32 Key,
                                     bool bDown) {
  return false;
}

bool FEditorViewportClient::InputAxis(FViewport *Viewport, int32 ControllerId,
                                      float Delta, float DeltaTime) {
  return false;
}

// 뷰 행렬 계산
FMatrix FEditorViewportClient::GetViewMatrix() const {
  FQuat WorldQuat = ViewRotation.Quaternion();
  FMatrix RotationMatrix = WorldQuat.ToFMatrix();

  FMatrix InverseTranslation = FMatrix::MakeTranslation(ViewLocation * -1.0f);
  FMatrix InverseRotation = RotationMatrix.GetTransposed();

  return InverseTranslation * InverseRotation;
}

// 원근 투영 행렬
FMatrix FEditorViewportClient::GetPerspectiveMatrix() const {
  const float HalfFOV = FMath::DegreesToRadians(ViewFOV) * 0.5f;

  const float YScale = 1.0f / tan(HalfFOV);
  const float XScale = YScale / AspectRatio;

  const float ZScale = FarClip / (FarClip - NearClip);
  const float ZOffset = -NearClip * FarClip / (FarClip - NearClip);

  return FMatrix(0.0f, 0.0f, ZScale, 1.0f, XScale, 0.0f, 0.0f, 0.0f, 0.0f,
                 YScale, 0.0f, 0.0f, 0.0f, 0.0f, ZOffset, 0.0f);
}

// 직교 투영 행렬
FMatrix FEditorViewportClient::GetOrthogonalMatrix() const {
  const float Width = OrthoWidth;
  const float Height = Width / AspectRatio;

  const float XScale = 2.0f / Width;
  const float YScale = 2.0f / Height;

  const float ZScale = 1.0f / (FarClip - NearClip);
  const float ZOffset = -NearClip / (FarClip - NearClip);

  return FMatrix(0.0f, 0.0f, ZScale, 0.0f, XScale, 0.0f, 0.0f, 0.0f, 0.0f,
                 YScale, 0.0f, 0.0f, 0.0f, 0.0f, ZOffset, 1.0f);
}

// 투영 행렬
FMatrix FEditorViewportClient::GetProjectionMatrix() const {
  if (bIsPerspective) {
    return GetPerspectiveMatrix();
  }
  return GetOrthogonalMatrix();
}

// 뷰 프로젝션 행렬
FMatrix FEditorViewportClient::GetViewProjectionMatrix() const {
  return GetViewMatrix() * GetProjectionMatrix();
}

// 전방 벡터 계산
FVector FEditorViewportClient::GetViewForward() const {
  FQuat WorldQuat = ViewRotation.Quaternion();
  return WorldQuat.GetForwardVector();
}

// 씬 뷰 정보 구성
FSceneView FEditorViewportClient::CalcSceneView(const FRect &InViewRect) const {
  FSceneView OutView;
  OutView.ViewIndex = ViewIndex;
  OutView.ViewLocation = ViewLocation;
  OutView.ViewRotation = ViewRotation;
  OutView.ViewForward = GetViewForward();
  OutView.ViewMatrix = GetViewMatrix();
  OutView.ProjectionMatrix = GetProjectionMatrix();
  OutView.ViewProjectionMatrix = GetViewProjectionMatrix();
  OutView.Frustum = ExtractFrustumPlanes(OutView.ViewProjectionMatrix);
  OutView.ViewRect = InViewRect;
  OutView.FieldOfView = ViewFOV;
  OutView.AspectRatio = AspectRatio;
  OutView.NearClip = NearClip;
  OutView.FarClip = FarClip;
  OutView.OrthoWidth = OrthoWidth;
  OutView.bIsPerspective = bIsPerspective;
  OutView.bIsWireframe = bWireframe;
  return OutView;
}

// 씬 뷰 생성
FSceneView FEditorViewportClient::CalcSceneView() const {
  return CalcSceneView(
      FRect(0.0f, 0.0f, static_cast<float>(Width), static_cast<float>(Height)));
}

// 뷰포트 크기 변경 및 타깃 생성
void FEditorViewportClient::Resize(const uint32 InWidth,
                                   const uint32 InHeight) {
  if (Width == InWidth && Height == InHeight && ColorTarget && DepthTarget) {
    return;
  }

  const uint32 OldWidth = Width;
  const uint32 OldHeight = Height;

  Width = InWidth;
  Height = InHeight;

  D3D11_TEXTURE2D_DESC Desc{};
  Desc.Width = Width;
  Desc.Height = Height;
  Desc.MipLevels = 1;
  Desc.ArraySize = 1;
  Desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  Desc.SampleDesc.Count = 1;
  Desc.Usage = D3D11_USAGE_DEFAULT;
  Desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
  ColorTarget = RenderCommand::CreateTexture2D(Desc);

  Desc.Format = DXGI_FORMAT_R24G8_TYPELESS;
  Desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
  DepthTarget = RenderCommand::CreateTexture2D(Desc);


  if (Height > 0) {
    SetAspectRatio(static_cast<float>(Width) / static_cast<float>(Height));
  }

  if (!bIsPerspective && OldWidth > 1 && Width > 1) {
    // 화면 크기 변경 시 직교 배율 유지
    OrthoWidth *= static_cast<float>(Width) / static_cast<float>(OldWidth);
  }
}

// 뷰포트 시점 설정
void FEditorViewportClient::SetViewportType(const ELevelViewportType InType) {
  ViewportType = InType;
  switch (InType) {
  case ELevelViewportType::Perspective:
    bIsPerspective = true;
    ViewLocation = FVector(-5.0f, -10.0f, 10.0f);
    ViewRotation = FRotator(30.0f, 45.0f, 0.0f);
    break;
  case ELevelViewportType::Top:
    bIsPerspective = false;
    ViewLocation = FVector(0.0f, 0.0f, 20.0f);
    ViewRotation = FRotator(90.0f, 0.0f, 0.0f);
    break;
  case ELevelViewportType::Front:
    bIsPerspective = false;
    ViewLocation = FVector(20.0f, 0.0f, 0.0f);
    ViewRotation = FRotator(0.0f, 180.0f, 0.0f);
    break;
  case ELevelViewportType::Right:
    bIsPerspective = false;
    ViewLocation = FVector(0.0f, 20.0f, 0.0f);
    ViewRotation = FRotator(0.0f, -90.0f, 0.0f);
    break;
  }
}

// 화면 좌표 역투영
FRay FEditorViewportClient::DeprojectScreenToWorld(
    const FVector2 &ScreenPos) const {
  if (Width == 0 || Height == 0) {
    return FRay();
  }
  const float ScreenWidth = static_cast<float>(Width);
  const float ScreenHeight = static_cast<float>(Height);
  const float NdcX = (ScreenPos.X / ScreenWidth) * 2.0f - 1.0f;
  const float NdcY = 1.0f - (ScreenPos.Y / ScreenHeight) * 2.0f;
  const float Aspect =
      AspectRatio > 0.0f ? AspectRatio : (ScreenWidth / ScreenHeight);
  const FQuat Rotation = ViewRotation.Quaternion();
  const FVector Forward = Rotation.GetForwardVector();
  const FVector Right = Rotation.GetRightVector();
  const FVector Up = Rotation.GetUpVector();

  if (bIsPerspective) {
    const float Tangent = tanf(FMath::DegreesToRadians(ViewFOV) * 0.5f);
    const FVector Direction =
        (Forward + Right * (NdcX * Tangent * Aspect) + Up * (NdcY * Tangent))
            .Normalized();
    return FRay(ViewLocation, Direction);
  } else {
    const float OrthoHeight = OrthoWidth / Aspect;
    const FVector Origin = ViewLocation + Forward * NearClip +
                           Right * (NdcX * OrthoWidth * 0.5f) +
                           Up * (NdcY * OrthoHeight * 0.5f);
    return FRay(Origin, Forward.Normalized());
  }
}
