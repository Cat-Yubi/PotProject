#include "Renderer_Shader_Defines.hlsli"

float4x4 g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
texture2D g_Texture;
texture2D g_NextTexture;

vector g_vColor;

//QTE_UI
int g_IconState;
float g_Time;
float g_MaxTime;
float g_Ratio;

struct VS_IN
{
    float3 vPosition : POSITION;
    float2 vTexcoord : TEXCOORD0;
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;
    

    vector vPosition = mul(vector(In.vPosition, 1.f), g_WorldMatrix);
    vPosition = mul(vPosition, g_ViewMatrix);
    vPosition = mul(vPosition, g_ProjMatrix);

    Out.vPosition = vPosition;
    Out.vTexcoord = In.vTexcoord;

    
    return Out;
}

struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
};

struct PS_OUT
{
    float4 vColor : SV_TARGET0;
};

PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out;

    Out.vColor = g_Texture.Sample(LinearSampler, In.vTexcoord);

	//Out.vColor.gb = Out.vColor.r;
    if (Out.vColor.a <= 0.1f)
        discard;
    
    return Out;
}

PS_OUT PS_QTE_UI(PS_IN In)
{
    PS_OUT Out;

    // �ؽ�ó ���ø�
    Out.vColor = g_Texture.Sample(LinearSampler, In.vTexcoord);
    
    // CORRECTLY_PRESSED ������ �� ���� ȸ������ ��ȯ
    if (g_IconState == 3) // CORRECTLY_PRESSED
    {
        Out.vColor.rgb *= 0.5f; // ȸ�� ȿ�� (0.5�� ���� ����)
    }
    // WRONG_PRESSED ������ �� ���� ���������� ��ȯ
    else if (g_IconState == 4) // WRONG_PRESSED
    {
        // �������� �����ϱ� ���� ���� ä���� ������Ű��, ����� �Ķ� ä���� ���ҽ�Ŵ
        Out.vColor.rgb = Out.vColor.rgb * float3(1.0f, 0.2f, 0.2f);
    }

    // ���İ��� ���� �ȼ��� ����
    if (Out.vColor.a <= 0.1f)
        discard;
    
    return Out;
}


PS_OUT PS_QTE_UI_GAUGE(PS_IN In)
{
    PS_OUT Out;

    // UV ��ǥ ��������
    float2 uv = In.vTexcoord.xy;

    // ratio�� g_Time / g_MaxTime���� ����ϰ� Ŭ���� (0.0 ~ 1.0)
    float ratio = saturate(g_Time / g_MaxTime);

    // ����ũ ����: uv.x <= (1.0 - ratio)�� ��� 1, �׷��� ������ 0
    float mask = (uv.x <= (1.0f - ratio)) ? 1.0f : 0.0f;

    // Second_Texture ���ø� (ä���� �κ�)
    float4 secondColor = g_NextTexture.Sample(LinearSampler, uv);

    // First_Texture ���ø� (���)
    float4 firstColor = g_Texture.Sample(LinearSampler, uv);

    // ����ũ�� �̿��� �� �ؽ�ó�� �����Ͽ� �ռ�
    // mask�� 1�� ������ secondColor, 0�� ������ firstColor
    Out.vColor = mask * secondColor + (1.0f - mask) * firstColor;

    // ���İ��� ���� �ȼ��� ���� (�ʿ� ��)
    if (Out.vColor.a <= 0.1f)
        discard;

    return Out;
}

PS_OUT PS_QTE_Hit_UI(PS_IN In)
{
    PS_OUT Out;

    // ratio ��� (0���� 1����)
    float ratio = g_Time / g_MaxTime;

    // �߽� ��ǥ ���� (�ؽ�ó ��ǥ ���� 0~1)
    float2 center = float2(0.5f, 0.5f);

    // ���� �ʱ� �������� �ּ� ������ ����
    float originalRadius = 0.5f; // �ʱ� ���� ������ (�ؽ�ó ����)
    float minRadius = 0.166666f; // ratio�� 1�� ���� �ּ� ������, 0.5/3 �� ��� (���� �� 3�� ŭ ���� ��)

    // ratio�� ���� ���� �������� ���� (0: originalRadius, 1: minRadius)
    float currentRadius = lerp(originalRadius, minRadius, ratio);

    // ������ ���� ��� (�������� ���̱� ���� originalRadius / currentRadius)
    float scaleFactor = originalRadius / currentRadius;

    // �ؽ�ó ��ǥ�� �߽��� �������� �����ϸ�
    float2 scaledTexcoord = (In.vTexcoord - center) * scaleFactor + center;

    // �ؽ�ó ���ø�
    Out.vColor = g_Texture.Sample(DestroySampler, scaledTexcoord);
    
    // ���İ��� ���� �ȼ��� ����
    if (Out.vColor.a <= 0.1f)
        discard;
    
    //��Ȳ�� ���� ����
    Out.vColor = float4(255.f / 255.f, 127.f / 255.f, 39.f / 255.f, 1.f);

    return Out;
}


