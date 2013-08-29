/*
 * Delta3D Open Source Game and Simulation Engine
 * Copyright (C) 2005, BMH Associates, Inc.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * William E. Johnson II
 */

#include <prefix/SimCorePrefix.h>
#include <SimCore/Actors/SoundActorProxy.h>
#include <dtDAL/enginepropertytypes.h>
#include <dtDAL/actorproxyicon.h>
#include <dtDAL/project.h>

#include <dtAudio/audiomanager.h>
#include <dtGame/gamemanager.h>
#include <dtGame/invokable.h>
#include <dtGame/basemessages.h>
#include <dtGame/messagetype.h>


#include <dtUtil/mathdefines.h>

namespace SimCore
{
   namespace Actors
   {
      ///////////////////////////////////////////////////////////////////////////////
      // CLASS CONSTANTS
      ///////////////////////////////////////////////////////////////////////////////
      const dtUtil::RefString SoundActorProxy::CLASS_NAME("dtAudio::Sound");
      const dtUtil::RefString SoundActorProxy::INVOKABLE_TIMER_HANDLER("HandleActorTimers");
      const dtUtil::RefString SoundActorProxy::PROPERTY_DIRECTION("Direction"); // "Direction"
      const dtUtil::RefString SoundActorProxy::PROPERTY_GAIN("Gain"); // "Gain"
      const dtUtil::RefString SoundActorProxy::PROPERTY_INITIAL_OFFSET_TIME("Initial offset Time"); // "Initial offset Time"
      const dtUtil::RefString SoundActorProxy::PROPERTY_LISTENER_RELATIVE("Listener Relative"); // "Listener Relative"
      const dtUtil::RefString SoundActorProxy::PROPERTY_LOOPING("Looping"); // "Looping"
      const dtUtil::RefString SoundActorProxy::PROPERTY_MAX_DISTANCE("Max Distance"); // "Max Distance"
      const dtUtil::RefString SoundActorProxy::PROPERTY_MAX_GAIN("Max Gain"); // "Max Gain"
      const dtUtil::RefString SoundActorProxy::PROPERTY_MIN_GAIN("Min Gain"); // "Min Gain"
      const dtUtil::RefString SoundActorProxy::PROPERTY_MAX_RANDOM_TIME("Max Random Time"); // "Max Random Time"
      const dtUtil::RefString SoundActorProxy::PROPERTY_MIN_RANDOM_TIME("Min Random Time"); // "Min Random Time"
      const dtUtil::RefString SoundActorProxy::PROPERTY_PITCH("Pitch"); // "Pitch"
      const dtUtil::RefString SoundActorProxy::PROPERTY_PLAY_AS_RANDOM("Play As Random SFX"); // "Play As Random SFX"
      const dtUtil::RefString SoundActorProxy::PROPERTY_PLAY_AT_STARTUP("Play Sound at Startup"); // "Play Sound at Startup"
      const dtUtil::RefString SoundActorProxy::PROPERTY_ROLLOFF_FACTOR("Rolloff Factor"); // "Rolloff Factor"
      const dtUtil::RefString SoundActorProxy::PROPERTY_SOUND_EFFECT("The Sound Effect"); // "The Sound Effect"
      const dtUtil::RefString SoundActorProxy::PROPERTY_VELOCITY("Velocity"); // "Velocity"
      const dtUtil::RefString SoundActorProxy::TIMER_NAME("PlaySoundTimer");
      const dtUtil::RefString SoundActorProxy::PLAY_END_TIMER_NAME("PlaySoundTimerEnd");

      const float SoundActorProxy::DEFAULT_RANDOM_TIME_MAX = 30.0f;
      const float SoundActorProxy::DEFAULT_RANDOM_TIME_MIN = 5.0f;



      //////////////////////////////////////////////////////////////////////////////
      // ACTOR CODE
      //////////////////////////////////////////////////////////////////////////////
      SoundActor::SoundActor(dtGame::GameActorProxy& proxy)
      : dtGame::GameActor(proxy)
      {
         //make sure AudioManager has been initialized
         if(!dtAudio::AudioManager::GetInstance().IsInitialized())
         {
            dtAudio::AudioManager::Instantiate();
         }
      }

      ///////////////////////////////////////////////////////////////////////////////
      SoundActor::~SoundActor()
      {
         DestroySound();
      }

      ///////////////////////////////////////////////////////////////////////////////
      void SoundActor::CreateSound()
      {
         if (!mSound.valid())
         {
            mSound = dtAudio::AudioManager::GetInstance().NewSound();
            AddChild(mSound.get());
         }
      }

      ///////////////////////////////////////////////////////////////////////////////
      void SoundActor::DestroySound()
      {
         if (mSound.valid())
         {
            // This is a big, ugly hack.  There is a bug in delta 2.4
            // where if you clear the sound, it won't unload the file when you call
            // free sound.  We actually don't want to unload the sound because it's not good to reload
            // the sound from disk every time. This, however, will make the sound NEVER get unloaded.
            mSound->Clear();
            mSound->Emancipate();
            dtAudio::AudioManager::GetInstance().FreeSound(mSound.get());
            mSound = NULL;
         }
      }

      ///////////////////////////////////////////////////////////////////////////////
      dtAudio::Sound* SoundActor::GetSound()
      {
         return mSound.get();
      }

      ///////////////////////////////////////////////////////////////////////////////
      const dtAudio::Sound* SoundActor::GetSound() const
      {
         return mSound.get();
      }



      ///////////////////////////////////////////////////////////////////////////////
      // PROXY CODE
      ///////////////////////////////////////////////////////////////////////////////
      SoundActorProxy::SoundActorProxy()
      : mRandomSoundEffect(false)
      , mMinRandomTime(SoundActorProxy::DEFAULT_RANDOM_TIME_MIN)
      , mMaxRandomTime(SoundActorProxy::DEFAULT_RANDOM_TIME_MAX)
      , mOffsetTime(0.0f)
      , mPlaySoundAtStartup(true)
      , mGain(0.0)
      , mPitch(0.0)
      , mMaxDistance(0.0)
      , mRolloffFactor(0.0)
      , mMinGain(0.0)
      , mMaxGain(0.0)
      , mLooping(true)
      , mListenerRelative(false)
      {
         /*
          * @note You must instantiate, configure, and shutdown the
          * audiomanager in your application
          * ex.
          * \code
          * dtAudio::AudioManager::Instantiate();
          * dtAudio::AudioManager::GetManager()->Config(AudioConfigData&)
          * \endcode
          */
         SetClassName(SoundActorProxy::CLASS_NAME.Get());
      }

      ///////////////////////////////////////////////////////////////////////////////
      SoundActorProxy::~SoundActorProxy()
      {
         dtAudio::Sound* snd = static_cast<SoundActor&>(GetGameActor()).GetSound();

         if (snd != NULL)
         {
            if (snd->GetFilename())
            {
               snd->UnloadFile();
            }
         }
      }

      ///////////////////////////////////////////////////////////////////////////////
      void SoundActorProxy::CreateDrawable()
      {
         SoundActor* actor = new SoundActor(*this);
         SetDrawable(*actor);

         // Create a sound to read the defaults.
         actor->CreateSound();
         dtAudio::Sound* sound = actor->GetSound();

         //Read the defaults.
         mLooping = sound->IsLooping();
         mListenerRelative = sound->IsListenerRelative();
         mRolloffFactor = sound->GetRolloffFactor();
         mGain = sound->GetGain();
         mPitch = sound->GetPitch();
         mMaxGain = sound->GetMaxGain();
         mMinGain = sound->GetMinGain();
         mMaxDistance = sound->GetMaxDistance();
         osg::Vec3 tmp;
         sound->GetDirection(tmp);
         mDirection = tmp;
         sound->GetVelocity(tmp);
         mVelocity = tmp;

         // destroy it when we're done so it doesn't hold open a source.
         actor->DestroySound();
      }

      ///////////////////////////////////////////////////////////////////////
      void SoundActorProxy::OnEnteredWorld()
      {
         if (mPlaySoundAtStartup)
         {
            PlayQueued(mOffsetTime);
         }
      }

      ///////////////////////////////////////////////////////////////////////
      void SoundActorProxy::OnRemovedFromWorld()
      {
         Stop();
         // This code is not necessary, but I want to leave it here because it could
         // become necessary if/when stop no longer deletes the dtAudio sound.
         SoundActor* soundActor;
         GetActor(soundActor);
         soundActor->DestroySound();

         // I don't think this has to be done.
         GetGameManager()->ClearTimer(SoundActorProxy::TIMER_NAME.Get(), this);
         GetGameManager()->ClearTimer(SoundActorProxy::PLAY_END_TIMER_NAME, this);
      }

