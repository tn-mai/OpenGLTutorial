/**
* @file Mesh.cpp
*/
#include "Mesh.h"
#include <glm/glm.hpp>
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <regex>
#include <fbxsdk.h>

namespace Mesh {

/// 頂点データ型.
struct Vertex
{
  glm::vec3 position; ///< 座標
  glm::vec4 color; ///< 色
  glm::vec2 texCoord; ///< テクスチャ座標.
  glm::vec3 normal; ///< 法線
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
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  glBindVertexArray(0);
  return vao;
}

/**
* コンストラクタ.
*/
Mesh::Mesh(const std::string& n, GLenum t, GLsizei s, GLsizeiptr off) :
  name(n), type(t), size(s), offset(reinterpret_cast<GLvoid*>(off))
{
}

/**
*
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
* FBXオブジェクトの破棄処理を自動化する.
*/
template<typename T>
struct Holder {
  explicit Holder(T* p) : ptr(p) {}
  ~Holder() {
    if (ptr) {
      ptr->Destroy();
    }
  }
  operator T*() { return ptr; }
  T* operator->() { return ptr; }
  T& operator*() { return *ptr; }
  T* ptr = nullptr;
};

/**
* FBXマテリアルデータ.
*/
struct MaterialData {
  glm::vec4 color = glm::vec4(1);
  std::vector<uint32_t> indexBuffer;
  std::vector<std::string> textureName;
};

/**
* FBXメッシュデータ.
*/
struct MeshData {
  std::string name;
  std::vector<MaterialData> materialList;
  std::vector<Vertex> vertexBuffer;
};

template<typename T>
glm::vec4 ToVec4(const T& fbxVec) {
  return glm::vec4(static_cast<float>(fbxVec[0]), static_cast<float>(fbxVec[1]), static_cast<float>(fbxVec[2]), static_cast<float>(fbxVec[3]));
}

template<typename T>
glm::vec3 ToVec3(const T& fbxVec) {
  return glm::vec3(static_cast<float>(fbxVec[0]), static_cast<float>(fbxVec[1]), static_cast<float>(fbxVec[2]));
}

template<typename T>
glm::vec2 ToVec2(const T& fbxVec) {
  return glm::vec2(static_cast<float>(fbxVec[0]), static_cast<float>(fbxVec[1]));
}

/**
* FBXデータをアプリケーション用データに変換する.
*/
struct FbxLoader {
  bool Load(const char* filename);
  bool Convert(FbxNode* node);
  bool LoadMesh(FbxNode* node);

