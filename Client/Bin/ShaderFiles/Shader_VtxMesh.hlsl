
#include "Renderer_Shader_Defines.hlsli"

float4x4 g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
texture2D g_DiffuseTexture; /* �����ؾ��ϴ� ��ǻ�� ������ �ȼ����� �ٸ��ٸ� �� �ȼ��� �׸��� ����޾ƿ;��Ѵ�. */
texture2D g_FontTexture;
texture2D g_SurfaceTexture;
texture2D g_LightTexture;

float g_fTime = 0.016f;
float g_fAlphaValue;

int g_iNumSprite;
int g_iSpriteIndex;
bool g_bXYSwitch;

vector g_vDiffColor;
vector g_vOutLineColor;

struct VS_IN
{
    float3 vPosition : POSITION;
    float3 vNormal : NORMAL;
    float2 vTexcoord : TEXCOORD0;
    float3 vTangent : TANGENT;
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;
    float4 vNormal : NORMAL;
    float2 vTexcoord : TEXCOORD0;
    float4 vWorldPos : TEXCOORD1;
    float4 vProjPos : TEXCOORD2;
    float3 vTangent : TANGENT;
    float3 vBinormal : BINORMAL;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;

	/* mul : ���ϱⰡ ������ ��� ���(�º��� ��, �캯�� �� ���ٸ�)�� ���ؼ� �� ���ϱ⸦ �������ش�. */
    vector vPosition = mul(vector(In.vPosition, 1.f), g_WorldMatrix);
    vPosition = mul(vPosition, g_ViewMatrix);
    vPosition = mul(vPosition, g_ProjMatrix);

    Out.vPosition = vPosition;
    Out.vNormal = normalize(mul(vector(In.vNormal, 0.f), g_WorldMatrix));
    Out.vTexcoord = In.vTexcoord;
    Out.vWorldPos = mul(vector(In.vPosition, 1.f), g_WorldMatrix);
    Out.vProjPos = vPosition;
    Out.vTangent = normalize(mul(vector(In.vTangent, 0.f), g_WorldMatrix));
    Out.vBinormal = normalize(cross(Out.vNormal, Out.vTangent));

    return Out;
}

struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float3 vNormal : NORMAL;
    float2 vTexcoord : TEXCOORD0;
    float4 vWorldPos : TEXCOORD1;
    float4 vProjPos : TEXCOORD2;
    float3 vTangent : TANGENT;
    float3 vBinormal : BINORMAL;
};

struct PS_OUT
{
    float4 vDiffuse : SV_TARGET0;
    float4 vNormal : SV_TARGET1;
    float4 vDepth : SV_TARGET2;
    float4 vPickDepth : SV_TARGET3;
};


PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out;

    Out.vDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
    Out.vNormal = vector(In.vNormal.xyz * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.w / 1000.f, In.vProjPos.z / In.vProjPos.w, 0.f, 0.f);
    Out.vPickDepth = vector(In.vProjPos.w / 1000.f, In.vProjPos.z / In.vProjPos.w, 0.f, 0.f);
    if (Out.vDiffuse.a < 0.1f)
        discard;

    return Out;
}

PS_OUT PS_MOVE_SKY(PS_IN In)
{
    PS_OUT Out;

    // g_fTime�� ����Ͽ� ���� �ӵ��� �ؽ�ó ��ǥ�� �̵��ϵ��� ����
    float2 movingTexcoord = In.vTexcoord;
    movingTexcoord.x += g_fTime * 0.01f; // g_fTime * 0.1f: x ���� �ӵ�
    movingTexcoord.y += g_fTime * 0.005f; // g_fTime * 0.1f: y ���� �ӵ�

    // �̵��� �ؽ�ó ��ǥ�� ���ø�
    Out.vDiffuse = g_DiffuseTexture.Sample(LinearSampler, movingTexcoord);
    if (Out.vDiffuse.a < 0.1f)
        discard;

    return Out;
}

PS_OUT PS_MOVE_DISPLAY(PS_IN In)
{
    PS_OUT Out;

    // g_fTime�� ����Ͽ� ���� �ӵ��� �ؽ�ó ��ǥ�� �̵��ϵ��� ����
    float2 LineTexcoord = In.vTexcoord;
    LineTexcoord.x += g_fTime;
    
    float2 FontTexcoord = In.vTexcoord;
    FontTexcoord.x *= 8.f;
    FontTexcoord.x -= g_fTime;
    
    
    // �̵��� �ؽ�ó ��ǥ�� ���ø�
    Out.vDiffuse = g_DiffuseTexture.Sample(LinearSampler, LineTexcoord);
    
    vector vFontTex = g_FontTexture.Sample(LinearSampler, FontTexcoord);
    
    vFontTex.a = vFontTex.r;
  
    if (vFontTex.r >= 0.5f)
        vFontTex = g_vOutLineColor;
    
    if (vFontTex.g >= 0.5f)
        vFontTex = g_vDiffColor;
    
    //vFontTex *= float4(0.f, 0.f, 1.f, 1.f);
    
    Out.vDiffuse.a = Out.vDiffuse.r;
    Out.vDiffuse.a -= g_fAlphaValue;
    Out.vDiffuse.a = saturate(Out.vDiffuse.a);
    
    Out.vDiffuse += vFontTex;
    
    //Out.vDiffuse.rgb = float3(1.f, 0.f, 1.f);
    if (Out.vDiffuse.a < 0.1f)
        discard;

    return Out;
}

