//
// Game.cpp
//

#include "pch.h"
#include "Game.h"

extern void ExitGame() noexcept;

using namespace DirectX;

using Microsoft::WRL::ComPtr;
using namespace DirectX::SimpleMath;

namespace
{
    // 4x MSAA (픽셀당 4샘플)
    constexpr UINT MSAA_COUNT = 4;
    // 품질 레벨 0 (기본값)
    constexpr UINT MSAA_QUALITY = 0;
}

Game::Game() noexcept(false)
{
    m_deviceResources = std::make_unique<DX::DeviceResources>(
        DXGI_FORMAT_B8G8R8A8_UNORM,
        DXGI_FORMAT_UNKNOWN // 깊이 버퍼 생성안함
   );
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
    // Y축 기준으로 회전 추가
    m_world = Matrix::CreateRotationY(cosf(static_cast<float>(timer.GetTotalSeconds())));
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
    // 블렌딩 없음
    context->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
    // 깊이 테스트 없음
    context->OMSetDepthStencilState(m_states->DepthNone(), 0);
    // 양면 그리기
    context->RSSetState(m_raster.Get());

    // 매 프레임 월드 행렬 갱신
    m_effect->SetWorld(m_world);
    // 셰이더 및 상수 버퍼 GPU에 적용
    m_effect->Apply(context);

    context->IASetInputLayout(m_inputLayout.Get());

    m_batch->Begin();

    // 그리드 범위: X축(-2 ~ 2), Z축(-2 ~ 2) (Y가 위 따라서 바닥은 XZ)
    Vector3 xaxis(2.f, 0.f, 0.f);
    Vector3 yaxis(0.f, 0.f, 2.f);
    Vector3 origin = Vector3::Zero;
    // 격자 분할 수 (선 개수 = divisions + 1
    constexpr size_t divisions = 20;
    
    // Z축 방향 선들(X위치를 -1 ~ 1로 등간격 배치)
    for (size_t i = 0; i <= divisions; i++)
    {
        float fPercnt = float(i) / float(divisions);
        // 0 ~ 1 -> -1 ~ 1정규화
        fPercnt = (fPercnt * 2.0f) - 1.0f;
        Vector3 scale = xaxis * fPercnt + origin;

        VertexPositionColor v1(scale - yaxis, Colors::White);
        VertexPositionColor v2(scale + yaxis, Colors::White);
        m_batch->DrawLine(v1, v2);
    }
    // X축 방향 선들(X위치를 -1 ~ 1로 등간격 배치)
    for (size_t i = 0; i <= divisions; i++)
    {
        float fPercnt = float(i) / float(divisions);
        // 0 ~ 1 -> -1 ~ 1정규화
        fPercnt = (fPercnt * 2.0f) - 1.0f;
        Vector3 scale = yaxis * fPercnt + origin;

        VertexPositionColor v1(scale - xaxis, Colors::White);
        VertexPositionColor v2(scale + xaxis, Colors::White);
        m_batch->DrawLine(v1, v2);
    }

    m_batch->End();

    // ... m_batch->End(); 다음

// MSAA 렌더 타겟 → 백버퍼로 Resolve (4샘플 → 1샘플 다운샘플)
    context->ResolveSubresource(
        m_deviceResources->GetRenderTarget(), 0,  // 목적지: 백버퍼
        m_offscreenRenderTarget.Get(), 0,         // 소스: MSAA 오프스크린 버퍼
        m_deviceResources->GetBackBufferFormat()
    );

    m_deviceResources->Present();
}

