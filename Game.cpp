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
    elapsedTime;
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
    context;

    m_deviceResources->PIXEndEvent();

    // SpriteBatc를 사용한 2D 렌더링 시작
    m_spriteBatch->Begin();
    // 배경 이미지를 전체 화면 크기로 그리기
    m_spriteBatch->Draw(m_background.Get(), m_fullscreenRect);
    // 고양이 텍스처를 화면 중앙에 그리기(원점을 이미지 중심으로 설정)
    m_spriteBatch->Draw(m_texture.Get(), m_screenPos, nullptr,
        Colors::White, 0.f, m_origin);
    // 렌더링 종료 및 GPU에 제출
    m_spriteBatch->End();

    // Show the new frame.
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
    device;
    // 추가
    // SpriteBatch 생성(2D 스프라이트 렌더링용)
    auto context = m_deviceResources->GetD3DDeviceContext();
    m_spriteBatch = std::make_unique<SpriteBatch>(context);
    // cat.dds 텍스쳐 파일 로드
    ComPtr<ID3D11Resource> resource;
    DX::ThrowIfFailed(
        CreateDDSTextureFromFile(device, L"cat.dds",
            resource.GetAddressOf(),
            m_texture.ReleaseAndGetAddressOf()));
    // 텍스처 정보를 얻기 위해 ID3D11Texture2D로 캐스팅
    ComPtr<ID3D11Texture2D> cat;
    DX::ThrowIfFailed(resource.As(&cat));
    // 텍스처 크기 정보 가져오기
    CD3D11_TEXTURE2D_DESC catDesc;
    cat->GetDesc(&catDesc);
    // 원점을 텍스처 중심으로 설정(회전/위치 지정시 중심 기준으로 동작)
    m_origin.x = float(catDesc.Width / 2);
    m_origin.y = float(catDesc.Height / 2);
    // CommonStates 생성(상태 객체 모음): 알파모드 떄문에 추가했으나 DDS사용으로 불필요해짐
    //m_states = std::make_unique<CommonStates>(device);
    // 배경이미지 로드
    DX::ThrowIfFailed(
        CreateWICTextureFromFile(device, L"sunset.jpg", nullptr,
            m_background.ReleaseAndGetAddressOf()));
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    // TODO: Initialize windows-size dependent objects here.
    // 추가
    // 현재 윈도우 크기 가져오기
    auto size = m_deviceResources->GetOutputSize();
    // 화면 중앙 좌표 계산(고양이 텍스처 위치)
    m_screenPos.x = float(size.right) / 2.f;
    m_screenPos.y = float(size.bottom) / 2.f;
    // 화면 중앙에 절반 크기로 그릴 영역 계산(타일형태로 배치할때 사용)
    //m_stretchRect.left = size.right / 4;
    //m_stretchRect.top = size.bottom / 4;
    //m_stretchRect.right = m_stretchRect.left + size.right / 2;
    //m_stretchRect.bottom = m_stretchRect.top + size.bottom / 2;
    // 전체 화면 영역 저장(배경 이미지용)
    m_fullscreenRect = m_deviceResources->GetOutputSize();
}

void Game::OnDeviceLost()
{
    // TODO: Add Direct3D resource cleanup here.
    // 추가
    // 디바이스 손실 시 모든 GPU 리소스 해제
    m_texture.Reset();
    m_spriteBatch.reset();
    m_states.reset();
    m_background.Reset();

}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion
