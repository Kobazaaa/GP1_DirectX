#pragma once
#include "pch.h"
#include <SDL_surface.h>
#include <string>
#include "ColorRGB.h"

namespace dae
{
	struct Vector2;

	class Texture
	{
	public:
		~Texture();

		static Texture* LoadFromFile(const std::string& path, ID3D11Device* pDevice);
		ID3D11ShaderResourceView* GetSRV() const;

	private:
		Texture(SDL_Surface* pSurface, ID3D11Device* pDevice);

		ID3D11ShaderResourceView* m_pSRV{};
		ID3D11Texture2D* m_pResource{};
	};
}
