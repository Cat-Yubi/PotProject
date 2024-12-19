
#include "Renderer_Shader_Defines.hlsli"

float4x4 g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
texture2D g_Texture;
float4 g_vColor = vector(1.f, 1.f, 1.f, 1.f);
float4 g_vCamPosition;
float g_Time;
float g_MaxTime;

float g_ScaleX = 7.f;
float g_ScaleY = 0.5f;

struct VS_IN
{
	/* ���� ���̴����� ���ŵǼ� ������ �����͵� */
    float3 vPosition : POSITION;

	/* ���� ���������� �ѹ��� ���ŵǴ� �����͵�. */
    // �ν��Ͻ� ���ۿ� �ִ� �༮��
    row_major matrix LocalMatrix : WORLD;
    float2 vLifeTime : TEXCOORD0;
    float3 vMoveDir : TEXCOORD1; // New field for moveDir
};

struct VS_OUT
{
    float4 vPosition : POSITION;
    float2 vPSize : PSIZE;
    float2 vLifeTime : TEXCOORD0;
    float3 vMoveDir : TEXCOORD1; // New field for moveDir
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;

    vector vPosition = mul(vector(In.vPosition, 1.f), In.LocalMatrix);
    Out.vPosition = mul(vPosition, g_WorldMatrix);
    Out.vPSize = float2(length(In.LocalMatrix._11_12_13), length(In.LocalMatrix._21_22_23));
    Out.vLifeTime = In.vLifeTime;
    Out.vMoveDir = In.vMoveDir;
    
    return Out;
}

struct GS_IN
{
    float4 vPosition : POSITION;
    float2 vPSize : PSIZE;
    float2 vLifeTime : TEXCOORD0;
    float3 vMoveDir : TEXCOORD1;
};

struct GS_OUT
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
    float2 vLifeTime : TEXCOORD1;
};

/* �簢���� �����ϱ����� ������ �����Ҳ���. */
/* �׻� ī�޶� �Ĵٺ��Բ� ������ٲ���. */
/* g_vCamPosition��ġ�� ������ ��ġ�� ���� ���� �簢���� �ٶ������ ���� ���Ұž�. */
/* ��������󿡼� ������ �Ͼ�ٰ� �� �� �ֱ⶧���� ������ ��ġ�� ��������� ��ȯ�Ͽ� �����Ѵ�. */

/* GeometryShader : ������ �߰� �������� �� �ִ�. */
/* Point�� �׸���. �Ķ���ͷ� ������ �ϳ��� �޴°ž�. point GS_IN In[1] */
/* Line�� �׸���. �Ķ���ͷ� ������ �ΰ��� �޴°ž�. line GS_IN In[2]*/
/* Triangle�� �׸���. �Ķ���ͷ� ������ ������ �޴°ž�. triangle GS_IN In[3]*/

[maxvertexcount(20)]
void GS_MAIN(point GS_IN In[1], inout TriangleStream<GS_OUT> VertexStream)
{
    GS_OUT Out[4];

    // ī�޶� ���ϴ� ���� ���
    float3 vLook = normalize(g_vCamPosition.xyz - In[0].vPosition.xyz);
    float3 vRight = normalize(cross(float3(0.f, 1.f, 0.f), vLook)) * In[0].vPSize.x * 0.5f;
    float3 vUp = normalize(cross(vLook, vRight)) * In[0].vPSize.y * 0.5f;

    // ���⿡�� ���ϴ� ������ �����մϴ�.
    float xScaleFactor = 5.f; // X�� ũ�� ����
    float yScaleFactor = 2.0f; // Y�� ũ�� ����

    vRight *= xScaleFactor;
    vUp *= yScaleFactor;

    matrix matVP = mul(g_ViewMatrix, g_ProjMatrix);

    // �簢���� �� ���� ���
    Out[0].vPosition = float4(In[0].vPosition.xyz + vRight + vUp, 1.f);
    Out[0].vTexcoord = float2(0.f, 0.f);
    Out[0].vPosition = mul(Out[0].vPosition, matVP);
    Out[0].vLifeTime = In[0].vLifeTime;

    Out[1].vPosition = float4(In[0].vPosition.xyz - vRight + vUp, 1.f);
    Out[1].vTexcoord = float2(1.f, 0.f);
    Out[1].vPosition = mul(Out[1].vPosition, matVP);
    Out[1].vLifeTime = In[0].vLifeTime;

    Out[2].vPosition = float4(In[0].vPosition.xyz - vRight - vUp, 1.f);
    Out[2].vTexcoord = float2(1.f, 1.f);
    Out[2].vPosition = mul(Out[2].vPosition, matVP);
    Out[2].vLifeTime = In[0].vLifeTime;

    Out[3].vPosition = float4(In[0].vPosition.xyz + vRight - vUp, 1.f);
    Out[3].vTexcoord = float2(0.f, 1.f);
    Out[3].vPosition = mul(Out[3].vPosition, matVP);
    Out[3].vLifeTime = In[0].vLifeTime;

    // �ﰢ�� ��Ʈ�� ����
    VertexStream.Append(Out[0]);
    VertexStream.Append(Out[1]);
    VertexStream.Append(Out[2]);
    VertexStream.RestartStrip();

    VertexStream.Append(Out[0]);
    VertexStream.Append(Out[2]);
    VertexStream.Append(Out[3]);
    VertexStream.RestartStrip();
}

