/* -*-c++-*-
 * SimulationCore
 * Copyright 2010, Alion Science and Technology
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
 * This software was developed by Alion Science and Technology Corporation under
 * circumstances in which the U. S. Government may have rights in the software.
 * 
 * David Guthrie
 */

#ifndef TRAILERHITCHACTCOMP_H_
#define TRAILERHITCHACTCOMP_H_

#include <dtGame/actorcomponent.h>
#include <dtUtil/refstring.h>
#include <dtUtil/getsetmacros.h>
#include <dtUtil/enumeration.h>
#include <dtCore/uniqueid.h>

#include <SimCore/Export.h>

#include <SimCore/Actors/IGActor.h>

class palLink;

namespace osg
{
   class Group;
}


namespace dtPhysics
{
   class PhysicsObject;
}

namespace SimCore
{

   namespace ActComps
   {

      class SIMCORE_EXPORT HitchTypeEnum : public dtUtil::Enumeration
      {
         DECLARE_ENUM(HitchTypeEnum);
      public:

         /// A spherical joint that allows rotation in any direction.
         static HitchTypeEnum HITCH_TYPE_SPHERICAL;
         /// A joint that allows twist and some pitch, but no roll.
         static HitchTypeEnum HITCH_TYPE_5TH_WHEEL;

      private:
         HitchTypeEnum(const std::string& name);
      };

      /**
       * Actor component that requires a physics actor component and manage a trailer hitch to
       * another actor that happens to be a trailer :-).  This actor component should be added the
       * tractor.  It can also be added to a trailer if that trailer can pull another trailer.
       */
      class SIMCORE_EXPORT TrailerHitchActComp : public dtGame::ActorComponent
      {
      public:
         typedef dtGame::ActorComponent BaseClass;

         static const dtUtil::RefString FRONT_HITCH_DOF_NAME_DEFAULT;
         static const dtUtil::RefString REAR_HITCH_DOF_NAME_DEFAULT;

         static const dtUtil::RefString TYPE;

         TrailerHitchActComp();

         virtual void BuildPropertyMap();

         virtual void OnEnteredWorld();

         virtual void OnRemovedFromWorld();

         /**
          * The max twist rotation for the hitch from straight behind the vehicle until it hits something.
          * Half the total allowed rotation.
          */
         DT_DECLARE_ACCESSOR(float, RotationMaxYaw);

         /// The max pitch and roll (roll only for spherical hitch).
         DT_DECLARE_ACCESSOR(float, RotationMaxCone);

         /// The kind of hitch to create. Defaults to HITCH_TYPE_SPHERICAL;
         DT_DECLARE_ACCESSOR(dtUtil::EnumerationPointer<HitchTypeEnum>, HitchType);

         /// The name of the DOF node used for the hitch on the tractor or pulling trailer
         DT_DECLARE_ACCESSOR(std::string, HitchNodeNameTractor);

         /// The name of the DOF node used for the hitch on the trailer
         DT_DECLARE_ACCESSOR(std::string, HitchNodeNameTrailer);

         /// The trailer actor id.
         DT_DECLARE_ACCESSOR(dtCore::UniqueId, TrailerActorId);

         /**
          * Current Hitch rotation in HPR, which is calculated via the tractor and trailer hitches and their relative rotation.
          *
          * If not attached, this will return an internal value which you can set via the setter and represents
          * the initial attach angle.
          */
         osg::Vec3 GetCurrentHitchRotHPR() const;

         /**
          * This is just simple setter unless the trailer is remote and UseCurrentHitchRotToMoveTrailerWhenRemote is true
          * in which case it will actually re-rotate the trailer on the hitch.
          * If you set this before attaching, the code will use this as the initial attach angle.
          */
         void SetCurrentHitchRotHPR(const osg::Vec3& hpr);

      private:
         osg::Vec3 mCurrentHitchRotHPR;
      public:

         /// true or false for removing the attached actors from the GM when the parent is removed.
         DT_DECLARE_ACCESSOR(bool, CascadeDeletes);

         /// Set this to true to make the CurrentHitchRotHPR move the trailer. Defaults to false.
         DT_DECLARE_ACCESSOR(bool, UseCurrentHitchRotToMoveTrailerWhenRemote);

         /// Call this to attach
         void Attach();

         void Detach();

         bool GetAttached() const;

         dtGame::GameActor* GetTrailer();

         /**
          * Call this if you move the tractor.  if CurrentHitchRotHPR is changed
          * and UseCurrentHitchRotToMoveTrailerWhenRemote is true, this will be called
          * automatically each frame.
          *
          * @return the hitch world position.
          */
         osg::Vec3d WarpTrailerToTractor(bool addAsChild = false);

         std::pair<osg::Group*, osg::Group* > GetHitchNodes() const;
         std::pair<dtPhysics::PhysicsObject*, dtPhysics::PhysicsObject*> GetPhysicsObjects() const;

         void CalcTransformsForTractorHitchAndTrailerVisual(dtCore::Transform& tractorHitchTransform, dtCore::Transform& trailerTransform);

         /// Finds the trailer actor by the id stored.  Returns the actor as a convenience.
         SimCore::Actors::IGActor* LookupTrailer();

         void ResetTrailerActor();

      protected:
         virtual ~TrailerHitchActComp();

      private:
         std::weak_ptr<SimCore::Actors::IGActor> mTrailerActor;
         palLink* mHitchJoint;
      };

   }

}

#endif /* TRAILERHITCHACTCOMP_H_ */
