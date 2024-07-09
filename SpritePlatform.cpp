#include "SpritePlatform.h"

void SpritePlatform::Initialize(DirectXCommon* dxCommon, PrimitiveDrawer* primitiveDrawer)
{

	//引数で受け取ってメンバ変数に記録する
	dxCommon_ = dxCommon;
	primitiveDrawer_ = primitiveDrawer;


}

void SpritePlatform::PreDraw()
{

	primitiveDrawer_->SetPipelineSet(dxCommon_->GetCommandList(), BlendMode::kBlendModeNone);
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

}
