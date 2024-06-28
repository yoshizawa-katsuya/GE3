#pragma once
#include "Model.h"

class Player
{
public:

	void Initialize(Model* model);

	void Update();

	void Draw(ID3D12GraphicsCommandList* commandList);


private:

	Model* model_ = nullptr;

};

