#include "Audio.hpp"

#include "NESResources.hpp"

#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include <queue>
#include <cmath>
#include <list>
#include <thread>

namespace NES
{
  SOUND::AudioSample::AudioSample()
  {
  }

  SOUND::AudioSample::AudioSample(std::string sWavFile, NES::ResourcePack *pack)
  {
    LoadFromFile(sWavFile, pack);
  }

  NES::rcode SOUND::AudioSample::LoadFromFile(std::string sWavFile, NES::ResourcePack *pack)
  {
    auto ReadWave = [&](std::istream &is)
    {
      char dump[4];
      is.read(dump, sizeof(char) * 4); // Read "RIFF"
      if (strncmp(dump, "RIFF", 4) != 0)
      {
        return NES::FAIL;
      }
      is.read(dump, sizeof(char) * 4); // Not Interested
      is.read(dump, sizeof(char) * 4); // Read "WAVE"
      if (strncmp(dump, "WAVE", 4) != 0)
      {
        return NES::FAIL;
      }

      // Read Wave description chunk
      is.read(dump, sizeof(char) * 4); // Read "fmt "
      unsigned int nHeaderSize = 0;
      is.read((char*)&nHeaderSize, sizeof(unsigned int)); // Not Interested
      is.read((char*)&wavHeader, nHeaderSize);// sizeof(WAVEFORMATEX)); // Read Wave Format Structure chunk
      // Note the -2, because the structure has 2 bytes to indicate its own size
      // which are not in the wav file

      // Just check if wave format is compatible with olcPGE
      if (wavHeader.wBitsPerSample != 16 || wavHeader.nSamplesPerSec != 44100)
      {
        return NES::FAIL;
      }

      // Search for audio data chunk
      uint32_t nChunksize = 0;
      is.read(dump, sizeof(char) * 4); // Read chunk header
      is.read((char*)&nChunksize, sizeof(uint32_t)); // Read chunk size
      while (strncmp(dump, "data", 4) != 0)
      {
        // Not audio data, so just skip it
        //std::fseek(f, nChunksize, SEEK_CUR);
        is.seekg(nChunksize, std::istream::cur);
        is.read(dump, sizeof(char) * 4);
        is.read((char*)&nChunksize, sizeof(uint32_t));
      }

      // Finally got to data, so read it all in and convert to float samples
      nSamples = nChunksize / (wavHeader.nChannels * (wavHeader.wBitsPerSample >> 3));
      nChannels = wavHeader.nChannels;
      
      // Create floating point buffer to hold audio sample
      fSample = new float[nSamples * nChannels];
      float *pSample = fSample;

      // Read in audio data and normalise
      for (long i = 0; i < nSamples; i++)
      {
        for (int c = 0; c < nChannels; c++)
        {
          short s = 0;
          if (!is.eof())
          {
            is.read((char*)&s, sizeof(short));
            
            *pSample = (float)s / (float)(SHRT_MAX);
            pSample++;
          }
        }
      }

      // All done, flag sound as valid
      bSampleValid = true;
      return NES::OK;
    };

    if (pack != nullptr)
    {
      NES::ResourceBuffer rb = pack->GetFileBuffer(sWavFile);
      std::istream is(&rb);
      return ReadWave(is);
    }
    else
    {
      // Read from file
      std::ifstream ifs(sWavFile, std::ifstream::binary);
      if (ifs.is_open())
      {
        return ReadWave(ifs);
      }
      else
      {
        return NES::FAIL;
      }
    }
  }

  // This vector holds all loaded sound samples in memory
  std::vector<SOUND::AudioSample> vecAudioSamples;

  // This structure represents a sound that is currently playing. It only
  // holds the sound ID and where this instance of it is up to for its
  // current playback
  void SOUND::SetUserSynthFunction(std::function<float(int, float, float)> func)
  {
    funcUserSynth = func;
  }

  void SOUND::SetUserFilterFunction(std::function<float(int, float, float)> func)
  {
    funcUserFilter = func;
  }

  // Load a 16-bit WAVE file @ 44100Hz ONLY into memory. A sample ID
  // number is returned if successful, otherwise -1
  int SOUND::LoadAudioSample(std::string sWavFile, NES::ResourcePack *pack)
  {
    SOUND::AudioSample a(sWavFile, pack);
    if (a.bSampleValid)
    {
      vecAudioSamples.push_back(a);
      return (unsigned int)vecAudioSamples.size();
    }
    else
    {
      return -1;
    }
  }

