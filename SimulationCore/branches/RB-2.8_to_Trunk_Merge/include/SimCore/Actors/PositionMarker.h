#ifndef _POSITION_MARKER_H_
#define _POSITION_MARKER_H_

#include <SimCore/Actors/BaseEntity.h>
#include <dtUtil/refstring.h>
#include <SimCore/Export.h>
#include <osg/ShapeDrawable>

namespace dtUtil
{
   class Log;
}

namespace dtGame
{
   class TimerElapsedMessage;
}

namespace SimCore
{

   namespace Actors
   {

      class SIMCORE_EXPORT PositionMarker : public BaseEntity
      {
         public:
            typedef BaseEntity BaseClass;

            static const std::string COLOR_UNIFORM;

            PositionMarker(dtGame::GameActorProxy& parent);
            virtual ~PositionMarker();

            void SetReportTime(double reportTime);
            double GetReportTime() const;

            /**
             * Sets this marker to go stale after the given amount of time in minutes from the report time.
             * The stale time should be shorter than the fade out time.
             * Set this to <= 0.0 if this marker should not fade out at all.
             */
            void SetStaleTime(float timeToFadeOut);
            /// @return Time in minutes after which this report becomes stale.
            float GetStaleTime() const;

            /**
             * Sets this marker to fade out over time using alpha.  The time starts from the report time.
             * The time is in minutes.
             * Set this to <= 0.0 if this marker should not fade out at all.
             */
            void SetFadeOutTime(float timeToFadeOut);
            /// this marker to fade out over time using alpha.  The time starts from the report time.
            float GetFadeOutTime() const;

            /// Sets if this actor should delete itself when it completely fades out.
            void SetDeleteOnFadeOut(bool);
            /// @return true if this actor should delete itself when it completely fades out, false if not.
            bool GetDeleteOnFadeOut() const;

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

            void SetInitialAlpha(float alpha);

            float GetInitialAlpha() const;

            float CalculateCurrentAlpha() const;
            osg::Vec3 CalculateCurrentColor() const;

            const osg::Vec3& GetInitialColor() const;
            osg::Vec4 GetInitialColorWithAlpha() const;

            void SetFriendlyColor(const osg::Vec4& vec);
            const osg::Vec4& GetFriendlyColor() const;

            void SetNeutralColor(const osg::Vec4& vec);
            const osg::Vec4& GetNeutralColor() const;

            void SetOpposingColor(const osg::Vec4& vec);
            const osg::Vec4& GetOpposingColor() const;

            void SetOtherColor(const osg::Vec4& vec);
            const osg::Vec4& GetOtherColor() const;

            void SetStaleColor(const osg::Vec4& vec);
            const osg::Vec4& GetStaleColor() const;

            /// overridden to set the colors when the force changes.
            virtual void SetForceAffiliation(BaseEntityActorProxy::ForceEnum& markerForce);

            virtual void OnTimer(const dtGame::TimerElapsedMessage& timerElapsed);

            virtual void OnEnteredWorld();

            osg::Vec4 GetCurrentColorUniform();

            void UpdateColorForForce();

            /// @return true if this position marker should be visible based on the options given.
            bool ShouldBeVisible(const SimCore::VisibilityOptions& options);

         protected:
            void SetInitialColor(const osg::Vec3& vec);
            void SetCurrentColorUniform(const osg::Vec4& vec);
            float CalculateFadeOutFraction() const;
            float CalculateStaleFraction() const;

         private:
            double mReportTime;
            float mStaleTime;
            float mFadeOutTime;
            osg::Vec3 mInitialColor;
            float mInitialAlpha;
            bool mDeleteOnFadeOut;
            dtUtil::RefString mSourceCallsign;
            dtCore::RefPtr<BaseEntity> mAssociatedEntity;
            BaseEntityActorProxy::ForceEnum* mSourceForce;
            BaseEntityActorProxy::ServiceEnum* mSourceService;
            osg::Vec4 mFriendlyColor, mNeutralColor, mOpposingColor, mOtherColor, mStaleColor;
            dtUtil::Log* mLogger;
      };

      class SIMCORE_EXPORT PositionMarkerActorProxy : public BaseEntityActorProxy
      {
         public:
            typedef BaseEntityActorProxy BaseClass;

            static const dtUtil::RefString PROPERTY_SOURCE_CALLSIGN;
            static const dtUtil::RefString PROPERTY_SOURCE_FORCE;
            static const dtUtil::RefString PROPERTY_SOURCE_SERVICE;
            static const dtUtil::RefString PROPERTY_REPORT_TIME;
            static const dtUtil::RefString PROPERTY_FADE_OUT_TIME;
            static const dtUtil::RefString PROPERTY_STALE_TIME;
            static const dtUtil::RefString PROPERTY_DELETE_ON_FADE_OUT;
            static const dtUtil::RefString PROPERTY_ASSOCIATED_ENTITY;
            static const dtUtil::RefString PROPERTY_MARKER_COLOR;
            static const dtUtil::RefString PROPERTY_FRIENDLY_COLOR;
            static const dtUtil::RefString PROPERTY_NEUTRAL_COLOR;
            static const dtUtil::RefString PROPERTY_OPPOSING_COLOR;
            static const dtUtil::RefString PROPERTY_OTHER_COLOR;
            static const dtUtil::RefString PROPERTY_STALE_COLOR;
            static const dtUtil::RefString PROPERTY_ICON_IMAGE;
            static const dtUtil::RefString PROPERTY_INITIAL_ALPHA;

            static const dtUtil::RefString INVOKABLE_TIME_ELAPSED;

            PositionMarkerActorProxy();
            virtual ~PositionMarkerActorProxy();

            virtual void BuildPropertyMap();
            virtual void BuildInvokables();
            virtual void CreateDrawable();
         protected:
            void SetAssociatedEntity(dtCore::ActorProxy* assocEntity);
            dtCore::DeltaDrawable* GetAssociatedEntity();
      };

   }

}

#endif /*_POSITION_MARKER_H_*/
