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
	float fovAngleY = 70.0f * XM_PI / 180.0f;

	// This is a simple example of change that can be made when the app is in
	// portrait or snapped view.
	if (aspectRatio < 1.0f)
	{
		fovAngleY *= 2.0f;
	}

	// Note that the OrientationTransform3D matrix is post-multiplied here
	// in order to correctly orient the scene to match the display orientation.
	// This post-multiplication step is required for any draw calls that are
	// made to the swap chain render target. For draw calls to other targets,
	// this transform should not be applied.

	// This sample makes use of a right-handed coordinate system using row-major matrices.
	XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, 0.01f, 100.0f);

	XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();

	XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

	XMStoreFloat4x4(&m_constantBufferData.projection, XMMatrixTranspose(perspectiveMatrix * orientationMatrix));

	// Eye is at (0,0.7,1.5), looking at point (0,-0.1,0) with the up-vector along the y-axis.
	static const XMVECTORF32 eye = { 0.0f, 1.7f, -10.5f, 0.0f };
	static const XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
	static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

	XMStoreFloat4x4(&m_camera, XMMatrixInverse(nullptr, XMMatrixLookAtLH(eye, at, up)));
	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixLookAtLH(eye, at, up)));


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

		Rotate(radians);
	}


	// Update or move camera here
	UpdateCamera(timer, 3.0f, 0.75f);

	m_SkyboxconstantBufferData = m_constantBufferData;
	XMStoreFloat4x4(&m_SkyboxconstantBufferData.model, XMMatrixTranspose(XMMatrixRotationY(3.1415f)));

	m_DittoconstantBufferData = m_constantBufferData;
	XMStoreFloat4x4(&m_DittoconstantBufferData.model, XMMatrixTranspose(XMMatrixRotationY(3.1415f)));

	m_PlatformconstantBufferData = m_constantBufferData;
	XMStoreFloat4x4(&m_PlatformconstantBufferData.model, XMMatrixTranspose(XMMatrixRotationY(3.1415f)));
}

// Rotate the 3D cube model a set amount of radians.
void Sample3DSceneRenderer::Rotate(float radians)
{
	// Prepare to pass the updated model matrix to the shader
	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixRotationY(radians)));
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
	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_camera))));
	XMStoreFloat4x4(&m_SkyboxconstantBufferData.view, XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_camera))));
	XMStoreFloat4x4(&m_DittoconstantBufferData.view, XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_camera))));
	/*DirectX::XMMATRIX scalePlat = { 0,0,0,1.0f/10.0f,
							0,0,0,1.0f/10.0f,
							0,0,0,1.0f/10.0f,
							0,0,0,1 };
	m_PlatformconstantBufferData.model._11 *= 100.0f;
	m_PlatformconstantBufferData.model._22 *= 100.0f;
	m_PlatformconstantBufferData.model._33 *= 100.0f;*/
	//XMStoreFloat4x4(&m_PlatformconstantBufferData.view, scalePlat);
	XMStoreFloat4x4(&m_PlatformconstantBufferData.view, XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_camera))));
	
#pragma region Pyrimid
	// Prepare the constant buffer to send it to the graphics device.
	context->UpdateSubresource1(m_constantBuffer.Get(), 0, NULL, &m_constantBufferData, 0, 0, 0);
	// Each vertex is one instance of the VertexPositionColor struct.
	UINT stride = sizeof(VertexPositionColor);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
	// Each index is one 16-bit unsigned integer (short).
	context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(m_inputLayout.Get());
	// Attach our vertex shader.
	context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers1(0, 1, m_constantBuffer.GetAddressOf(), nullptr, nullptr);
	// Attach our pixel shader.
	context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
	// Draw the objects.
	context->DrawIndexed(m_indexCount, 0, 0);
#pragma endregion

#pragma region skybox
	// Prepare the constant buffer to send it to the graphics device.
	context->UpdateSubresource1(m_SkyboxconstantBuffer.Get(), 0, NULL, &m_SkyboxconstantBufferData, 0, 0, 0);
	// Each vertex is one instance of the VertexPositionColor struct.
	stride = sizeof(VertexPositionColor);
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
	//context->DrawIndexed(m_SkyboxindexCount, 0, 0);
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

	context->DrawIndexed(m_DittoindexCount, 0, 0);
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

	//context->DrawIndexed(m_PlatformindexCount, 0, 0);

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

