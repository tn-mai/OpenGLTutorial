/**
* @file Mesh.cpp
*/
#include "Mesh.h"
#include <fbxsdk.h>
#include <iostream>

/**
* モデルデータ管理のための名前空間.
*/
namespace Mesh {

/// 頂点データ型.
struct Vertex
{
  glm::vec3 position; ///< 座標
  glm::vec4 color; ///< 色
  glm::vec2 texCoord; ///< テクスチャ座標.
  glm::vec3 normal; ///< 法線
  glm::vec4 tangent;
};

/**
* Vertex Buffer Objectを作成する.
*
* @param size 頂点データのサイズ.
* @param data 頂点データへのポインタ.
*
* @return 作成したVBOのID.
*/
GLuint CreateVBO(GLsizeiptr size, const GLvoid* data)
{
  GLuint vbo = 0;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  return vbo;
}

/**
* Index Buffer Objectを作成する.
*
* @param size インデックスデータのサイズ.
* @param data インデックスデータへのポインタ.
*
* @return 作成したIBO.
*/
GLuint CreateIBO(GLsizeiptr size, const GLvoid* data)
{
  GLuint ibo = 0;
  glGenBuffers(1, &ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STREAM_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  return ibo;
}

/**
* 頂点アトリビュートを設定する.
*
* @param index 頂点アトリビュートのインデックス.
* @param cls   頂点データ型名.
* @param mbr   頂点アトリビュートに設定するclsのメンバ変数名.
*/
#define SetVertexAttribPointer(index, cls, mbr) \
  SetVertexAttribPointerI(index, sizeof(cls::mbr) / sizeof(float), sizeof(cls), reinterpret_cast<GLvoid*>(offsetof(cls, mbr)))
void SetVertexAttribPointerI(GLuint index, GLint size, GLsizei stride, const GLvoid* pointer)
{
  glEnableVertexAttribArray(index);
  glVertexAttribPointer(index, size, GL_FLOAT, GL_FALSE, stride, pointer);
}

/**
* Vertex Array Objectを作成する.
*
* @param vbo VAOに関連付けられるVBO.
* @param ibo VAOに関連付けられるIBO.
*
* @return 作成したVAO.
*/
GLuint CreateVAO(GLuint vbo, GLuint ibo)
{
  GLuint vao = 0;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  SetVertexAttribPointer(0, Vertex, position);
  SetVertexAttribPointer(1, Vertex, color);
  SetVertexAttribPointer(2, Vertex, texCoord);
  SetVertexAttribPointer(3, Vertex, normal);
  SetVertexAttribPointer(4, Vertex, tangent);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  glBindVertexArray(0);
  return vao;
}

/**
* FBXベクトルをglmベクトルに変換する.
*
* @param fbxVec FBXベクトル.
*
* @return glmベクトル.
*/
template<typename T>
glm::vec4 ToVec4(const T& fbxVec) {
  return glm::vec4(static_cast<float>(fbxVec[0]), static_cast<float>(fbxVec[1]), static_cast<float>(fbxVec[2]), static_cast<float>(fbxVec[3]));
}

/**
* FBXベクトルをglmベクトルに変換する.
*
* @param fbxVec FBXベクトル.
*
* @return glmベクトル.
*/
template<typename T>
glm::vec3 ToVec3(const T& fbxVec) {
  return glm::vec3(static_cast<float>(fbxVec[0]), static_cast<float>(fbxVec[1]), static_cast<float>(fbxVec[2]));
}

/**
* FBXベクトルをglmベクトルに変換する.
*
* @param fbxVec FBXベクトル.
*
* @return glmベクトル.
*/
template<typename T>
glm::vec2 ToVec2(const T& fbxVec) {
  return glm::vec2(static_cast<float>(fbxVec[0]), static_cast<float>(fbxVec[1]));
}

/**
* FBXオブジェクトを破棄するためのヘルパークラス.
*/
template<typename T>
struct Deleter {
  void operator()(T* p) { if (p) { p->Destroy(); } }
};

/**
* マテリアルの仮データ.
*/
struct TemporaryMaterial {
  glm::vec4 color = glm::vec4(1);
  std::vector<uint32_t> indexBuffer;
  std::vector<Vertex> vertexBuffer;
  std::vector<std::string> textureName;
};

/**
* メッシュの仮データ.
*/
struct TemporaryMesh {
  std::string name;
  std::vector<TemporaryMaterial> materialList;
};

/**
* FBXデータを中間データに変換するクラス.
*/
struct FbxLoader {
  bool Load(const char* filename);
  bool Convert(FbxNode* node);
  bool LoadMesh(FbxNode* node);

  std::unique_ptr<FbxGeometryConverter> geoConverter;
  std::vector<TemporaryMesh> meshList;
};

/**
* FBXファイルを読み込む.
*
* @param filename FBXファイル名.
*
* @retval true  読み込み成功.
* @retval false 読み込み失敗.
*/
bool FbxLoader::Load(const char* filename)
{
  std::unique_ptr<FbxManager, Deleter<FbxManager>> fbxManager(FbxManager::Create());
  if (!fbxManager) {
    return false;
  }
//  fbxManager->SetIOSettings(FbxIOSettings::Create(fbxManager.get(), IOSROOT));
//  fbxManager->LoadPluginsDirectory(FbxGetApplicationDirectory());
  FbxScene* fbxScene = FbxScene::Create(fbxManager.get(), "");
  if (!fbxScene) {
    return false;
  } else {
    std::unique_ptr<FbxImporter, Deleter<FbxImporter>> fbxImporter(FbxImporter::Create(fbxManager.get(), ""));
    const bool importStatus = fbxImporter->Initialize(filename);//, -1 , fbxManager->GetIOSettings());
    if (!importStatus) {
      std::cerr << "ERROR: " << filename << "の読み込みに失敗\n" << fbxImporter->GetStatus().GetErrorString() << std::endl;
      return false;
    }
    if (!fbxImporter->IsFBX()) {
      std::cerr << "ERROR: " << filename << "はFBXファイルではありません" << std::endl;
      return false;
    }
    if (!fbxImporter->Import(fbxScene)) {
      std::cerr << "ERROR: " << filename << "の読み込みに失敗\n" << fbxImporter->GetStatus().GetErrorString() << std::endl;
      return false;
    }
  }
  geoConverter = std::make_unique<FbxGeometryConverter>(fbxManager.get());
  if (!Convert(fbxScene->GetRootNode())) {
    std::cerr << "ERROR: " << filename << "の変換に失敗" << std::endl;
  }
  return true;
}

/**
* FBXデータを仮データに変換する.
*
* @param fbxNode 変換対象のFBXノードへのポインタ.
*
* @retval true  変換成功.
* @retval false 変換失敗.
*/
bool FbxLoader::Convert(FbxNode* fbxNode)
{
  if (!fbxNode) {
    return true;
  }
  if (!LoadMesh(fbxNode)) {
    return false;
  }
  const int childCount = fbxNode->GetChildCount();
  for (int i = 0; i < childCount; ++i) {
    if (!Convert(fbxNode->GetChild(i))) {
      return false;
    }
  }
  return true;
}

template<typename T>
T GetElement(
  FbxGeometryElement::EMappingMode mappingMode,
  bool isDirectRef,
  const FbxLayerElementArrayTemplate<int>* pIndexList,
  const FbxLayerElementArrayTemplate<T>* pList,
  int cpIndex,
  int polygonVertex,
  const T& defaultValue)
{
  switch (mappingMode) {
  case FbxLayerElement::eByControlPoint:
    return (*pList)[isDirectRef ? cpIndex : (*pIndexList)[cpIndex]];
  case FbxLayerElement::eByPolygonVertex:
    return (*pList)[isDirectRef ? polygonVertex : (*pIndexList)[polygonVertex]];
  default:
    return defaultValue;
  }
}

/**
* FBXメッシュを中間データに変換する.
*
* @param fbxNode 変換対象のFBXノードへのポインタ.
*
* @retval true  変換成功.
* @retval false 変換失敗.
*/
bool FbxLoader::LoadMesh(FbxNode* fbxNode)
{
  FbxMesh* fbxMesh = fbxNode->GetMesh();
  if (!fbxMesh) {
    return true;
  }
  TemporaryMesh mesh;
  mesh.name = fbxMesh->GetNode()->GetName();

  if (!fbxMesh->IsTriangleMesh()) {
    std::cerr << "WARNING: " << mesh.name << "には三角形以外の面が含まれています" << std::endl;
    fbxMesh = static_cast<FbxMesh*>(geoConverter->Triangulate(fbxMesh, true));
  }

  // マテリアル情報を読み取る.
  const int materialCount = fbxNode->GetMaterialCount();
  mesh.materialList.reserve(materialCount);
  for (int i = 0; i < materialCount; ++i) {
    TemporaryMaterial material;
    if (FbxSurfaceMaterial* fbxMaterial = fbxNode->GetMaterial(i)) {
      // 色情報を読み取る.
      const FbxClassId classId = fbxMaterial->GetClassId();
      if (classId == FbxSurfaceLambert::ClassId || classId == FbxSurfacePhong::ClassId) {
        const FbxSurfaceLambert* pLambert = static_cast<const FbxSurfaceLambert*>(fbxMaterial);
        material.color = glm::vec4(ToVec3(pLambert->Diffuse.Get()), static_cast<float>(1.0f - pLambert->TransparencyFactor));
      }

      // テクスチャファイル名を読み取る.
      const char* const propNameList[] = {
        FbxSurfaceMaterial::sDiffuse,
        FbxSurfaceMaterial::sSpecular,
        FbxSurfaceMaterial::sNormalMap,
        FbxSurfaceMaterial::sAmbient,
        FbxSurfaceMaterial::sEmissive,
      };
      for (const char* propName : propNameList) {
        FbxProperty prop = fbxMaterial->FindProperty(propName);
        const int layeredCount = prop.GetSrcObjectCount<FbxLayeredTexture>();
        if (layeredCount > 0) {
          for (int i = 0; i < layeredCount; ++i) {
            FbxLayeredTexture* tex = prop.GetSrcObject<FbxLayeredTexture>(i);
            const int texCount = tex->GetSrcObjectCount<FbxTexture>();
            for (int j = 0; j < texCount; ++j) {
              tex->GetSrcObject<FbxTexture>(j)->UVSet;
              material.textureName.emplace_back(tex->GetSrcObject<FbxTexture>(j)->GetName());
            }
          }
        } else {
          const int texCount = prop.GetSrcObjectCount<FbxTexture>();
          for (int j = 0; j < texCount; ++j) {
            material.textureName.emplace_back(prop.GetSrcObject<FbxTexture>(j)->GetName());
          }
        }
      }
    }
    mesh.materialList.push_back(material);
  }
  if (mesh.materialList.empty()) {
    mesh.materialList.push_back(TemporaryMaterial());
  }

  // 頂点情報の有無を取得する.
  const bool hasColor = fbxMesh->GetElementVertexColorCount() > 0;
  const bool hasTexcoord = fbxMesh->GetElementUVCount() > 0;
  const bool hasNormal = fbxMesh->GetElementNormalCount() > 0;
  const bool hasTangent = fbxMesh->GetElementTangentCount() > 0;

  // UVデータ名を取得する.
  FbxStringList uvSetNameList;
  fbxMesh->GetUVSetNames(uvSetNameList);

  // 色情報を読み取る準備.
  // @note 座標/UV/法線以外のパラメータには直接読み取る関数が提供されていないため、
  //       「FbxGeometryElement???」クラスから読み取らなくてはならない.
  FbxGeometryElement::EMappingMode colorMappingMode = FbxLayerElement::eNone;
  bool IsColorDirectRef = true;
  const FbxLayerElementArrayTemplate<int>* colorIndexList = nullptr;
  const FbxLayerElementArrayTemplate<FbxColor>* colorList = nullptr;
  if (hasColor) {
    const FbxGeometryElementVertexColor* fbxColorList = fbxMesh->GetElementVertexColor();
    colorMappingMode = fbxColorList->GetMappingMode();
    IsColorDirectRef = fbxColorList->GetReferenceMode() == FbxLayerElement::eDirect;
    colorIndexList = &fbxColorList->GetIndexArray();
    colorList = &fbxColorList->GetDirectArray();
  }

  FbxGeometryElement::EMappingMode tangentMappingMode = FbxLayerElement::eNone;
  bool isTangentDirectRef = true;
  const FbxLayerElementArrayTemplate<int>* tangentIndexList = nullptr;
  const FbxLayerElementArrayTemplate<FbxVector4>* tangentList = nullptr;
  FbxGeometryElement::EMappingMode binormalMappingMode = FbxLayerElement::eNone;
  bool isBinormalDirectRef = true;
  const FbxLayerElementArrayTemplate<int>* binormalIndexList = nullptr;
  const FbxLayerElementArrayTemplate<FbxVector4>* binormalList = nullptr;
  if (hasTangent) {
    const FbxGeometryElementTangent* fbxTangentList = fbxMesh->GetElementTangent();
    tangentMappingMode = fbxTangentList->GetMappingMode();
    isTangentDirectRef = fbxTangentList->GetReferenceMode() == FbxLayerElement::eDirect;
    tangentIndexList = &fbxTangentList->GetIndexArray();
    tangentList = &fbxTangentList->GetDirectArray();
    const FbxGeometryElementBinormal* fbxBinormaltList = fbxMesh->GetElementBinormal();
    binormalMappingMode = fbxBinormaltList->GetMappingMode();
    isBinormalDirectRef = fbxBinormaltList->GetReferenceMode() == FbxLayerElement::eDirect;
    binormalIndexList = &fbxBinormaltList->GetIndexArray();
    binormalList = &fbxBinormaltList->GetDirectArray();
  }

  // マテリアルが存在する場合は、頂点のマテリアルインデックスリストを取得する.
  const FbxLayerElementArrayTemplate<int>* materialIndexList = nullptr;
  if (FbxGeometryElementMaterial* fbxMaterialLayer = fbxMesh->GetElementMaterial()) {
    materialIndexList = &fbxMaterialLayer->GetIndexArray();
  }

  // 総ポリゴン数から仮データの容量を予測し、容量を確保しておく.
  const int polygonCount = fbxMesh->GetPolygonCount();
  for (auto& e : mesh.materialList) {
    const size_t avarageCapacity = polygonCount / mesh.materialList.size();
    e.indexBuffer.reserve(avarageCapacity);
    e.vertexBuffer.reserve(avarageCapacity);
  }

  // 頂点情報を読み取る.
  const FbxAMatrix matTRS(fbxNode->EvaluateGlobalTransform());
  const FbxAMatrix matR(FbxVector4(0, 0, 0, 1), matTRS.GetR(), FbxVector4(1, 1, 1, 1));
  const FbxVector4* const fbxControlPoints = fbxMesh->GetControlPoints();
  int polygonVertex = 0;
  for (int polygonIndex = 0; polygonIndex < polygonCount; ++polygonIndex) {
    for (int pos = 0; pos < 3; ++pos) {
      Vertex v;
      const int cpIndex = fbxMesh->GetPolygonVertex(polygonIndex, pos);
      v.position = ToVec3(matTRS.MultT(fbxControlPoints[cpIndex]));
      v.color = glm::vec4(1);
      if (hasColor) {
        v.color = ToVec4(GetElement(colorMappingMode, IsColorDirectRef, colorIndexList, colorList, cpIndex, polygonVertex, FbxColor(1, 1, 1, 1)));
      }
      v.texCoord = glm::vec2(0);
      if (hasTexcoord) {
        FbxVector2 uv;
        bool unmapped;
        fbxMesh->GetPolygonVertexUV(polygonIndex, pos, uvSetNameList[0], uv, unmapped);
        v.texCoord = ToVec2(uv);
      }
      v.normal = glm::vec3(0, 0, 1);
      if (hasNormal) {
        FbxVector4 normal;
        fbxMesh->GetPolygonVertexNormal(polygonIndex, pos, normal);
        v.normal = glm::normalize(ToVec3(matR.MultT(normal)));
      }
      v.tangent = glm::vec4(1, 0, 0, 1);
      if (hasTangent) {
        const FbxVector4 binormal = matR.MultT(GetElement(binormalMappingMode, isBinormalDirectRef, binormalIndexList, binormalList, cpIndex, polygonVertex, FbxVector4(0, 1, 0, 1)));
        const FbxVector4 tangent = GetElement(tangentMappingMode, isTangentDirectRef, tangentIndexList, tangentList, cpIndex, polygonVertex, FbxVector4(1, 0, 0, 1));
        v.tangent = glm::vec4(glm::normalize(ToVec3(matR.MultT(tangent))), 1);
        const glm::vec3 binormalTmp = glm::normalize(glm::cross(glm::vec3(v.normal), glm::vec3(v.tangent)));
        if (glm::dot(ToVec3(binormal), binormalTmp) < 0) {
          v.tangent.w = -1;
        }
      }
      TemporaryMaterial& materialData = mesh.materialList[materialIndexList ? (*materialIndexList)[polygonIndex] : 0];
      materialData.indexBuffer.push_back(static_cast<uint32_t>(materialData.vertexBuffer.size()));
      materialData.vertexBuffer.push_back(v);
      ++polygonVertex;
    }
  }
  meshList.push_back(std::move(mesh));
  return true;
}

/**
* コンストラクタ.
*
* @param n     メッシュデータ名.
* @param begin 描画するマテリアルの先頭インデックス.
* @param end   描画するマテリアルの終端インデックス.
*/
Mesh::Mesh(const std::string& n, size_t begin, size_t end) :
  name(n), beginMaterial(begin), endMaterial(end)
{
}

/**
* メッシュを描画する.
*
* @param buffer  描画に使用するバッファオブジェクトへのポインタ.
*/
void Mesh::Draw(const BufferPtr& buffer) const
{
  if (!buffer) {
    return;
  }
  if (buffer->GetMesh(name.c_str()).get() != this) {
    std::cerr << "WARNING: バッファに存在しないメッシュ'" << name << "'を描画しようとしました" << std::endl;
    return;
  }
  for (size_t i = beginMaterial; i < endMaterial; ++i) {
    const Material& m = buffer->GetMaterial(i);
    glDrawElementsBaseVertex(GL_TRIANGLES, m.size, m.type, m.offset, m.baseVertex);
  }
}

/**
* メッシュバッファを作成する.
*
* @param vboSize  バッファに格納可能な総頂点数.
* @param iboSize  バッファに格納可能な総インデックス数.
*/
BufferPtr Buffer::Create(int vboSize, int iboSize)
{
  struct Impl : Buffer { Impl() {} ~Impl() {} };
  BufferPtr p = std::make_shared<Impl>();
  p->vbo = CreateVBO(vboSize * sizeof(Vertex), nullptr);
  if (!p->vbo) {
    return {};
  }
  p->ibo = CreateIBO(iboSize * sizeof(uint32_t), nullptr);
  if (!p->ibo) {
    return {};
  }
  p->vao = CreateVAO(p->vbo, p->ibo);
  if (!p->vao) {
    return {};
  }
  return p;
}

/**
* デストラクタ.
*/
Buffer::~Buffer()
{
  if (vao) {
    glDeleteVertexArrays(1, &vao);
  }
  if (ibo) {
    glDeleteBuffers(1, &ibo);
  }
  if (vbo) {
    glDeleteBuffers(1, &vbo);
  }
}

/**
* メッシュをファイルから読み込む.
*
* @param filename メッシュファイル名.
*
* @retval true  読み込み成功.
* @retval false 読み込み失敗.
*/
bool Buffer::LoadMeshFromFile(const char* filename)
{
  FbxLoader loader;
  if (!loader.Load(filename)) {
    return false;
  }
  GLint64 vboSize = 0;
  GLint64 iboSize = 0;
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glGetBufferParameteri64v(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &vboSize);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  glGetBufferParameteri64v(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &iboSize);
  for (TemporaryMesh& e : loader.meshList) {
    for (TemporaryMaterial& material : e.materialList) {
      const GLsizeiptr verticesBytes = material.vertexBuffer.size() * sizeof(Vertex);
      if (vboEnd + verticesBytes >= vboSize) {
        std::cerr << "WARNING: VBOサイズが不足しています(" << vboEnd << '/' << vboSize << ')' << std::endl;
        continue;
      }
      const GLsizei indexSize = static_cast<GLsizei>(material.indexBuffer.size());
      const GLsizeiptr indicesBytes = indexSize * sizeof(uint32_t);
      if (iboEnd + indicesBytes >= iboSize) {
        std::cerr << "WARNING: IBOサイズが不足しています(" << iboEnd << '/' << iboSize << ')' << std::endl;
        continue;
      }
      glBufferSubData(GL_ARRAY_BUFFER, vboEnd, verticesBytes, material.vertexBuffer.data());
      glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, iboEnd, indicesBytes, material.indexBuffer.data());
      const GLint baseVertex = static_cast<uint32_t>(vboEnd / sizeof(Vertex));
      materialList.push_back({ GL_UNSIGNED_INT, indexSize, reinterpret_cast<GLvoid*>(iboEnd), baseVertex, material.color });
      vboEnd += verticesBytes;
      iboEnd += indicesBytes;
    }

    struct Impl : public Mesh {
      Impl(const std::string& n, size_t b, size_t e) : Mesh(n, b, e) {}
      ~Impl() {}
    };
    const size_t endMaterial = materialList.size();
    const size_t beginMaterial = endMaterial - e.materialList.size();
    meshList.insert(std::make_pair(e.name, std::make_shared<Impl>(e.name, beginMaterial, endMaterial)));
  }
  return true;
}

/**
* メッシュを取得する.
*
* @param name メッシュ名.
*
* @return nameに対応するメッシュへのポインタ.
*/
const MeshPtr& Buffer::GetMesh(const char* name) const
{
  auto itr = meshList.find(name);
  if (itr == meshList.end()) {
    static const MeshPtr dummy;
    return dummy;
  }
  return itr->second;
}

/**
* マテリアルを取得する.
*
* @param index マテリアルインデックス.
*
* @return indexに対応するマテリアル.
*/
const Material& Buffer::GetMaterial(size_t index) const
{
  if (index >= materialList.size()) {
    static const Material dummy{ GL_UNSIGNED_BYTE, 0, 0, 0, glm::vec4(1) };
    return dummy;
  }
  return materialList[index];
}

/**
* バッファが保持するVAOをOpenGLの処理対象に設定する.
*/
void Buffer::BindVAO() const
{
  glBindVertexArray(vao);
}

} // namespace Mesh
