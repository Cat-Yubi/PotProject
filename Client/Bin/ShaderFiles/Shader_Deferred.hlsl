
#include "Renderer_Shader_Defines.hlsli"

float4x4		g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

float4x4		g_ViewMatrixInv, g_ProjMatrixInv;

float4x4		g_LightViewMatrix, g_LightProjMatrix;
int             g_iPlayerDirection;
float4			g_vCamPosition;

float4			g_vLightDir;
float4			g_vLightPos;
float			g_fLightRange;
float4			g_vLightDiffuse;
float4			g_vLightAmbient;
float4			g_vLightSpecular;
bool g_isChase;
float g_fAccBlackTime;
float g_fLightAccTime;
float g_fLightLifeTime;
bool g_isStartBlackOut;
bool g_isEndWhiteOut;
bool g_isUsingEffectLight;
texture2D		g_Texture;
texture2D		g_NormalTexture;
/* �ȼ����� �����ؾ��ϴ� ���� ������ �޶��ٶ�� �� �ȼ��� �׸��� ��ü���� �������� �� ���� Ÿ���� �߰��� �����Ͽ� �޾ƿԾ���Ѵ�. */
/* ���� ���� ��� ���ٶ�� Diffuse + Ambient + Normal */
/* �ȼ����� �ٸ��� ���� �������� ������ ó���Ѵٸ� ������ ���� ������ �ϳ� �����ص� �ǰڴ�. */
texture2D		g_DiffuseTexture; /* �����ؾ��ϴ� ��ǻ�� ������ �ȼ����� �ٸ��ٸ� �� �ȼ��� �׸��� ����޾ƿ;��Ѵ�. */
float4			g_vMtrlAmbient = float4(1.f, 1.f, 1.f, 1.f); /* �����ؾ��ϴ� �ں��Ʈ ������ �ȼ����� �ٸ��ٸ� �� �ȼ��� �׸��� ����޾ƿ;��Ѵ�. */
float4			g_vMtrlSpecular = float4(1.f, 1.f, 1.f, 1.f); /* �����ؾ��ϴ� �ں��Ʈ ������ �ȼ����� �ٸ��ٸ� �� �ȼ��� �׸��� ����޾ƿ;��Ѵ�. */
texture2D		g_ShadeTexture;
texture2D		g_DepthTexture;
texture2D		g_SpecularTexture;
texture2D		g_LightDepthTexture;
texture2D g_MetallicTexture;
float2 g_fSpriteSize;
float2 g_fSpriteCurPos;
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
	VS_OUT			Out;
	
	vector		vPosition = mul(vector(In.vPosition, 1.f), g_WorldMatrix);
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
	float4	vColor : SV_TARGET0;
};

PS_OUT PS_MAIN_DEBUG(PS_IN In)
{
	PS_OUT			Out = (PS_OUT)0;	

	Out.vColor = g_Texture.Sample(LinearSampler, In.vTexcoord);

	return Out;
}

struct PS_OUT_LIGHT
{
	float4	vShade : SV_TARGET0;
	float4	vSpecular : SV_TARGET1;
};