// Helper method to clear the back buffers.
void Game::Clear()
{
    m_deviceResources->PIXBeginEvent(L"Clear");

    // Clear the views.
    auto context = m_deviceResources->GetD3DDeviceContext();
    // 수정
    // 기존: DeviceResources 기본 RTV/DSV
    // 변경: MSAA 전용 RTV/DSV로 교체
    auto renderTarget = m_offscreenRenderTargetSRV.Get();
    auto depthStencil = m_depthStencilSRV.Get();

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
    // 초기 월드 행렬 = 단위 행렬
    m_world = Matrix::Identity;
    
    m_states = std::make_unique<CommonStates>(device);
    m_effect = std::make_unique<BasicEffect>(device);
    // 텍스처 대신정점별 색상 사용
    m_effect->SetVertexColorEnabled(true);

    // MultisampleEnable=TRUE, AntialiasedLineEnable=FALSE → MSAA 모드
    CD3D11_RASTERIZER_DESC rastDesc(
        D3D11_FILL_SOLID,
        D3D11_CULL_NONE,
        FALSE, // FrontCounterClockwise
        D3D11_DEFAULT_DEPTH_BIAS,
        D3D11_DEFAULT_DEPTH_BIAS_CLAMP,
        D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
        TRUE,  // DepthClipEnable
        FALSE, // ScissorEnable
        TRUE, // MultisampleEnable ← TRUE
        FALSE   // AntialiasedLineEnable ← FALSE
    );

    DX::ThrowIfFailed(device->CreateRasterizerState(&rastDesc,
        m_raster.ReleaseAndGetAddressOf()));
    // BasicEffect 기준으로 VertexPositionColor에 맞는 InputLayout 자동 생성
    DX::ThrowIfFailed(
        CreateInputLayoutFromEffect<VertexPositionColor>(device, m_effect.get(),
            m_inputLayout.ReleaseAndGetAddressOf())
    );

    auto context = m_deviceResources->GetD3DDeviceContext();
    m_batch = std::make_unique<PrimitiveBatch<VertexPositionColor>>(context);
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    // TODO: Initialize windows-size dependent objects here.
    // 추가 
    auto size = m_deviceResources->GetOutputSize();
    auto device = m_deviceResources->GetD3DDevice();
    auto width = static_cast<UINT>(size.right);
    auto height = static_cast<UINT>(size.bottom);

    // MSAA 렌더 타겟 텍스처 생성
    CD3D11_TEXTURE2D_DESC rtDesc(
        m_deviceResources->GetBackBufferFormat(),
        width, height, 1, 1,
        D3D11_BIND_RENDER_TARGET,
        D3D11_USAGE_DEFAULT, 0,
        MSAA_COUNT, MSAA_QUALITY  // ← 멀티샘플 설정
    );
    DX::ThrowIfFailed(device->CreateTexture2D(&rtDesc, nullptr,
        m_offscreenRenderTarget.ReleaseAndGetAddressOf()));

    // 렌더 타겟 뷰 (TEXTURE2DMS = 멀티샘플 텍스처용 차원)
    CD3D11_RENDER_TARGET_VIEW_DESC rtvDesc(D3D11_RTV_DIMENSION_TEXTURE2DMS);
    DX::ThrowIfFailed(device->CreateRenderTargetView(m_offscreenRenderTarget.Get(),
        &rtvDesc, m_offscreenRenderTargetSRV.ReleaseAndGetAddressOf()));

    // MSAA 깊이/스텐실 버퍼 생성
    // ※ Feature Level 9.x 장치는 DXGI_FORMAT_D24_UNORM_S8_UINT 사용
    CD3D11_TEXTURE2D_DESC dsDesc(
        DXGI_FORMAT_D32_FLOAT,
        width, height, 1, 1,
        D3D11_BIND_DEPTH_STENCIL,
        D3D11_USAGE_DEFAULT, 0,
        MSAA_COUNT, MSAA_QUALITY
    );
    ComPtr<ID3D11Texture2D> depthBuffer;
    DX::ThrowIfFailed(device->CreateTexture2D(&dsDesc, nullptr, depthBuffer.GetAddressOf()));

    CD3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc(D3D11_DSV_DIMENSION_TEXTURE2DMS);
    DX::ThrowIfFailed(device->CreateDepthStencilView(depthBuffer.Get(),
        &dsvDesc, m_depthStencilSRV.ReleaseAndGetAddressOf()));

    // 카메라: (2,2,2) 위치에서 원점(0,0,0)을 바라봄, Y촉이 위쪽
    m_view = Matrix::CreateLookAt(Vector3(2.f, 2.f, 2.f), 
        Vector3::Zero, Vector3::UnitY);
    // 원근 투영: FOV 45도, 화면 비율, 근거리 0.1 / 원거리 10
    m_proj = Matrix::CreatePerspectiveFieldOfView(XM_PI / 4.f,
        float(size.right) / float(size.bottom), 0.1f, 10.f);

    m_effect->SetView(m_view);
    m_effect->SetProjection(m_proj);
}

void Game::OnDeviceLost()
{
    // TODO: Add Direct3D resource cleanup here.
    // 추가
    // ComPtr → Reset(), unique_ptr → reset()
    m_states.reset();
    m_effect.reset();
    m_batch.reset();
    m_inputLayout.Reset();
    m_raster.Reset();
    m_offscreenRenderTarget.Reset();
    m_offscreenRenderTargetSRV.Reset();
    m_depthStencilSRV.Reset();
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion
