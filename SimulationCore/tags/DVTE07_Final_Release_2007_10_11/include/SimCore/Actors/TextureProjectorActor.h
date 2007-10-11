///////////////////////////////////////////////////////////////////////////////
// Author Email   : adanklefsen@alionscience.com
// Date Originated: 12/7/2006
///////////////////////////////////////////////////////////////////////////////
#ifndef _TEXTURE_PROJECTOR_ACTOR_
#define _TEXTURE_PROJECTOR_ACTOR_

#include <SimCore/Export.h>

#include <dtGame/gameactor.h>

#include <SimCore/Projector.h>

#include <osg/observer_ptr>

namespace dtGame
{
   class Message;
}

namespace SimCore
{
   namespace Actors
   {
      class SIMCORE_EXPORT TextureProjectorActor: public dtGame::GameActor
      {
         public:
            /// constructor for NxAgeiaBaseActor
            TextureProjectorActor(dtGame::GameActorProxy &proxy);
         
            /**
            * This method is an invokable called when an object is local and
            * receives a tick.
            * @param tickMessage A message containing tick related information.
            */
            virtual void TickLocal(const dtGame::Message &tickMessage);

            /**
            * This method is an invokable called when an object is remote and
            * receives a tick.
            * @param tickMessage A message containing tick related information.
            */
            virtual void TickRemote(const dtGame::Message &tickMessage);

            /**
            * Generic handler for messages. Overridden from base class.
            * This is the default invokable on GameActorProxy.
            */
            virtual void ProcessMessage(const dtGame::Message &message);

            // SUPER NEEDS TO BE CALLED AFTER YOUR OTHER CODE, MODEL MUST BE LOADED IN!
            // Called when the actor has been added to the game manager.
            // You can respond to OnEnteredWorld on either the proxy or actor or both.
            virtual void OnEnteredWorld();

         protected:
            /// destructor
            virtual ~TextureProjectorActor(void);

         public:
            
            // gets
            float GetCurrentTime() {return mCurrentTime;}
            float GetMaxTime()     {return mMaxTime;}
            float GetCurrentAlpha(){return mCurrentAlpha;}
            std::string GetImageProjectorFile() {return mImageProjectorFile;}

            // sets
            void SetCurrentTime(float value) {mCurrentTime = value;}
            void SetMaxTime(float value)     {mMaxTime = value;}
            void SetCurrentAlpha(float value){mCurrentAlpha = value;}
            void SetImageProjectorFile(const std::string& value) {mImageProjectorFile = value;}
            void SetAttachementEntity(osg::Node* nodeToAttachTo) { mEntityToAttachTo = nodeToAttachTo;}

         private: 
            float                      mCurrentTime;
            float                      mMaxTime;
            float                      mCurrentAlpha;
            std::string                mImageProjectorFile;
            dtCore::RefPtr<Projector>     mProjector;
            osg::observer_ptr<osg::Node>  mEntityToAttachTo;
      };

      ////////////////////////////////////////////////////////
      // PROXY
      ////////////////////////////////////////////////////////
      class SIMCORE_EXPORT TextureProjectorActorProxy : public dtGame::GameActorProxy
      {
         public:
            TextureProjectorActorProxy();
            virtual void BuildPropertyMap();

         protected:
            virtual ~TextureProjectorActorProxy();
            void CreateActor();
            virtual void OnEnteredWorld();
      };
   }
}
#endif