[maxvertexcount(20)]
void GS_MAIN_NOTBillBoard(point GS_IN In[1], inout TriangleStream<GS_OUT> VertexStream)
{
  // moveDir�� ����ȭ�Ͽ� ���� ���� ����
    float3 vDir = normalize(In[0].vMoveDir);

    // vDir�� ������ ���� ���
    float3 worldUp = float3(0.0f, 1.0f, 0.0f);
    if (abs(dot(vDir, worldUp)) > 0.99f)
    {
        worldUp = float3(1.0f, 0.0f, 0.0f);
    }
    float3 vPerp = normalize(cross(worldUp, vDir));

    // �����ϸ� ���� ����
    float lengthDir = g_ScaleX * In[0].vPSize.x;
    float lengthPerp = g_ScaleY * In[0].vPSize.y;

    // �߽� ��ġ
    float3 center = In[0].vPosition.xyz;

    // �� ������ ��� (���� ����)
    float3 corner0 = center - (vDir * lengthDir * 0.5f) + (vPerp * lengthPerp * 0.5f); // �»��
    float3 corner1 = center + (vDir * lengthDir * 0.5f) + (vPerp * lengthPerp * 0.5f); // ����
    float3 corner2 = center + (vDir * lengthDir * 0.5f) - (vPerp * lengthPerp * 0.5f); // ���ϴ�
    float3 corner3 = center - (vDir * lengthDir * 0.5f) - (vPerp * lengthPerp * 0.5f); // ���ϴ�

    // �ؽ�ó ��ǥ ���� (���� ����)
    float2 texcoords[4] =
    {
        float2(0.0f, 0.0f), // corner0 (���ϴ�)
        float2(1.0f, 0.0f), // corner1 (���ϴ�)
        float2(1.0f, 1.0f), // corner2 (����)
        float2(0.0f, 1.0f) // corner3 (�»��)
    };

    // ��-�������� ��� ���
    matrix matVP = mul(g_ViewMatrix, g_ProjMatrix);

    // ��� ����ü�� �� ���� �� ��ȯ
    GS_OUT vertices[4];
    float3 corners[4] = { corner0, corner1, corner2, corner3 };
    for (int i = 0; i < 4; ++i)
    {
        vertices[i].vPosition = mul(float4(corners[i], 1.0f), matVP);
        vertices[i].vTexcoord = texcoords[i];
        vertices[i].vLifeTime = In[0].vLifeTime;
    }

    // �ﰢ�� ���� (���� ����)
    VertexStream.Append(vertices[0]); // corner0
    VertexStream.Append(vertices[1]); // corner1
    VertexStream.Append(vertices[2]); // corner2

    VertexStream.Append(vertices[0]); // corner0
    VertexStream.Append(vertices[2]); // corner2
    VertexStream.Append(vertices[3]); // corner3
}



[maxvertexcount(20)]
void GS_QTE_PARTICLE(point GS_IN In[1], inout TriangleStream<GS_OUT> VertexStream)
{
    GS_OUT Out[4];

    float3 vRight = float3(In[0].vPSize.x * 0.5f, 0.f, 0.f);
    float3 vUp = float3(0.f, In[0].vPSize.y * 0.5f, 0.f);

    matrix matVP = mul(g_ViewMatrix, g_ProjMatrix);
    Out[0].vPosition = vector(In[0].vPosition.xyz - vRight + vUp, 1.f);
    Out[0].vTexcoord = float2(0.f, 0.f);
    Out[0].vPosition = mul(Out[0].vPosition, matVP);
    Out[0].vLifeTime = In[0].vLifeTime;

    Out[1].vPosition = vector(In[0].vPosition.xyz + vRight + vUp, 1.f);
    Out[1].vTexcoord = float2(1.f, 0.f);
    Out[1].vPosition = mul(Out[1].vPosition, matVP);
    Out[1].vLifeTime = In[0].vLifeTime;

    Out[2].vPosition = vector(In[0].vPosition.xyz + vRight - vUp, 1.f);
    Out[2].vTexcoord = float2(1.f, 1.f);
    Out[2].vPosition = mul(Out[2].vPosition, matVP);
    Out[2].vLifeTime = In[0].vLifeTime;

    Out[3].vPosition = vector(In[0].vPosition.xyz - vRight - vUp, 1.f);
    Out[3].vTexcoord = float2(0.f, 1.f);
    Out[3].vPosition = mul(Out[3].vPosition, matVP);
    Out[3].vLifeTime = In[0].vLifeTime;
	
    VertexStream.Append(Out[0]);
    VertexStream.Append(Out[1]);
    VertexStream.Append(Out[2]);
    VertexStream.RestartStrip();

    VertexStream.Append(Out[0]);
    VertexStream.Append(Out[2]);
    VertexStream.Append(Out[3]);
    VertexStream.RestartStrip();
}


