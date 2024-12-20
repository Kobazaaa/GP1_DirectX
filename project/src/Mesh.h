#pragma once
#include "pch.h"
#include "Vector3.h"
#include "ColorRGB.h"
#include "Effect.h"

#include <vector>

using namespace dae;

struct Vertex
{
	Vector3 position	{};
	ColorRGB color		{ colors::White };
	Vector2 uv			{};
	Vector3 normal		{};
	Vector3 tangent		{};
};

class Mesh final
{
public:
	Mesh(ID3D11Device* pDevice, const std::string& objFilePath, BaseEffect* pEffect);
	~Mesh();
	
	Mesh(const Mesh&) = delete;
	Mesh(Mesh&&) noexcept = delete;
	Mesh& operator=(const Mesh&) = delete;
	Mesh& operator=(Mesh&&) noexcept = delete;

	void Render(ID3D11DeviceContext* pDeviceContext);

	void SetWorldMatrix(const Matrix& newWorldMatrix);
	Matrix GetWorldMatrix() const;

	void ToggleTechniqueIndex();
private:
	std::vector<Vertex> m_vVertices{};
	std::vector<uint32_t> m_vIndices{};


	BaseEffect* m_pEffect{};

	ID3D11InputLayout* m_pInputLayout{};
	ID3D11Buffer* m_pVertexBuffer{};
	ID3D11Buffer* m_pIndexBuffer{};

	uint32_t m_NumIndices{};

	Matrix worldMatrix{ };

	uint32_t m_TechniqueIndex{ 0 };
};