#pragma region pyrimid
	// Load shaders asynchronously.
	auto loadVSTask = DX::ReadDataAsync(L"SampleVertexShader.cso");
	auto loadPSTask = DX::ReadDataAsync(L"SamplePixelShader.cso");

	// After the vertex shader file is loaded, create the shader and input layout.
	auto createVSTask = loadVSTask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateVertexShader(&fileData[0], fileData.size(), nullptr, &m_vertexShader));

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "UV", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
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
		static const VertexPositionColor cubeVertices[] =
		{
			{ XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
			{ XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(-0.5f,  -0.5f, 0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
			{ XMFLOAT3(0.5f,  -0.5f,  0.5f), XMFLOAT3(0.0f, 1.0f, 1.0f) },
			{ XMFLOAT3(0.0f, -1.5f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) }
		};

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
		static const unsigned short cubeIndices[] =
		{
			3,1,0,
			2,3,0,

			0,1,4,
			1,3,4,
			2,4,3,
			0,4,2,
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
	loadVSTask = DX::ReadDataAsync(L"SampleVertexShader.cso");
	loadPSTask = DX::ReadDataAsync(L"SamplePixelShader.cso");

	// After the vertex shader file is loaded, create the shader and input layout.
	createVSTask = loadVSTask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateVertexShader(&fileData[0], fileData.size(), nullptr, &m_SkyboxvertexShader));

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "UV", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateInputLayout(vertexDesc, ARRAYSIZE(vertexDesc), &fileData[0], fileData.size(), &m_SkyboxinputLayout));
	});

	// After the pixel shader file is loaded, create the shader and constant buffer.
	createPSTask = loadPSTask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreatePixelShader(&fileData[0], fileData.size(), nullptr, &m_SkyboxpixelShader));

		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, &m_SkyboxconstantBuffer));
	});

	// Once both shaders are loaded, create the mesh.
	auto createSkyTask = (createPSTask && createVSTask).then([this]()
	{
		// Load mesh vertices. Each vertex has a position and a color.
		static const VertexPositionColor cubeVertices[] =
		{
			{ XMFLOAT3(-50.5f, -50.5f, -50.5f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
			{ XMFLOAT3(-50.5f, -50.5f,  50.5f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(-50.5f,  50.5f, -50.5f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
			{ XMFLOAT3(-50.5f,  50.5f,  50.5f), XMFLOAT3(0.0f, 1.0f, 1.0f) },
			{ XMFLOAT3(50.5f, -50.5f, -50.5f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
			{ XMFLOAT3(50.5f, -50.5f,  50.5f), XMFLOAT3(1.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(50.5f,  50.5f, -50.5f), XMFLOAT3(1.0f, 1.0f, 0.0f) },
			{ XMFLOAT3(50.5f,  50.5f,  50.5f), XMFLOAT3(1.0f, 1.0f, 1.0f) },
		};

		D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
		vertexBufferData.pSysMem = cubeVertices;
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(cubeVertices), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_SkyboxvertexBuffer));

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

		LoadObjFile("Assets/haunter.obj", vertuvposnorm, index, false);

		D3D11_SAMPLER_DESC textsample_desc;
		ZeroMemory(&textsample_desc, sizeof(textsample_desc));
		textsample_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		textsample_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		textsample_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		textsample_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateSamplerState(&textsample_desc, &m_DittoSampleState));
		DX::ThrowIfFailed(CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"Assets/haunter.dds", NULL, &m_DittoResourceView));

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

		LoadObjFile("Assets/MMstage.obj", vertuvposnorm, index,false);

		D3D11_SAMPLER_DESC textsample_desc;
		ZeroMemory(&textsample_desc, sizeof(textsample_desc));
		textsample_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		textsample_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		textsample_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		textsample_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateSamplerState(&textsample_desc, &m_PlatformSampleState));
		DX::ThrowIfFailed(CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"Assets/plat.dds", NULL, &m_PlatformResourceView));

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