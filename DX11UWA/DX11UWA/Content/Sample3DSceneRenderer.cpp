#include "pch.h"
#include "Sample3DSceneRenderer.h"

#include "..\Common\DirectXHelper.h"

using namespace DX11UWA;

using namespace DirectX;
using namespace Windows::Foundation;

// Loads vertex and pixel shaders from files and instantiates the cube geometry.
Sample3DSceneRenderer::Sample3DSceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_loadingComplete(false),
	m_degreesPerSecond(45),
	m_indexCount(0),
	m_tracking(false),
	m_deviceResources(deviceResources)
{
	memset(m_kbuttons, 0, sizeof(m_kbuttons));
	m_currMousePos = nullptr;
	m_prevMousePos = nullptr;
	memset(&m_camera, 0, sizeof(XMFLOAT4X4));

	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}

// Initializes view parameters when the window size changes.
void Sample3DSceneRenderer::CreateWindowSizeDependentResources(void)
{
	Size outputSize = m_deviceResources->GetOutputSize();
	float aspectRatio = outputSize.Width / outputSize.Height;
	//float fovAngleY = 70.0f * XM_PI / 180.0f;
	fovVal = 70.0f;
#pragma region viewport
	m_viewport1 = new D3D11_VIEWPORT;
	m_viewport1->Height = outputSize.Height;
	m_viewport1->Width = outputSize.Width;
	m_viewport1->MinDepth = 0.0f;
	m_viewport1->MaxDepth = 0.1f;
	m_viewport1->TopLeftX = 0.0f;
	m_viewport1->TopLeftY = 0.0f;


	m_viewport2 = new D3D11_VIEWPORT;
	m_viewport2->Height = outputSize.Height;
	m_viewport2->Width = outputSize.Width/2;
	m_viewport2->MinDepth = 0.0f;
	m_viewport2->MaxDepth = 0.1f;
	m_viewport2->TopLeftX = outputSize.Width / 2;
	m_viewport2->TopLeftY = 0.0f;
	
	m_viewport3 = new D3D11_VIEWPORT;
	m_viewport3->Height = outputSize.Height;
	m_viewport3->Width = outputSize.Width/2;
	m_viewport3->MinDepth = 0.0f;
	m_viewport3->MaxDepth = 0.1f;
	m_viewport3->TopLeftX = 0.0f;
	m_viewport3->TopLeftY = 0.0f;
#pragma endregion


	// This is a simple example of change that can be made when the app is in
	// portrait or snapped view.
	if (aspectRatio < 1.0f)
	{
		//fovAngleY *= 2.0f;
		fovVal *= 2.0f;
	}

	// Note that the OrientationTransform3D matrix is post-multiplied here
	// in order to correctly orient the scene to match the display orientation.
	// This post-multiplication step is required for any draw calls that are
	// made to the swap chain render target. For draw calls to other targets,
	// this transform should not be applied.

	// This sample makes use of a right-handed coordinate system using row-major matrices.
	XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovLH(fovVal, aspectRatio, nearPlane, farPlane);

	XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();

	XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

	XMStoreFloat4x4(&m_constantBufferData.projection, XMMatrixTranspose(perspectiveMatrix * orientationMatrix));
	XMStoreFloat4x4(&m_DittoconstantBufferData.projection, XMMatrixTranspose(perspectiveMatrix * orientationMatrix));
	XMStoreFloat4x4(&m_zamconstantBufferData.projection, XMMatrixTranspose(perspectiveMatrix * orientationMatrix));
	XMStoreFloat4x4(&m_LandconstantBufferData.projection, XMMatrixTranspose(perspectiveMatrix * orientationMatrix));
	XMStoreFloat4x4(&m_PlatformconstantBufferData.projection, XMMatrixTranspose(perspectiveMatrix * orientationMatrix));
	XMStoreFloat4x4(&m_SkyboxconstantBufferData.projection, XMMatrixTranspose(perspectiveMatrix * orientationMatrix));
	XMStoreFloat4x4(&m_textureCubeconstantBufferData.projection, XMMatrixTranspose(perspectiveMatrix * orientationMatrix));

	// Eye is at (0,0.7,1.5), looking at point (0,-0.1,0) with the up-vector along the y-axis.
	static const XMVECTORF32 eye = { 0.0f, 1.7f, -7.5f, 0.0f };
	static const XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
	static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

	XMStoreFloat4x4(&m_camera, XMMatrixInverse(nullptr, XMMatrixLookAtLH(eye, at, up)));
	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixLookAtLH(eye, at, up)));
	XMStoreFloat4x4(&m_DittoconstantBufferData.view, XMMatrixTranspose(XMMatrixLookAtLH(eye, at, up)));
	XMStoreFloat4x4(&m_zamconstantBufferData.view, XMMatrixTranspose(XMMatrixLookAtLH(eye, at, up)));
	XMStoreFloat4x4(&m_LandconstantBufferData.view, XMMatrixTranspose(XMMatrixLookAtLH(eye, at, up)));
	XMStoreFloat4x4(&m_PlatformconstantBufferData.view, XMMatrixTranspose(XMMatrixLookAtLH(eye, at, up)));
	XMStoreFloat4x4(&m_SkyboxconstantBufferData.view, XMMatrixTranspose(XMMatrixLookAtLH(eye, at, up)));
	XMStoreFloat4x4(&m_textureCubeconstantBufferData.view, XMMatrixTranspose(XMMatrixLookAtLH(eye, at, up)));
}

// Called once per frame, rotates the cube and calculates the model and view matrices.
void Sample3DSceneRenderer::Update(DX::StepTimer const& timer)
{
	if (!m_tracking)
	{
		// Convert degrees to radians, then convert seconds to rotation angle
		float radiansPerSecond = XMConvertToRadians(m_degreesPerSecond);
		double totalRotation = timer.GetTotalSeconds() * radiansPerSecond;
		float radians = static_cast<float>(fmod(totalRotation, XM_2PI));
		
		
		m_PlatformconstantBufferData.light.x =  radians;
		m_PlatformconstantBufferData.light.y = radians;
		m_PlatformconstantBufferData.light.z = radians;
		//m_DittoconstantBufferData = m_constantBufferData;
		//XMStoreFloat4x4(&m_PlatformconstantBufferData.light, XMMatrixTranspose(XMMatrixRotationZ(radians)));
		m_DittoconstantBufferData.light = m_PlatformconstantBufferData.light;
		m_constantBufferData.light = m_DittoconstantBufferData.light;
		m_LandconstantBufferData.light = m_DittoconstantBufferData.light;
		m_zamconstantBufferData.light = m_DittoconstantBufferData.light;
		Rotate(radians);
	}


	// Update or move camera here
	UpdateCamera(timer, 3.0f, 0.75f);

	//m_SkyboxconstantBufferData = m_constantBufferData;
	//XMStoreFloat4x4(&m_SkyboxconstantBufferData.model, XMMatrixTranspose(XMMatrixRotationY(3.1415f)));
	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixRotationY(0.0f)));
	XMStoreFloat4x4(&m_textureCubeconstantBufferData.model, XMMatrixTranspose(XMMatrixRotationY(0.0f)));

	//XMFLOAT3 trans = { 6.0f, .05f, 3.0f };
	//XMVECTOR translat = XMLoadFloat3(trans);

	XMStoreFloat4x4(&m_DittoconstantBufferData.model, XMMatrixTranspose(XMMatrixRotationY(3.1415f)));
	//XMStoreFloat4x4(&m_DittoconstantBufferData.model, XMMatrixTranslation(6.0f, .05f, 3.0f));
	XMStoreFloat4x4(&m_DittoconstantBufferData.model, XMMatrixTranspose(XMMatrixTranslation(6.0f, .05f, 3.0f)));
	XMStoreFloat4x4(&m_zamconstantBufferData.model, XMMatrixTranspose(XMMatrixRotationY(3.1415f)));
	XMStoreFloat4x4(&m_zamconstantBufferData.model, XMMatrixTranspose(XMMatrixTranslation(3.0f, .05f, 3.0f)));
	XMStoreFloat4x4(&m_LandconstantBufferData.model, XMMatrixTranspose(XMMatrixRotationY(3.1415f)));
	//XMStoreFloat4x4(&m_LandconstantBufferData.model, XMMatrixTranspose(XMMatrixTranslation(0.0f, .05f, 3.0f)));
	//m_PlatformconstantBufferData = m_constantBufferData;
	XMStoreFloat4x4(&m_PlatformconstantBufferData.model, XMMatrixTranspose(XMMatrixRotationY(3.1415f)));
	XMStoreFloat4x4(&m_PlatformconstantBufferData.model, XMMatrixTranspose(XMMatrixTranslation(0.0f, .05f, 3.0f)));
}

// Rotate the 3D cube model a set amount of radians.
void Sample3DSceneRenderer::Rotate(float radians)
{
	// Prepare to pass the updated model matrix to the shader
	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixRotationY(radians)));
	//XMStoreFloat4x4(&m_DittoconstantBufferData.model, XMMatrixTranspose(XMMatrixRotationY(radians)));
}

void Sample3DSceneRenderer::UpdateCamera(DX::StepTimer const& timer, float const moveSpd, float const rotSpd)
{
	const float delta_time = (float)timer.GetElapsedSeconds();

	if (m_kbuttons['W'])
	{
		XMMATRIX translation = XMMatrixTranslation(0.0f, 0.0f, moveSpd * delta_time);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}
	if (m_kbuttons['S'])
	{
		XMMATRIX translation = XMMatrixTranslation(0.0f, 0.0f, -moveSpd * delta_time);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}
	if (m_kbuttons['A'])
	{
		XMMATRIX translation = XMMatrixTranslation(-moveSpd * delta_time, 0.0f, 0.0f);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}
	if (m_kbuttons['D'])
	{
		XMMATRIX translation = XMMatrixTranslation(moveSpd * delta_time, 0.0f, 0.0f);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}
	if (m_kbuttons['X'])
	{
		XMMATRIX translation = XMMatrixTranslation( 0.0f, -moveSpd * delta_time, 0.0f);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}
	if (m_kbuttons[VK_SPACE])
	{
		XMMATRIX translation = XMMatrixTranslation( 0.0f, moveSpd * delta_time, 0.0f);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}
	if (m_kbuttons['C'])
	{
		m_viewbool = true;
	}
	if (m_kbuttons['V'])
	{
		m_viewbool = false;
	}
	bool wasmoded = false;
	if (m_kbuttons['U'])
	{
		fovVal += (moveSpd * delta_time)*5;
		wasmoded = true;
	}
	if (m_kbuttons['I'])
	{
		fovVal -= (moveSpd * delta_time) * 5;
		wasmoded = true;
	}
	if (m_kbuttons['J'])
	{
		nearPlane *= 1.4;
		wasmoded = true;
	}
	if (m_kbuttons['K'])
	{
			nearPlane /= 1.4;
			wasmoded = true;		
	}
	if (m_kbuttons['N'])
	{
		farPlane *=1.4;
		wasmoded = true;
	}
	if (m_kbuttons['M'])
	{
			farPlane /= 1.4;
			wasmoded = true;
	}
	if (m_currMousePos) 
	{
		if (m_currMousePos->Properties->IsRightButtonPressed && m_prevMousePos)
		{
			float dx = m_currMousePos->Position.X - m_prevMousePos->Position.X;
			float dy = m_currMousePos->Position.Y - m_prevMousePos->Position.Y;

			XMFLOAT4 pos = XMFLOAT4(m_camera._41, m_camera._42, m_camera._43, m_camera._44);

			m_camera._41 = 0;
			m_camera._42 = 0;
			m_camera._43 = 0;

			XMMATRIX rotX = XMMatrixRotationX(dy * rotSpd * delta_time);
			XMMATRIX rotY = XMMatrixRotationY(dx * rotSpd * delta_time);

			XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
			temp_camera = XMMatrixMultiply(rotX, temp_camera);
			temp_camera = XMMatrixMultiply(temp_camera, rotY);

			XMStoreFloat4x4(&m_camera, temp_camera);

			m_camera._41 = pos.x;
			m_camera._42 = pos.y;
			m_camera._43 = pos.z;
		}
		m_prevMousePos = m_currMousePos;
	}

	if (wasmoded)
	{
		Size outputSize = m_deviceResources->GetOutputSize();
		float aspectRatio = outputSize.Width / outputSize.Height;
		float fovAngleY = fovVal * XM_PI / 180.0f;
		XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, nearPlane, farPlane);
		XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();
		XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);
		XMStoreFloat4x4(&m_constantBufferData.projection, XMMatrixTranspose(perspectiveMatrix * orientationMatrix));
		XMStoreFloat4x4(&m_DittoconstantBufferData.projection, XMMatrixTranspose(perspectiveMatrix * orientationMatrix));
		XMStoreFloat4x4(&m_zamconstantBufferData.projection, XMMatrixTranspose(perspectiveMatrix * orientationMatrix));
		XMStoreFloat4x4(&m_LandconstantBufferData.projection, XMMatrixTranspose(perspectiveMatrix * orientationMatrix));
		XMStoreFloat4x4(&m_PlatformconstantBufferData.projection, XMMatrixTranspose(perspectiveMatrix * orientationMatrix));
		XMStoreFloat4x4(&m_SkyboxconstantBufferData.projection, XMMatrixTranspose(perspectiveMatrix * orientationMatrix));
		XMStoreFloat4x4(&m_textureCubeconstantBufferData.projection, XMMatrixTranspose(perspectiveMatrix * orientationMatrix));
	}

}

void Sample3DSceneRenderer::SetKeyboardButtons(const char* list)
{
	memcpy_s(m_kbuttons, sizeof(m_kbuttons), list, sizeof(m_kbuttons));
}

void Sample3DSceneRenderer::SetMousePosition(const Windows::UI::Input::PointerPoint^ pos)
{
	m_currMousePos = const_cast<Windows::UI::Input::PointerPoint^>(pos);
}

void Sample3DSceneRenderer::SetInputDeviceData(const char* kb, const Windows::UI::Input::PointerPoint^ pos)
{
	SetKeyboardButtons(kb);
	SetMousePosition(pos);
}

void DX11UWA::Sample3DSceneRenderer::StartTracking(void)
{
	m_tracking = true;
}

// When tracking, the 3D cube can be rotated around its Y axis by tracking pointer position relative to the output screen width.
void Sample3DSceneRenderer::TrackingUpdate(float positionX)
{
	if (m_tracking)
	{
		float radians = XM_2PI * 2.0f * positionX / m_deviceResources->GetOutputSize().Width;
		Rotate(radians);
	}
}

void Sample3DSceneRenderer::StopTracking(void)
{
	m_tracking = false;
}

// Renders one frame using the vertex and pixel shaders.
void Sample3DSceneRenderer::Render(void)
{
	// Loading is asynchronous. Only draw geometry after it's loaded.
	if (!m_loadingComplete)
	{
		return;
	}
	auto context = m_deviceResources->GetD3DDeviceContext();

	if (m_viewbool)
	{
		context->RSSetViewports(1, m_viewport2);
		Renderthing(context);

		context->RSSetViewports(1, m_viewport3);
		Renderthing(context);
	}
	else
	{
		context->RSSetViewports(1, m_viewport1);
		Renderthing(context);
	}
   
}

