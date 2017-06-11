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

/// ���_�f�[�^�^.
struct Vertex
{
  glm::vec3 position; ///< ���W
  glm::vec4 color; ///< �F
  glm::vec2 texCoord; ///< �e�N�X�`�����W.
  glm::vec3 normal; ///< �@��
};

/**
* Vertex Buffer Object���쐬����.
*
* @param size ���_�f�[�^�̃T�C�Y.
* @param data ���_�f�[�^�ւ̃|�C���^.
*
* @return �쐬����VBO��ID.
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
* Index Buffer Object���쐬����.
*
* @param size �C���f�b�N�X�f�[�^�̃T�C�Y.
* @param data �C���f�b�N�X�f�[�^�ւ̃|�C���^.
*
* @return �쐬����IBO.
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
* ���_�A�g���r���[�g��ݒ肷��.
*
* @param index ���_�A�g���r���[�g�̃C���f�b�N�X.
* @param cls   ���_�f�[�^�^��.
* @param mbr   ���_�A�g���r���[�g�ɐݒ肷��cls�̃����o�ϐ���.
*/
#define SetVertexAttribPointer(index, cls, mbr) \
  SetVertexAttribPointerI(index, sizeof(cls::mbr) / sizeof(float), sizeof(cls), reinterpret_cast<GLvoid*>(offsetof(cls, mbr)))
void SetVertexAttribPointerI(GLuint index, GLint size, GLsizei stride, const GLvoid* pointer)
{
  glEnableVertexAttribArray(index);
  glVertexAttribPointer(index, size, GL_FLOAT, GL_FALSE, stride, pointer);
}

/**
* Vertex Array Object���쐬����.
*
* @param vbo VAO�Ɋ֘A�t������VBO.
* @param ibo VAO�Ɋ֘A�t������IBO.
*
* @return �쐬����VAO.
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
* �R���X�g���N�^.
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
* �f�X�g���N�^.
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
* FBX�I�u�W�F�N�g�̔j������������������.
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
* FBX�}�e���A���f�[�^.
*/
struct MaterialData {
  glm::vec4 color = glm::vec4(1);
  std::vector<uint32_t> indexBuffer;
  std::vector<std::string> textureName;
};

/**
* FBX���b�V���f�[�^.
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
* FBX�f�[�^���A�v���P�[�V�����p�f�[�^�ɕϊ�����.
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
* FBX���b�V�����A�v���P�[�V�����p�f�[�^�ɕϊ�����.
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
    std::cerr << "WARNING: " << mesh.name << "�ɂ͎O�p�`�ȊO�̖ʂ��܂܂�Ă��܂�" << std::endl;
    fbxMesh = static_cast<FbxMesh*>(geoConverter->Triangulate(fbxMesh, true));
  }

  // �}�e���A������ǂݎ��.
  const int materialCount = fbxNode->GetMaterialCount();
  mesh.materialList.reserve(materialCount);
  for (int i = 0; i < materialCount; ++i) {
    MaterialData material;
    if (FbxSurfaceMaterial* fbxMaterial = fbxNode->GetMaterial(i)) {
      // �F����ǂݎ��.
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

      // �e�N�X�`���t�@�C������ǂݎ��.
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

  // ���_����ǂݎ��.
  const bool hasColor = fbxMesh->GetElementVertexColorCount() > 0;
  const bool hasTexcoord = fbxMesh->GetElementUVCount() > 0;
  const bool hasNormal = fbxMesh->GetElementNormalCount() > 0;

  FbxStringList uvSetNameList;
  fbxMesh->GetUVSetNames(uvSetNameList);
  const int polygonCount = fbxMesh->GetPolygonCount();
  mesh.vertexBuffer.reserve(polygonCount * 3);

  const FbxAMatrix matRotation = fbxNode->EvaluateGlobalTransform();

  // �F����ǂݎ�鏀��.
  // @note ���W/UV/�@���ȊO�̃p�����[�^�ɂ͒��ړǂݎ��֐����񋟂���Ă��Ȃ����߁A
  //       �uFbxGeometryElement???�v�N���X����ǂݎ��Ȃ��Ă͂Ȃ�Ȃ�.
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
* FBX�f�[�^���A�v���P�[�V�����p�f�[�^�ɕϊ�����.
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
* FBX�t�@�C����ǂݍ���.
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
      std::cerr << "ERROR: " << filename << "�̓ǂݍ��݂Ɏ��s" << std::endl;
      return false;
    }
    if (!fbxImporter->IsFBX()) {
      std::cerr << "ERROR: " << filename << "��FBX�t�@�C���ł͂���܂���" << std::endl;
      return false;
    }
    if (!fbxImporter->Import(fbxScene)) {
      return false;
    }
  }
  geoConverter = std::make_unique<FbxGeometryConverter>(fbxManager);
  if (!Convert(fbxScene->GetRootNode())) {
    std::cerr << "ERROR: " << filename << "�̕ϊ��Ɏ��s" << std::endl;
  }
  return true;
}

/**
* ���b�V�����t�@�C������ǂݍ���.
*
* @param filename ���b�V���t�@�C����.
*
* @retval true  �ǂݍ��ݐ���.
* @retval false �ǂݍ��ݎ��s.
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
* ���b�V����`�悷��.
*
* @param mesh  �`�悷�郁�b�V���ւ̃|�C���^.
*/
void Buffer::Draw(const Mesh* mesh) const
{
  if (!mesh) {
    return;
  }
  glDrawElements(mesh->Mode(), mesh->Size(), mesh->Type(), mesh->Offset());
}

} // namespace Mesh