PS_OUT PS_ANIM_TEX(PS_IN In)
{
    PS_OUT Out;

    float fImageRatio = (1.f / g_iNumSprite);
    float fStartSprite = fImageRatio * (g_iSpriteIndex);
    float fEndSprite = fImageRatio * (g_iSpriteIndex + 1);
    
    In.vTexcoord.y += g_fTime;
    
    if (g_bXYSwitch == true)
        In.vTexcoord.x = lerp(fStartSprite, fEndSprite, In.vTexcoord.x);
    else if (g_bXYSwitch == false)
        In.vTexcoord.y = lerp(fStartSprite, fEndSprite, In.vTexcoord.y);
    
    Out.vDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
    Out.vNormal = vector(In.vNormal.xyz * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.w / 1000.f, In.vProjPos.z / In.vProjPos.w, 0.f, 0.f);
    Out.vPickDepth = vector(In.vProjPos.w / 1000.f, In.vProjPos.z / In.vProjPos.w, 0.f, 0.f);
    if (Out.vDiffuse.a < 0.1f)
        discard;

    return Out;
}

PS_OUT PS_WATER(PS_IN In)
{
    PS_OUT Out;
    
    float2 fWaterTexcoord = In.vTexcoord;
    fWaterTexcoord *= 10.f;
    fWaterTexcoord.y += g_fTime;

    
    Out.vDiffuse = g_DiffuseTexture.Sample(LinearSampler, fWaterTexcoord);
    vector vLightTex = g_LightTexture.Sample(LinearSampler, In.vTexcoord * 10.f);
    
    vLightTex.a = vLightTex.r;
    Out.vDiffuse.a = 1.f;
    Out.vDiffuse.rgb = (1.f - Out.vDiffuse.rgb) * float3(0.274f, 0.517f, 0.713f);
    
    Out.vDiffuse += vLightTex;
    
    Out.vNormal = vector(In.vNormal.xyz * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.w / 1000.f, In.vProjPos.z / In.vProjPos.w, 0.f, 0.f);
    Out.vPickDepth = vector(In.vProjPos.w / 1000.f, In.vProjPos.z / In.vProjPos.w, 0.f, 0.f);
    
    if (Out.vDiffuse.a < 0.1f)
        discard;

    return Out;
}

PS_OUT PS_SURFACE(PS_IN In)
{
    PS_OUT Out;

    Out.vDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
    
    float2 vSurTexcoord = In.vTexcoord;
    vSurTexcoord *= 5.f;
    
    vector vSurfaceTex = g_SurfaceTexture.Sample(LinearSampler, vSurTexcoord);
    
    
    float fLength = length(In.vTexcoord - float2(0.5f, 0.5f));
    if (fLength >= 0.4f)
    {
        Out.vDiffuse = lerp(Out.vDiffuse, float4(0.160f, 0.439f, 0.701f, 1.f), fLength );
        vSurfaceTex *= (1.f - (fLength * 2.f));

    }
        
    Out.vDiffuse += vSurfaceTex;
    
    
    
    
    Out.vNormal = vector(In.vNormal.xyz * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.w / 1000.f, In.vProjPos.z / In.vProjPos.w, 0.f, 0.f);
    Out.vPickDepth = vector(In.vProjPos.w / 1000.f, In.vProjPos.z / In.vProjPos.w, 0.f, 0.f);
   

    return Out;
}


technique11 DefaultTechnique
{
//0
    pass Default
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        HullShader = NULL;
        DomainShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }

//1
    pass Move_Sky
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        HullShader = NULL;
        DomainShader = NULL;
        PixelShader = compile ps_5_0 PS_MOVE_SKY();
    }

//2
    pass Move_Display
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        HullShader = NULL;
        DomainShader = NULL;
        PixelShader = compile ps_5_0 PS_MOVE_DISPLAY();
    }

//3
    pass ANIM_TEX
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        HullShader = NULL;
        DomainShader = NULL;
        PixelShader = compile ps_5_0 PS_ANIM_TEX();
    }

//4
    pass WATER
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        HullShader = NULL;
        DomainShader = NULL;
        PixelShader = compile ps_5_0 PS_WATER();
    }

//5
    pass SURFACE
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        HullShader = NULL;
        DomainShader = NULL;
        PixelShader = compile ps_5_0 PS_SURFACE();
    }
}