void Sample3DSceneRenderer::Renderthing(ID3D11DeviceContext3* context)
{
	XMFLOAT3 cam = { m_camera._41 ,m_camera._42 ,m_camera._43 };
	XMVECTOR camera = XMLoadFloat3(&cam);
	XMFLOAT3 dit = { m_DittoconstantBufferData.model._14 ,m_DittoconstantBufferData.model._24 ,m_DittoconstantBufferData.model._34 };
	XMVECTOR ditto = XMLoadFloat3(&dit);
	XMFLOAT3 plat = { m_PlatformconstantBufferData.model._14 ,m_PlatformconstantBufferData.model._24 ,m_PlatformconstantBufferData.model._34 };
	XMVECTOR platform = XMLoadFloat3(&plat);
	XMFLOAT3 za = { m_zamconstantBufferData.model._14 ,m_zamconstantBufferData.model._24 ,m_zamconstantBufferData.model._34 };
	XMVECTOR zam = XMLoadFloat3(&za);


	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_camera))));
	XMStoreFloat4x4(&m_SkyboxconstantBufferData.view, XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_camera))));
	XMStoreFloat4x4(&m_DittoconstantBufferData.view, XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_camera))));
	XMStoreFloat4x4(&m_zamconstantBufferData.view, XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_camera))));
	XMStoreFloat4x4(&m_LandconstantBufferData.view, XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_camera))));
	XMStoreFloat4x4(&m_PlatformconstantBufferData.view, XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_camera))));
	XMStoreFloat4x4(&m_textureCubeconstantBufferData.view, XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_camera))));
	//XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_camera))));

	ID3D11RenderTargetView * const targets[1] = { m_deviceResources->GetBackBufferRenderTargetView() };
	context->OMSetRenderTargets(1, targets, m_deviceResources->GetDepthStencilView());

	float blendFactor[4] = { 0.5f, 0.5f, 0.5f, 0.5f };
	UINT sampleMask = 0xffffffff;

	float blendFactorzero[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	sampleMask = 0xffffffff;

	context->OMSetBlendState(blenderstatereset.Get(), blendFactor, sampleMask);


#pragma region Pyrimid
	// Prepare the constant buffer to send it to the graphics device.
	context->UpdateSubresource1(m_constantBuffer.Get(), 0, NULL, &m_constantBufferData, 0, 0, 0);
	// Each vertex is one instance of the VertexPositionColor struct.
	UINT stride = sizeof(VertexPositionUVNormal);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
	// Each index is one 16-bit unsigned integer (short).
	context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(m_inputLayout.Get());
	// Attach our vertex shader.
	context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers1(0, 1, m_constantBuffer.GetAddressOf(), nullptr, nullptr);
	// Attach our pixel shader.
	context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

	context->PSSetShaderResources(0, 2, m_ResourceView[0].GetAddressOf());
	context->PSSetSamplers(0, 1, m_SampleState.GetAddressOf());
	// Draw the objects.
	//context->DrawIndexed(m_indexCount, 0, 0);
#pragma endregion

#pragma region skybox
	// Prepare the constant buffer to send it to the graphics device.
	context->UpdateSubresource1(m_SkyboxconstantBuffer.Get(), 0, NULL, &m_SkyboxconstantBufferData, 0, 0, 0);
	// Each vertex is one instance of the VertexPositionColor struct.
	stride = sizeof(VertexPositionUVNormal);
	offset = 0;
	context->IASetVertexBuffers(0, 1, m_SkyboxvertexBuffer.GetAddressOf(), &stride, &offset);
	// Each index is one 16-bit unsigned integer (short).
	context->IASetIndexBuffer(m_SkyboxindexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(m_SkyboxinputLayout.Get());
	// Attach our vertex shader.
	context->VSSetShader(m_SkyboxvertexShader.Get(), nullptr, 0);
	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers1(0, 1, m_SkyboxconstantBuffer.GetAddressOf(), nullptr, nullptr);
	// Attach our pixel shader.
	context->PSSetShader(m_SkyboxpixelShader.Get(), nullptr, 0);
	// Draw the objects.
	context->PSSetShaderResources(0, 1, m_SkyboxResourceView.GetAddressOf());
	context->PSSetSamplers(0, 1, m_SkyboxSampleState.GetAddressOf());

	context->DrawIndexed(m_SkyboxindexCount, 0, 0);
#pragma endregion

#pragma region Land
	// Prepare the constant buffer to send it to the graphics device.
	context->UpdateSubresource1(m_LandconstantBuffer.Get(), 0, NULL, &m_LandconstantBufferData, 0, 0, 0);
	// Each vertex is one instance of the VertexPositionColor struct.
	stride = sizeof(VertexPositionUVNormal);
	offset = 0;
	context->IASetVertexBuffers(0, 1, m_LandvertexBuffer.GetAddressOf(), &stride, &offset);
	// Each index is one 16-bit unsigned integer (short).
	context->IASetIndexBuffer(m_LandindexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(m_LandinputLayout.Get());
	// Attach our vertex shader.
	context->VSSetShader(m_LandvertexShader.Get(), nullptr, 0);
	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers1(0, 1, m_LandconstantBuffer.GetAddressOf(), nullptr, nullptr);
	// Attach our pixel shader.
	context->PSSetShader(m_LandpixelShader.Get(), nullptr, 0);
	// Draw the objects.
	context->PSSetShaderResources(0, 1, m_LandResourceView.GetAddressOf());
	context->PSSetSamplers(0, 1, m_LandSampleState.GetAddressOf());

	context->DrawIndexed(m_LandindexCount, 0, 0);
#pragma endregion


	if (XMVector2Greater(XMVector2Length(ditto - camera), XMVector2Length(zam - camera)) && XMVector2Greater(XMVector2Length(ditto - camera), XMVector2Length(platform - camera)))
	{
#pragma region Ditto
		// Prepare the constant buffer to send it to the graphics device.
		context->UpdateSubresource1(m_DittoconstantBuffer.Get(), 0, NULL, &m_DittoconstantBufferData, 0, 0, 0);
		// Each vertex is one instance of the VertexPositionColor struct.
		stride = sizeof(VertexPositionUVNormal);
		offset = 0;
		context->IASetVertexBuffers(0, 1, m_DittovertexBuffer.GetAddressOf(), &stride, &offset);
		// Each index is one 16-bit unsigned integer (short).
		context->IASetIndexBuffer(m_DittoindexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->IASetInputLayout(m_DittoinputLayout.Get());
		// Attach our vertex shader.
		context->VSSetShader(m_DittovertexShader.Get(), nullptr, 0);
		// Send the constant buffer to the graphics device.
		context->VSSetConstantBuffers1(0, 1, m_DittoconstantBuffer.GetAddressOf(), nullptr, nullptr);
		// Attach our pixel shader.
		context->PSSetShader(m_DittopixelShader.Get(), nullptr, 0);
		// Draw the objects.
		context->PSSetShaderResources(0, 1, m_DittoResourceView.GetAddressOf());
		context->PSSetSamplers(0, 1, m_DittoSampleState.GetAddressOf());

		context->OMSetBlendState(blenderstate.Get(), blendFactor, sampleMask);

		context->RSSetState(rasterstatefront.Get());

		context->DrawIndexed(m_DittoindexCount, 0, 0);

		context->RSSetState(rasterstateback.Get());

		context->DrawIndexed(m_DittoindexCount, 0, 0);


		context->OMSetBlendState(blenderstatereset.Get(), blendFactorzero, sampleMask);

		context->RSSetState(rasterstatereset.Get());

#pragma endregion
#pragma region zam
		// Prepare the constant buffer to send it to the graphics device.
		context->UpdateSubresource1(m_zamconstantBuffer.Get(), 0, NULL, &m_zamconstantBufferData, 0, 0, 0);
		// Each vertex is one instance of the VertexPositionColor struct.
		stride = sizeof(VertexPositionUVNormal);
		offset = 0;
		context->IASetVertexBuffers(0, 1, m_zamvertexBuffer.GetAddressOf(), &stride, &offset);
		// Each index is one 16-bit unsigned integer (short).
		context->IASetIndexBuffer(m_zamindexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->IASetInputLayout(m_zaminputLayout.Get());
		// Attach our vertex shader.
		context->VSSetShader(m_zamvertexShader.Get(), nullptr, 0);
		// Send the constant buffer to the graphics device.
		context->VSSetConstantBuffers1(0, 1, m_zamconstantBuffer.GetAddressOf(), nullptr, nullptr);
		// Attach our pixel shader.
		context->PSSetShader(m_zampixelShader.Get(), nullptr, 0);
		// Draw the objects.
		context->PSSetShaderResources(0, 1, m_zamResourceView.GetAddressOf());
		context->PSSetSamplers(0, 1, m_zamSampleState.GetAddressOf());

		context->OMSetBlendState(blenderstate.Get(), blendFactor, sampleMask);

		context->RSSetState(rasterstatefront.Get());

		context->DrawIndexed(m_zamindexCount, 0, 0);

		context->RSSetState(rasterstateback.Get());

		context->DrawIndexed(m_zamindexCount, 0, 0);

#pragma endregion
#pragma region Platform
		// Prepare the constant buffer to send it to the graphics device.
		context->UpdateSubresource1(m_PlatformconstantBuffer.Get(), 0, NULL, &m_PlatformconstantBufferData, 0, 0, 0);
		// Each vertex is one instance of the VertexPositionColor struct.
		stride = sizeof(VertexPositionUVNormal);
		offset = 0;
		context->IASetVertexBuffers(0, 1, m_PlatformvertexBuffer.GetAddressOf(), &stride, &offset);
		// Each index is one 16-bit unsigned integer (short).
		context->IASetIndexBuffer(m_PlatformindexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->IASetInputLayout(m_PlatforminputLayout.Get());
		// Attach our vertex shader.
		context->VSSetShader(m_PlatformvertexShader.Get(), nullptr, 0);
		// Send the constant buffer to the graphics device.
		context->VSSetConstantBuffers1(0, 1, m_PlatformconstantBuffer.GetAddressOf(), nullptr, nullptr);
		// Attach our pixel shader.
		context->PSSetShader(m_PlatformpixelShader.Get(), nullptr, 0);
		// Draw the objects.
		context->PSSetShaderResources(0, 1, m_PlatformResourceView.GetAddressOf());
		context->PSSetSamplers(0, 1, m_PlatformSampleState.GetAddressOf());

		context->OMSetBlendState(blenderstate.Get(), blendFactor, sampleMask);

		context->RSSetState(rasterstatefront.Get());

		context->DrawIndexed(m_PlatformindexCount, 0, 0);

		context->RSSetState(rasterstateback.Get());

		context->DrawIndexed(m_PlatformindexCount, 0, 0);

#pragma endregion
	}
	else if (XMVector2Greater(XMVector2Length(platform - camera), XMVector2Length(zam - camera)) && XMVector2Greater(XMVector2Length(platform - camera), XMVector2Length(ditto - camera)))
	{
#pragma region Platform
		// Prepare the constant buffer to send it to the graphics device.
		context->UpdateSubresource1(m_PlatformconstantBuffer.Get(), 0, NULL, &m_PlatformconstantBufferData, 0, 0, 0);
		// Each vertex is one instance of the VertexPositionColor struct.
		stride = sizeof(VertexPositionUVNormal);
		offset = 0;
		context->IASetVertexBuffers(0, 1, m_PlatformvertexBuffer.GetAddressOf(), &stride, &offset);
		// Each index is one 16-bit unsigned integer (short).
		context->IASetIndexBuffer(m_PlatformindexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->IASetInputLayout(m_PlatforminputLayout.Get());
		// Attach our vertex shader.
		context->VSSetShader(m_PlatformvertexShader.Get(), nullptr, 0);
		// Send the constant buffer to the graphics device.
		context->VSSetConstantBuffers1(0, 1, m_PlatformconstantBuffer.GetAddressOf(), nullptr, nullptr);
		// Attach our pixel shader.
		context->PSSetShader(m_PlatformpixelShader.Get(), nullptr, 0);
		// Draw the objects.
		context->PSSetShaderResources(0, 1, m_PlatformResourceView.GetAddressOf());
		context->PSSetSamplers(0, 1, m_PlatformSampleState.GetAddressOf());

		context->OMSetBlendState(blenderstate.Get(), blendFactor, sampleMask);

		context->RSSetState(rasterstatefront.Get());

		context->DrawIndexed(m_PlatformindexCount, 0, 0);

		context->RSSetState(rasterstateback.Get());

		context->DrawIndexed(m_PlatformindexCount, 0, 0);

#pragma endregion
#pragma region zam
		// Prepare the constant buffer to send it to the graphics device.
		context->UpdateSubresource1(m_zamconstantBuffer.Get(), 0, NULL, &m_zamconstantBufferData, 0, 0, 0);
		// Each vertex is one instance of the VertexPositionColor struct.
		stride = sizeof(VertexPositionUVNormal);
		offset = 0;
		context->IASetVertexBuffers(0, 1, m_zamvertexBuffer.GetAddressOf(), &stride, &offset);
		// Each index is one 16-bit unsigned integer (short).
		context->IASetIndexBuffer(m_zamindexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->IASetInputLayout(m_zaminputLayout.Get());
		// Attach our vertex shader.
		context->VSSetShader(m_zamvertexShader.Get(), nullptr, 0);
		// Send the constant buffer to the graphics device.
		context->VSSetConstantBuffers1(0, 1, m_zamconstantBuffer.GetAddressOf(), nullptr, nullptr);
		// Attach our pixel shader.
		context->PSSetShader(m_zampixelShader.Get(), nullptr, 0);
		// Draw the objects.
		context->PSSetShaderResources(0, 1, m_zamResourceView.GetAddressOf());
		context->PSSetSamplers(0, 1, m_zamSampleState.GetAddressOf());

		context->OMSetBlendState(blenderstate.Get(), blendFactor, sampleMask);

		context->RSSetState(rasterstatefront.Get());

		context->DrawIndexed(m_zamindexCount, 0, 0);

		context->RSSetState(rasterstateback.Get());

		context->DrawIndexed(m_zamindexCount, 0, 0);

#pragma endregion
#pragma region Ditto
		// Prepare the constant buffer to send it to the graphics device.
		context->UpdateSubresource1(m_DittoconstantBuffer.Get(), 0, NULL, &m_DittoconstantBufferData, 0, 0, 0);
		// Each vertex is one instance of the VertexPositionColor struct.
		stride = sizeof(VertexPositionUVNormal);
		offset = 0;
		context->IASetVertexBuffers(0, 1, m_DittovertexBuffer.GetAddressOf(), &stride, &offset);
		// Each index is one 16-bit unsigned integer (short).
		context->IASetIndexBuffer(m_DittoindexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->IASetInputLayout(m_DittoinputLayout.Get());
		// Attach our vertex shader.
		context->VSSetShader(m_DittovertexShader.Get(), nullptr, 0);
		// Send the constant buffer to the graphics device.
		context->VSSetConstantBuffers1(0, 1, m_DittoconstantBuffer.GetAddressOf(), nullptr, nullptr);
		// Attach our pixel shader.
		context->PSSetShader(m_DittopixelShader.Get(), nullptr, 0);
		// Draw the objects.
		context->PSSetShaderResources(0, 1, m_DittoResourceView.GetAddressOf());
		context->PSSetSamplers(0, 1, m_DittoSampleState.GetAddressOf());

		context->OMSetBlendState(blenderstate.Get(), blendFactor, sampleMask);

		context->RSSetState(rasterstatefront.Get());

		context->DrawIndexed(m_DittoindexCount, 0, 0);

		context->RSSetState(rasterstateback.Get());

		context->DrawIndexed(m_DittoindexCount, 0, 0);


		context->OMSetBlendState(blenderstatereset.Get(), blendFactorzero, sampleMask);

		context->RSSetState(rasterstatereset.Get());

#pragma endregion
	}
	else
	{
#pragma region zam
		// Prepare the constant buffer to send it to the graphics device.
		context->UpdateSubresource1(m_zamconstantBuffer.Get(), 0, NULL, &m_zamconstantBufferData, 0, 0, 0);
		// Each vertex is one instance of the VertexPositionColor struct.
		stride = sizeof(VertexPositionUVNormal);
		offset = 0;
		context->IASetVertexBuffers(0, 1, m_zamvertexBuffer.GetAddressOf(), &stride, &offset);
		// Each index is one 16-bit unsigned integer (short).
		context->IASetIndexBuffer(m_zamindexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->IASetInputLayout(m_zaminputLayout.Get());
		// Attach our vertex shader.
		context->VSSetShader(m_zamvertexShader.Get(), nullptr, 0);
		// Send the constant buffer to the graphics device.
		context->VSSetConstantBuffers1(0, 1, m_zamconstantBuffer.GetAddressOf(), nullptr, nullptr);
		// Attach our pixel shader.
		context->PSSetShader(m_zampixelShader.Get(), nullptr, 0);
		// Draw the objects.
		context->PSSetShaderResources(0, 1, m_zamResourceView.GetAddressOf());
		context->PSSetSamplers(0, 1, m_zamSampleState.GetAddressOf());

		context->OMSetBlendState(blenderstate.Get(), blendFactor, sampleMask);

		context->RSSetState(rasterstatefront.Get());

		context->DrawIndexed(m_zamindexCount, 0, 0);

		context->RSSetState(rasterstateback.Get());

		context->DrawIndexed(m_zamindexCount, 0, 0);

#pragma endregion
	}


	context->OMSetRenderTargets(1, m_textureCubeRenderTargetView.GetAddressOf(), m_deviceResources->GetDepthStencilView());
	context->ClearRenderTargetView(m_textureCubeRenderTargetView.Get(), DirectX::Colors::Purple);
	context->ClearDepthStencilView(m_deviceResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0.0f);
	context->OMSetBlendState(blenderstatereset.Get(), blendFactor, sampleMask);

#pragma region dupes
#pragma region Pyrimid
	// Prepare the constant buffer to send it to the graphics device.
	context->UpdateSubresource1(m_constantBuffer.Get(), 0, NULL, &m_constantBufferData, 0, 0, 0);
	// Each vertex is one instance of the VertexPositionColor struct.
	stride = sizeof(VertexPositionUVNormal);
	offset = 0;
	context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
	// Each index is one 16-bit unsigned integer (short).
	context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(m_inputLayout.Get());
	// Attach our vertex shader.
	context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers1(0, 1, m_constantBuffer.GetAddressOf(), nullptr, nullptr);
	// Attach our pixel shader.
	context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

	context->PSSetShaderResources(0, 2, m_ResourceView[0].GetAddressOf());
	context->PSSetSamplers(0, 1, m_SampleState.GetAddressOf());
	// Draw the objects.
	//context->DrawIndexed(m_indexCount, 0, 0);
#pragma endregion

#pragma region skybox
	// Prepare the constant buffer to send it to the graphics device.
	context->UpdateSubresource1(m_SkyboxconstantBuffer.Get(), 0, NULL, &m_SkyboxconstantBufferData, 0, 0, 0);
	// Each vertex is one instance of the VertexPositionColor struct.
	stride = sizeof(VertexPositionUVNormal);
	offset = 0;
	context->IASetVertexBuffers(0, 1, m_SkyboxvertexBuffer.GetAddressOf(), &stride, &offset);
	// Each index is one 16-bit unsigned integer (short).
	context->IASetIndexBuffer(m_SkyboxindexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(m_SkyboxinputLayout.Get());
	// Attach our vertex shader.
	context->VSSetShader(m_SkyboxvertexShader.Get(), nullptr, 0);
	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers1(0, 1, m_SkyboxconstantBuffer.GetAddressOf(), nullptr, nullptr);
	// Attach our pixel shader.
	context->PSSetShader(m_SkyboxpixelShader.Get(), nullptr, 0);
	// Draw the objects.
	context->PSSetShaderResources(0, 1, m_SkyboxResourceView.GetAddressOf());
	context->PSSetSamplers(0, 1, m_SkyboxSampleState.GetAddressOf());

	context->DrawIndexed(m_SkyboxindexCount, 0, 0);
#pragma endregion

#pragma region Land
	// Prepare the constant buffer to send it to the graphics device.
	context->UpdateSubresource1(m_LandconstantBuffer.Get(), 0, NULL, &m_LandconstantBufferData, 0, 0, 0);
	// Each vertex is one instance of the VertexPositionColor struct.
	stride = sizeof(VertexPositionUVNormal);
	offset = 0;
	context->IASetVertexBuffers(0, 1, m_LandvertexBuffer.GetAddressOf(), &stride, &offset);
	// Each index is one 16-bit unsigned integer (short).
	context->IASetIndexBuffer(m_LandindexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(m_LandinputLayout.Get());
	// Attach our vertex shader.
	context->VSSetShader(m_LandvertexShader.Get(), nullptr, 0);
	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers1(0, 1, m_LandconstantBuffer.GetAddressOf(), nullptr, nullptr);
	// Attach our pixel shader.
	context->PSSetShader(m_LandpixelShader.Get(), nullptr, 0);
	// Draw the objects.
	context->PSSetShaderResources(0, 1, m_LandResourceView.GetAddressOf());
	context->PSSetSamplers(0, 1, m_LandSampleState.GetAddressOf());

	context->DrawIndexed(m_LandindexCount, 0, 0);
#pragma endregion

#pragma region Ditto
	// Prepare the constant buffer to send it to the graphics device.
	context->UpdateSubresource1(m_DittoconstantBuffer.Get(), 0, NULL, &m_DittoconstantBufferData, 0, 0, 0);
	// Each vertex is one instance of the VertexPositionColor struct.
	stride = sizeof(VertexPositionUVNormal);
	offset = 0;
	context->IASetVertexBuffers(0, 1, m_DittovertexBuffer.GetAddressOf(), &stride, &offset);
	// Each index is one 16-bit unsigned integer (short).
	context->IASetIndexBuffer(m_DittoindexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(m_DittoinputLayout.Get());
	// Attach our vertex shader.
	context->VSSetShader(m_DittovertexShader.Get(), nullptr, 0);
	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers1(0, 1, m_DittoconstantBuffer.GetAddressOf(), nullptr, nullptr);
	// Attach our pixel shader.
	context->PSSetShader(m_DittopixelShader.Get(), nullptr, 0);
	// Draw the objects.
	context->PSSetShaderResources(0, 1, m_DittoResourceView.GetAddressOf());
	context->PSSetSamplers(0, 1, m_DittoSampleState.GetAddressOf());

	context->OMSetBlendState(blenderstate.Get(), blendFactor, sampleMask);

	context->RSSetState(rasterstatefront.Get());

	context->DrawIndexed(m_DittoindexCount, 0, 0);

	context->RSSetState(rasterstateback.Get());

	context->DrawIndexed(m_DittoindexCount, 0, 0);


	context->OMSetBlendState(blenderstatereset.Get(), blendFactorzero, sampleMask);

	context->RSSetState(rasterstatereset.Get());

#pragma endregion
#pragma region zam
	// Prepare the constant buffer to send it to the graphics device.
	context->UpdateSubresource1(m_zamconstantBuffer.Get(), 0, NULL, &m_zamconstantBufferData, 0, 0, 0);
	// Each vertex is one instance of the VertexPositionColor struct.
	stride = sizeof(VertexPositionUVNormal);
	offset = 0;
	context->IASetVertexBuffers(0, 1, m_zamvertexBuffer.GetAddressOf(), &stride, &offset);
	// Each index is one 16-bit unsigned integer (short).
	context->IASetIndexBuffer(m_zamindexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(m_zaminputLayout.Get());
	// Attach our vertex shader.
	context->VSSetShader(m_zamvertexShader.Get(), nullptr, 0);
	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers1(0, 1, m_zamconstantBuffer.GetAddressOf(), nullptr, nullptr);
	// Attach our pixel shader.
	context->PSSetShader(m_zampixelShader.Get(), nullptr, 0);
	// Draw the objects.
	context->PSSetShaderResources(0, 1, m_zamResourceView.GetAddressOf());
	context->PSSetSamplers(0, 1, m_zamSampleState.GetAddressOf());

	context->OMSetBlendState(blenderstate.Get(), blendFactor, sampleMask);

	context->RSSetState(rasterstatefront.Get());

	context->DrawIndexed(m_zamindexCount, 0, 0);

	context->RSSetState(rasterstateback.Get());

	context->DrawIndexed(m_zamindexCount, 0, 0);

#pragma endregion
#pragma region Platform
	// Prepare the constant buffer to send it to the graphics device.
	context->UpdateSubresource1(m_PlatformconstantBuffer.Get(), 0, NULL, &m_PlatformconstantBufferData, 0, 0, 0);
	// Each vertex is one instance of the VertexPositionColor struct.
	stride = sizeof(VertexPositionUVNormal);
	offset = 0;
	context->IASetVertexBuffers(0, 1, m_PlatformvertexBuffer.GetAddressOf(), &stride, &offset);
	// Each index is one 16-bit unsigned integer (short).
	context->IASetIndexBuffer(m_PlatformindexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(m_PlatforminputLayout.Get());
	// Attach our vertex shader.
	context->VSSetShader(m_PlatformvertexShader.Get(), nullptr, 0);
	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers1(0, 1, m_PlatformconstantBuffer.GetAddressOf(), nullptr, nullptr);
	// Attach our pixel shader.
	context->PSSetShader(m_PlatformpixelShader.Get(), nullptr, 0);
	// Draw the objects.
	context->PSSetShaderResources(0, 1, m_PlatformResourceView.GetAddressOf());
	context->PSSetSamplers(0, 1, m_PlatformSampleState.GetAddressOf());

	context->OMSetBlendState(blenderstate.Get(), blendFactor, sampleMask);

	context->RSSetState(rasterstatefront.Get());

	context->DrawIndexed(m_PlatformindexCount, 0, 0);

	context->RSSetState(rasterstateback.Get());

	context->DrawIndexed(m_PlatformindexCount, 0, 0);

#pragma endregion
	context->OMSetBlendState(blenderstatereset.Get(), blendFactor, sampleMask);
#pragma endregion

#pragma region texture cube
	// Prepare the constant buffer to send it to the graphics device.
	context->UpdateSubresource1(m_textureCubeconstantBuffer.Get(), 0, NULL, &m_textureCubeconstantBufferData, 0, 0, 0);

	context->OMSetRenderTargets(1, targets, m_deviceResources->GetDepthStencilView());
	// Each vertex is one instance of the VertexPositionColor struct.
	stride = sizeof(VertexPositionUVNormal);
	offset = 0;
	context->IASetVertexBuffers(0, 1, m_textureCubevertexBuffer.GetAddressOf(), &stride, &offset);
	// Each index is one 16-bit unsigned integer (short).
	context->IASetIndexBuffer(m_textureCubeindexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(m_textureCubeinputLayout.Get());
	// Attach our vertex shader.
	context->VSSetShader(m_textureCubevertexShader.Get(), nullptr, 0);
	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers1(0, 1, m_textureCubeconstantBuffer.GetAddressOf(), nullptr, nullptr);
	// Attach our pixel shader.
	context->PSSetShader(m_textureCubepixelShader.Get(), nullptr, 0);

	context->PSSetShaderResources(0, 1, m_textureCubeResourceView.GetAddressOf());
	context->PSSetSamplers(0, 1, m_textureCubeSampleState.GetAddressOf());
	// Draw the objects.
	context->DrawIndexed(m_textureCubeindexCount, 0, 0);
#pragma endregion
}

void Sample3DSceneRenderer::LoadObjFile(const char * path, std::vector<VertexPositionUVNormal> & vertuvnorm, std::vector<unsigned int> &index, bool backwards)
{
	std::vector<unsigned int> vertIndices, uvIndices, normIndices;
	std::vector<XMFLOAT3> temp_vert;
	std::vector<XMFLOAT3> temp_uv;
	std::vector<XMFLOAT3> temp_norm;

	vertuvnorm.clear();
	index.clear();

	uint32 vert1 = 0, vert2 = 0, vert3 = 0, uv1 = 0, uv2 = 0, uv3 = 0, norm1 = 0, norm2 = 0, norm3 = 0;

#pragma warning(disable : 4996) 
	FILE * file = fopen(path, "r");

	int count = 0;
	while (true)
	{
		char linelead[128];


		int res = fscanf(file, "%s\n", linelead);
		if (res == EOF)
		{
			break;
		}

		OutputDebugStringA(linelead);

		if (strcmp(linelead, "v") == 0)
		{
			++count;
			XMFLOAT3 vert;
			fscanf(file, "%f %f %f\n", &vert.x, &vert.y, &vert.z);
			vert.x /= 100;
			vert.y /= 100;
			vert.z /= 100;
			temp_vert.push_back(vert);
		}
		else if (strcmp(linelead, "vt") == 0)
		{
			++count;
			XMFLOAT3 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y);
			uv.y = 1 - uv.y;
			uv.z = 1;
			temp_uv.push_back(uv);
		}
		else if (strcmp(linelead, "vn") == 0)
		{
			++count;
			XMFLOAT3 norm;
			fscanf(file, "%f %f %f\n", &norm.x, &norm.y, &norm.z);
			temp_norm.push_back(norm);
		}
		else if (strcmp(linelead, "f") == 0)
		{
			++count;
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vert1, &uv1, &norm1, &vert2, &uv2, &norm2, &vert3, &uv3, &norm3);
			if (matches != 9)
			{
				return;
			}
			if (backwards == false)
			{
				vertIndices.push_back(vert1);
				vertIndices.push_back(vert2);
				vertIndices.push_back(vert3);
				uvIndices.push_back(uv1);
				uvIndices.push_back(uv2);
				uvIndices.push_back(uv3);
				normIndices.push_back(norm1);
				normIndices.push_back(norm2);
				normIndices.push_back(norm3);				
			}
			else
			{
				vertIndices.push_back(vert1);
				vertIndices.push_back(vert3);
				vertIndices.push_back(vert2);
				uvIndices.push_back(uv1);
				uvIndices.push_back(uv3);
				uvIndices.push_back(uv2);
				normIndices.push_back(norm1);
				normIndices.push_back(norm3);
				normIndices.push_back(norm2);
			}
		}
	}
	for (unsigned int i = 0; i < vertIndices.size(); i++)
	{
		VertexPositionUVNormal tempo;
		tempo.pos = temp_vert[vertIndices[i] - 1];
		tempo.uv = temp_uv[uvIndices[i] - 1];
		tempo.normal = temp_norm[normIndices[i] - 1];

		vertuvnorm.push_back(tempo);
		index.push_back(i);
	}
	return;
}

void Sample3DSceneRenderer::CreateDeviceDependentResources(void)
{
	CD3D11_RASTERIZER_DESC rasterfront = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT());
	CD3D11_RASTERIZER_DESC rasterback = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT());
	CD3D11_RASTERIZER_DESC rasterreset = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT());
	CD3D11_BLEND_DESC blenddesc = CD3D11_BLEND_DESC(CD3D11_DEFAULT());
	CD3D11_BLEND_DESC blenddescreset = CD3D11_BLEND_DESC(CD3D11_DEFAULT());

	rasterfront.FillMode = D3D11_FILL_SOLID;
	rasterfront.CullMode = D3D11_CULL_FRONT;
	rasterfront.DepthBias = 100;
	DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateRasterizerState(&rasterfront, &rasterstatefront));

	rasterback.FillMode = D3D11_FILL_SOLID;
	rasterback.CullMode = D3D11_CULL_BACK;
	rasterback.DepthBias = 100;
	DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateRasterizerState(&rasterback, &rasterstateback));

	rasterback.FillMode = D3D11_FILL_SOLID;
	rasterback.CullMode = D3D11_CULL_NONE;
	rasterback.DepthBias = 100;
	DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateRasterizerState(&rasterback, &rasterstateback));

	blenddesc.AlphaToCoverageEnable = TRUE;
	blenddesc.RenderTarget[0].BlendEnable = TRUE;
	blenddesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
	blenddesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_DEST_ALPHA;
	blenddesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blenddesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blenddesc.RenderTarget[0].DestBlend = D3D11_BLEND_DEST_ALPHA;
	blenddesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blenddesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;


	DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBlendState(&blenddesc, &blenderstate));

	blenddescreset.AlphaToCoverageEnable = TRUE;
	blenddescreset.RenderTarget[0].BlendEnable = FALSE;
	blenddescreset.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
	blenddescreset.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_DEST_ALPHA;
	blenddescreset.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blenddescreset.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blenddescreset.RenderTarget[0].DestBlend = D3D11_BLEND_DEST_ALPHA;
	blenddescreset.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blenddescreset.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;


	DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBlendState(&blenddescreset, &blenderstatereset));

#pragma region pyrimid
	// Load shaders asynchronously.
	auto loadVSTask = DX::ReadDataAsync(L"NormVertexShader.cso");
	auto loadPSTask = DX::ReadDataAsync(L"NormPixelShader.cso");

	// After the vertex shader file is loaded, create the shader and input layout.
	auto createVSTask = loadVSTask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateVertexShader(&fileData[0], fileData.size(), nullptr, &m_vertexShader));

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "UV", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateInputLayout(vertexDesc, ARRAYSIZE(vertexDesc), &fileData[0], fileData.size(), &m_inputLayout));
	});

	// After the pixel shader file is loaded, create the shader and constant buffer.
	auto createPSTask = loadPSTask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreatePixelShader(&fileData[0], fileData.size(), nullptr, &m_pixelShader));

		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, &m_constantBuffer));
	});

	// Once both shaders are loaded, create the mesh.
	auto createCubeTask = (createPSTask && createVSTask).then([this]()
	{
		// Load mesh vertices. Each vertex has a position and a color.
		static const VertexPositionUVNormal cubeVertices[] =
		{
			{ XMFLOAT3(-10.0f, -1.0f, -10.0f), XMFLOAT3(0.10f, 0.10f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
			{ XMFLOAT3(-10.0f, -1.0f, 10.0f), XMFLOAT3(0.10f, .90f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
			{ XMFLOAT3(10.0f,  -1.0f, -10.0f), XMFLOAT3(.90f, 0.10f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
			{ XMFLOAT3(10.0f,  -1.0f,  10.0f) ,XMFLOAT3(.90f, .90f,  1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		};

		XMFLOAT3 vertEdge0;
		vertEdge0.x= cubeVertices[1].pos.x - cubeVertices[0].pos.x;
		vertEdge0.y = cubeVertices[1].pos.y - cubeVertices[0].pos.y;
		vertEdge0.z = cubeVertices[1].pos.z - cubeVertices[0].pos.z;



		D3D11_SAMPLER_DESC textsample_desc;
		ZeroMemory(&textsample_desc, sizeof(textsample_desc));
		textsample_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		textsample_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		textsample_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		textsample_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateSamplerState(&textsample_desc, &m_SampleState));
	//	DX::ThrowIfFailed(CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"Assets/MidBoss_Floor_Diffuse.dds", NULL, &m_ResourceView[0]));
		DX::ThrowIfFailed(CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"Assets/normtest.dds", NULL, &m_ResourceView[1]));

		/*D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
		vertexBufferData.pSysMem = vertuvposnorm.data();
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(vertuvposnorm.size() * sizeof(VertexPositionUVNormal), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&vertexBufferDesc,
				&vertexBufferData,
				&m_PlatformvertexBuffer
			)
		);

		m_PlatformindexCount = index.size();

		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = index.data();
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(index.size() * sizeof(unsigned int), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&indexBufferDesc,
				&indexBufferData,
				&m_PlatformindexBuffer
			)
		);*/

		D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
		vertexBufferData.pSysMem = cubeVertices;
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(cubeVertices), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_vertexBuffer));

		// Load mesh indices. Each trio of indices represents
		// a triangle to be rendered on the screen.
		// For example: 0,2,1 means that the vertices with indexes
		// 0, 2 and 1 from the vertex buffer compose the 
		// first triangle of this mesh.
		static const unsigned int cubeIndices[] =
		{
			0,3,2,
			0,1,3,
		};	
		m_indexCount = ARRAYSIZE(cubeIndices);

		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = cubeIndices;
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(sizeof(cubeIndices), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&indexBufferDesc, &indexBufferData, &m_indexBuffer));
	});

	// Once the cube is loaded, the object is ready to be rendered.
	createCubeTask.then([this]()
	{
		m_loadingComplete = true;
	});
#pragma endregion

#pragma region skybox
	auto skyloadVSTask = DX::ReadDataAsync(L"SkyVertexShader.cso");
	auto skyloadPSTask = DX::ReadDataAsync(L"SkyPixelShader.cso");

	// After the vertex shader file is loaded, create the shader and input layout.
	auto skycreateVSTask = skyloadVSTask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateVertexShader(&fileData[0], fileData.size(), nullptr, &m_SkyboxvertexShader));

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "UV", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateInputLayout(vertexDesc, ARRAYSIZE(vertexDesc), &fileData[0], fileData.size(), &m_SkyboxinputLayout));
	});

	// After the pixel shader file is loaded, create the shader and constant buffer.
	auto skycreatePSTask = skyloadPSTask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreatePixelShader(&fileData[0], fileData.size(), nullptr, &m_SkyboxpixelShader));

		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, &m_SkyboxconstantBuffer));
	});

	// Once both shaders are loaded, create the mesh.
	auto createSkyTask = (skycreatePSTask && skycreateVSTask).then([this]()
	{
		// Load mesh vertices. Each vertex has a position and a color.
		static const VertexPositionUVNormal cubeVertices[] =
		{
			{ XMFLOAT3(-50.5f, -50.5f, -50.5f), XMFLOAT3(-1.0f, -1.0f, -1.0f),XMFLOAT3(1.0f, 1.0f, 1.0f) },
			{ XMFLOAT3(-50.5f, -50.5f,  50.5f), XMFLOAT3(-1.0f, -1.0f, 1.0f) ,XMFLOAT3(1.0f, 1.0f, -1.0f) },
			{ XMFLOAT3(-50.5f,  50.5f, -50.5f), XMFLOAT3(-1.0f, 1.0f, -1.0f) ,XMFLOAT3(1.0f, -1.0f, 1.0f) },
			{ XMFLOAT3(-50.5f,  50.5f,  50.5f), XMFLOAT3(-1.0f, 1.0f, 1.0f)  ,XMFLOAT3(1.0f, -1.0f, -1.0f) },
			{ XMFLOAT3(50.5f, -50.5f, -50.5f), XMFLOAT3(1.0f, -1.0f, -1.0f)  ,XMFLOAT3(-1.0f, 1.0f, 1.0f) },
			{ XMFLOAT3(50.5f, -50.5f,  50.5f), XMFLOAT3(1.0f, -1.0f, 1.0f)   ,XMFLOAT3(-1.0f, 1.0f, -1.0f) },
			{ XMFLOAT3(50.5f,  50.5f, -50.5f), XMFLOAT3(1.0f, 1.0f, -1.0f)   ,XMFLOAT3(-1.0f, 1.0f, 1.0f) },
			{ XMFLOAT3(50.5f,  50.5f,  50.5f), XMFLOAT3(1.0f, 1.0f, 1.0f)    ,XMFLOAT3(-1.0f, -1.0f, -1.0f) },
		};

		D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
		vertexBufferData.pSysMem = cubeVertices;
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(cubeVertices), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_SkyboxvertexBuffer));

		D3D11_SAMPLER_DESC textsample_desc;
		ZeroMemory(&textsample_desc, sizeof(textsample_desc));
		textsample_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		textsample_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		textsample_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		textsample_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateSamplerState(&textsample_desc, &m_SkyboxSampleState));
		DX::ThrowIfFailed(CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"Assets/SunsetSkybox.dds", NULL, &m_SkyboxResourceView));

		// Load mesh indices. Each trio of indices represents
		// a triangle to be rendered on the screen.
		// For example: 0,2,1 means that the vertices with indexes
		// 0, 2 and 1 from the vertex buffer compose the 
		// first triangle of this mesh.
		static const unsigned short cubeIndices[] =
		{
			2,1,0, // -x
			2,3,1,

			5,6,4, // +x
			7,6,5,

			1,5,0, // -y
			5,4,0,

			6,7,2, // +y
			7,3,2,

			4,6,0, // -z
			6,2,0,

			3,7,1, // +z
			7,5,1,
		};

		m_SkyboxindexCount = ARRAYSIZE(cubeIndices);

		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = cubeIndices;
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(sizeof(cubeIndices), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&indexBufferDesc, &indexBufferData, &m_SkyboxindexBuffer));
	});

	// Once the cube is loaded, the object is ready to be rendered.
	createSkyTask.then([this]()
	{
		m_loadingComplete = true;
	});
