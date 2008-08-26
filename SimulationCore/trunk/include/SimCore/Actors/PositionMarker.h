#ifndef _POSITION_MARKER_H_
#define _POSITION_MARKER_H_

#include <SimCore/Actors/BaseEntity.h>
#include <dtUtil/refstring.h>
#include <SimCore/Export.h>
#include <osg/ShapeDrawable>

namespace SimCore
{

   namespace Actors
   {

      class SIMCORE_EXPORT PositionMarker : public BaseEntity
      {
         public:
            static const std::string COLOR_UNIFORM;

            typedef BaseEntity BaseClass;
            PositionMarker(dtGame::GameActorProxy& proxy);
            virtual ~PositionMarker();

            void SetReportTime(double reportTime);
            double GetReportTime() const;

            void SetSourceForce(BaseEntityActorProxy::ForceEnum& sourceForce);
            BaseEntityActorProxy::ForceEnum& GetSourceForce() const;

            void SetSourceService(BaseEntityActorProxy::ServiceEnum& sourceService);
            BaseEntityActorProxy::ServiceEnum& GetSourceService() const;

            void SetSourceCallsign(const std::string& sourceCallsign);
            const std::string& GetSourceCallsign() const;

            void SetAssociatedEntity(BaseEntity* assocEntity);
            const BaseEntity* GetAssociatedEntity() const;
            BaseEntity* GetAssociatedEntity();

            void LoadImage(const std::string& theFile);

            /// Implemented from the base class
            virtual void HandleModelDrawToggle(bool active);

            void SetColor(const osg::Vec4& vec);
            const osg::Vec4 GetColor();

            /// overridden to set the colors when the force changes.
            virtual void SetForceAffiliation(BaseEntityActorProxy::ForceEnum& markerForce);

            virtual void OnEnteredWorld();
         private:

            double mReportTime;
            dtUtil::RefString mSourceCallsign;
            dtCore::RefPtr<BaseEntity> mAssociatedEntity;
            BaseEntityActorProxy::ForceEnum* mSourceForce;
            BaseEntityActorProxy::ServiceEnum* mSourceService;
      };

      class SIMCORE_EXPORT PositionMarkerActorProxy : public BaseEntityActorProxy
      {
         public:
            typedef BaseEntityActorProxy BaseClass;

            static const dtUtil::RefString PROPERTY_SOURCE_CALLSIGN;
            static const dtUtil::RefString PROPERTY_SOURCE_FORCE;
            static const dtUtil::RefString PROPERTY_SOURCE_SERVICE;
            static const dtUtil::RefString PROPERTY_REPORT_TIME;
            static const dtUtil::RefString PROPERTY_ASSOCIATED_ENTITY;
            static const dtUtil::RefString PROPERTY_MARKER_COLOR;
            static const dtUtil::RefString PROPERTY_ICON_IMAGE;

            PositionMarkerActorProxy();
            virtual ~PositionMarkerActorProxy();

            virtual void BuildPropertyMap();
            virtual void CreateActor();
         protected:
            void SetAssociatedEntity(dtDAL::ActorProxy* assocEntity);
            dtCore::DeltaDrawable* GetAssociatedEntity();
      };

   }

}

#endif /*_POSITION_MARKER_H_*/