      ///////////////////////////////////////////////////////////////////////////////
      void SoundActorProxy::BuildInvokables()
      {
         dtGame::GameActorProxy::BuildInvokables();

         AddInvokable(*new dtGame::Invokable(SoundActorProxy::INVOKABLE_TIMER_HANDLER.Get(),
                  dtUtil::MakeFunctor(&SoundActorProxy::HandleActorTimers, this)));

         RegisterForMessagesAboutSelf(dtGame::MessageType::INFO_TIMER_ELAPSED,
                  SoundActorProxy::INVOKABLE_TIMER_HANDLER.Get());
      }

      ///////////////////////////////////////////////////////////////////////////////
      void SoundActorProxy::HandleActorTimers(const dtGame::TimerElapsedMessage& msg)
      {
         if (msg.GetTimerName() == SoundActorProxy::TIMER_NAME)
         {
            Play();

            if (mRandomSoundEffect)
            {
               PlayQueued(0.0f);
            }
         }
         else if (msg.GetTimerName() == SoundActorProxy::PLAY_END_TIMER_NAME)
         {
            Stop();
         }
      }

      ///////////////////////////////////////////////////////////////////////////////
      void SoundActorProxy::BuildPropertyMap()
      {
         static const dtUtil::RefString GROUPNAME = "Sound";
         GameActorProxy::BuildPropertyMap();

         // This property toggles whether or not a sound loops. A
         // value of true will loop the sound, while a value of false
         // will not loop/stop looping a sound.
         // Default is false
         AddProperty(new dtDAL::BooleanActorProperty(
                  PROPERTY_LOOPING,
                  PROPERTY_LOOPING,
                  dtDAL::BooleanActorProperty::SetFuncType(this, &SoundActorProxy::SetLooping),
                  dtDAL::BooleanActorProperty::GetFuncType(this, &SoundActorProxy::IsLooping),
                  "Toggles if a sound loops continuously.", GROUPNAME));

         // This property manipulates the gain of a sound. It uses
         // a float type to represent the gain value.
         // Clamped between 0 - 1 by default.
         // Default is 1.0f
         AddProperty(new dtDAL::FloatActorProperty(
                  PROPERTY_GAIN,
                  PROPERTY_GAIN,
                  dtDAL::FloatActorProperty::SetFuncType(this, &SoundActorProxy::SetGain),
                  dtDAL::FloatActorProperty::GetFuncType(this, &SoundActorProxy::GetGain),
                  "Sets the gain of a sound.", GROUPNAME));

         // This property manipulates the pitch of a sound. It uses
         // a float type to represent the pitch value.
         // Default is 1.0f
         AddProperty(new dtDAL::FloatActorProperty(
                  PROPERTY_PITCH,
                  PROPERTY_PITCH,
                  dtDAL::FloatActorProperty::SetFuncType(this, &SoundActorProxy::SetPitch),
                  dtDAL::FloatActorProperty::GetFuncType(this, &SoundActorProxy::GetPitch),
                  "Sets the pitch of a sound.", GROUPNAME));

         // This property toggles whether or not a sound is listerner
         // relative.
         // Default is false
         AddProperty(new dtDAL::BooleanActorProperty(
                  PROPERTY_LISTENER_RELATIVE,
                  PROPERTY_LISTENER_RELATIVE,
                  dtDAL::BooleanActorProperty::SetFuncType(this, &SoundActorProxy::SetListenerRelative),
                  dtDAL::BooleanActorProperty::GetFuncType(this, &SoundActorProxy::IsListenerRelative),
                  "Toggles if a sound is relative to the listener.", GROUPNAME));

         // This property manipulates the maximum distance of a sound. It uses
         // a float type to represent the maximum distance.
         // Default is 3.402823466e+38F (OxFFFFFFFF).
         AddProperty(new dtDAL::FloatActorProperty(
                  PROPERTY_MAX_DISTANCE,
                  PROPERTY_MAX_DISTANCE,
                  dtDAL::FloatActorProperty::SetFuncType(this, &SoundActorProxy::SetMaxDistance),
                  dtDAL::FloatActorProperty::GetFuncType(this, &SoundActorProxy::GetMaxDistance),
                  "Sets the maximum distance of a sound.", GROUPNAME));

         // This property manipulates the roll off factor of a sound. It uses
         // a float type to represent the roll off factor.
         // Default is 1.0f
         AddProperty(new dtDAL::FloatActorProperty(
                  PROPERTY_ROLLOFF_FACTOR,
                  PROPERTY_ROLLOFF_FACTOR,
                  dtDAL::FloatActorProperty::SetFuncType(this, &SoundActorProxy::SetRolloffFactor),
                  dtDAL::FloatActorProperty::GetFuncType(this, &SoundActorProxy::GetRolloffFactor),
                  "Sets the rolloff factor of a sound.", GROUPNAME));

         // This property manipulates the minimum gain of a sound. It uses
         // a float type to represent the minimum gain.
         // Default is 0.0f
         AddProperty(new dtDAL::FloatActorProperty(
                  PROPERTY_MIN_GAIN,
                  PROPERTY_MIN_GAIN,
                  dtDAL::FloatActorProperty::SetFuncType(this, &SoundActorProxy::SetMinGain),
                  dtDAL::FloatActorProperty::GetFuncType(this, &SoundActorProxy::GetMinGain),
                  "Sets the minimum gain of a sound.", GROUPNAME));

         // This property manipulates the maximum gain of a sound. It uses
         // a float type to represent the maximum gain.
         // Default is 1.0f
         AddProperty(new dtDAL::FloatActorProperty(
                  PROPERTY_MAX_GAIN,
                  PROPERTY_MAX_GAIN,
                  dtDAL::FloatActorProperty::SetFuncType(this, &SoundActorProxy::SetMaxGain),
                  dtDAL::FloatActorProperty::GetFuncType(this, &SoundActorProxy::GetMaxGain),
                  "Sets the maximum gain of a sound.", GROUPNAME));

         // This property manipulates the direction of a sound. It uses
         // 3 values to represent the sound's direction.
         // Default is 0, 1, 0
         AddProperty(new dtDAL::Vec3ActorProperty(
                  PROPERTY_DIRECTION,
                  PROPERTY_DIRECTION,
                  dtDAL::Vec3ActorProperty::SetFuncType(this, &SoundActorProxy::SetDirection),
                  dtDAL::Vec3ActorProperty::GetFuncType(this, &SoundActorProxy::GetDirection),
                  "Sets the direction of a sound.", GROUPNAME));

         // This property manipulates the velocity of a sound. It uses
         // 3 values to represent the velocity.
         // Default is 0, 0, 0
         AddProperty(new dtDAL::Vec3ActorProperty(
                  PROPERTY_VELOCITY,
                  PROPERTY_VELOCITY,
                  dtDAL::Vec3ActorProperty::SetFuncType(this, &SoundActorProxy::SetVelocity),
                  dtDAL::Vec3ActorProperty::GetFuncType(this, &SoundActorProxy::GetVelocity),
                  "Sets the velocity of a sound.", GROUPNAME));

         // new properties
         AddProperty(new dtDAL::FloatActorProperty(
                  PROPERTY_MAX_RANDOM_TIME,
                  PROPERTY_MAX_RANDOM_TIME,
                  dtDAL::FloatActorProperty::SetFuncType(this, &SoundActorProxy::SetMaxRandomTime),
                  dtDAL::FloatActorProperty::GetFuncType(this, &SoundActorProxy::GetMaxRandomTime),
                  "Maximum seconds to wait between random executions of the sound", GROUPNAME));

         AddProperty(new dtDAL::FloatActorProperty(
                  PROPERTY_MIN_RANDOM_TIME,
                  PROPERTY_MIN_RANDOM_TIME,
                  dtDAL::FloatActorProperty::SetFuncType(this, &SoundActorProxy::SetMinRandomTime),
                  dtDAL::FloatActorProperty::GetFuncType(this, &SoundActorProxy::GetMinRandomTime),
                  "Minimum seconds to wait between random executions of the sound", GROUPNAME));

         AddProperty(new dtDAL::FloatActorProperty(
                  PROPERTY_INITIAL_OFFSET_TIME,
                  PROPERTY_INITIAL_OFFSET_TIME,
                  dtDAL::FloatActorProperty::SetFuncType(this, &SoundActorProxy::SetOffsetTime),
                  dtDAL::FloatActorProperty::GetFuncType(this, &SoundActorProxy::GetOffsetTime),
                  "Time in seconds to wait before the sound is played when it enters the Game Manager", GROUPNAME));

         AddProperty(new dtDAL::BooleanActorProperty(
                  PROPERTY_PLAY_AS_RANDOM,
                  PROPERTY_PLAY_AS_RANDOM,
                  dtDAL::BooleanActorProperty::SetFuncType(this, &SoundActorProxy::SetToHaveRandomSoundEffect),
                  dtDAL::BooleanActorProperty::GetFuncType(this, &SoundActorProxy::IsARandomSoundEffect),
                  "Will have a timer go off and play sound so often", GROUPNAME));

         AddProperty(new dtDAL::BooleanActorProperty(
                  PROPERTY_PLAY_AT_STARTUP,
                  PROPERTY_PLAY_AT_STARTUP,
                  dtDAL::BooleanActorProperty::SetFuncType(this, &SoundActorProxy::SetPlayAtStartup),
                  dtDAL::BooleanActorProperty::GetFuncType(this, &SoundActorProxy::IsPlayedAtStartup),
                  "Will play sound at startup", GROUPNAME));

         AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::SOUND,
                  PROPERTY_SOUND_EFFECT,
                  PROPERTY_SOUND_EFFECT,
                  dtDAL::ResourceActorProperty::SetFuncType(this, &SoundActorProxy::SetSoundResource),
                  "Loads the sound for this to use"));
      }

      ///////////////////////////////////////////////////////////////////////////////
      void SoundActorProxy::SetSoundResource(const std::string& fileName)
      {
         mSoundResourceFile = fileName;
      }