  // Add sample 'id' to the mixers sounds to play list
  void SOUND::PlaySample(int id, bool bLoop)
  {
    SOUND::sCurrentlyPlayingSample a;
    a.nAudioSampleID = id;
    a.nSamplePosition = 0;
    a.bFinished = false;
    a.bFlagForStop = false;
    a.bLoop = bLoop;
    SOUND::listActiveSamples.push_back(a);
  }
  
  void SOUND::StopSample(int id)
  {
    // Find first occurence of sample id
    auto s = std::find_if(listActiveSamples.begin(), listActiveSamples.end(),
                          [&](const SOUND::sCurrentlyPlayingSample &s) {
                            return s.nAudioSampleID == id;
                          });
    if (s != listActiveSamples.end())
    {
      s->bFlagForStop = true;
    }
  }
  
  void SOUND::StopAll()
  {
    for (auto &s : listActiveSamples)
    {
      s.bFlagForStop = true;
    }
  }
  
  float SOUND::GetMixerOutput(int nChannel, float fGlobalTime, float fTimeStep)
  {
    // Accumulate sample for this channel
    float fMixerSample = 0.0f;
    
    for (auto &s : listActiveSamples)
    {
      if (m_bAudioThreadActive)
      {
        if (s.bFlagForStop)
        {
          s.bLoop = false;
          s.bFinished = true;
        }
        else
        {
          // Calculate sample position
          s.nSamplePosition += roundf((float)vecAudioSamples[s.nAudioSampleID - 1].wavHeader.nSamplesPerSec * fTimeStep);
          
          // If sample position is valid add to the mix
          if (s.nSamplePosition < vecAudioSamples[s.nAudioSampleID - 1].nSamples)
          {
            fMixerSample += vecAudioSamples[s.nAudioSampleID - 1].fSample[(s.nSamplePosition * vecAudioSamples[s.nAudioSampleID - 1].nChannels) + nChannel];
          }
          else
          {
            if (s.bLoop)
            {
              s.nSamplePosition = 0;
            }
            else
            {
              s.bFinished = true; // Else sound has completed
            }
          }
        }
      }
      else
      {
        return 0.0f;
      }
    }
    
    // If sounds have completed then remove them
    listActiveSamples.remove_if([](const sCurrentlyPlayingSample &s) {return s.bFinished; });
    
    // The users application might be generating sound, so grab that if it exists
    if (funcUserSynth != nullptr)
      fMixerSample += funcUserSynth(nChannel, fGlobalTime, fTimeStep);
    
    // Return the sample via an optional user override to filter the sound
    if (funcUserFilter != nullptr)
      return funcUserFilter(nChannel, fGlobalTime, fMixerSample);
    else
      return fMixerSample;
  }

  std::thread SOUND::m_AudioThread;
  std::atomic<bool> SOUND::m_bAudioThreadActive{ false };
  std::atomic<float> SOUND::m_fGlobalTime{ 0.0f };
  std::list<SOUND::sCurrentlyPlayingSample> SOUND::listActiveSamples;
  std::function<float(int, float, float)> SOUND::funcUserSynth = nullptr;
  std::function<float(int, float, float)> SOUND::funcUserFilter = nullptr;
  
  bool SOUND::InitialiseAudio(unsigned int nSampleRate, unsigned int nChannels, unsigned int nBlocks, unsigned int nBlockSamples)
  {
    // Initialise Sound Engine
    m_bAudioThreadActive = false;
    m_nSampleRate = nSampleRate;
    m_nChannels = nChannels;
    m_nBlockCount = nBlocks;
    m_nBlockSamples = nBlockSamples;
    m_pBlockMemory = nullptr;

    // Open the device and create the context
    m_pDevice = alcOpenDevice(NULL);
    if (m_pDevice)
    {
      m_pContext = alcCreateContext(m_pDevice, NULL);
      alcMakeContextCurrent(m_pContext);
    }
    else
    {
      return DestroyAudio();
    }

    // Allocate memory for sound data
    alGetError();
    m_pBuffers = new ALuint[m_nBlockCount];
    alGenBuffers(m_nBlockCount, m_pBuffers);
    alGenSources(1, &m_nSource);

    for (unsigned int i = 0; i < m_nBlockCount; i++)
    {
      m_qAvailableBuffers.push(m_pBuffers[i]);
    }

    listActiveSamples.clear();

    // Allocate Wave|Block Memory
    m_pBlockMemory = new short[m_nBlockSamples];
    if (m_pBlockMemory == nullptr)
    {
      return DestroyAudio();
    }
    std::fill(m_pBlockMemory, m_pBlockMemory + m_nBlockSamples, 0);

    m_bAudioThreadActive = true;
    m_AudioThread = std::thread(&SOUND::AudioThread);
    return true;
  }

