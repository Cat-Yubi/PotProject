#pragma once

namespace Engine
{
	template<typename T>
	void Safe_Delete(T& pPointer)
	{
		if (nullptr != pPointer)
		{
			delete pPointer;
			pPointer = nullptr;
		}
	}


	template<typename T>
	void Safe_Delete_Array(T& pPointer)
	{
		if (nullptr != pPointer)
		{
			delete[] pPointer;
			pPointer = nullptr;
		}
	}

	template<typename T>
	unsigned int Safe_AddRef(T& pInstance)
	{
		unsigned int iRefCnt = 0;

		if (nullptr != pInstance)
			iRefCnt = pInstance->AddRef();

		return iRefCnt;
	}

	template<typename T>
	unsigned int Safe_Release(T& pInstance)
	{
		unsigned int iRefCnt = 0;

		if (nullptr != pInstance)
		{
			iRefCnt = pInstance->Release();
			if (0 == iRefCnt)
				pInstance = nullptr;
		}

		return iRefCnt;
	}

#pragma region ��ƿ �Լ�

	// 1. ������ ���̸� ���ϴ� �Լ�
	inline _float GetVectorLength(const _vector& v)
	{
		_vector length = XMVector3Length(v);
		return XMVectorGetX(length);
	}

	// 2. ����
	inline _float DotProduct(const _vector& v1, const _vector& v2)
	{
		XMVECTOR dot = XMVector3Dot(v1, v2);
		return XMVectorGetX(dot);
	}

	// 3. Matrix�� Float4x4
	inline _float4x4 MatrixToFloat4x4(const _matrix& mat)
	{
		_float4x4 floatMat;
		XMStoreFloat4x4(&floatMat, mat);
		return floatMat;
	}

	//4. Float4x4�� Matrix�� �ٲ���
	inline _matrix Float4x4ToMatrix(const _float4x4& floatMat)
	{
		return XMLoadFloat4x4(&floatMat);
	}

	// 5. _float4 ���ϱ�
	inline _float4 AddFloat4(const _float4& f1, const _float4& f2)
	{
		_float4 result;
		result.x = f1.x + f2.x;
		result.y = f1.y + f2.y;
		result.z = f1.z + f2.z;
		result.w = f1.w + f2.w;
		return result;
	}

	// 6. _float4 ����
	inline _float4 SubtractFloat4(const _float4& f1, const _float4& f2)
	{
		_float4 result;
		result.x = f1.x - f2.x;
		result.y = f1.y - f2.y;
		result.z = f1.z - f2.z;
		result.w = f1.w - f2.w;
		return result;
	}

	// 7. _float4 ���ϱ�
	inline _float4 MultiplyFloat4(const _float4& f1, const _float4& f2)
	{
		_float4 result;
		result.x = f1.x * f2.x;
		result.y = f1.y * f2.y;
		result.z = f1.z * f2.z;
		result.w = f1.w * f2.w;
		return result;
	}

	// 8. _float4 ������
	inline _float4 DivideFloat4(const _float4& f1, const _float4& f2)
	{
		_float4 result;
		result.x = f1.x / f2.x;
		result.y = f1.y / f2.y;
		result.z = f1.z / f2.z;
		result.w = f1.w / f2.w;
		return result;
	}

	//9. _float4x4�� _matrix�� ���ϴ� �Լ�
	inline _matrix Multiply_Float4x4_Matrix(const _float4x4& f4x4, const _matrix& mat)
	{
		// _float4x4�� XMMATRIX�� ��ȯ
		_matrix xmMat1 = XMLoadFloat4x4(&f4x4);

		// �� XMMATRIX�� ���ϰ� ��� ��ȯ
		_matrix result = XMMatrixMultiply(xmMat1, mat);

		return result;
	}

	//10. _float4x4�� _matrix�� ���ϴ� �Լ�
	inline _matrix Multiply_Matrix_Float4x4(const _matrix& mat, const _float4x4& f4x4)
	{
		// _float4x4�� XMMATRIX�� ��ȯ
		_matrix xmMat1 = XMLoadFloat4x4(&f4x4);

		// �� XMMATRIX�� ���ϰ� ��� ��ȯ
		_matrix result = XMMatrixMultiply(mat, xmMat1);

		return result;
	}

	//11. ȸ�� �� ����(Right, Up, Look)�κ��� ȸ�� ��� ����
	inline _matrix CreateRotationMatrix(const _float3& right, const _float3& up, const _float3& look) {
		// �� ���͸� DirectXMath�� XMVECTOR�� �ε�
		_vector vRight = XMVectorSet(right.x, right.y, right.z, 0.0f);
		_vector vUp = XMVectorSet(up.x, up.y, up.z, 0.0f);
		_vector vLook = XMVectorSet(look.x, look.y, look.z, 0.0f);

		// ȸ�� ��� ���� (�� �࿡ Right, Up, Look ���͸� �Ҵ�)
		_matrix rotationMatrix;
		rotationMatrix.r[0] = vRight; // Right ����
		rotationMatrix.r[1] = vUp;    // Up ����
		rotationMatrix.r[2] = vLook;  // Look ����
		rotationMatrix.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f); // ������ ���� (0, 0, 0, 1)

		return rotationMatrix;
	}

	//12. ���� ��Ʈ���� �����
	inline _matrix CreateWorldMatrix(const _float3& position, const _fvector& rotQuat)
	{
		_matrix worldMatrix;

		// 1. ���ʹϾ� ����ȭ
		_vector normalizedQuat = XMQuaternionNormalize(rotQuat);

		// 2. ȸ�� ��� ����
		_matrix rotationMatrix = XMMatrixRotationQuaternion(normalizedQuat);

		// 3. ���� ���� ����
		// Right ���� (ù ��° ��)
		_vector right = XMVectorSet(
			XMVectorGetX(rotationMatrix.r[0]),
			XMVectorGetY(rotationMatrix.r[0]),
			XMVectorGetZ(rotationMatrix.r[0]),
			0.0f // w = 0
		);

		// Up ���� (�� ��° ��)
		_vector up = XMVectorSet(
			XMVectorGetX(rotationMatrix.r[1]),
			XMVectorGetY(rotationMatrix.r[1]),
			XMVectorGetZ(rotationMatrix.r[1]),
			0.0f // w = 0
		);

		// Look ���� (�� ��° ��)
		_vector look = XMVectorSet(
			XMVectorGetX(rotationMatrix.r[2]),
			XMVectorGetY(rotationMatrix.r[2]),
			XMVectorGetZ(rotationMatrix.r[2]),
			0.0f // w = 0
		);

		// 4. ��ġ ���� ���� (�� ��° ��)
		_vector pos = XMVectorSet(
			position.x,
			position.y,
			position.z,
			1.0f // w = 1
		);

		// ���� ��Ʈ������ ��ֶ����� �ؼ� �Ҵ�
		worldMatrix.r[0] = XMVector4Normalize(right); // Right ����
		worldMatrix.r[1] = XMVector4Normalize(up);    // Up ����
		worldMatrix.r[2] = XMVector4Normalize(look);  // Look ����
		worldMatrix.r[3] = pos;   // Position ����

		return worldMatrix;
	}

	//13. ���� �Ÿ����ϱ�
	inline _float ComputeDistance(_vector pos1, _vector pos2)
	{
		// �� ������ ���̸� ���մϴ�.
		_vector diff = XMVectorSubtract(pos1, pos2);

		// ���� ������ ���̸� ����մϴ�.
		_vector distanceVec = XMVector3Length(diff);

		// XMVector���� float ���� �����մϴ�.
		float distance = XMVectorGetX(distanceVec);

		return distance;
	}

	//string�� wstring����
	inline string WStringToString(const _wstring& wstr)
	{
		if (wstr.empty())
			return std::string();

		// �ʿ��� ���� ũ�⸦ ����ϴ�.
		int size_needed = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), (int)wstr.length(), NULL, 0, NULL, NULL);
		std::string strTo(size_needed, 0);

		// ��ȯ�� �����մϴ�.
		WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), (int)wstr.length(), &strTo[0], size_needed, NULL, NULL);

		return strTo;
	}

	inline _wstring Trim(const _wstring& str)
	{
		const _wstring whitespace = L" \t\n\r";
		size_t start = str.find_first_not_of(whitespace);
		if (start == _wstring::npos)
			return L"";
		size_t end = str.find_last_not_of(whitespace);
		return str.substr(start, end - start + 1);
	}

	inline _float3 AddFloat3(const _float3& f1, const _float3& f2)
	{
		_float3 result;
		result.x = f1.x + f2.x;
		result.y = f1.y + f2.y;
		result.z = f1.z + f2.z;
		return result;
	}

	inline _float ZeroIfSmall(_float value) {
		return (fabs(value) < 1e-5) ? 0.0f : value;
	}

	inline _float Clamp(_float value, _float min, _float max)
	{
		if (value < min) return min;
		if (value > max) return max;
		return value;
	}

	inline _float Lerp(_float start, _float end, _float t)
	{
		return start + t * (end - start);
	}

	inline _float EaseInOut(_float t)
	{
		// ��¡ �Լ�: Ease-In-Out Quad
		if (t < 0.5f)
			return 2.0f * t * t;
		else
			return -1.0f + (4.0f - 2.0f * t) * t;
	}

	inline _float RandomBetween(float min, float max)
	{
		return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
	}
#pragma endregion
}