PS_OUT_LIGHT PS_MAIN_DIRECTIONAL_PLAYER(PS_IN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;

    vector vNormalDesc = g_NormalTexture.Sample(LinearSampler, In.vTexcoord);
    vector vDepthDesc = g_DepthTexture.Sample(LinearSampler, In.vTexcoord);
    float fViewZ = vDepthDesc.x * 1000.f;
    
    float4 vPlayerLightDir = g_vLightDir;
    vPlayerLightDir.x *= g_iPlayerDirection;
    
	/* 0 ~ 1 -> -1 ~ 1 */
    float4 vNormal = float4(vNormalDesc.xyz * 2.f - 1.f, 0.f);

	 // �ں��Ʈ ����
    float4 vAmbient = g_vLightAmbient * g_vMtrlAmbient;

    float shadeIntensity = max(dot(normalize(vPlayerLightDir) * -1.f, vNormal), 0.f);
    shadeIntensity = saturate(shadeIntensity);

    float shadeStep = 2.0f;
    shadeIntensity = (floor(shadeIntensity * shadeStep) / shadeStep);
    float4 vResultShade = ((g_vLightDiffuse * (vNormalDesc.w ? 1.f : shadeIntensity) * 1.f) + vAmbient);
    vResultShade = (g_isChase == true ? vResultShade * g_vLightDiffuse : vResultShade);

    //vNormalDesc.w �� ����ũ��
    //vResultShade = (vNormalDesc.w ? 1.f : vResultShade);
    //vResultShade = (vNormalDesc.w ? g_vLightDiffuse * 2 * vAmbient : vResultShade);
    vResultShade.a = vNormalDesc.w ? 1.f : 0;
    Out.vShade = vResultShade;
    
    float4 vWorldPos;

	/* ������ġ * ������� * ����� * ������� / View.z */
	/* ������������ ��ġ�� ���� ���Ѵ�. */
    vWorldPos.x = In.vTexcoord.x * 2.f - 1.f;
    vWorldPos.y = In.vTexcoord.y * -2.f + 1.f;
    vWorldPos.z = vDepthDesc.y;
    vWorldPos.w = 1.f;

	/* ������ġ * ������� * ����� * �������  */
    vWorldPos = vWorldPos * fViewZ;

	/* ������ġ * ������� * ����� */
    vWorldPos = mul(vWorldPos, g_ProjMatrixInv);

	/* ������ġ * ������� */
    vWorldPos = mul(vWorldPos, g_ViewMatrixInv);

    float4 vReflect = reflect(normalize(vPlayerLightDir), vNormal);
    float4 vLook = vWorldPos - g_vCamPosition;
	// ���� ���̱����� �и�
    float specularIntensity = max(dot(normalize(vReflect) * -1.f, normalize(vLook)), 0.f);
	
	//���̱�
    float specularStep = 2.0f;
    specularIntensity = floor(specularIntensity * specularStep) / specularStep;

	// �ּҰ�
    specularIntensity = max(specularIntensity, 0.1f);
	
    float specularThreshold = 0.8f; // 0.9 �̻��� ���� ����ŧ�� ����
	
    //if (specularIntensity >= specularThreshold)
    //{
    //    // ����ŧ�� ���� ����
    //    Out.vSpecular = (g_vLightSpecular * g_vMtrlSpecular) * pow(specularIntensity, 1.5f);

    //}
    //else
    //{
    //    // ����ŧ�� ������ (Ȥ�� �ſ� ���ϰ� ����)
       
    //}
    Out.vSpecular = float4(0.f, 0.f, 0.f, 1.0f); // ����ŧ���� ����
    return Out;
}

PS_OUT_LIGHT PS_MAIN_DIRECTIONAL(PS_IN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;
	
	vector			vNormalDesc = g_NormalTexture.Sample(LinearSampler, In.vTexcoord);
	vector			vDepthDesc = g_DepthTexture.Sample(LinearSampler, In.vTexcoord);
	float			fViewZ = vDepthDesc.x * 1000.f;

    float4 vNormal = float4(vNormalDesc.xyz * 2.f - 1.f, 0.f);

    float4 vAmbient = g_vLightAmbient * g_vMtrlAmbient;

	float4			vWorldPos;
	/* ������ġ * ������� * ����� * ������� / View.z */
	/* ������������ ��ġ�� ���� ���Ѵ�. */
    vWorldPos.x = In.vTexcoord.x * 2.f - 1.f;
    vWorldPos.y = In.vTexcoord.y * -2.f + 1.f;
    vWorldPos.z = vDepthDesc.y;
    vWorldPos.w = 1.f;

	/* ������ġ * ������� * ����� * �������  */
    vWorldPos = vWorldPos * fViewZ;

	/* ������ġ * ������� * ����� */
    vWorldPos = mul(vWorldPos, g_ProjMatrixInv);

	/* ������ġ * ������� */
    vWorldPos = mul(vWorldPos, g_ViewMatrixInv);

	float4			vReflect = reflect(normalize(g_vLightDir), vNormal);
	float4			vLook = vWorldPos - g_vCamPosition;
	
	Out.vShade = g_vLightDiffuse * max(dot(normalize(g_vLightDir) * -1.f, vNormal), 0.f) + vAmbient;
	Out.vSpecular = (g_vLightSpecular * g_vMtrlSpecular) * pow(max(dot(normalize(vReflect) * -1.f, normalize(vLook)), 0.f), 30.f);

    //float4 vReflect = reflect(normalize(g_vLightDir), vNormal);
    //float4 vLook = vWorldPos - g_vCamPosition;
   // ���� ���̱����� �и�
    float specularIntensity = max(dot(normalize(vReflect) * -1.f, normalize(vLook)), 0.f);
   
   //���̱�
    float specularStep = 2.0f;
    specularIntensity = floor(specularIntensity * specularStep) / specularStep;

   // �ּҰ�
    specularIntensity = max(specularIntensity, 0.1f);
   
    float specularThreshold = 0.8f; // 0.9 �̻��� ���� ����ŧ�� ����
   
    if (specularIntensity >= specularThreshold)
    {
        // ����ŧ�� ���� ����
        Out.vSpecular = (g_vLightSpecular * g_vMtrlSpecular) * pow(specularIntensity, 1.5f);

    }
    else
    {
        // ����ŧ�� ������ (Ȥ�� �ſ� ���ϰ� ����)
        Out.vSpecular = float4(0.f, 0.f, 0.f, 1.0f); // ����ŧ���� ����
    }
    return Out;
}

