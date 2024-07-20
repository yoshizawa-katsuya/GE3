#pragma once
#include "Struct.h"
#include "WinApp.h"

//カメラ
class Camera
{
public:

	Camera();

	//更新
	void Update();

	//getter
	const Matrix4x4& GetWorldMatrix() const { return worldMatrix_; }
	const Matrix4x4& GetViewMatrix() const { return viewMatrix_; }
	const Matrix4x4& GetProjection() const { return projectionMatrix_; }
	const Matrix4x4& GetViewProjection() const { return viewProjectionMatrix_; }

	const Vector3& GetRotate() const { return transform_.rotate; }
	Vector3& GetRotate() { return transform_.rotate; }

	const Vector3& GetTranslate() const { return transform_.translate; }
	Vector3& GetTranslate() { return transform_.translate; }

	//setter
	void SetRotate(const Vector3& rotate) { transform_.rotate = rotate; }
	void SetTransform(const Vector3& translate) { transform_.translate = translate; }
	void SetFovY(float fovY) { fovY_ = fovY; }
	void SetAspectRatio(float aspectRatio) { aspectRatio_ = aspectRatio; }
	void SetNearClip(float nearClip) { nearClip_ = nearClip; }
	void SetFarClip(float farClip) { farClip_ = farClip; }

private:

	//カメラの変数
	Transforms transform_;
	Matrix4x4 worldMatrix_;
	Matrix4x4 viewMatrix_;
	Matrix4x4 projectionMatrix_;
	float fovY_;	//水平方向視野角
	float aspectRatio_;	//アスペクト比
	float nearClip_;	//ニアクリップ距離
	float farClip_;	//ファークリップ距離
	Matrix4x4 viewProjectionMatrix_;

};

