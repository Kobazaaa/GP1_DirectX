#include "Mesh.h"
#include "Utils.h"
#include <cassert>

Mesh::Mesh(ID3D11Device* pDevice, const std::string& objFilePath, BaseEffect* pEffect)
{
	// Parse the OBJ Mesh
	Utils::ParseOBJ(objFilePath, m_vVertices, m_vIndices);

	// Load the FullShadeEffect
	m_pEffect = pEffect;
	auto pTechnique = m_pEffect->GetTechnique(0);

	// Create Vertex Layout
	static constexpr uint32_t numElements{ 5 };
	D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};

	vertexDesc[0].SemanticName = "POSITION";
	vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[0].AlignedByteOffset = offsetof(Vertex, position);
	vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[1].SemanticName = "COLOR";
	vertexDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[1].AlignedByteOffset = offsetof(Vertex, color);
	vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[2].SemanticName = "TEXCOORD";
	vertexDesc[2].Format = DXGI_FORMAT_R32G32_FLOAT;
	vertexDesc[2].AlignedByteOffset = offsetof(Vertex, uv);
	vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[3].SemanticName = "NORMAL";
	vertexDesc[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[3].AlignedByteOffset = offsetof(Vertex, normal);
	vertexDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[4].SemanticName = "TANGENT";
	vertexDesc[4].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[4].AlignedByteOffset = offsetof(Vertex, tangent);
	vertexDesc[4].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	// Create Input Layout
	D3DX11_PASS_DESC passDesc{};
	pTechnique->GetPassByIndex(0)->GetDesc(&passDesc);

	const HRESULT resultIL = pDevice->CreateInputLayout(
		vertexDesc,
		numElements,
		passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize,
		&m_pInputLayout);

	if (FAILED(resultIL))
		assert(false);

	// Create Vertex Buffer
	D3D11_BUFFER_DESC bd{};
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(Vertex) * static_cast<uint32_t>(m_vVertices.size());
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData{};
	initData.pSysMem = m_vVertices.data();

	HRESULT result = pDevice->CreateBuffer(&bd, &initData, &m_pVertexBuffer);
	if (FAILED(result))
		return;

	// Create Vertex Buffer
	m_NumIndices = static_cast <uint32_t>(m_vIndices.size());
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(uint32_t) * m_NumIndices;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	initData.pSysMem = m_vIndices.data();
	result = pDevice->CreateBuffer(&bd, &initData, &m_pIndexBuffer);
	if (FAILED(result))
		return;
}

Mesh::~Mesh()
{
	m_pIndexBuffer->Release();
	m_pVertexBuffer->Release();
	m_pInputLayout->Release();
	//delete m_pEffect;
}

void Mesh::Render(ID3D11DeviceContext* pDeviceContext)
{
	//1. Set Primitive Topology
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//2. Set Input Layout
	pDeviceContext->IASetInputLayout(m_pInputLayout);

	//3. Set Vertex Buffer
	constexpr UINT stride = sizeof(Vertex);
	constexpr UINT offset = 0;
	pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	//4. Set Index Buffer
	pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//5. Draw
	D3DX11_TECHNIQUE_DESC techDesc{};
	m_pEffect->GetTechnique(m_TechniqueIndex)->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_pEffect->GetTechnique(m_TechniqueIndex)->GetPassByIndex(p)->Apply(0, pDeviceContext);
		pDeviceContext->DrawIndexed(m_NumIndices, 0, 0);
	}
}

void Mesh::SetWorldMatrix(const Matrix& newWorldMatrix)
{
	worldMatrix = newWorldMatrix;
}

Matrix Mesh::GetWorldMatrix() const
{
	return worldMatrix;
}

void Mesh::ToggleTechniqueIndex()
{
	++m_TechniqueIndex;
	m_TechniqueIndex %= 3;
}