#pragma endregion

#pragma region Ditto
	// Load shaders asynchronously.
	auto DloadVSTask = DX::ReadDataAsync(L"MyVertexShader.cso");
	auto DloadPSTask = DX::ReadDataAsync(L"MyPixelShader.cso");

	// After the vertex shader file is loaded, create the shader and input layout.
	auto createDittoVSTask = DloadVSTask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateVertexShader(&fileData[0], fileData.size(), nullptr, &m_DittovertexShader));

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "UV", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateInputLayout(vertexDesc, ARRAYSIZE(vertexDesc), &fileData[0], fileData.size(), &m_DittoinputLayout));
	});

	// After the pixel shader file is loaded, create the shader and constant buffer.
	auto createDittoPSTask = DloadPSTask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreatePixelShader(&fileData[0], fileData.size(), nullptr, &m_DittopixelShader));

		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, &m_DittoconstantBuffer));
	});



	auto createModel = (createDittoPSTask && createDittoVSTask).then([this]()
	{

		std::vector<VertexPositionUVNormal> vertuvposnorm;
		std::vector<unsigned int> index;

		LoadObjFile("Assets/spyro.obj", vertuvposnorm, index, false);
		
		for (unsigned int i = 0; i < vertuvposnorm.size(); i++)
		{
			vertuvposnorm[i].pos.x *= 95;
			vertuvposnorm[i].pos.x += 0.0f;
			vertuvposnorm[i].pos.y *= 95;
			vertuvposnorm[i].pos.y += 0.0f;
			vertuvposnorm[i].pos.z *= 95;
			vertuvposnorm[i].pos.z -= 0.0f;
		}
		D3D11_SAMPLER_DESC textsample_desc;
		ZeroMemory(&textsample_desc, sizeof(textsample_desc));
		textsample_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		textsample_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		textsample_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		textsample_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateSamplerState(&textsample_desc, &m_DittoSampleState));
		DX::ThrowIfFailed(CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"Assets/spyro.dds", NULL, &m_DittoResourceView));

		D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
		vertexBufferData.pSysMem = vertuvposnorm.data();
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(vertuvposnorm.size() * sizeof(VertexPositionUVNormal), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&vertexBufferDesc,
				&vertexBufferData,
				&m_DittovertexBuffer
			)
		);

		m_DittoindexCount = index.size();

		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = index.data();
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(index.size() * sizeof(unsigned int), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&indexBufferDesc,
				&indexBufferData,
				&m_DittoindexBuffer
			)
		);
	});

	createModel.then([this]() {
		m_loadingComplete = true;
	});
