// dear imgui: Renderer Backend for DirectX11
// This needs to be used along with a Platform Backend (e.g. Win32, GLFW, SDL, etc.)

#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <d3dcompiler.h>

// Data
static ID3D11Device*                g_pd3dDevice = NULL;
static ID3D11DeviceContext*         g_pd3dDeviceContext = NULL;
static ID3D11Buffer*                g_pVertexBuffer = NULL;
static ID3D11Buffer*                g_pIndexBuffer = NULL;
static ID3D11VertexShader*          g_pVertexShader = NULL;
static ID3D11PixelShader*           g_pPixelShader = NULL;
static ID3D11InputLayout*           g_pInputLayout = NULL;
static ID3D11Buffer*                g_pConstantBuffer = NULL;
static ID3D11ShaderResourceView*    g_pFontTextureView = NULL;
static ID3D11SamplerState*          g_pFontSampler = NULL;
static ID3D11BlendState*            g_pBlendState = NULL;
static ID3D11RasterizerState*       g_pRasterizerState = NULL;
static ID3D11DepthStencilState*     g_pDepthStencilState = NULL;

// Forward declarations of helper functions
static bool ImGui_ImplDX11_CreateDeviceObjects();
static void ImGui_ImplDX11_DestroyDeviceObjects();

// Vertex shader bytecode
static const char* g_VertexShaderBytecode = ""
"#define VERTEX_SHADER\n"
"cbuffer cbPerFrame : register(b0)\n"
"{\n"
"    float4x4 g_ProjMtx;\n"
"}\n"
"struct VS_INPUT\n"
"{\n"
"    float2 Pos : POSITION;\n"
"    float2 UV : TEXCOORD0;\n"
"    float4 Color : COLOR0;\n"
"};\n"
"struct PS_INPUT\n"
"{\n"
"    float4 Pos : SV_POSITION;\n"
"    float2 UV : TEXCOORD0;\n"
"    float4 Color : COLOR0;\n"
"};\n"
"PS_INPUT vs_main(VS_INPUT input)\n"
"{\n"
"    PS_INPUT output;\n"
"    output.Pos = mul(g_ProjMtx, float4(input.Pos.xy, 0.0f, 1.0f));\n"
"    output.UV = input.UV;\n"
"    output.Color = input.Color;\n"
"    return output;\n"
"}\n";

// Pixel shader bytecode
static const char* g_PixelShaderBytecode = ""
"#define PIXEL_SHADER\n"
"SamplerState g_sampler : register(s0);\n"
"Texture2D g_texture : register(t0);\n"
"struct PS_INPUT\n"
"{\n"
"    float4 Pos : SV_POSITION;\n"
"    float2 UV : TEXCOORD0;\n"
"    float4 Color : COLOR0;\n"
"};\n"
"float4 ps_main(PS_INPUT input) : SV_Target\n"
"{\n"
"    float4 texColor = g_texture.Sample(g_sampler, input.UV);\n"
"    return texColor * input.Color;\n"
"}\n";

// ImGui_ImplDX11_Init
bool ImGui_ImplDX11_Init(ID3D11Device* device, ID3D11DeviceContext* device_context)
{
    g_pd3dDevice = device;
    g_pd3dDeviceContext = device_context;

    // Setup back-end capabilities flags
    ImGuiIO& io = ImGui::GetIO();
    io.BackendRendererName = "imgui_impl_dx11";
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

    return true;
}

// ImGui_ImplDX11_Shutdown
void ImGui_ImplDX11_Shutdown()
{
    ImGui_ImplDX11_DestroyDeviceObjects();
    g_pd3dDevice = NULL;
    g_pd3dDeviceContext = NULL;
}

// ImGui_ImplDX11_NewFrame
void ImGui_ImplDX11_NewFrame()
{
    if (!g_pFontTextureView)
        ImGui_ImplDX11_CreateDeviceObjects();
}