PS_OUT_LIGHT PS_MAIN_POINT_PLAYER(PS_IN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;

    vector vNormalDesc = g_NormalTexture.Sample(LinearSampler, In.vTexcoord);
    vector vDepthDesc = g_DepthTexture.Sample(LinearSampler, In.vTexcoord);
    float fViewZ = vDepthDesc.x * 1000.f;
    
    float4 vPlayerLightDir = g_vLightDir;
    vPlayerLightDir.x *= g_iPlayerDirection;
    
	/* 0 ~ 1 -> -1 ~ 1 */
    float4 vNormal = float4(vNormalDesc.xyz * 2.f - 1.f, 0.f);

	 // �ں��Ʈ ����
    float4 vAmbient = g_vLightAmbient * g_vMtrlAmbient;

    float shadeIntensity = max(dot(normalize(vPlayerLightDir) * -1.f, vNormal), 0.f);
    shadeIntensity = saturate(shadeIntensity);

    float shadeStep = 2.0f;
    shadeIntensity = floor(shadeIntensity * shadeStep) / shadeStep;

    float4 vResultShade = ((g_vLightDiffuse * shadeIntensity * 1.5f) + vAmbient);
    vResultShade = saturate(vNormalDesc.w ? g_vLightDiffuse * 0.8f : vResultShade);
    Out.vShade = vResultShade;
    //Out.vShade = (g_vLightDiffuse * shadeIntensity * 1.f) + vAmbient;
    

    float4 vWorldPos;
	/* ������ġ * ������� * ����� * ������� / View.z */
	/* ������������ ��ġ�� ���� ���Ѵ�. */
    vWorldPos.x = In.vTexcoord.x * 2.f - 1.f;
    vWorldPos.y = In.vTexcoord.y * -2.f + 1.f;
    vWorldPos.z = vDepthDesc.y;
    vWorldPos.w = 1.f;

	/* ������ġ * ������� * ����� * �������  */
    vWorldPos = vWorldPos * fViewZ;

	/* ������ġ * ������� * ����� */
    vWorldPos = mul(vWorldPos, g_ProjMatrixInv);

	/* ������ġ * ������� */
    vWorldPos = mul(vWorldPos, g_ViewMatrixInv);
    float4 vReflect = reflect(normalize(vPlayerLightDir), vNormal);
    float4 vLook = vWorldPos - g_vCamPosition;
    Out.vSpecular = (g_vLightSpecular * g_vMtrlSpecular) * pow(max(dot(normalize(vReflect) * -1.f, normalize(vLook)), 0.f), 30.f);
    
    return Out;
}