#pragma endregion

#pragma region land
	// Load shaders asynchronously.
	auto LloadVSTask = DX::ReadDataAsync(L"MyVertexShader.cso");
	auto LloadPSTask = DX::ReadDataAsync(L"MyPixelShader.cso");

	// After the vertex shader file is loaded, create the shader and input layout.
	auto createLandVSTask = LloadVSTask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateVertexShader(&fileData[0], fileData.size(), nullptr, &m_LandvertexShader));

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "UV", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateInputLayout(vertexDesc, ARRAYSIZE(vertexDesc), &fileData[0], fileData.size(), &m_LandinputLayout));
	});

	// After the pixel shader file is loaded, create the shader and constant buffer.
	auto createLandPSTask = LloadPSTask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreatePixelShader(&fileData[0], fileData.size(), nullptr, &m_LandpixelShader));

		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, &m_LandconstantBuffer));
	});


	auto createLandModel = (createLandPSTask && createLandVSTask).then([this]()
	{

		std::vector<VertexPositionUVNormal> vertuvposnorm;
		std::vector<unsigned int> index;

		LoadObjFile("Assets/Artisans Hub.obj", vertuvposnorm, index, false);
		for (unsigned int i = 0; i < vertuvposnorm.size(); i++)
		{
			vertuvposnorm[i].pos.x *= 25;
			vertuvposnorm[i].pos.y *= 25;
			vertuvposnorm[i].pos.z *= 25;
		}
		D3D11_SAMPLER_DESC textsample_desc;
		ZeroMemory(&textsample_desc, sizeof(textsample_desc));
		textsample_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		textsample_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		textsample_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		textsample_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateSamplerState(&textsample_desc, &m_LandSampleState));
		DX::ThrowIfFailed(CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"Assets/High.dds", NULL, &m_LandResourceView));

		D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
		vertexBufferData.pSysMem = vertuvposnorm.data();
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(vertuvposnorm.size() * sizeof(VertexPositionUVNormal), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&vertexBufferDesc,
				&vertexBufferData,
				&m_LandvertexBuffer
			)
		);

		m_LandindexCount = index.size();

		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = index.data();
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(index.size() * sizeof(unsigned int), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&indexBufferDesc,
				&indexBufferData,
				&m_LandindexBuffer
			)
		);
	});

	createLandModel.then([this]() {
		m_loadingComplete = true;
	});
