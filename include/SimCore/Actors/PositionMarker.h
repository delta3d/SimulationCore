#ifndef _POSITION_MARKER_H_
#define _POSITION_MARKER_H_

#include <SimCore/Actors/BaseEntity.h>
#include <dtUtil/refstring.h>
#include <SimCore/Export.h>

namespace SimCore
{

   namespace Actors
   {

      class SIMCORE_EXPORT PositionMarker : public BaseEntity
      {
         public:
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

            PositionMarkerActorProxy();
            virtual ~PositionMarkerActorProxy();

            virtual void BuildPropertyMap();
         protected:
            void SetAssociatedEntity(dtDAL::ActorProxy* assocEntity);
            dtCore::DeltaDrawable* GetAssociatedEntity();
      };

   }

}

#endif /*_POSITION_MARKER_H_*/
