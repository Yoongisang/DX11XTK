//
// Game.cpp
//

#include "pch.h"
#include "Game.h"

extern void ExitGame() noexcept;

using namespace DirectX;

using Microsoft::WRL::ComPtr;
using namespace DirectX::SimpleMath;

Game::Game() noexcept(false)
{
    m_deviceResources = std::make_unique<DX::DeviceResources>();
    // TODO: Provide parameters for swapchain format, depth/stencil format, and backbuffer count.
    //   Add DX::DeviceResources::c_AllowTearing to opt-in to variable rate displays.
    //   Add DX::DeviceResources::c_EnableHDR for HDR10 display.
    m_deviceResources->RegisterDeviceNotify(this);
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{
    m_deviceResources->SetWindow(window, width, height);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

    // TODO: Change the timer settings if you want something other than the default variable timestep mode.
    // e.g. for 60 FPS fixed timestep update logic, call:
    /*
    m_timer.SetFixedTimeStep(true);
    m_timer.SetTargetElapsedSeconds(1.0 / 60);
    */
}

#pragma region Frame Update
// Executes the basic game loop.
void Game::Tick()
{
    m_timer.Tick([&]()
        {
            Update(m_timer);
        });

    Render();
}

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{
    float elapsedTime = float(timer.GetElapsedSeconds());

    // TODO: Add your game logic here.
    // 총 실행 시간으로 조명 회전 애니메이션
    auto time = static_cast<float>(m_timer.GetTotalSeconds());
    // 시간에 따른 각도 변화
    float yaw = time * 0.4f;
    float pitch = time * 0.7f;
    float roll = time * 1.1f;
    // 쿼터니언으로 회전 생성
    auto quat = Quaternion::CreateFromYawPitchRoll(pitch, yaw, roll);
    // 조명 방향을 회전
    auto light = XMVector3Rotate(g_XMOne, quat);
    // 조명 0번의 방향 설정
    m_effect->SetLightDirection(0, light);
}
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void Game::Render()
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    Clear();

    m_deviceResources->PIXBeginEvent(L"Render");
    auto context = m_deviceResources->GetD3DDeviceContext();

    // TODO: Add your rendering code here.
    // 추가
    // 렌더링 상태 설정(불투명)
    context->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
    // 깊이/스텐실 상태 설정(깊이 테스트 비활성화)
    context->OMSetDepthStencilState(m_states->DepthNone(), 0);
    // 래스터라이저 상태(컬링 없음)
    context->RSSetState(m_states->CullNone());
    // 셰이더에 렌더링 효과 적용
    m_effect->Apply(context);

    // 샘플러 상태 설정(텍스처 샘플링 방식)
    auto sampler = m_states->LinearClamp();
    context->PSSetSamplers(0, 1, &sampler);

    // 정점 입력 레이아웃 설정
    context->IASetInputLayout(m_inputLayout.Get());

    // 그리기 시작
    m_batch->Begin();
    // 픽셀 좌표로 정점 생성(좌표, 노말, 텍스처좌표)
    VertexPositionNormalTexture v1(Vector3(400.f, 150.f, 0.f), -Vector3::UnitZ, Vector2(.5f, 0));
    VertexPositionNormalTexture v2(Vector3(600.f, 450.f, 0.f), -Vector3::UnitZ, Vector2(1, 1));
    VertexPositionNormalTexture v3(Vector3(200.f, 450.f, 0.f), -Vector3::UnitZ, Vector2(0, 1));
    // 삼각형 그리기
    m_batch->DrawTriangle(v1, v2, v3);

    m_batch->End();

    m_deviceResources->PIXEndEvent();
    m_deviceResources->Present();
}

