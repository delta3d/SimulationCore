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

            const osg::Vec4 GetColor();

            void SetFriendlyColor(const osg::Vec4& vec);
            const osg::Vec4& GetFriendlyColor();

            void SetNeutralColor(const osg::Vec4& vec);
            const osg::Vec4& GetNeutralColor();

            void SetOpposingColor(const osg::Vec4& vec);
            const osg::Vec4& GetOpposingColor();

            void SetOtherColor(const osg::Vec4& vec);
            const osg::Vec4& GetOtherColor();

            /// overridden to set the colors when the force changes.
            virtual void SetForceAffiliation(BaseEntityActorProxy::ForceEnum& markerForce);

            virtual void OnEnteredWorld();

         protected:
            void SetColor(const osg::Vec4& vec);

         private:
            double mReportTime;
            dtUtil::RefString mSourceCallsign;
            dtCore::RefPtr<BaseEntity> mAssociatedEntity;
            BaseEntityActorProxy::ForceEnum* mSourceForce;
            BaseEntityActorProxy::ServiceEnum* mSourceService;
            osg::Vec4 mFriendlyColor, mNeutralColor, mOpposingColor, mOtherColor;
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
            static const dtUtil::RefString PROPERTY_FRIENDLY_COLOR;
            static const dtUtil::RefString PROPERTY_NEUTRAL_COLOR;
            static const dtUtil::RefString PROPERTY_OPPOSING_COLOR;
            static const dtUtil::RefString PROPERTY_OTHER_COLOR;
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
