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
    // 위치와 노말 텍스처 정보를 가진 정점 구조체
    using VertexType = DirectX::VertexPositionNormalTexture;
    // 렌더링 상태 관리 객체(블렌딩, 깊이/스텐실, 래스터라이저 등)
    std::unique_ptr<DirectX::CommonStates> m_states;
    // 노말 맵 렌더링 효과 객체 생성
    std::unique_ptr<DirectX::NormalMapEffect> m_effect;
    // 동적으로 기본 도형(primitive)들을 그리기 위한 배치 시스템
    std::unique_ptr<DirectX::PrimitiveBatch<VertexType>> m_batch;
    // GPU에게 정점 데이터 형식을 알려주는 레이아웃(정점 구조 설명)
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
    // 텍스처 변수
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture;
    // 노말 맵 변수
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_normalMap;
};