PS_OUT_LIGHT PS_MAIN_POINT(PS_IN In)
{
	PS_OUT_LIGHT		Out = (PS_OUT_LIGHT)0;

	vector			vNormalDesc = g_NormalTexture.Sample(LinearSampler, In.vTexcoord);
	vector			vDepthDesc = g_DepthTexture.Sample(LinearSampler, In.vTexcoord);
	float			fViewZ = vDepthDesc.x * 1000.f;

	float4			vWorldPos;

	/* ������ġ * ������� * ����� * ������� / View.z */
	/* ������������ ��ġ�� ���� ���Ѵ�. */
	vWorldPos.x = In.vTexcoord.x * 2.f - 1.f;
	vWorldPos.y = In.vTexcoord.y * -2.f + 1.f;
	vWorldPos.z = vDepthDesc.y;
	vWorldPos.w = 1.f;

	/* ������ġ * ������� * ����� * �������  */
	vWorldPos = vWorldPos * fViewZ;

	/* ������ġ * ������� * ����� */
	vWorldPos = mul(vWorldPos, g_ProjMatrixInv);

	/* ������ġ * ������� */
	vWorldPos = mul(vWorldPos, g_ViewMatrixInv);

	/* 0 ~ 1 -> -1 ~ 1 */
	float4			vNormal = float4(vNormalDesc.xyz * 2.f - 1.f, 0.f);

	float4			vAmbient = g_vLightAmbient * g_vMtrlAmbient;

	float4			vLightDir = vWorldPos - g_vLightPos;

	float			fAtt = max(g_fLightRange - length(vLightDir), 0.0f) / g_fLightRange;

	Out.vShade = (g_vLightDiffuse * max(dot(normalize(vLightDir) * -1.f, vNormal), 0.f) + vAmbient) * fAtt;

	float4			vReflect = reflect(normalize(vLightDir), vNormal);
	float4			vLook = vWorldPos - g_vCamPosition;

	Out.vSpecular = (g_vLightSpecular * g_vMtrlSpecular) * pow(max(dot(normalize(vReflect) * -1.f, normalize(vLook)), 0.f), 30.f) * fAtt;

	return Out;
}

bool isOutLine = false;
float CalculateNormalDiff(float2 vTexcoord, float4 vNormal)
{
    float2 fOffsetRight = float2(1.0f / 1920.f, 0.0f);
    float2 fOffsetDown = float2(0.0f, 1.0f / 1080.f);
    float2 fOffsetLeft = float2(-1.0f / 1920.f, 0.0f);
    float2 fOffsetUp = float2(0.0f, -1.0f / 1080.f);

    float3 vNormalRight = g_NormalTexture.Sample(LinearSampler, vTexcoord + fOffsetRight).xyz * 2.f - 1.f;
    float3 vNormalDown = g_NormalTexture.Sample(LinearSampler, vTexcoord + fOffsetDown).xyz * 2.f - 1.f;
    float3 vNormalLeft = g_NormalTexture.Sample(LinearSampler, vTexcoord + fOffsetLeft).xyz * 2.f - 1.f;
    float3 vNormalUp = g_NormalTexture.Sample(LinearSampler, vTexcoord + fOffsetUp).xyz * 2.f - 1.f;

    float fNormalDiff = length(vNormal.xyz - vNormalRight) + length(vNormal.xyz - vNormalDown)
                        + length(vNormal.xyz - vNormalLeft) + length(vNormal.xyz - vNormalUp);

    return fNormalDiff;
}

float CalculateDepthDiff(float2 vTexcoord, float fViewZ)
{

    float2 fOffsetRight = float2(1.0f / 1920.f, 0.0f);
    float2 fOffsetDown = float2(0.0f, 1.0f / 1080.f);
    float2 fOffsetLeft = float2(-1.0f / 1920.f, 0.0f);
    float2 fOffsetUp = float2(0.0f, -1.0f / 1080.f);

    float2 fOffsetRightUp = float2(1.0f / 1920.f, -1.0f / 1080.f);
    float2 fOffsetRightDown = float2(1.0f / 1920.f, 1.0f / 1080.f);
    float2 fOffsetLeftUp = float2(-1.0f / 1920.f, -1.0f / 1080.f);
    float2 fOffsetLeftDown = float2(-1.0f / 1920.f, 1.0f / 1080.f);


    float fDepthRight = g_DepthTexture.Sample(LinearSampler, vTexcoord + fOffsetRight).x * 1000.f;
    float fDepthDown = g_DepthTexture.Sample(LinearSampler, vTexcoord + fOffsetDown).x * 1000.f;
    float fDepthLeft = g_DepthTexture.Sample(LinearSampler, vTexcoord + fOffsetLeft).x * 1000.f;
    float fDepthUp = g_DepthTexture.Sample(LinearSampler, vTexcoord + fOffsetUp).x * 1000.f;


    float fDepthRightUp = g_DepthTexture.Sample(LinearSampler, vTexcoord + fOffsetRightUp).x * 1000.f;
    float fDepthRightDown = g_DepthTexture.Sample(LinearSampler, vTexcoord + fOffsetRightDown).x * 1000.f;
    float fDepthLeftUp = g_DepthTexture.Sample(LinearSampler, vTexcoord + fOffsetLeftUp).x * 1000.f;
    float fDepthLeftDown = g_DepthTexture.Sample(LinearSampler, vTexcoord + fOffsetLeftDown).x * 1000.f;

    float fDepthDiff = abs(fViewZ - fDepthRight) + abs(fViewZ - fDepthDown)
                     + abs(fViewZ - fDepthLeft) + abs(fViewZ - fDepthUp)
                     + abs(fViewZ - fDepthRightUp) + abs(fViewZ - fDepthRightDown)
                     + abs(fViewZ - fDepthLeftUp) + abs(fViewZ - fDepthLeftDown);

    return fDepthDiff;
}

// �ܰ��� ���� �Լ�
float CalculateEdge(float2 vTexcoord, float fViewZ, float4 vNormal, float fEdgeThreshold, float fEdgeNormalThreshold, float fEdgeDepthThreshold)
{

   // float fNormalDiff = CalculateNormalDiff(vTexcoord, vNormal);
   
    float fDepthDiff = CalculateDepthDiff(vTexcoord, fViewZ);
   
    float fEdge = step(fEdgeDepthThreshold, fDepthDiff); // * step(fEdgeNormalThreshold, fNormalDiff);

    return fEdge;
}

PS_OUT PS_MAIN_DEFERRED(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    vector vDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
    clip(vDiffuse.a - 0.98f);

    vector vShade = g_ShadeTexture.Sample(LinearSampler, In.vTexcoord);
    //clip(vShade.a - 0.98f);
    vector vSpecular = g_SpecularTexture.Sample(LinearSampler, In.vTexcoord);

    //Out.vColor = vDiffuse * (g_isUsingEffectLight ? 1.0 : vShade) + vSpecular;
    Out.vColor = vDiffuse * vShade + vSpecular;

    vector vNormalDesc = g_NormalTexture.Sample(LinearSampler, In.vTexcoord);
    vector vDepthDesc = g_DepthTexture.Sample(LinearSampler, In.vTexcoord);
    vector vMetallicDesc = g_MetallicTexture.Sample(LinearSampler, In.vTexcoord);

    
    float fViewZ = vDepthDesc.x * 1000.f;
    float4 vNormal = float4(vNormalDesc.xyz * 2.f - 1.f, 0.f);

    float fEdgeNormalThreshold = 0.2f;
    float fEdgeDepthThreshold = 0.1f;
	
    float fEdge = CalculateEdge(In.vTexcoord, fViewZ, vNormal, 0.f, fEdgeNormalThreshold, fEdgeDepthThreshold);

    vector vOutlineBlack = float4(0.f, 0.f, 0.f, 1.f);
    Out.vColor = lerp(Out.vColor, vOutlineBlack, fEdge);
   
    /* �������� */
    vector vMaskColor = { 0.f, 0.f, 0.f, 1.f };
    
    bool bLight = step(0.5f, vMetallicDesc.b * vMetallicDesc.a);
    bool bMid = step(0.4f, vMetallicDesc.r * vMetallicDesc.a) * (1 - bLight);
    bool bDark = step(0.9f, vMetallicDesc.a - vMetallicDesc.r) * step(0.39f, vDepthDesc.b);
       
    float fMaskFactor = step(0.39f, vDepthDesc.b) * ((0.6f * bLight) + (0.2f * bMid) + (-0.5f * bDark));
    
    /* �Ӹ� �޽����� ��Ʈ ���� ���� �ֵ� ����*/
    float fIn_RectOut = step(0.5f,
    /* �ϳ��� ��Ʈ ���� �ȿ��� �Ӹ��޽�              �Ӹ��޽��ε� * ���� ��Ʈ ���� �ȿ� �ȵ������ */
    (1 - step(0.5f, bLight + bMid + bDark)) + step(0.39f, vDepthDesc.b * (1 - step(0.5f, bLight + bMid + bDark))));    
    
    /* ��Ʈ ���� ���� �Ӹ��޽��� ���� �ܰ� �������� �ȹޱ� ������ ���� �ϴ� ���� */
    vMaskColor.rgb = (Out.vColor.rgb * 0.6f) * fIn_RectOut;
    
    /* �⺻���� ���� �� ��Ż �ؽ��Ŀ� ����� ���͸� �����Ѹ޽��� �Ÿ��� */
    Out.vColor.rgb = (Out.vColor.rgb + (Out.vColor.rgb * fMaskFactor)) + (vMaskColor.rgb * step(0.39f, vDepthDesc.b));

    return Out;
}

