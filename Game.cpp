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
    // 추가
    // 총 실행 시간 계산
    auto time = static_cast<float>(timer.GetTotalSeconds());
    // 시간에 따른 애니메이션 로직(Y축 기준) -> 자전효과
    m_world = Matrix::CreateRotationY(time);
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
    // 셰이더 효과에 world 행렬 바인드
    m_effect->SetWorld(m_world);
    // 구체 렌더링(셰이더 효과, 및 inputLayout 적용)
    m_shape->Draw(m_effect.get(), m_inputLayout.Get());

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
    // DeviceContext 가져오기(렌더링 명령 실행)
    auto context = m_deviceResources->GetD3DDeviceContext();
    // 기본효과 초기화(텍스처, 조명)
    m_effect = std::make_unique<BasicEffect>(device);
    m_effect->SetTextureEnabled(true);      // 텍스처 사용 활성화
    m_effect->SetPerPixelLighting(true);    // 픽셀 단위 조명(고품질) 설정
    m_effect->SetLightingEnabled(true);     // 조명 연산 활성화
    // 0번 조명 설정(흰색 광원을 카메라 정면 방향에서 조사)
    m_effect->SetLightEnabled(0, true);
    m_effect->SetLightDiffuseColor(0, Colors::White);
    m_effect->SetLightDirection(0, -Vector3::UnitZ);

    // 수체 도형 생성 및 렌더 컨텍스트 설정
    m_shape = GeometricPrimitive::CreateSphere(context);
    // 도형의 정점 구조와 셰이더 효과를 연결하는 InputLayout 생성
    m_shape->CreateInputLayout(m_effect.get(),
        m_inputLayout.ReleaseAndGetAddressOf());

    // 텍스처 로드
    DX::ThrowIfFailed(
        CreateWICTextureFromFile(device, L"earth.bmp", nullptr,
            m_texture.ReleaseAndGetAddressOf()));
    // 셰이더 효과에 로드한 텍스처 바인딩
    m_effect->SetTexture(m_texture.Get());

    // world 행렬을 단위 행렬로 초기화
    m_world = Matrix::Identity;
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    // TODO: Initialize windows-size dependent objects here.
    // 추가
    // 현재 윈도우 출력 크기(해상도) 가져오기
    auto size = m_deviceResources->GetOutputSize();
    // view시점 행렬 초기화 : 카메라의 위치와 방향
    m_view = Matrix::CreateLookAt(Vector3(2.f, 2.f, 2.f),
        Vector3::Zero, Vector3::UnitY);
    // 투영행렬 설정 : 원근감 및 화면 비율
    m_proj = Matrix::CreatePerspectiveFieldOfView(XM_PI / 4.f,
        float(size.right) / float(size.bottom), 0.1f, 10.f);
    // 셰이더 효과에 View시점 바인드
    m_effect->SetView(m_view);
    // 셰이더 효과에 원근감 및 화면 비율 바인드
    m_effect->SetProjection(m_proj);
}

void Game::OnDeviceLost()
{
    // TODO: Add Direct3D resource cleanup here.
    // 추가
    // Device 손실 시 리소스 해제
    m_shape.reset();
    m_texture.Reset();
    m_effect.reset();
    m_inputLayout.Reset();
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion
