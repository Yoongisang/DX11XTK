//
// Game.h
//

#pragma once

#include "DeviceResources.h"
#include "StepTimer.h"

#include <memory>


// A basic game implementation that creates a D3D11 device and
// provides a game loop.
class Game final : public DX::IDeviceNotify
{
public:

    Game() noexcept(false);
    ~Game() = default;

    Game(Game&&) = default;
    Game& operator= (Game&&) = default;

    Game(Game const&) = delete;
    Game& operator= (Game const&) = delete;

    // Initialization and management
    void Initialize(HWND window, int width, int height);

    // Basic game loop
    void Tick();

    // IDeviceNotify
    void OnDeviceLost() override;
    void OnDeviceRestored() override;

    // Messages
    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowMoved();
    void OnDisplayChange();
    void OnWindowSizeChanged(int width, int height);

    // Properties
    void GetDefaultSize( int& width, int& height ) const noexcept;

private:

    void Update(DX::StepTimer const& timer);
    void Render();

    void Clear();

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

    // Device resources.
    std::unique_ptr<DX::DeviceResources>    m_deviceResources;

    // Rendering loop timer.
    DX::StepTimer                           m_timer;
    // 추가
    // 고양이 텍스처
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture; 
    // 2D 스프라이트 렌더링 도구
    std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
    // 텍스처 그릴 화면 위치
    DirectX::SimpleMath::Vector2 m_screenPos;
    // 텍스처 원점(중심점)
    DirectX::SimpleMath::Vector2 m_origin;
    // 공용 렌더 상태 모음(알파모드용)
    std::unique_ptr<DirectX::CommonStates> m_states;
    // 타일링용 영역
    RECT m_tileRect;
    // 늘리기용 영역
    RECT m_stretchRect;
    // 전체 화면 영역(배경)
    RECT m_fullscreenRect;
    // 배경 텍스처
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_background;
};
