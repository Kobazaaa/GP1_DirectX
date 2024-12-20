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
		Texture(const Texture& other) = delete;
		Texture(Texture&& other) noexcept = delete;
		Texture& operator=(const Texture& other) = delete;
		Texture& operator=(Texture&& other) noexcept = delete;

		static Texture* LoadFromFile(const std::string& path, ID3D11Device* pDevice);
		ID3D11ShaderResourceView* GetSRV() const;

	private:
		Texture(SDL_Surface* pSurface, ID3D11Device* pDevice);

		ID3D11ShaderResourceView* m_pSRV{};
		ID3D11Texture2D* m_pResource{};
	};
}
