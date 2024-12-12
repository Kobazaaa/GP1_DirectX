#pragma once
#include "pch.h"
#include "Texture.h"

class Effect final
{
public:
	Effect(ID3D11Device* pDevice, const std::wstring& assetFile);
	~Effect();

	Effect(const Effect& other) = delete;
	Effect(Effect&& other) noexcept = delete;
	Effect& operator=(const Effect& rhs) = delete;
	Effect& operator=(Effect&& rhs) noexcept = delete;

	static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile);
	void SetDiffuseMap(Texture* pDiffuseTexture);
	ID3DX11Effect* GetEffect() const;
	ID3DX11EffectTechnique* GetTechnique() const;

private:
	ID3DX11Effect* m_pEffect{};
	ID3DX11EffectTechnique* m_pTechnique{};
	ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable{};
};