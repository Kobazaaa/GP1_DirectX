#include "Effect.h"

#pragma region BaseEffect
BaseEffect::BaseEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
{
	m_pEffect = LoadEffect(pDevice, assetFile);
	if (m_pEffect)
	{
		m_vpTechnique.clear();

		m_vpTechnique.push_back(m_pEffect->GetTechniqueByName("PointSamplingTechnique"));
		m_vpTechnique.push_back(m_pEffect->GetTechniqueByName("LinearSamplingTechnique"));
		m_vpTechnique.push_back(m_pEffect->GetTechniqueByName("AnisotropicSamplingTechnique"));

		for (int index{}; index < m_vpTechnique.size(); ++index)
		{
			if (!m_vpTechnique[index]->IsValid())
			{
				std::wcout << L"Technique with index ";
				std::wcout << index;
				std::wcout << L" not valid\n";
			}
		}

		m_pMatWorldViewProjVariable = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();
		if (!m_pMatWorldViewProjVariable->IsValid())
			std::wcout << L"m_pMatWorldViewProjVariable not valid!\n";

	}
}
BaseEffect::~BaseEffect()
{
	for (auto& technique : m_vpTechnique)
	{
		if(technique) technique->Release();
	}

	if(m_pEffect)				m_pEffect->Release();
}

ID3DX11Effect* BaseEffect::LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
{
	HRESULT result;
	ID3D10Blob* pErrorBlob = nullptr;
	ID3DX11Effect* pEffect;

	DWORD shaderFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	shaderFlags |= D3DCOMPILE_DEBUG;
	shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	result = D3DX11CompileEffectFromFile(assetFile.c_str(),
		nullptr,
		nullptr,
		shaderFlags,
		0,
		pDevice,
		&pEffect,
		&pErrorBlob);

	if (FAILED(result))
	{
		if (pErrorBlob != nullptr)
		{
			const char* pErrors = static_cast<char*>(pErrorBlob->GetBufferPointer());

			std::wstringstream ss;
			for (unsigned int i{}; i < pErrorBlob->GetBufferSize(); ++i)
				ss << pErrors[i];

			OutputDebugStringW(ss.str().c_str());
			pErrorBlob->Release();
			pErrorBlob = nullptr;

			std::wcout << ss.str() << "\n";
			return nullptr;
		}
		else
		{
			std::wstringstream ss;
			ss << "EffectLoader: Failed to CreateEffectFromFile!\nPath: " << assetFile;
			std::wcout << ss.str() << "\n";
			return nullptr;
		}
	}

	return pEffect;
}

ID3DX11Effect* BaseEffect::GetEffect() const
{
	return m_pEffect;
}
ID3DX11EffectTechnique* BaseEffect::GetTechnique(const uint32_t index) const
{
	return m_vpTechnique.at(index);
}

void BaseEffect::SetWorldViewProjectionMatrix(const Matrix& worldViewProjectionMatrix)
{
	m_pMatWorldViewProjVariable->SetMatrix(reinterpret_cast<const float*>(&worldViewProjectionMatrix));
}
#pragma endregion

#pragma region Effect
FullShadeEffect::FullShadeEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
	: BaseEffect(pDevice, assetFile)
{
	if (m_pEffect)
	{
		m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
		if (!m_pDiffuseMapVariable->IsValid())
			std::wcout << L"m_pDiffuseMapVariable not valid!\n";
		m_pNormalMapVariable = m_pEffect->GetVariableByName("gNormalMap")->AsShaderResource();
		if (!m_pNormalMapVariable->IsValid())
			std::wcout << L"m_pNormalMapVariable not valid!\n";
		m_pSpecularMapVariable = m_pEffect->GetVariableByName("gSpecularMap")->AsShaderResource();
		if (!m_pSpecularMapVariable->IsValid())
			std::wcout << L"m_pSpecularMapVariable not valid!\n";
		m_pGlossinessMapVariable = m_pEffect->GetVariableByName("gGlossinessMap")->AsShaderResource();
		if (!m_pGlossinessMapVariable->IsValid())
			std::wcout << L"m_pGlossinessMapVariable not valid!\n";
		m_pMatWorldVariable = m_pEffect->GetVariableByName("gWorldMatrix")->AsMatrix();
		if (!m_pMatWorldVariable->IsValid())
			std::wcout << L"m_pMatWorldVariable not valid!\n";
		m_pVecCameraVariable = m_pEffect->GetVariableByName("gCameraPos")->AsVector();
		if (!m_pVecCameraVariable->IsValid())
			std::wcout << L"m_pVecCameraVariable not valid!\n";
	}
}

FullShadeEffect::~FullShadeEffect()
{
	if (m_pVecCameraVariable)		m_pVecCameraVariable->Release();
	if (m_pMatWorldVariable)		m_pMatWorldVariable->Release();
	if (m_pDiffuseMapVariable)		m_pDiffuseMapVariable->Release();
	if (m_pNormalMapVariable)		m_pNormalMapVariable->Release();
	if (m_pSpecularMapVariable)		m_pSpecularMapVariable->Release();
	if (m_pGlossinessMapVariable)	m_pGlossinessMapVariable->Release();
}

void FullShadeEffect::SetDiffuseMap(const Texture* pDiffuseTexture)
{
	m_pDiffuseMapVariable->SetResource(pDiffuseTexture->GetSRV());
}

void FullShadeEffect::SetNormalMap(const Texture* pNormalTexture)
{
	m_pNormalMapVariable->SetResource(pNormalTexture->GetSRV());
}

void FullShadeEffect::SetSpecularMap(const Texture* pSpecularTexture)
{
	m_pSpecularMapVariable->SetResource(pSpecularTexture->GetSRV());
}

void FullShadeEffect::SetGlossinessMap(const Texture* pGlossinessTexture)
{
	m_pGlossinessMapVariable->SetResource(pGlossinessTexture->GetSRV());
}

void FullShadeEffect::SetWorldMatrix(const Matrix& worldMatrix)
{
	m_pMatWorldVariable->SetMatrix(reinterpret_cast<const float*>(&worldMatrix));
}

void FullShadeEffect::SetCameraPosition(const Vector3& cameraPosition)
{
	m_pVecCameraVariable->SetFloatVector(reinterpret_cast<const float*>(&cameraPosition));
}
#pragma endregion

#pragma region FlatShadeEffect
FlatShadeEffect::FlatShadeEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
	: BaseEffect(pDevice, assetFile)
{
	if (m_pEffect)
	{
		m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
		if (!m_pDiffuseMapVariable->IsValid())
			std::wcout << L"m_pDiffuseMapVariable not valid!\n";
		m_pMatWorldVariable = m_pEffect->GetVariableByName("gWorldMatrix")->AsMatrix();
		if (!m_pMatWorldVariable->IsValid())
			std::wcout << L"m_pMatWorldVariable not valid!\n";
	}
}

FlatShadeEffect::~FlatShadeEffect()
{
	if (m_pDiffuseMapVariable)		m_pDiffuseMapVariable->Release();
}

void FlatShadeEffect::SetDiffuseMap(const Texture* pDiffuseTexture)
{
	m_pDiffuseMapVariable->SetResource(pDiffuseTexture->GetSRV());
}

void FlatShadeEffect::SetWorldMatrix(const Matrix& worldMatrix)
{
	m_pMatWorldVariable->SetMatrix(reinterpret_cast<const float*>(&worldMatrix));
}
#pragma endregion