//      void SoundActorProxy::SetSoundResource(const dtDAL::ResourceDescriptor* soundResource)
//      {
//         if (soundResource == NULL)
//         {
//            mSoundResource = dtDAL::ResourceDescriptor("");
//         }
//         else
//         {
//            mSoundResource = *soundResource;
//         }
//      }

//      ///////////////////////////////////////////////////////////////////////////////
//      dtDAL::ResourceDescriptor* SoundActorProxy::GetSoundResource()
//      {
//         if (mSoundResource.GetResourceIdentifier().empty())
//         {
//            return NULL;
//         }
//
//         return &mSoundResource;
//      }

      ///////////////////////////////////////////////////////////////////////////////
      void SoundActorProxy::SetDirection(const osg::Vec3& dir)
      {
         mDirection = dir;
      }

      ///////////////////////////////////////////////////////////////////////////////
      osg::Vec3 SoundActorProxy::GetDirection()
      {
         return mDirection;
      }

      ///////////////////////////////////////////////////////////////////////////////
      void SoundActorProxy::SetVelocity(const osg::Vec3& vel)
      {
         mVelocity = vel;
      }

      ///////////////////////////////////////////////////////////////////////////////
      osg::Vec3 SoundActorProxy::GetVelocity()
      {
         return mVelocity;
      }

      ///////////////////////////////////////////////////////////////////////////////
      void SoundActorProxy::Play()
      {
//         if (mSoundResource.GetResourceIdentifier().empty())
//         {
//            throw dtUtil::Exception("Unable to play SimCore::SoundActor \"" + GetName() + "\" because no sound resource has been assigned.",
//                  __FILE__, __LINE__);
//         }
//
//         std::string file = dtDAL::Project::GetInstance().GetResourcePath(mSoundResource);

         std::string file = mSoundResourceFile;
         if (mSoundResourceFile.empty())
         {
            throw dtUtil::Exception("Unable to play SimCore::SoundActor \"" + GetName() + "\" because no sound resource has been assigned.",
                     __FILE__, __LINE__);
         }

         SoundActor* soundActor;
         GetActor(soundActor);
         dtAudio::Sound* sound = soundActor->GetSound();
         if (sound == NULL)
         {
            soundActor->CreateSound();
            sound = soundActor->GetSound();
         }

         sound->LoadFile(file.c_str());

         sound->SetDirection(GetDirection());
         sound->SetVelocity(GetVelocity());
         sound->SetGain(GetGain());
         sound->SetMaxDistance(GetMaxDistance());
         sound->SetMinGain(GetMinGain());
         sound->SetMaxGain(GetMaxGain());
         sound->SetListenerRelative(IsListenerRelative());
         sound->SetLooping(IsLooping());
         sound->SetPitch(GetPitch());
         sound->SetRolloffFactor(GetRolloffFactor());

         ALint buf = sound->GetBuffer();
         ALint freq = 0, size = 0, bits = 0, channels = 0;
         alGetBufferi(buf, AL_FREQUENCY, &freq);
         alGetBufferi(buf, AL_SIZE, &size);
         alGetBufferi(buf, AL_BITS, &bits);
         alGetBufferi(buf, AL_CHANNELS, &channels);

         int bytesPerSec = freq * (bits/8) * channels;

         float seconds = float(size) / float(bytesPerSec);
         // Add a half a second just to be safe.
         seconds += 0.5f;

         sound->Play();

         if (!sound->IsLooping())
         {
            GetGameManager()->SetTimer(SoundActorProxy::PLAY_END_TIMER_NAME, this, seconds);
         }
      }

      ///////////////////////////////////////////////////////////////////////////////
      void SoundActorProxy::Stop()
      {
         SoundActor* soundActor;
         GetActor(soundActor);
         dtAudio::Sound* sound = soundActor->GetSound();
         if (sound != NULL)
         {
            sound->Stop();
            soundActor->DestroySound();
         }
      }

      ///////////////////////////////////////////////////////////////////////////////
      void SoundActorProxy::PlayQueued(float offsetSeconds)
      {
         if (mRandomSoundEffect)
         {
            offsetSeconds += float(dtUtil::RandFloat(mMinRandomTime, mMaxRandomTime));
         }

         GetGameManager()->SetTimer(SoundActorProxy::TIMER_NAME, this, offsetSeconds);
      }

      ///////////////////////////////////////////////////////////////////////////////
      dtDAL::ActorProxyIcon* SoundActorProxy::GetBillBoardIcon()
      {
         if (!mBillBoardIcon.valid())
         {
            mBillBoardIcon =
                     new dtDAL::ActorProxyIcon(dtDAL::ActorProxyIcon::IMAGE_BILLBOARD_SOUND);
         }

         return mBillBoardIcon.get();
      }

      ///////////////////////////////////////////////////////////////////////////////
      dtAudio::Sound* SoundActorProxy::GetSound()
      {
         SoundActor* actor = NULL;
         GetActor(actor);
         return actor->GetSound();
      }

      ///////////////////////////////////////////////////////////////////////////////
      const dtAudio::Sound* SoundActorProxy::GetSound() const
      {
         const SoundActor* actor = NULL;
         GetActor(actor);
         return actor->GetSound();
      }

      ///////////////////////////////////////////////////////////////////////////////
      void SoundActorProxy::SetLooping(bool looping)
      {
         mLooping = looping;
      }

      ///////////////////////////////////////////////////////////////////////////////
      bool SoundActorProxy::IsLooping() const
      {
         return mLooping;
      }

      ///////////////////////////////////////////////////////////////////////////////
      void SoundActorProxy::SetGain(float gain)
      {
         mGain = gain;
      }

      ///////////////////////////////////////////////////////////////////////////////
      float SoundActorProxy::GetGain() const
      {
         return mGain;
      }

      ///////////////////////////////////////////////////////////////////////////////
      void SoundActorProxy::SetPitch(float pitch)
      {
         mPitch = pitch;
      }

      ///////////////////////////////////////////////////////////////////////////////
      float SoundActorProxy::GetPitch() const
      {
         return mPitch;
      }

      ///////////////////////////////////////////////////////////////////////////////
      void SoundActorProxy::SetListenerRelative(bool lisrel)
      {
         mListenerRelative = lisrel;
      }

      ///////////////////////////////////////////////////////////////////////////////
      bool SoundActorProxy::IsListenerRelative() const
      {
         return mListenerRelative;
      }

      ///////////////////////////////////////////////////////////////////////////////
      void SoundActorProxy::SetMaxDistance(float max)
      {
         mMaxDistance = max;
      }

      ///////////////////////////////////////////////////////////////////////////////
      float SoundActorProxy::GetMaxDistance() const
      {
         return mMaxDistance;
      }

      ///////////////////////////////////////////////////////////////////////////////
      void SoundActorProxy::SetRolloffFactor(float rolloff)
      {
         mRolloffFactor = rolloff;
      }

      ///////////////////////////////////////////////////////////////////////////////
      float SoundActorProxy::GetRolloffFactor() const
      {
         return mRolloffFactor;
      }

      ///////////////////////////////////////////////////////////////////////////////
      void SoundActorProxy::SetMinGain(float min)
      {
         mMinGain = min;
      }

      ///////////////////////////////////////////////////////////////////////////////
      float SoundActorProxy::GetMinGain() const
      {
         return mMinGain;
      }

      ///////////////////////////////////////////////////////////////////////////////
      void SoundActorProxy::SetMaxGain(float max)
      {
         mMaxGain = max;
      }

      ///////////////////////////////////////////////////////////////////////////////
      float SoundActorProxy::GetMaxGain() const
      {
         return mMaxGain;
      }
   }
}