  std::unique_ptr<FbxGeometryConverter> geoConverter;
  std::vector<MeshData> meshList;
};

/**
*
*/
FbxAMatrix GetGeometry(const FbxNode* pNode)
{
  const FbxVector4 vT = pNode->GetGeometricTranslation(FbxNode::eSourcePivot);
  const FbxVector4 vR = pNode->GetGeometricRotation(FbxNode::eSourcePivot);
  const FbxVector4 vS = pNode->GetGeometricScaling(FbxNode::eSourcePivot);
  return FbxAMatrix(vT, vR, vS);
}
/**
* FBXメッシュをアプリケーション用データに変換する.
*/
bool FbxLoader::LoadMesh(FbxNode* fbxNode)
{
  FbxMesh* fbxMesh = fbxNode->GetMesh();
  if (!fbxMesh) {
    return true;
  }
  MeshData mesh;
  mesh.name = fbxMesh->GetNode()->GetName();

  if (!fbxMesh->IsTriangleMesh()) {
    std::cerr << "WARNING: " << mesh.name << "には三角形以外の面が含まれています" << std::endl;
    fbxMesh = static_cast<FbxMesh*>(geoConverter->Triangulate(fbxMesh, true));
  }

  // マテリアル情報を読み取る.
  const int materialCount = fbxNode->GetMaterialCount();
  mesh.materialList.reserve(materialCount);
  for (int i = 0; i < materialCount; ++i) {
    MaterialData material;
    if (FbxSurfaceMaterial* fbxMaterial = fbxNode->GetMaterial(i)) {
      // 色情報を読み取る.
      if (fbxMaterial->GetClassId() == FbxSurfaceLambert::ClassId) {
        const FbxSurfaceLambert* pLambert = static_cast<const FbxSurfaceLambert*>(fbxMaterial);
        material.color.r = static_cast<float>(pLambert->Diffuse.Get()[0]);
        material.color.g = static_cast<float>(pLambert->Diffuse.Get()[1]);
        material.color.b = static_cast<float>(pLambert->Diffuse.Get()[2]);
        material.color.a = static_cast<float>(1.0f - pLambert->TransparencyFactor.Get());
      } else if (fbxMaterial->GetClassId() == FbxSurfacePhong::ClassId) {
        const FbxSurfacePhong* pPhong = static_cast<const FbxSurfacePhong*>(fbxMaterial);
        material.color.r = static_cast<float>(pPhong->Diffuse.Get()[0]);
        material.color.g = static_cast<float>(pPhong->Diffuse.Get()[1]);
        material.color.b = static_cast<float>(pPhong->Diffuse.Get()[2]);
        material.color.a = static_cast<float>(1.0f - pPhong->TransparencyFactor.Get());
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
    mesh.materialList.push_back(MaterialData());
  }

  // 頂点情報を読み取る.
  const bool hasColor = fbxMesh->GetElementVertexColorCount() > 0;
  const bool hasTexcoord = fbxMesh->GetElementUVCount() > 0;
  const bool hasNormal = fbxMesh->GetElementNormalCount() > 0;

  FbxStringList uvSetNameList;
  fbxMesh->GetUVSetNames(uvSetNameList);
  const int polygonCount = fbxMesh->GetPolygonCount();
  mesh.vertexBuffer.reserve(polygonCount * 3);

  const FbxAMatrix matRotation = fbxNode->EvaluateGlobalTransform();

  // 色情報を読み取る準備.
  // @note 座標/UV/法線以外のパラメータには直接読み取る関数が提供されていないため、
  //       「FbxGeometryElement???」クラスから読み取らなくてはならない.
  FbxGeometryElement::EMappingMode colorMappingMode = FbxLayerElement::eNone;
  FbxGeometryElement::EReferenceMode colorRefMode = FbxLayerElement::eDirect;
  const FbxLayerElementArrayTemplate<int>* colorIndexList = nullptr;
  const FbxLayerElementArrayTemplate<FbxColor>* colorList = nullptr;
  if (hasColor) {
    const FbxGeometryElementVertexColor* fbxColorList = fbxMesh->GetElementVertexColor();
    colorMappingMode = fbxColorList->GetMappingMode();
    colorRefMode = fbxColorList->GetReferenceMode();
    colorIndexList = &fbxColorList->GetIndexArray();
    colorList = &fbxColorList->GetDirectArray();
  }
  const FbxLayerElementArrayTemplate<int>* materialIndexList = nullptr;
  if (FbxGeometryElementMaterial* fbxMaterialLayer = fbxMesh->GetElementMaterial()) {
    materialIndexList = &fbxMaterialLayer->GetIndexArray();
  }
  const FbxVector4* const fbxControlPoints = fbxMesh->GetControlPoints();
  int vertexIndex = 0;
  for (int polygonIndex = 0; polygonIndex < polygonCount; ++polygonIndex) {
    const int size = fbxMesh->GetPolygonSize(polygonIndex);
    for (int pos = 0; pos < 3; ++pos) {
      const int cpIndex = fbxMesh->GetPolygonVertex(polygonIndex, pos);
      Vertex v;
      v.position = ToVec3(matRotation.MultT(fbxControlPoints[cpIndex]));
      v.color = glm::vec4(1);
      if (hasColor) {
        switch (colorMappingMode) {
        case FbxLayerElement::eByControlPoint:
          v.color = ToVec4((*colorList)[colorRefMode == FbxLayerElement::eDirect ? cpIndex : (*colorIndexList)[cpIndex]]);
          break;
        case FbxLayerElement::eByPolygonVertex:
          v.color = ToVec4((*colorList)[colorRefMode == FbxLayerElement::eDirect ? vertexIndex : (*colorIndexList)[vertexIndex]]);
          break;
        default:
          break;
        }
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
        v.normal = ToVec3(matRotation.MultT(normal));
      }
      mesh.materialList[materialIndexList ? (*materialIndexList)[polygonIndex] : 0].indexBuffer.push_back(mesh.vertexBuffer.size());
      mesh.vertexBuffer.push_back(v);
      ++vertexIndex;
    }
  }
  meshList.push_back(std::move(mesh));
  return true;
}

/**
* FBXデータをアプリケーション用データに変換する.
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

/**
* FBXファイルを読み込む.
*/
bool FbxLoader::Load(const char* filename)
{
  Holder<FbxManager> fbxManager(FbxManager::Create());
  if (!fbxManager) {
    return false;
  }
  fbxManager->SetIOSettings(FbxIOSettings::Create(fbxManager, IOSROOT));
  fbxManager->LoadPluginsDirectory(FbxGetApplicationDirectory());
  FbxScene* fbxScene = FbxScene::Create(fbxManager, "RootScene");
  if (!fbxScene) {
    return false;
  } else {
    Holder<FbxImporter> fbxImporter(FbxImporter::Create(fbxManager, ""));
    const bool importStatus = fbxImporter->Initialize(filename, -1, fbxManager->GetIOSettings());
    if (!importStatus) {
      std::cerr << "ERROR: " << filename << "の読み込みに失敗" << std::endl;
      return false;
    }
    if (!fbxImporter->IsFBX()) {
      std::cerr << "ERROR: " << filename << "はFBXファイルではありません" << std::endl;
      return false;
    }
    if (!fbxImporter->Import(fbxScene)) {
      return false;
    }
  }
  geoConverter = std::make_unique<FbxGeometryConverter>(fbxManager);
  if (!Convert(fbxScene->GetRootNode())) {
    std::cerr << "ERROR: " << filename << "の変換に失敗" << std::endl;
  }
  return true;
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
  for (const MeshData& e : loader.meshList) {
    const GLsizeiptr verticesBytes = e.vertexBuffer.size() * sizeof(Vertex);
    if (vboEnd + verticesBytes >= vboSize) {
      break;
    }
    glBufferSubData(GL_ARRAY_BUFFER, vboEnd, verticesBytes, e.vertexBuffer.data());
    vboEnd += verticesBytes;
    GLsizei indexCount = 0;
    for (const MaterialData& material : e.materialList) {
      const GLsizeiptr indicesBytes = material.indexBuffer.size() * sizeof(uint32_t);
      if (iboEnd + indicesBytes >= iboSize) {
        break;
      }
      glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, iboEnd, indicesBytes, material.indexBuffer.data());
      iboEnd += indicesBytes;
      indexCount += material.indexBuffer.size();
    }
    meshList.insert(std::make_pair(e.name, Mesh(e.name, GL_UNSIGNED_INT, indexCount, iboEnd - indexCount * sizeof(uint32_t))));
  }
  return true;
}

/**
*/
const Mesh* Buffer::GetMesh(const char* name) const
{
  auto itr = meshList.find(name);
  if (itr == meshList.end()) {
    return nullptr;
  }
  return &itr->second;
}

/**
* メッシュを描画する.
*
* @param mesh  描画するメッシュへのポインタ.
*/
void Buffer::Draw(const Mesh* mesh) const
{
  if (!mesh) {
    return;
  }
  glDrawElements(mesh->Mode(), mesh->Size(), mesh->Type(), mesh->Offset());
}

} // namespace Mesh