#pragma endregion

#pragma region Platform
	// Load shaders asynchronously.
	auto PloadVSTask = DX::ReadDataAsync(L"MyVertexShader.cso");
	auto PloadPSTask = DX::ReadDataAsync(L"MyPixelShader.cso");

	// After the vertex shader file is loaded, create the shader and input layout.
	auto createPlatformVSTask = PloadVSTask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateVertexShader(&fileData[0], fileData.size(), nullptr, &m_PlatformvertexShader));

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "UV", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateInputLayout(vertexDesc, ARRAYSIZE(vertexDesc), &fileData[0], fileData.size(), &m_PlatforminputLayout));
	});

	// After the pixel shader file is loaded, create the shader and constant buffer.
	auto createPlatformPSTask = PloadPSTask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreatePixelShader(&fileData[0], fileData.size(), nullptr, &m_PlatformpixelShader));

		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, &m_PlatformconstantBuffer));
	});


	auto createPlatform = (createPlatformPSTask && createPlatformVSTask).then([this]()
	{

		std::vector<VertexPositionUVNormal> vertuvposnorm;
		std::vector<unsigned int> index;

		LoadObjFile("Assets/Ngin.obj", vertuvposnorm, index,false);
		for (unsigned int i = 0; i < vertuvposnorm.size(); i++)
		{
			vertuvposnorm[i].pos.x *= 3;
			vertuvposnorm[i].pos.x += 0.0f;
			vertuvposnorm[i].pos.y *= 3;
			vertuvposnorm[i].pos.y += 0.0f;
			vertuvposnorm[i].pos.z *= 3;
			vertuvposnorm[i].pos.z -= 0.0f;
		}
		D3D11_SAMPLER_DESC textsample_desc;
		ZeroMemory(&textsample_desc, sizeof(textsample_desc));
		textsample_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		textsample_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		textsample_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		textsample_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateSamplerState(&textsample_desc, &m_PlatformSampleState));
		DX::ThrowIfFailed(CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"Assets/Ngin.dds", NULL, &m_PlatformResourceView));

		D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
		vertexBufferData.pSysMem = vertuvposnorm.data();
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(vertuvposnorm.size() * sizeof(VertexPositionUVNormal), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&vertexBufferDesc,
				&vertexBufferData,
				&m_PlatformvertexBuffer
			)
		);

		m_PlatformindexCount = index.size();

		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = index.data();
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(index.size() * sizeof(unsigned int), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&indexBufferDesc,
				&indexBufferData,
				&m_PlatformindexBuffer
			)
		);
	});

	createModel.then([this]() {
		m_loadingComplete = true;
	});
