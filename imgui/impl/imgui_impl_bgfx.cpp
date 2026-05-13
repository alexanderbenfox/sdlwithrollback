#include "imgui_impl_bgfx.h"
#include <bgfx/bgfx.h>
#include <bx/math.h>
#include <fstream>

struct ImGui_ImplBgfx_Data
{
  uint16_t viewId;
  bgfx::ProgramHandle program;
  bgfx::UniformHandle texUniform;
  bgfx::TextureHandle fontTexture;
  bgfx::VertexLayout vertexLayout;
};

static ImGui_ImplBgfx_Data* ImGui_ImplBgfx_GetBackendData()
{
  return ImGui::GetCurrentContext() ? (ImGui_ImplBgfx_Data*)ImGui::GetIO().BackendRendererUserData : nullptr;
}

static bgfx::ShaderHandle LoadShaderFromFile(const char* path)
{
  std::ifstream file(path, std::ios::binary | std::ios::ate);
  if (!file.is_open())
    return BGFX_INVALID_HANDLE;

  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);

  const bgfx::Memory* mem = bgfx::alloc(uint32_t(size) + 1);
  file.read(reinterpret_cast<char*>(mem->data), size);
  mem->data[size] = '\0';

  return bgfx::createShader(mem);
}

bool ImGui_ImplBgfx_Init(uint16_t viewId)
{
  ImGuiIO& io = ImGui::GetIO();
  IM_ASSERT(io.BackendRendererUserData == nullptr && "Already initialized!");

  ImGui_ImplBgfx_Data* bd = IM_NEW(ImGui_ImplBgfx_Data)();
  io.BackendRendererUserData = (void*)bd;
  io.BackendRendererName = "imgui_impl_bgfx";

  bd->viewId = viewId;

  // Vertex layout for ImGui (must match vs_sprite: pos3f, uv2f, color4b)
  bd->vertexLayout
    .begin()
    .add(bgfx::Attrib::Position,  3, bgfx::AttribType::Float)
    .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
    .add(bgfx::Attrib::Color0,    4, bgfx::AttribType::Uint8, true)
    .end();

  // Load shaders (reuse sprite shaders — they handle textured quads with color)
  bgfx::ShaderHandle vs = LoadShaderFromFile("resources/shaders/metal/vs_sprite.bin");
  bgfx::ShaderHandle fs = LoadShaderFromFile("resources/shaders/metal/fs_sprite.bin");
  bd->program = bgfx::createProgram(vs, fs, true);
  bd->texUniform = bgfx::createUniform("s_texture", bgfx::UniformType::Sampler);

  // Create font texture
  unsigned char* pixels;
  int width, height;
  io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

  const bgfx::Memory* mem = bgfx::copy(pixels, width * height * 4);
  bd->fontTexture = bgfx::createTexture2D(
    uint16_t(width), uint16_t(height), false, 1,
    bgfx::TextureFormat::RGBA8,
    BGFX_TEXTURE_NONE | BGFX_SAMPLER_POINT,
    mem
  );

  io.Fonts->SetTexID((ImTextureID)(intptr_t)bd->fontTexture.idx);

  return true;
}

void ImGui_ImplBgfx_Shutdown()
{
  ImGui_ImplBgfx_Data* bd = ImGui_ImplBgfx_GetBackendData();
  if (!bd) return;

  ImGuiIO& io = ImGui::GetIO();

  bgfx::destroy(bd->program);
  bgfx::destroy(bd->texUniform);
  bgfx::destroy(bd->fontTexture);

  io.BackendRendererName = nullptr;
  io.BackendRendererUserData = nullptr;
  IM_DELETE(bd);
}

void ImGui_ImplBgfx_NewFrame()
{
  // Nothing needed per-frame for bgfx backend
}

void ImGui_ImplBgfx_RenderDrawData(ImDrawData* drawData)
{
  ImGui_ImplBgfx_Data* bd = ImGui_ImplBgfx_GetBackendData();
  if (!bd) return;

  // Avoid rendering when minimized
  int fbWidth = (int)(drawData->DisplaySize.x * drawData->FramebufferScale.x);
  int fbHeight = (int)(drawData->DisplaySize.y * drawData->FramebufferScale.y);
  if (fbWidth <= 0 || fbHeight <= 0)
    return;

  // Setup orthographic projection
  float ortho[16];
  float L = drawData->DisplayPos.x;
  float R = drawData->DisplayPos.x + drawData->DisplaySize.x;
  float T = drawData->DisplayPos.y;
  float B = drawData->DisplayPos.y + drawData->DisplaySize.y;

  const bgfx::Caps* caps = bgfx::getCaps();
  bx::mtxOrtho(ortho, L, R, B, T, 0.0f, 1000.0f, 0.0f, caps->homogeneousDepth);

  float identity[16];
  bx::mtxIdentity(identity);
  bgfx::setViewTransform(bd->viewId, identity, ortho);
  bgfx::setViewRect(bd->viewId, 0, 0, uint16_t(fbWidth), uint16_t(fbHeight));

  ImVec2 clipOff = drawData->DisplayPos;
  ImVec2 clipScale = drawData->FramebufferScale;

  for (int n = 0; n < drawData->CmdListsCount; n++)
  {
    const ImDrawList* cmdList = drawData->CmdLists[n];

    bgfx::TransientVertexBuffer tvb;
    bgfx::TransientIndexBuffer tib;

    uint32_t numVertices = (uint32_t)cmdList->VtxBuffer.Size;
    uint32_t numIndices = (uint32_t)cmdList->IdxBuffer.Size;

    if (!bgfx::getAvailTransientVertexBuffer(numVertices, bd->vertexLayout) ||
        !bgfx::getAvailTransientIndexBuffer(numIndices))
      break;

    bgfx::allocTransientVertexBuffer(&tvb, numVertices, bd->vertexLayout);
    bgfx::allocTransientIndexBuffer(&tib, numIndices);

    // Copy vertex data — ImGui vertex layout matches ours (pos2f, uv2f, col4b)
    // but we need 3D position for our shader. Pack into our layout.
    struct ImGuiVertex { float x, y, u, v; uint32_t col; };
    const ImDrawVert* vtxSrc = cmdList->VtxBuffer.Data;

    // Our vertex layout: pos3f, uv2f, col4b
    struct BgfxImGuiVert { float x, y, z; float u, v; uint32_t col; };
    BgfxImGuiVert* vtxDst = (BgfxImGuiVert*)tvb.data;

    for (uint32_t i = 0; i < numVertices; i++)
    {
      vtxDst[i].x = vtxSrc[i].pos.x;
      vtxDst[i].y = vtxSrc[i].pos.y;
      vtxDst[i].z = 0.0f;
      vtxDst[i].u = vtxSrc[i].uv.x;
      vtxDst[i].v = vtxSrc[i].uv.y;
      // ImGui stores color as 0xAABBGGRR (ABGR uint32) = RGBA bytes in memory
      // bgfx UChar4Normalized reads bytes in order as r,g,b,a — same convention
      vtxDst[i].col = vtxSrc[i].col;
    }

    // Copy index data
    memcpy(tib.data, cmdList->IdxBuffer.Data, numIndices * sizeof(ImDrawIdx));

    uint32_t offset = 0;
    for (int cmdIdx = 0; cmdIdx < cmdList->CmdBuffer.Size; cmdIdx++)
    {
      const ImDrawCmd& cmd = cmdList->CmdBuffer[cmdIdx];

      if (cmd.UserCallback)
      {
        cmd.UserCallback(cmdList, &cmd);
      }
      else
      {
        // Clip rect
        ImVec4 clipRect;
        clipRect.x = (cmd.ClipRect.x - clipOff.x) * clipScale.x;
        clipRect.y = (cmd.ClipRect.y - clipOff.y) * clipScale.y;
        clipRect.z = (cmd.ClipRect.z - clipOff.x) * clipScale.x;
        clipRect.w = (cmd.ClipRect.w - clipOff.y) * clipScale.y;

        if (clipRect.x < fbWidth && clipRect.y < fbHeight && clipRect.z >= 0.0f && clipRect.w >= 0.0f)
        {
          uint16_t sx = uint16_t(bx::max(clipRect.x, 0.0f));
          uint16_t sy = uint16_t(bx::max(clipRect.y, 0.0f));
          uint16_t sw = uint16_t(bx::min(clipRect.z, (float)fbWidth) - sx);
          uint16_t sh = uint16_t(bx::min(clipRect.w, (float)fbHeight) - sy);

          bgfx::setScissor(sx, sy, sw, sh);

          bgfx::TextureHandle texHandle;
          texHandle.idx = uint16_t((intptr_t)cmd.TextureId);
          bgfx::setTexture(0, bd->texUniform, texHandle);

          bgfx::setVertexBuffer(0, &tvb, 0, numVertices);
          bgfx::setIndexBuffer(&tib, offset, cmd.ElemCount);

          uint64_t state = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A
                         | BGFX_STATE_BLEND_ALPHA
                         | BGFX_STATE_MSAA;
          bgfx::setState(state);
          bgfx::submit(bd->viewId, bd->program);
        }
      }
      offset += cmd.ElemCount;
    }
  }
}
