#pragma once
#include "pch.h"
#include "Vector3.h"
#include "ColorRGB.h"
#include "Effect.h"

#include <vector>

using namespace dae;

struct Vertex_PosCol
{
	Vector3 position	{};
	ColorRGB color		{ colors::White };
};

class Mesh final
{
public:
	Mesh(ID3D11Device* pDevice, const std::vector<Vertex_PosCol>& vertices, const std::vector<uint32_t>& indices);
	~Mesh();
	
	Mesh(const Mesh&) = delete;
	Mesh(Mesh&&) noexcept = delete;
	Mesh& operator=(const Mesh&) = delete;
	Mesh& operator=(Mesh&&) noexcept = delete;

	void Render(ID3D11DeviceContext* pDeviceContext);

private:
	Effect* m_pEffect{};

	ID3D11InputLayout* m_pInputLayout{};
	ID3D11Buffer* m_pVertexBuffer{};
	ID3D11Buffer* m_pIndexBuffer{};

	uint32_t m_NumIndices{};
};