PS_OUT PS_MAIN_DEFERRED_PART(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    vector vDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
    clip(vDiffuse.a - 0.98f);

    vector vShade = g_ShadeTexture.Sample(LinearSampler, In.vTexcoord);
    //clip(vShade.a - 0.98f);
    vector vSpecular = g_SpecularTexture.Sample(LinearSampler, In.vTexcoord);

    //Out.vColor = vDiffuse * (g_isUsingEffectLight ? 1.0 : vShade) + vSpecular;
    Out.vColor = vDiffuse * vShade + vSpecular;

    vector vNormalDesc = g_NormalTexture.Sample(LinearSampler, In.vTexcoord);
    vector vDepthDesc = g_DepthTexture.Sample(LinearSampler, In.vTexcoord);
    vector vMetallicDesc = g_MetallicTexture.Sample(LinearSampler, In.vTexcoord);

    
    float fViewZ = vDepthDesc.x * 1000.f;
    float4 vNormal = float4(vNormalDesc.xyz * 2.f - 1.f, 0.f);

    float fEdgeNormalThreshold = 0.2f;
    float fEdgeDepthThreshold = 0.1f;
	
    float fEdge = CalculateEdge(In.vTexcoord, fViewZ, vNormal, 0.f, fEdgeNormalThreshold, fEdgeDepthThreshold);

    vector vOutlineBlack = float4(0.f, 0.f, 0.f, 1.f);
    Out.vColor = lerp(Out.vColor, vOutlineBlack, fEdge);
   
    
        /* ����Ÿ ���� */
    
    bool bLight = step(0.5, vMetallicDesc.b * vMetallicDesc.a);
    bool bMid = step(0.4, vMetallicDesc.r * vMetallicDesc.a) * (1 - bLight);
    bool bDark = step(0.9, vMetallicDesc.a - vMetallicDesc.r);
// �⺻            
    Out.vColor.rgb = (Out.vColor.rgb * (1.6f * bLight)) + (Out.vColor.rgb * (0.8f * bMid)) + (Out.vColor.rgb * (0.4f * bDark));
    return Out;
}

