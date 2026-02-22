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
    // 공통 렌더 상태
    std::unique_ptr<DirectX::CommonStates> m_states;
    // 기본 셰이더 이펙트 
    std::unique_ptr<DirectX::BasicEffect> m_effect;
    // PrimitiveBatch: 즉시 모드 방식으로 점/선/삼각형을 GPU에 전달하는 헬퍼
    std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>> m_batch;
    // 정점 레이아웃
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
    // 오브젝트 변환 행렬
    DirectX::SimpleMath::Matrix m_world;
    //카메라 변환 행렬
    DirectX::SimpleMath::Matrix m_view;
    // 투영 변환 행렬
    DirectX::SimpleMath::Matrix m_proj;
};