  // Stop and clean up audio system
  bool SOUND::DestroyAudio()
  {
    m_bAudioThreadActive = false;
    if(m_AudioThread.joinable())
    {
      m_AudioThread.join();
    }

    alDeleteBuffers(m_nBlockCount, m_pBuffers);
    delete[] m_pBuffers;
    alDeleteSources(1, &m_nSource);

    alcMakeContextCurrent(NULL);
    alcDestroyContext(m_pContext);
    alcCloseDevice(m_pDevice);
    return false;
  }


  // Audio thread. This loop responds to requests from the soundcard to fill 'blocks'
  // with audio data. If no requests are available it goes dormant until the sound
  // card is ready for more data. The block is fille by the "user" in some manner
  // and then issued to the soundcard.
  void SOUND::AudioThread()
  {
    m_fGlobalTime = 0.0f;
    static float fTimeStep = 1.0f / (float)m_nSampleRate;

    // Goofy hack to get maximum integer for a type at run-time
    short nMaxSample = (short)pow(2, (sizeof(short) * 8) - 1) - 1;
    float fMaxSample = (float)nMaxSample;
    short nPreviousSample = 0;

    std::vector<ALuint> vProcessed;

    while (m_bAudioThreadActive)
    {
      ALint nState, nProcessed;
      alGetSourcei(m_nSource, AL_SOURCE_STATE, &nState);
      alGetSourcei(m_nSource, AL_BUFFERS_PROCESSED, &nProcessed);

      // Add processed buffers to our queue
      vProcessed.resize(nProcessed);
      alSourceUnqueueBuffers(m_nSource, nProcessed, vProcessed.data());
      for (ALint nBuf : vProcessed)
      {
        m_qAvailableBuffers.push(nBuf);
      }

      // Wait until there is a free buffer (ewww)
      if (m_qAvailableBuffers.empty())
      {
        continue;
      }

      short nNewSample = 0;

      auto clip = [](float fSample, float fMax)
      {
        if (fSample >= 0.0)
        {
          return fmin(fSample, fMax);
        }
        else
        {
          return fmax(fSample, -fMax);
        }
      };

      for (unsigned int n = 0; n < m_nBlockSamples; n += m_nChannels)
      {
        // User Process
        for (unsigned int c = 0; c < m_nChannels; c++)
        {
          nNewSample = (short)(clip(GetMixerOutput(c, m_fGlobalTime, fTimeStep), 1.0) * fMaxSample);
          m_pBlockMemory[n + c] = nNewSample;
          nPreviousSample = nNewSample;
        }
        m_fGlobalTime = m_fGlobalTime + fTimeStep;
      }

      // Fill OpenAL data buffer
      alBufferData(
                   m_qAvailableBuffers.front(),
                   m_nChannels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16,
                   m_pBlockMemory,
                   2 * m_nBlockSamples,
                   m_nSampleRate
                   );

      // Add it to the OpenAL queue
      alSourceQueueBuffers(m_nSource, 1, &m_qAvailableBuffers.front());
      // Remove it from ours
      m_qAvailableBuffers.pop();

      // If it's not playing for some reason, change that
      if (nState != AL_PLAYING)
      {
        alSourcePlay(m_nSource);
      }
    }
  }

  std::queue<ALuint> SOUND::m_qAvailableBuffers;
  ALuint *SOUND::m_pBuffers = nullptr;
  ALuint SOUND::m_nSource = 0;
  ALCdevice *SOUND::m_pDevice = nullptr;
  ALCcontext *SOUND::m_pContext = nullptr;
  unsigned int SOUND::m_nSampleRate = 0;
  unsigned int SOUND::m_nChannels = 0;
  unsigned int SOUND::m_nBlockCount = 0;
  unsigned int SOUND::m_nBlockSamples = 0;
  short* SOUND::m_pBlockMemory = nullptr;
}