PS_OUT PS_MAIN_BLACKOUT(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    vector vDiffuse = g_Texture.Sample(LinearSampler, In.vTexcoord);
    
    vDiffuse.a = saturate(vDiffuse.a - (0.8f - g_fAccBlackTime));
    
    Out.vColor = vDiffuse;
    return Out;
}
PS_OUT PS_MAIN_WHITEOUT(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    float2 vTexcoord = In.vTexcoord;
    
    vector vRockDiffuse;
    float2 texFramePos = g_fSpriteCurPos * g_fSpriteSize;
    vTexcoord = vTexcoord * g_fSpriteSize + texFramePos;
    
    vRockDiffuse.rgb = float3(0.035294f, 0.0078431f, 0.00392156f);
    vRockDiffuse.a = g_DiffuseTexture.Sample(LinearSampler, vTexcoord).a;
    
    vector vDiffuse = g_Texture.Sample(LinearSampler, In.vTexcoord);
    
    vDiffuse.a = saturate(vDiffuse.a - (0.5f - g_fAccBlackTime));
    Out.vColor = vDiffuse;
    if (g_isEndWhiteOut == false)
    {
    
        vRockDiffuse.rgb = vRockDiffuse.rgb + (vDiffuse.rgb * saturate(g_fAccBlackTime - 0.5f));
    
        vDiffuse.rgb = vDiffuse.rgb * (1.f - vRockDiffuse.a) + vRockDiffuse.rgb * vRockDiffuse.a;
        vDiffuse.a = vDiffuse.a * (1.f - vRockDiffuse.a) + vRockDiffuse.a;
        Out.vColor = vDiffuse * g_isStartBlackOut + (vRockDiffuse * (1.f - g_isStartBlackOut));
    }

    return Out;
}

PS_OUT PS_MAIN_EFFECT_OUTLINE(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    vector vDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
    if (0.0f == vDiffuse.a)
        discard;

    Out.vColor = vDiffuse;

    vector vNormalDesc = g_NormalTexture.Sample(LinearSampler, In.vTexcoord);
    vector vDepthDesc = g_DepthTexture.Sample(LinearSampler, In.vTexcoord);
   
    float fViewZ = vDepthDesc.x * 1000.f;
    float4 vNormal = float4(vNormalDesc.xyz * 2.f - 1.f, 0.f);

    float fEdgeNormalThreshold = 0.2f;
    float fEdgeDepthThreshold = 0.7f;
	
    float fEdge = CalculateEdge(In.vTexcoord, fViewZ, vNormal, 0.f, fEdgeNormalThreshold, fEdgeDepthThreshold);

    vector vOutlineBlack = float4(0.f, 0.f, 0.f, 1.f);
    Out.vColor = lerp(Out.vColor, vOutlineBlack, fEdge);
   
    return Out;
}
PS_OUT PS_MAIN_DEFERRED_STAGE(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    vector vDepthDesc = g_DepthTexture.Sample(LinearSampler, In.vTexcoord);
    float fViewZ = vDepthDesc.x * 10000.f;

    float4 vWorldPos;
    
    vWorldPos.x = In.vTexcoord.x * 2.f - 1.f;
    vWorldPos.y = In.vTexcoord.y * -2.f + 1.f;
    vWorldPos.z = vDepthDesc.y;
    vWorldPos.w = 1.f;

	/* ������ġ * ������� * ����� * �������  */
    vWorldPos = vWorldPos * fViewZ;

	/* ������ġ * ������� * ����� */
    vWorldPos = mul(vWorldPos, g_ProjMatrixInv);

	/* ������ġ * ������� */
    vWorldPos = mul(vWorldPos, g_ViewMatrixInv);

    vWorldPos = mul(vWorldPos, g_LightViewMatrix);
    vWorldPos = mul(vWorldPos, g_LightProjMatrix);


	/* ������������ ��ǥ�� ��ȯ�ϳ�. */
    float2 vTexcoord = vWorldPos.xy / vWorldPos.w;

    vTexcoord.x = saturate(vTexcoord.x * 0.5f + 0.5f);
    vTexcoord.y = saturate(vTexcoord.y * -0.5f + 0.5f);

    vector vOldLightDepth = g_LightDepthTexture.Sample(LinearSampler, vTexcoord);
	
    float fDepth = vWorldPos.w;

    if (vOldLightDepth.x * 10000.f < fDepth - 0.3f)
        Out.vColor = vector(0.f, 0.f, 0.f, 0.4f);
    
    return Out;
}