PS_OUT PS_QTE_CONTINUOUS_GAUGE(PS_IN In)
{
    PS_OUT Out;

    // UV ��ǥ ��������
    float2 uv = In.vTexcoord;

    // g_Ratio�� 0���� 1 ���̷� Ŭ����
    float ratio = saturate(g_Ratio);

    // ����ũ ����: uv.x <= ratio�� ��� 1, �׷��� ������ 0
    float mask = (uv.x <= ratio) ? 1.0f : 0.0f;

    // Second_Texture ���ø�
    float4 secondColor = g_NextTexture.Sample(LinearSampler, uv);

    // First_Texture ���ø�
    float4 firstColor = g_Texture.Sample(LinearSampler, uv);

    // ����ũ�� �̿��� �� �ؽ�ó�� �����Ͽ� �ռ�
    // mask�� 1�� ������ secondColor, 0�� ������ firstColor
    Out.vColor = mask * secondColor + (1.0f - mask) * firstColor;

    // ���İ��� ���� �ȼ��� ���� (�ʿ� ��)
    if (Out.vColor.a <= 0.1f)
        discard;

    return Out;
}

PS_OUT PS_QTE_Hit_Effect(PS_IN In)
{
    PS_OUT Out;

     // �ؽ�ó ���ø�
    float4 baseColor = g_Texture.Sample(DestroySampler, In.vTexcoord);

    if (baseColor.a < 0.1)
        discard;
    
    // �߽ɿ����� �Ÿ� ���
    float2 center = float2(0.5, 0.5);
    float distanceFromCenter = distance(In.vTexcoord, center);

    // �ð� ���� ���
    float ratio = saturate(g_Time / g_MaxTime);

    // �Ÿ� ��� ǥ�� ���� ����
    float visibilityRadius = ratio * 1.5; // ratio�� ���� ���� �ݰ��� Ȯ���

    // �ݰ� �ٱ����� ������ �ʵ��� discard
    if (distanceFromCenter > visibilityRadius)
    {
        discard;
    }

    // ���� ���̵� ��/�ƿ�
    float alpha;
    if (ratio < 0.1)
    {
        alpha = saturate(ratio / 0.1); // 0���� 0.1���� ���̵� ��
    }
    else if (ratio > 0.8)
    {
        alpha = saturate((1.0 - ratio) / 0.2); // 0.8���� 1.0���� ���̵� �ƿ�
    }
    else
    {
        alpha = 1.0; // �߰� ���������� ���� ������
    }

    Out.vColor = baseColor;
    // ���� ����� ���� ����
    Out.vColor.a = alpha;

    return Out;
}


technique11 DefaultTechnique
{
	/* PASS�� ���� : ���̴� ����� ĸ��ȭ. */

//0
    pass Default
    {

        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        HullShader = NULL;
        DomainShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }

//1
    pass QTE_UI
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
 
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        HullShader = NULL;
        DomainShader = NULL;
        PixelShader = compile ps_5_0 PS_QTE_UI();
    }

//2
    pass QTE_UI_GAUGE
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
 
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        HullShader = NULL;
        DomainShader = NULL;
        PixelShader = compile ps_5_0 PS_QTE_UI_GAUGE();
    }

//3
    pass QTE_Hit_UI
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
 
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        HullShader = NULL;
        DomainShader = NULL;
        PixelShader = compile ps_5_0 PS_QTE_Hit_UI();
    }

//4
    pass QTE_CONTINUOUS_GAUGE
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
 
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        HullShader = NULL;
        DomainShader = NULL;
        PixelShader = compile ps_5_0 PS_QTE_CONTINUOUS_GAUGE();
    }


//5
    pass QTE_HIT_EFFECT
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
 
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        HullShader = NULL;
        DomainShader = NULL;
        PixelShader = compile ps_5_0 PS_QTE_Hit_Effect();
    }
}