// ImGui_ImplDX11_RenderDrawData
void ImGui_ImplDX11_RenderDrawData(ImDrawData* draw_data)
{
    // If no draw data, return early
    if (!draw_data || draw_data->TotalVtxCount == 0) 
        return;

    // Create device objects if needed
    if (!g_pFontTextureView)
        ImGui_ImplDX11_CreateDeviceObjects();

    // Backup current state
    ID3D11RenderTargetView* backup_rtv = NULL;
    ID3D11DepthStencilView* backup_dsv = NULL;
    ID3D11RasterizerState* backup_rs = NULL;
    ID3D11BlendState* backup_bs = NULL;
    ID3D11DepthStencilState* backup_dss = NULL;
    ID3D11SamplerState* backup_sampler = NULL;
    ID3D11ShaderResourceView* backup_srv = NULL;
    ID3D11VertexShader* backup_vs = NULL;
    ID3D11PixelShader* backup_ps = NULL;
    ID3D11InputLayout* backup_il = NULL;
    ID3D11Buffer* backup_vb = NULL;
    ID3D11Buffer* backup_ib = NULL;
    UINT backup_stride = 0;
    UINT backup_offset = 0;

    g_pd3dDeviceContext->OMGetRenderTargets(1, &backup_rtv, &backup_dsv);
    g_pd3dDeviceContext->RSGetState(&backup_rs);
    g_pd3dDeviceContext->OMGetBlendState(&backup_bs, NULL, NULL);
    g_pd3dDeviceContext->OMGetDepthStencilState(&backup_dss, NULL);
    g_pd3dDeviceContext->PSGetSamplers(0, 1, &backup_sampler);
    g_pd3dDeviceContext->PSGetShaderResources(0, 1, &backup_srv);
    g_pd3dDeviceContext->VSGetShader(&backup_vs, NULL, NULL);
    g_pd3dDeviceContext->PSGetShader(&backup_ps, NULL, NULL);
    g_pd3dDeviceContext->IAGetInputLayout(&backup_il);
    g_pd3dDeviceContext->IAGetVertexBuffers(0, 1, &backup_vb, &backup_stride, &backup_offset);
    g_pd3dDeviceContext->IAGetIndexBuffer(&backup_ib, NULL, NULL);

    // Setup render state
    g_pd3dDeviceContext->RSSetState(g_pRasterizerState);
    g_pd3dDeviceContext->OMSetBlendState(g_pBlendState, NULL, 0xffffff);
    g_pd3dDeviceContext->OMSetDepthStencilState(g_pDepthStencilState, 0);
    g_pd3dDeviceContext->PSSetSamplers(0, 1, &g_pFontSampler);
    g_pd3dDeviceContext->PSSetShaderResources(0, 1, &g_pFontTextureView);
    g_pd3dDeviceContext->VSSetShader(g_pVertexShader, NULL, 0);
    g_pd3dDeviceContext->PSSetShader(g_pPixelShader, NULL, 0);
    g_pd3dDeviceContext->IASetInputLayout(g_pInputLayout);

    // Setup orthographic projection matrix
    const float L = draw_data->DisplayPos.x;
    const float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
    const float T = draw_data->DisplayPos.y;
    const float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
    const float ortho_projection[4][4] = {
        { 2.0f/(R-L),   0.0f,         0.0f,   0.0f },
        { 0.0f,         2.0f/(T-B),   0.0f,   0.0f },
        { 0.0f,         0.0f,         -1.0f,  0.0f },
        { (R+L)/(L-R),  (T+B)/(B-T),   0.0f,   1.0f },
    };
    g_pd3dDeviceContext->UpdateSubresource(g_pConstantBuffer, 0, NULL, ortho_projection, 0, 0);
    g_pd3dDeviceContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);

    // Render command lists
    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];

        // Update vertex and index buffers
        D3D11_BUFFER_DESC vb_desc;
        memset(&vb_desc, 0, sizeof(vb_desc));
        vb_desc.ByteWidth = (UINT)(sizeof(ImDrawVert) * cmd_list->VtxBuffer.Size);
        vb_desc.Usage = D3D11_USAGE_DYNAMIC;
        vb_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vb_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        if (g_pVertexBuffer)
            g_pVertexBuffer->Release();
        if (FAILED(g_pd3dDevice->CreateBuffer(&vb_desc, NULL, &g_pVertexBuffer)))
            return;

        D3D11_BUFFER_DESC ib_desc;
        memset(&ib_desc, 0, sizeof(ib_desc));
        ib_desc.ByteWidth = (UINT)(sizeof(ImDrawIdx) * cmd_list->IdxBuffer.Size);
        ib_desc.Usage = D3D11_USAGE_DYNAMIC;
        ib_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        ib_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        if (g_pIndexBuffer)
            g_pIndexBuffer->Release();
        if (FAILED(g_pd3dDevice->CreateBuffer(&ib_desc, NULL, &g_pIndexBuffer)))
            return;

        // Copy data to buffers
        D3D11_MAPPED_SUBRESOURCE mapped_vb, mapped_ib;
        if (FAILED(g_pd3dDeviceContext->Map(g_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_vb)))
            return;
        memcpy(mapped_vb.pData, cmd_list->VtxBuffer.Data, (size_t)vb_desc.ByteWidth);
        g_pd3dDeviceContext->Unmap(g_pVertexBuffer, 0);

        if (FAILED(g_pd3dDeviceContext->Map(g_pIndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_ib)))
            return;
        memcpy(mapped_ib.pData, cmd_list->IdxBuffer.Data, (size_t)ib_desc.ByteWidth);
        g_pd3dDeviceContext->Unmap(g_pIndexBuffer, 0);

        // Setup buffers
        UINT stride = sizeof(ImDrawVert);
        UINT offset = 0;
        g_pd3dDeviceContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
        g_pd3dDeviceContext->IASetIndexBuffer(g_pIndexBuffer, sizeof(ImDrawIdx) == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0);
        g_pd3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // Render commands
        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback)
            {
                // User callback
                pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
                // Set scissor rectangle
                D3D11_RECT scissor_rect;
                scissor_rect.left = (LONG)pcmd->ClipRect.x;
                scissor_rect.top = (LONG)pcmd->ClipRect.y;
                scissor_rect.right = (LONG)pcmd->ClipRect.z;
                scissor_rect.bottom = (LONG)pcmd->ClipRect.w;
                g_pd3dDeviceContext->RSSetScissorRects(1, &scissor_rect);

                // Draw
                g_pd3dDeviceContext->DrawIndexed(pcmd->ElemCount, pcmd->IdxOffset, pcmd->VtxOffset);
            }
        }
    }

    // Restore backup state
    g_pd3dDeviceContext->OMSetRenderTargets(1, &backup_rtv, backup_dsv);
    g_pd3dDeviceContext->RSSetState(backup_rs);
    g_pd3dDeviceContext->OMSetBlendState(backup_bs, NULL, 0xffffff);
    g_pd3dDeviceContext->OMSetDepthStencilState(backup_dss, 0);
    g_pd3dDeviceContext->PSSetSamplers(0, 1, &backup_sampler);
    g_pd3dDeviceContext->PSSetShaderResources(0, 1, &backup_srv);
    g_pd3dDeviceContext->VSSetShader(backup_vs, NULL, 0);
    g_pd3dDeviceContext->PSGetShader(&backup_ps, NULL, NULL);
    g_pd3dDeviceContext->IASetInputLayout(backup_il);
    g_pd3dDeviceContext->IASetVertexBuffers(0, 1, &backup_vb, &backup_stride, &backup_offset);
    g_pd3dDeviceContext->IASetIndexBuffer(backup_ib, sizeof(ImDrawIdx) == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0);

    // Release backup objects
    if (backup_rs) backup_rs->Release();
    if (backup_bs) backup_bs->Release();
    if (backup_dss) backup_dss->Release();
    if (backup_sampler) backup_sampler->Release();
    if (backup_srv) backup_srv->Release();
    if (backup_vs) backup_vs->Release();
    if (backup_ps) backup_ps->Release();
    if (backup_il) backup_il->Release();
    if (backup_vb) backup_vb->Release();
    if (backup_ib) backup_ib->Release();
    if (backup_rtv) backup_rtv->Release();
    if (backup_dsv) backup_dsv->Release();
}

// ImGui_ImplDX11_CreateDeviceObjects
static bool ImGui_ImplDX11_CreateDeviceObjects()
{
    // Create vertex shader
    ID3D10Blob* vs_blob = NULL;
    if (FAILED(D3DCompile(g_VertexShaderBytecode, strlen(g_VertexShaderBytecode), NULL, NULL, NULL, "vs_main", "vs_5_0", 0, 0, &vs_blob, NULL)))
        return false;

    if (FAILED(g_pd3dDevice->CreateVertexShader(vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), NULL, &g_pVertexShader)))
    {
        vs_blob->Release();
        return false;
    }

    // Create pixel shader
    ID3D10Blob* ps_blob = NULL;
    if (FAILED(D3DCompile(g_PixelShaderBytecode, strlen(g_PixelShaderBytecode), NULL, NULL, NULL, "ps_main", "ps_5_0", 0, 0, &ps_blob, NULL)))
    {
        vs_blob->Release();
        return false;
    }

    if (FAILED(g_pd3dDevice->CreatePixelShader(ps_blob->GetBufferPointer(), ps_blob->GetBufferSize(), NULL, &g_pPixelShader)))
    {
        vs_blob->Release();
        ps_blob->Release();
        return false;
    }

    // Create input layout
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    if (FAILED(g_pd3dDevice->CreateInputLayout(layout, 3, vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), &g_pInputLayout)))
    {
        vs_blob->Release();
        ps_blob->Release();
        return false;
    }

    vs_blob->Release();
    ps_blob->Release();

    // Create constant buffer
    D3D11_BUFFER_DESC cb_desc;
    memset(&cb_desc, 0, sizeof(cb_desc));
    cb_desc.ByteWidth = 64;
    cb_desc.Usage = D3D11_USAGE_DYNAMIC;
    cb_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cb_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    if (FAILED(g_pd3dDevice->CreateBuffer(&cb_desc, NULL, &g_pConstantBuffer)))
        return false;

    // Create blend state
    D3D11_BLEND_DESC bs_desc;
    memset(&bs_desc, 0, sizeof(bs_desc));
    bs_desc.AlphaToCoverageEnable = false;
    bs_desc.IndependentBlendEnable = false;
    bs_desc.RenderTarget[0].BlendEnable = true;
    bs_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    bs_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    bs_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    bs_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    bs_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
    bs_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    bs_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    if (FAILED(g_pd3dDevice->CreateBlendState(&bs_desc, &g_pBlendState)))
        return false;

    // Create rasterizer state
    D3D11_RASTERIZER_DESC rs_desc;
    memset(&rs_desc, 0, sizeof(rs_desc));
    rs_desc.FillMode = D3D11_FILL_SOLID;
    rs_desc.CullMode = D3D11_CULL_NONE;
    rs_desc.FrontCounterClockwise = false;
    rs_desc.DepthBias = 0;
    rs_desc.DepthBiasClamp = 0.0f;
    rs_desc.SlopeScaledDepthBias = 0.0f;
    rs_desc.DepthClipEnable = true;
    rs_desc.ScissorEnable = true;
    rs_desc.MultisampleEnable = false;
    rs_desc.AntialiasedLineEnable = false;
    if (FAILED(g_pd3dDevice->CreateRasterizerState(&rs_desc, &g_pRasterizerState)))
        return false;

    // Create depth stencil state
    D3D11_DEPTH_STENCIL_DESC dss_desc;
    memset(&dss_desc, 0, sizeof(dss_desc));
    dss_desc.DepthEnable = false;
    dss_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dss_desc.DepthFunc = D3D11_COMPARISON_LESS;
    dss_desc.StencilEnable = false;
    if (FAILED(g_pd3dDevice->CreateDepthStencilState(&dss_desc, &g_pDepthStencilState)))
        return false;

    // Create sampler state
    D3D11_SAMPLER_DESC sampler_desc;
    memset(&sampler_desc, 0, sizeof(sampler_desc));
    sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampler_desc.MipLODBias = 0.0f;
    sampler_desc.MaxAnisotropy = 1;
    sampler_desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    sampler_desc.BorderColor[0] = 0.0f;
    sampler_desc.BorderColor[1] = 0.0f;
    sampler_desc.BorderColor[2] = 0.0f;
    sampler_desc.BorderColor[3] = 0.0f;
    sampler_desc.MinLOD = 0.0f;
    sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;
    if (FAILED(g_pd3dDevice->CreateSamplerState(&sampler_desc, &g_pFontSampler)))
        return false;

    // Create fonts texture
    ImGuiIO& io = ImGui::GetIO();
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    D3D11_TEXTURE2D_DESC tex_desc;
    memset(&tex_desc, 0, sizeof(tex_desc));
    tex_desc.Width = width;
    tex_desc.Height = height;
    tex_desc.MipLevels = 1;
    tex_desc.ArraySize = 1;
    tex_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    tex_desc.SampleDesc.Count = 1;
    tex_desc.SampleDesc.Quality = 0;
    tex_desc.Usage = D3D11_USAGE_DEFAULT;
    tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    tex_desc.CPUAccessFlags = 0;
    tex_desc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA subresource;
    memset(&subresource, 0, sizeof(subresource));
    subresource.pSysMem = pixels;
    subresource.SysMemPitch = width * 4;
    subresource.SysMemSlicePitch = 0;

    ID3D11Texture2D* texture = NULL;
    if (FAILED(g_pd3dDevice->CreateTexture2D(&tex_desc, &subresource, &texture)))
        return false;

    D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
    memset(&srv_desc, 0, sizeof(srv_desc));
    srv_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srv_desc.Texture2D.MipLevels = 1;
    srv_desc.Texture2D.MostDetailedMip = 0;

    if (FAILED(g_pd3dDevice->CreateShaderResourceView(texture, &srv_desc, &g_pFontTextureView)))
    {
        texture->Release();
        return false;
    }

    texture->Release();
    io.Fonts->SetTexID((ImTextureID)g_pFontTextureView);

    return true;
}

// ImGui_ImplDX11_DestroyDeviceObjects
static void ImGui_ImplDX11_DestroyDeviceObjects()
{
    if (g_pVertexBuffer) { g_pVertexBuffer->Release(); g_pVertexBuffer = NULL; }
    if (g_pIndexBuffer) { g_pIndexBuffer->Release(); g_pIndexBuffer = NULL; }
    if (g_pVertexShader) { g_pVertexShader->Release(); g_pVertexShader = NULL; }
    if (g_pPixelShader) { g_pPixelShader->Release(); g_pPixelShader = NULL; }
    if (g_pInputLayout) { g_pInputLayout->Release(); g_pInputLayout = NULL; }
    if (g_pConstantBuffer) { g_pConstantBuffer->Release(); g_pConstantBuffer = NULL; }
    if (g_pFontTextureView) { g_pFontTextureView->Release(); g_pFontTextureView = NULL; }
    if (g_pFontSampler) { g_pFontSampler->Release(); g_pFontSampler = NULL; }
    if (g_pBlendState) { g_pBlendState->Release(); g_pBlendState = NULL; }
    if (g_pRasterizerState) { g_pRasterizerState->Release(); g_pRasterizerState = NULL; }
    if (g_pDepthStencilState) { g_pDepthStencilState->Release(); g_pDepthStencilState = NULL; }

    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->SetTexID(NULL);
}
