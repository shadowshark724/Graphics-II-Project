﻿#pragma once

#include "..\Common\DeviceResources.h"
#include "ShaderStructures.h"
#include "..\Common\StepTimer.h"
#include "Common\DDSTextureLoader.h"


namespace DX11UWA
{
	// This sample renderer instantiates a basic rendering pipeline.
	class Sample3DSceneRenderer
	{
	public:
		Sample3DSceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		void CreateDeviceDependentResources(void);
		void CreateWindowSizeDependentResources(void);
		void ReleaseDeviceDependentResources(void);
		void Update(DX::StepTimer const& timer);
		void Render(void);
		void StartTracking(void);
		void TrackingUpdate(float positionX);
		void StopTracking(void);
		inline bool IsTracking(void) { return m_tracking; }
		void LoadObjFile(const char * path, std::vector<VertexPositionUVNormal> & vertuvnorm, std::vector<unsigned int> &index, bool backwards);

		// Helper functions for keyboard and mouse input
		void SetKeyboardButtons(const char* list);
		void SetMousePosition(const Windows::UI::Input::PointerPoint^ pos);
		void SetInputDeviceData(const char* kb, const Windows::UI::Input::PointerPoint^ pos);


	private:
		void Rotate(float radians);
		void UpdateCamera(DX::StepTimer const& timer, float const moveSpd, float const rotSpd);

	private:
		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		// Direct3D resources for cube geometry.
		Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_inputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_indexBuffer;
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_constantBuffer;


		// Skybox
		Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_SkyboxinputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_SkyboxvertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_SkyboxindexBuffer;
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_SkyboxvertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_SkyboxpixelShader;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_SkyboxconstantBuffer;
		Microsoft::WRL::ComPtr<ID3D11SamplerState>  m_SkyboxSampleState;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_SkyboxResourceView;

		// Ditto
		Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_DittoinputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_DittovertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_DittoindexBuffer;
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_DittovertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_DittopixelShader;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_DittoconstantBuffer;
		Microsoft::WRL::ComPtr<ID3D11SamplerState>  m_DittoSampleState;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_DittoResourceView;
		
		// SimplePlatform
		Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_PlatforminputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_PlatformvertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_PlatformindexBuffer;
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_PlatformvertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_PlatformpixelShader;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_PlatformconstantBuffer;
		Microsoft::WRL::ComPtr<ID3D11SamplerState>  m_PlatformSampleState;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_PlatformResourceView;

		// System resources for cube geometry.
		ModelViewProjectionConstantBuffer	m_constantBufferData;
		uint32	m_indexCount;

		// Skybox
		ModelViewProjectionConstantBuffer	m_SkyboxconstantBufferData;
		uint32	m_SkyboxindexCount;


		// Ditto
		ModelViewProjectionConstantBuffer m_DittoconstantBufferData;
		uint32 m_DittoindexCount;

		// Ditto
		ModelViewProjectionConstantBuffer m_PlatformconstantBufferData;
		uint32 m_PlatformindexCount;

		// Variables used with the rendering loop.
		bool	m_loadingComplete;
		float	m_degreesPerSecond;
		bool	m_tracking;

		// Data members for keyboard and mouse input
		char	m_kbuttons[256];
		Windows::UI::Input::PointerPoint^ m_currMousePos;
		Windows::UI::Input::PointerPoint^ m_prevMousePos;

		// Matrix data member for the camera
		DirectX::XMFLOAT4X4 m_camera;
	};
}