[maxvertexcount(20)]
void GS_DUST(point GS_IN In[1], inout TriangleStream<GS_OUT> VertexStream)
{
    GS_OUT Out[4];

    float3 vLook, vRight, vUp;
    vLook = g_vCamPosition.xyz - In[0].vPosition.xyz;
    vRight = normalize(cross(float3(0.f, 1.f, 0.f), vLook)) * In[0].vPSize.x * 0.5f;
    vUp = normalize(cross(vLook, vRight)) * In[0].vPSize.y * 0.5f;

    matrix matVP = mul(g_ViewMatrix, g_ProjMatrix);
    Out[0].vPosition = vector(In[0].vPosition.xyz - vRight + vUp, 1.f);
    Out[0].vTexcoord = float2(0.f, 0.f);
    Out[0].vPosition = mul(Out[0].vPosition, matVP);
    Out[0].vLifeTime = In[0].vLifeTime;

    Out[1].vPosition = vector(In[0].vPosition.xyz + vRight + vUp, 1.f);
    Out[1].vTexcoord = float2(1.f, 0.f);
    Out[1].vPosition = mul(Out[1].vPosition, matVP);
    Out[1].vLifeTime = In[0].vLifeTime;

    Out[2].vPosition = vector(In[0].vPosition.xyz + vRight - vUp, 1.f);
    Out[2].vTexcoord = float2(1.f, 1.f);
    Out[2].vPosition = mul(Out[2].vPosition, matVP);
    Out[2].vLifeTime = In[0].vLifeTime;

    Out[3].vPosition = vector(In[0].vPosition.xyz - vRight - vUp, 1.f);
    Out[3].vTexcoord = float2(0.f, 1.f);
    Out[3].vPosition = mul(Out[3].vPosition, matVP);
    Out[3].vLifeTime = In[0].vLifeTime;
	
    VertexStream.Append(Out[0]);
    VertexStream.Append(Out[1]);
    VertexStream.Append(Out[2]);
    VertexStream.RestartStrip();

    VertexStream.Append(Out[0]);
    VertexStream.Append(Out[2]);
    VertexStream.Append(Out[3]);
    VertexStream.RestartStrip();
}


struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
    float2 vLifeTime : TEXCOORD1;
};

struct PS_OUT
{
    float4 vColor : SV_TARGET0;
};

PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out;

    // �߽� ��ǥ
    float2 center = float2(0.5f, 0.5f);

    // �߽����κ����� �Ÿ� ���
    float distance = length(In.vTexcoord - center);

    // �Ÿ� ������� �׶��̼� ���� ��� (0.0 ~ 1.0)
    float gradient = saturate(1.0f - distance * 1.5f); // �ּ��� 0, �������� 1

    // ���(���� �κ�)�� g_vColor(�����)�� ȥ��
    float4 white = float4(1.0f, 1.0f, 1.0f, 1.0f);
    Out.vColor = saturate(lerp(g_vColor, white, gradient));

    // ���� �� ���ǿ� ���� �ȼ� ������
    if (In.vLifeTime.y >= In.vLifeTime.x || Out.vColor.a < 0.1f)
        discard;

    return Out;
}


PS_OUT PS_QTE_PARTICLE(PS_IN In)
{
    PS_OUT Out;

    Out.vColor = g_vColor;

    if (In.vLifeTime.y >= In.vLifeTime.x || Out.vColor.a < 0.1f)
        discard;

    return Out;
}

PS_OUT PS_RUN_DUST(PS_IN In)
{
    PS_OUT Out;

    Out.vColor = g_Texture.Sample(LinearSampler, In.vTexcoord);
    //Out.vColor.a = Out.vColor.r;

    if (In.vLifeTime.y >= In.vLifeTime.x || Out.vColor.a < 0.1f)
        discard;
    
    Out.vColor.a *= min((In.vLifeTime.x - In.vLifeTime.y) + 0.5f, 1.f);
    //Out.vColor.rgb = Out.vColor.rgb * float3(0.529, 0.290, 0.078);

    return Out;
}

technique11 DefaultTechnique
{
	/* PASS�� ���� : ���̴� ����� ĸ��ȭ. */
//0
    pass Default
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = compile gs_5_0 GS_MAIN();
        HullShader = NULL;
        DomainShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }

//1
    pass QTE_PARTICLE
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = compile gs_5_0 GS_QTE_PARTICLE();
        HullShader = NULL;
        DomainShader = NULL;
        PixelShader = compile ps_5_0 PS_QTE_PARTICLE();
    }

//2 
    pass RUN_DUST
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = compile gs_5_0 GS_DUST();
        HullShader = NULL;
        DomainShader = NULL;
        PixelShader = compile ps_5_0 PS_RUN_DUST();
    }

//3
    pass Default_NotBillBoard
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = compile gs_5_0 GS_MAIN_NOTBillBoard();
        HullShader = NULL;
        DomainShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
}