#pragma endregion

#pragma region Zam
	// Load shaders asynchronously.
	auto ZloadVSTask = DX::ReadDataAsync(L"MyVertexShader.cso");
	auto ZloadPSTask = DX::ReadDataAsync(L"MyPixelShader.cso");

	// After the vertex shader file is loaded, create the shader and input layout.
	auto createZamVSTask = ZloadVSTask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateVertexShader(&fileData[0], fileData.size(), nullptr, &m_zamvertexShader));

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "UV", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateInputLayout(vertexDesc, ARRAYSIZE(vertexDesc), &fileData[0], fileData.size(), &m_zaminputLayout));
	});

	// After the pixel shader file is loaded, create the shader and constant buffer.
	auto createZamPSTask = ZloadPSTask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreatePixelShader(&fileData[0], fileData.size(), nullptr, &m_zampixelShader));

		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, &m_zamconstantBuffer));
	});



	auto createzamModel = (createZamPSTask && createZamVSTask).then([this]()
	{

		std::vector<VertexPositionUVNormal> vertuvposnorm;
		std::vector<unsigned int> index;

		LoadObjFile("Assets/zam.obj", vertuvposnorm, index, false);

		for (unsigned int i = 0; i < vertuvposnorm.size(); i++)
		{
			vertuvposnorm[i].pos.x *= 5;
			vertuvposnorm[i].pos.x += 0.0f;
			vertuvposnorm[i].pos.y *= 5;
			vertuvposnorm[i].pos.y += 0.00f;
			vertuvposnorm[i].pos.z *= 5;
			vertuvposnorm[i].pos.z -= 0.0f;
		
		}

		D3D11_SAMPLER_DESC textsample_desc;
		ZeroMemory(&textsample_desc, sizeof(textsample_desc));
		textsample_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		textsample_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		textsample_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		textsample_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateSamplerState(&textsample_desc, &m_zamSampleState));
		DX::ThrowIfFailed(CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"Assets/zam.dds", NULL, &m_zamResourceView));

		D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
		vertexBufferData.pSysMem = vertuvposnorm.data();
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(vertuvposnorm.size() * sizeof(VertexPositionUVNormal), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&vertexBufferDesc,
				&vertexBufferData,
				&m_zamvertexBuffer
			)
		);

		m_zamindexCount = index.size();

		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = index.data();
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(index.size() * sizeof(unsigned int), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&indexBufferDesc,
				&indexBufferData,
				&m_zamindexBuffer
			)
		);
	});

	createzamModel.then([this]() {
		m_loadingComplete = true;
	});
#pragma endregion

#pragma region texturebox
	auto cubeloadVSTask = DX::ReadDataAsync(L"MyVertexShader.cso");
	auto cubeloadPSTask = DX::ReadDataAsync(L"PostPixelShader.cso");

	// After the vertex shader file is loaded, create the shader and input layout.
	auto cubecreateVSTask = cubeloadVSTask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateVertexShader(&fileData[0], fileData.size(), nullptr, &m_textureCubevertexShader));

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "UV", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateInputLayout(vertexDesc, ARRAYSIZE(vertexDesc), &fileData[0], fileData.size(), &m_textureCubeinputLayout));
	});

	// After the pixel shader file is loaded, create the shader and constant buffer.
	auto cubecreatePSTask = cubeloadPSTask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreatePixelShader(&fileData[0], fileData.size(), nullptr, &m_textureCubepixelShader));

		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, &m_textureCubeconstantBuffer));
	});

	// Once both shaders are loaded, create the mesh.
	auto createtextureCubeTask = (cubecreatePSTask && cubecreateVSTask).then([this]()
	{
		D3D11_TEXTURE2D_DESC desc2d;
		ZeroMemory(&desc2d, sizeof(desc2d));
		Size outputSize = m_deviceResources->GetOutputSize();
		desc2d.MipLevels = 1;
		desc2d.Width = outputSize.Width;
		desc2d.Height = outputSize.Height;
		desc2d.ArraySize = 1;
		desc2d.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		desc2d.SampleDesc.Count = 1;
		desc2d.Usage = D3D11_USAGE_DEFAULT;
		desc2d.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		desc2d.CPUAccessFlags = 0;
		desc2d.MiscFlags = 0;

		m_deviceResources->GetD3DDevice()->CreateTexture2D(&desc2d, NULL, &m_textureRenderTargetTextureMap);

		D3D11_RENDER_TARGET_VIEW_DESC targetviewdesc;
        targetviewdesc.Format = desc2d.Format;
        targetviewdesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        targetviewdesc.Texture2D.MipSlice = 0;

        m_deviceResources->GetD3DDevice()->CreateRenderTargetView(m_textureRenderTargetTextureMap.Get(), &targetviewdesc, &m_textureCubeRenderTargetView);

        D3D11_SHADER_RESOURCE_VIEW_DESC resourceviewdesc;
        resourceviewdesc.Format = desc2d.Format;
        resourceviewdesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
        resourceviewdesc.Texture2D.MostDetailedMip = 0;
        resourceviewdesc.Texture2D.MipLevels = 1;

        m_deviceResources->GetD3DDevice()->CreateShaderResourceView(m_textureRenderTargetTextureMap.Get(), &resourceviewdesc, &m_textureCubeResourceView);

		// Load mesh vertices. Each vertex has a position and a color.
		static VertexPositionUVNormal cubeVertices[] =
		{
			
			{ XMFLOAT3(-5.0f, 1.0f, -0.5f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
			{ XMFLOAT3(-4.0f, 1.0f, -0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
			{ XMFLOAT3(-5.0f, 0.0f, -0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
			{ XMFLOAT3(-4.0f, 0.0f, -0.5f), XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },

			
			{ XMFLOAT3(-4.0f, 1.0f, -0.5f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
			{ XMFLOAT3(-4.0f, 1.0f, 0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f),  XMFLOAT3(1.0f, 0.0f, 0.0f) },
			{ XMFLOAT3(-4.0f, 0.0f, -0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
			{ XMFLOAT3(-4.0f, 0.0f, 0.5f), XMFLOAT3(1.0f, 1.0f, 0.0f),  XMFLOAT3(1.0f, 0.0f, 0.0f) },

			{ XMFLOAT3(-4.0f, 1.0f, 0.5f), XMFLOAT3(0.0f, 0.0f, 0.0f),  XMFLOAT3(0.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(-5.0f, 1.0f, 0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f),  XMFLOAT3(0.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(-4.0f, 0.0f, 0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(-5.0f, 0.0f, 0.5f), XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
			
			{ XMFLOAT3(-5.0f, 1.0f, 0.5f), XMFLOAT3(0.0f, 0.0f, 0.0f),  XMFLOAT3(-1.0f, 0.0f, 0.0f) },
			{ XMFLOAT3(-5.0f, 1.0f, -0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
			{ XMFLOAT3(-5.0f, 0.0f, 0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f),  XMFLOAT3(-1.0f, 0.0f, 0.0f) },
			{ XMFLOAT3(-5.0f, 0.0f, -0.5f), XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },

			{ XMFLOAT3(-5.0f, 1.0f, 0.5f), XMFLOAT3(0.0f, 0.0f, 0.0f),  XMFLOAT3(0.0f, 1.0f, 0.0f) },
			{ XMFLOAT3(-4.0f, 1.0f, 0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f),  XMFLOAT3(0.0f, 1.0f, 0.0f) },
			{ XMFLOAT3(-5.0f, 1.0f, -0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
			{ XMFLOAT3(-4.0f, 1.0f, -0.5f), XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },

			{ XMFLOAT3(-5.0f, 0.0f, -0.5f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
			{ XMFLOAT3(-4.0f, 0.0f, -0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
			{ XMFLOAT3(-5.0f, 0.0f, 0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f),  XMFLOAT3(0.0f, -1.0f, 0.0f) },
			{ XMFLOAT3(-4.0f, 0.0f, 0.5f), XMFLOAT3(1.0f, 1.0f, 0.0f),  XMFLOAT3(0.0f, -1.0f, 0.0f) },
		};

		for (int i = 0; i < ARRAYSIZE(cubeVertices); i++)
		{
			//cubeVertices[i].pos.x *= 5;
			cubeVertices[i].pos.x += -0.65f;
			//cubeVertices[i].pos.y *= 5;
			cubeVertices[i].pos.y += 1.25f;
			//cubeVertices[i].pos.z *= 5;
			cubeVertices[i].pos.z += 13.5f;
		}

		D3D11_SAMPLER_DESC textsample_desc;
		ZeroMemory(&textsample_desc, sizeof(textsample_desc));
		textsample_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		textsample_desc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
		textsample_desc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
		textsample_desc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;

		/*DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateSamplerState(&textsample_desc, &m_textureCubeSampleState));
		DX::ThrowIfFailed(CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"Assets/graveller.dds", NULL, &m_textureCubeResourceView));*/

		D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
		vertexBufferData.pSysMem = cubeVertices;
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(cubeVertices), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_textureCubevertexBuffer));

		// Load mesh indices. Each trio of indices represents
		// a triangle to be rendered on the screen.
		// For example: 0,2,1 means that the vertices with indexes
		// 0, 2 and 1 from the vertex buffer compose the 
		// first triangle of this mesh.
		static const unsigned int cubeIndices[] =
		{
			0,1,3,
			3,2,0,

			4,5,7,
			7,6,4,

			8,9,11,
			11,10,8,

			12,13,15,
			15,14,12,

			16,17,19,
			19,18,16,

			20,21,23,
			23,22,20,
		};

		m_textureCubeindexCount = ARRAYSIZE(cubeIndices);

		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = cubeIndices;
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(sizeof(cubeIndices), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&indexBufferDesc, &indexBufferData, &m_textureCubeindexBuffer));
	});

	// Once the cube is loaded, the object is ready to be rendered.
	createtextureCubeTask.then([this]()
	{
		m_loadingComplete = true;
	});


#pragma endregion

}

void Sample3DSceneRenderer::ReleaseDeviceDependentResources(void)
{
	m_loadingComplete = false;
	m_vertexShader.Reset();
	m_inputLayout.Reset();
	m_pixelShader.Reset();
	m_constantBuffer.Reset();
	m_vertexBuffer.Reset();
	m_indexBuffer.Reset();

	m_SkyboxvertexShader.Reset();
	m_SkyboxinputLayout.Reset();
	m_SkyboxpixelShader.Reset();
	m_SkyboxconstantBuffer.Reset();
	m_SkyboxvertexBuffer.Reset();
	m_SkyboxindexBuffer.Reset();

	m_DittoinputLayout.Reset();
	m_DittovertexBuffer.Reset();
	m_DittoindexBuffer.Reset();
	m_DittovertexShader.Reset();
	m_DittopixelShader.Reset();
	m_DittoconstantBuffer.Reset();
	m_DittoSampleState.Reset();
	m_DittoResourceView.Reset();

	m_PlatforminputLayout.Reset();
	m_PlatformvertexBuffer.Reset();
	m_PlatformindexBuffer.Reset();
	m_PlatformvertexShader.Reset();
	m_PlatformpixelShader.Reset();
	m_PlatformconstantBuffer.Reset();
	m_PlatformSampleState.Reset();
	m_PlatformResourceView.Reset();
}