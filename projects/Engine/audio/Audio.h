#pragma once
#include <wrl.h>
#include <xaudio2.h>
#include "Struct.h"

#pragma comment(lib,"xaudio2.lib")

class Audio
{
public:

	//初期化
	void Initialize();

	SoundData SoundLoadWave(const std::string& fileName);

	//音声再生
	void SoundPlawWave(const SoundData& soundData);

	//音声データ解放
	void SoundUnload(SoundData* soundData);

private:

	Microsoft::WRL::ComPtr<IXAudio2> xAudio2_;
	IXAudio2MasteringVoice* masterVoice_;

};

