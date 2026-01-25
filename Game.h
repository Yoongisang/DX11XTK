//
// Game.h
//

#pragma once

#include "DeviceResources.h"
#include "StepTimer.h"
#include <memory>
#include "AnimatedTexture.h"
#include "ScrollingBackground.h"

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
    //  2D 스프라이트 그리기용 
    std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
    // 애니메이션 시킬 우주선관리 객체 AnimationTeture 헤더에서 정의
    std::unique_ptr<AnimatedTexture> m_ship;
    // 우주선 스프라이트 시트 텍스처 셰이더 리소스 뷰
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture;
    // 우주선의 화면상 위치
    DirectX::SimpleMath::Vector2 m_shipPos;
    // 배경 스크롤 애니메이션을 위해 배경 텍스처를 받을 객체 ScrollingBackground 헤더에서 정의
    std::unique_ptr<ScrollingBackground> m_stars;
    // 배경 텍스처 셰이더 리소스 뷰
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_backgroundTex;
};