// Helper method to clear the back buffers.
void Game::Clear()
{
    m_deviceResources->PIXBeginEvent(L"Clear");

    // Clear the views.
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto renderTarget = m_deviceResources->GetRenderTargetView();
    auto depthStencil = m_deviceResources->GetDepthStencilView();

    context->ClearRenderTargetView(renderTarget, Colors::CornflowerBlue);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->OMSetRenderTargets(1, &renderTarget, depthStencil);

    // Set the viewport.
    const auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    m_deviceResources->PIXEndEvent();
}
#pragma endregion

#pragma region Message Handlers
// Message handlers
void Game::OnActivated()
{
    // TODO: Game is becoming active window.
}

void Game::OnDeactivated()
{
    // TODO: Game is becoming background window.
}

void Game::OnSuspending()
{
    // TODO: Game is being power-suspended (or minimized).
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();

    // TODO: Game is being power-resumed (or returning from minimize).
}

void Game::OnWindowMoved()
{
    const auto r = m_deviceResources->GetOutputSize();
    m_deviceResources->WindowSizeChanged(r.right, r.bottom);
}

void Game::OnDisplayChange()
{
    m_deviceResources->UpdateColorSpace();
}

void Game::OnWindowSizeChanged(int width, int height)
{
    if (!m_deviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();

    // TODO: Game window is being resized.
}

// Properties
void Game::GetDefaultSize(int& width, int& height) const noexcept
{
    // TODO: Change to desired default window size (note minimum size is 320x200).
    width = 800;
    height = 600;
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Game::CreateDeviceDependentResources()
{
    auto device = m_deviceResources->GetD3DDevice();

    // TODO: Initialize device dependent objects here (independent of window size).
    // 추가
    // 렌더링 상태 관리 객체 생성
    m_states = std::make_unique<CommonStates>(device);
    // 노말 맵 효과 객체 생성
    m_effect = std::make_unique<NormalMapEffect>(device);

    // InputLayout생성(Effect의 셰이더와 VertexType 연결)
    DX::ThrowIfFailed(
        CreateInputLayoutFromEffect<VertexType>(device, m_effect.get(),
            m_inputLayout.ReleaseAndGetAddressOf())
    );
    // 텍스처 파일 로드
    DX::ThrowIfFailed(
        CreateWICTextureFromFile(device, L"rocks.jpg", nullptr,
            m_texture.ReleaseAndGetAddressOf()));
    // 노말 맵 파일 로드
    DX::ThrowIfFailed(
        CreateDDSTextureFromFile(device, L"rocks_normalmap.dds", nullptr,
            m_normalMap.ReleaseAndGetAddressOf()));

    // 텍스처를 셰이더에 설정
    m_effect->SetTexture(m_texture.Get());
    // 노말 텍스처를 셰이더에 설정
    m_effect->SetNormalTexture(m_normalMap.Get());
    // 기본 조명 사용 활성화 
    m_effect->EnableDefaultLighting();
    // 조명 0번 확산광 색상 설정
    m_effect->SetLightDiffuseColor(0, Colors::Gray);
    //  DeviceContext 가져오기(렌더링 명령 실행)
    auto context = m_deviceResources->GetD3DDeviceContext();
    // 동적 기본 도형 렌더링 객체 생성
    m_batch = std::make_unique<PrimitiveBatch<VertexType>>(context);
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    // TODO: Initialize windows-size dependent objects here.
    // 추가
    // 윈도우 사이즈 크기 가져오기
    auto size = m_deviceResources->GetOutputSize();
    // 투영 행렬 생성
    Matrix proj = Matrix::CreateScale(2.f / float(size.right),
        -2.f / float(size.bottom), 1.f)
        * Matrix::CreateTranslation(-1.f, 1.f, 0.f);
    m_effect->SetProjection(proj);


}

void Game::OnDeviceLost()
{
    // TODO: Add Direct3D resource cleanup here.
    // 추가
    // Device 손실 시 리소스 해제
    m_states.reset();
    m_effect.reset();
    m_batch.reset();
    m_inputLayout.Reset();
    m_texture.Reset();
    m_normalMap.Reset();
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion
