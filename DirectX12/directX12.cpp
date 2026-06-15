#include <windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>
#include <cstring>
#include <cmath>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

ID3D12Device* dev; ID3D12CommandQueue* cq; IDXGISwapChain3* sc;
ID3D12DescriptorHeap* rtvH; ID3D12Resource* rt[2], * vb;
ID3D12CommandAllocator* ca; ID3D12GraphicsCommandList* cl;
ID3D12RootSignature* rs; ID3D12PipelineState* pso;
D3D12_VERTEX_BUFFER_VIEW vbV; HANDLE ev; ID3D12Fence* fnc; UINT64 fv = 0;
ID3D12Resource* cb;

struct CBData
{
    float m[16];
};

float angle = 0.0f;

void Init(HWND hw) {
    D3D12CreateDevice(0, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), (void**)&dev);
    D3D12_COMMAND_QUEUE_DESC qd = { D3D12_COMMAND_LIST_TYPE_DIRECT };
    dev->CreateCommandQueue(&qd, __uuidof(ID3D12CommandQueue), (void**)&cq);
    DXGI_SWAP_CHAIN_DESC sd = { {800,600,{60,1},DXGI_FORMAT_R8G8B8A8_UNORM},{1,0},32,2,hw,1,DXGI_SWAP_EFFECT_FLIP_DISCARD };
    IDXGIFactory4* f; CreateDXGIFactory1(__uuidof(IDXGIFactory4), (void**)&f);
    f->CreateSwapChain(cq, &sd, (IDXGISwapChain**)&sc);
    D3D12_DESCRIPTOR_HEAP_DESC hd = { D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2 };
    dev->CreateDescriptorHeap(&hd, __uuidof(ID3D12DescriptorHeap), (void**)&rtvH);
    UINT sz = dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    D3D12_CPU_DESCRIPTOR_HANDLE h = rtvH->GetCPUDescriptorHandleForHeapStart();
    for (int i = 0; i < 2; i++) {
        sc->GetBuffer(i, __uuidof(ID3D12Resource), (void**)&rt[i]);
        dev->CreateRenderTargetView(rt[i], 0, h); h.ptr += sz;
    }
    D3D12_ROOT_PARAMETER rp = {};
    rp.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rp.Descriptor.ShaderRegister = 0;
    rp.Descriptor.RegisterSpace = 0;
    rp.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

    D3D12_ROOT_SIGNATURE_DESC rd = {};
    rd.NumParameters = 1;
    rd.pParameters = &rp;
    rd.Flags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    ID3DBlob* rb, * eb; D3D12SerializeRootSignature(&rd, D3D_ROOT_SIGNATURE_VERSION_1, &rb, &eb);
    dev->CreateRootSignature(0, rb->GetBufferPointer(), rb->GetBufferSize(), __uuidof(ID3D12RootSignature), (void**)&rs);
    const char* c =
        "cbuffer CB : register(b0)"
        "{"
        " float4x4 rot;"
        "};"
        "struct V"
        "{"
        " float4 p : SV_POSITION;"
        " float4 c : COLOR;"
        "};"
        "V VS(float4 p : POSITION, float4 c : COLOR)"
        "{"
        " V o;"
        " o.p = mul(rot, p);"
        " o.c = c;"
        " return o;"
        "}"
        "float4 PS(V i) : SV_TARGET"
        "{"
        " return i.c;"
        "}";
    ID3DBlob* vs, * ps; D3DCompile(c, strlen(c), 0, 0, 0, "VS", "vs_5_0", 0, 0, &vs, 0); D3DCompile(c, strlen(c), 0, 0, 0, "PS", "ps_5_0", 0, 0, &ps, 0);
    D3D12_INPUT_ELEMENT_DESC ied[] = { {"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},{"COLOR",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,12,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0} };
    D3D12_GRAPHICS_PIPELINE_STATE_DESC pd = {};
    pd.pRootSignature = rs; pd.VS = { vs->GetBufferPointer(), vs->GetBufferSize() }; pd.PS = { ps->GetBufferPointer(), ps->GetBufferSize() };
    pd.InputLayout = { ied, 2 }; pd.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID; pd.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    pd.BlendState.RenderTarget[0].RenderTargetWriteMask = 15; pd.SampleMask = -1;
    pd.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; pd.NumRenderTargets = 1;
    pd.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; pd.SampleDesc.Count = 1;
    dev->CreateGraphicsPipelineState(&pd, __uuidof(ID3D12PipelineState), (void**)&pso);
    dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void**)&ca);
    dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, ca, pso, __uuidof(ID3D12GraphicsCommandList), (void**)&cl); cl->Close();
    float v[] = { 0,.5,0, 1,0,0,1, .5,-.5,0, 0,1,0,1, -.5,-.5,0, 0,0,1,1 };
    D3D12_HEAP_PROPERTIES hp = { D3D12_HEAP_TYPE_UPLOAD };
    D3D12_RESOURCE_DESC res = { D3D12_RESOURCE_DIMENSION_BUFFER,0,sizeof(v),1,1,1,DXGI_FORMAT_UNKNOWN,{1,0},D3D12_TEXTURE_LAYOUT_ROW_MAJOR };
    dev->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE, &res, D3D12_RESOURCE_STATE_GENERIC_READ, 0, __uuidof(ID3D12Resource), (void**)&vb);
    void* p; vb->Map(0, 0, &p); memcpy(p, v, sizeof(v)); vb->Unmap(0, 0);
    vbV = { vb->GetGPUVirtualAddress(), (UINT)sizeof(v), 28 };
    D3D12_RESOURCE_DESC cbDesc = {};
    cbDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    cbDesc.Width = 256;
    cbDesc.Height = 1;
    cbDesc.DepthOrArraySize = 1;
    cbDesc.MipLevels = 1;
    cbDesc.SampleDesc.Count = 1;
    cbDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    dev->CreateCommittedResource(
        &hp,
        D3D12_HEAP_FLAG_NONE,
        &cbDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        0,
        __uuidof(ID3D12Resource),
        (void**)&cb
    );
    CBData initData =
    {
        {
            1,0,0,0,
            0,1,0,0,
            0,0,1,0,
            0,0,0,1
        }
    };

    void* cp;
    cb->Map(0, 0, &cp);
    memcpy(cp, &initData, sizeof(initData));
    cb->Unmap(0, 0);
    dev->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void**)&fnc); ev = CreateEvent(0, 0, 0, 0);
}

void Render() {
    angle += 0.02f;
    float s = sinf(angle);
    float cs = cosf(angle);
    CBData data =
    {
        {
             cs, -s, 0, 0,
             s,  cs, 0, 0,
             0,  0, 1, 0,
             0,  0, 0, 1
        }
    };
    void* cp;
    cb->Map(0, 0, &cp);
    memcpy(cp, &data, sizeof(data));
    cb->Unmap(0, 0);
    ca->Reset(); cl->Reset(ca, pso);
    UINT idx = sc->GetCurrentBackBufferIndex();
    D3D12_RESOURCE_BARRIER b = { D3D12_RESOURCE_BARRIER_TYPE_TRANSITION, (D3D12_RESOURCE_BARRIER_FLAGS)0, {rt[idx], D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET} };
    cl->ResourceBarrier(1, &b);
    D3D12_CPU_DESCRIPTOR_HANDLE h = rtvH->GetCPUDescriptorHandleForHeapStart();
    h.ptr += idx * dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    float c[] = { .1f,.1f,.1f,1 }; cl->ClearRenderTargetView(h, c, 0, 0);
    D3D12_VIEWPORT vp = { 0,0,800,600,0,1 }; D3D12_RECT sr = { 0,0,800,600 };
    cl->RSSetViewports(1, &vp); cl->RSSetScissorRects(1, &sr); cl->OMSetRenderTargets(1, &h, 0, 0);
    cl->SetGraphicsRootSignature(rs);
    cl->SetGraphicsRootConstantBufferView(
        0,
        cb->GetGPUVirtualAddress()
    );
    cl->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    cl->IASetVertexBuffers(0, 1, &vbV); cl->DrawInstanced(3, 1, 0, 0);
    b.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET; b.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    cl->ResourceBarrier(1, &b); cl->Close();
    ID3D12CommandList* l[] = { cl }; cq->ExecuteCommandLists(1, l); sc->Present(1, 0);
    const UINT64 f = ++fv; cq->Signal(fnc, f);
    if (fnc->GetCompletedValue() < f) { fnc->SetEventOnCompletion(f, ev); WaitForSingleObject(ev, -1); }
}

LRESULT CALLBACK WndProc(HWND hw, UINT m, WPARAM wp, LPARAM lp) {
    if (m == WM_DESTROY) PostQuitMessage(0); return DefWindowProc(hw, m, wp, lp);
}

int WINAPI WinMain(HINSTANCE hI, HINSTANCE, LPSTR, int) {
    WNDCLASS wc = { 0, WndProc, 0, 0, hI, 0, 0, 0, 0, L"C" }; RegisterClass(&wc);
    HWND hw = CreateWindow(L"C", L"D3D12", 0x10CF0000, 100, 100, 800, 600, 0, 0, hI, 0);
    Init(hw);
    MSG m = {};
    while (m.message != WM_QUIT) {
        if (PeekMessage(&m, 0, 0, 0, 1)) { TranslateMessage(&m); DispatchMessage(&m); }
        else Render();
    }
    return 0;
}