PS_OUT PS_MAIN_DEFERRED_MAP(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    vector vDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
    
    clip(vDiffuse.a - 0.01f);

    vector vShade = g_ShadeTexture.Sample(LinearSampler, In.vTexcoord);
    vector vSpecular = g_SpecularTexture.Sample(LinearSampler, In.vTexcoord);

    Out.vColor = vDiffuse * vShade + vSpecular;
	
    vector vDepthDesc = g_DepthTexture.Sample(LinearSampler, In.vTexcoord);
    float fViewZ = vDepthDesc.x * 10000.f;

    float4 vWorldPos;
    
    vWorldPos.x = In.vTexcoord.x * 2.f - 1.f;
    vWorldPos.y = In.vTexcoord.y * -2.f + 1.f;
    vWorldPos.z = vDepthDesc.y;
    vWorldPos.w = 1.f;

	/* ������ġ * ������� * ����� * �������  */
    vWorldPos = vWorldPos * fViewZ;

	/* ������ġ * ������� * ����� */
    vWorldPos = mul(vWorldPos, g_ProjMatrixInv);

	/* ������ġ * ������� */
    vWorldPos = mul(vWorldPos, g_ViewMatrixInv);

    vWorldPos = mul(vWorldPos, g_LightViewMatrix);
    vWorldPos = mul(vWorldPos, g_LightProjMatrix);


	/* ������������ ��ǥ�� ��ȯ�ϳ�. */
    float2 vTexcoord = vWorldPos.xy / vWorldPos.w;

    vTexcoord.x = saturate(vTexcoord.x * 0.5f + 0.5f);
    vTexcoord.y = saturate(vTexcoord.y * -0.5f + 0.5f);

    vector vOldLightDepth = g_LightDepthTexture.Sample(LinearSampler, vTexcoord);
	
    float fDepth = vWorldPos.w;

    if (vOldLightDepth.x * 10000.f < fDepth - 0.3f)
        Out.vColor = Out.vColor * 0.5f;
    
    return Out;
}





PS_OUT PS_MAIN_ALLBLACKOUT(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    vector vDiffuse = g_Texture.Sample(LinearSampler, In.vTexcoord);
    
    vDiffuse.a = saturate(vDiffuse.a - (1.f - saturate(g_fAccBlackTime)));
    
    Out.vColor = vDiffuse;
    return Out;
}

technique11		DefaultTechnique
{	
	pass Debug
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_None, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		HullShader = NULL;
		DomainShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN_DEBUG();
	}	

	pass Light_Directional
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_None, 0);
		SetBlendState(BS_OneBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		HullShader = NULL;
		DomainShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN_DIRECTIONAL();
	}


	pass Light_Point
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_None, 0);
		SetBlendState(BS_OneBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		HullShader = NULL;
		DomainShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN_POINT();
	}

	pass Deferred // 3
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_None, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		HullShader = NULL;
		DomainShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN_DEFERRED();
	}

    pass Deferred_MapObj //4
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN(); 
        GeometryShader = NULL;
        HullShader = NULL;
        DomainShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_DEFERRED_MAP();
    }

    pass Player_Light_Directional //5
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        HullShader = NULL;
        DomainShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_DIRECTIONAL_PLAYER();
    }

    pass Player_Light_Point //6
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_MultiplyBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        HullShader = NULL;
        DomainShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_POINT_PLAYER();
    }

    pass Effect_OutLine //7
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
//BS_MultiplyBlend �̰� ��� �����϶� �Ұ�
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        HullShader = NULL;
        DomainShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_EFFECT_OUTLINE();
    }

    pass BlackOut // 8
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        HullShader = NULL;
        DomainShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_BLACKOUT();
    }

    pass Deferred_Part // 9
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        HullShader = NULL;
        DomainShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_DEFERRED_PART();
    }

    pass WhiteOut // 10
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        HullShader = NULL;
        DomainShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_WHITEOUT();
    }

    pass Player_Light_Directional_Chase //11
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_MultiplyBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        HullShader = NULL;
        DomainShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_DIRECTIONAL_PLAYER();
    }
    pass StageDefferd //12
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        HullShader = NULL;
        DomainShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_DEFERRED_STAGE();
    }

    pass AllBlackOut // 13
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        HullShader = NULL;
        DomainShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_ALLBLACKOUT();
    }
}








