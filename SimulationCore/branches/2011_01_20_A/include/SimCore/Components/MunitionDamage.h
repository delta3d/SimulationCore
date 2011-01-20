/* -*-c++-*-
* Simulation Core
* Copyright 2007-2008, Alion Science and Technology
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
 * @author Chris Rodgers
 */

#ifndef _MUNITION_DAMAGE_H_
#define _MUNITION_DAMAGE_H_

#include <SimCore/Export.h>
#include <dtCore/base.h>
#include <dtCore/refptr.h>
#include <dtUtil/enumeration.h>

#include <osg/Vec3>
#include <osg/Vec4>

// High Explosives (HE) use: Carleton Damage Model
// Improved Conventional Munition (ICM) use: Cookie Cutter Model

namespace SimCore
{
   namespace Components
   {
      //////////////////////////////////////////////////////////
      // Damage Type Code
      //////////////////////////////////////////////////////////
      class SIMCORE_EXPORT DamageType : public dtUtil::Enumeration
      {
         DECLARE_ENUM(DamageType);
         public:
            static DamageType DAMAGE_NONE;
            static DamageType DAMAGE_MOBILITY;
            static DamageType DAMAGE_FIREPOWER;
            static DamageType DAMAGE_MOBILITY_FIREPOWER;
            static DamageType DAMAGE_KILL;

         private:
            DamageType(const std::string &name) : dtUtil::Enumeration(name)
            {
               AddInstance(this);
            }
      };



      //////////////////////////////////////////////////////////
      // Damage Probability Code
      //////////////////////////////////////////////////////////
      class SIMCORE_EXPORT DamageProbability : public dtCore::Base
      {
         public:

            // Constructor
            DamageProbability( const std::string& name );

            void SetAbsoluteMode( bool absolute ) { mAbsoluteMode = absolute; }
            bool GetAbsoluteMode() const { return mAbsoluteMode; }

            void SetNoDamage( float probability ) { mNoDamage = probability; }
            float GetNoDamage() const { return mNoDamage; }

            void SetMobilityDamage( float probability ) { mMobilityDamage = probability; }
            float GetMobilityDamage() const { return mMobilityDamage; }

            void SetFirepowerDamage( float probability ) { mFirepowerDamage = probability; }
            float GetFirepowerDamage() const { return mFirepowerDamage; }

            void SetMobilityFirepowerDamage( float probability ) { mMobilityFirepowerDamage = probability; }
            float GetMobilityFirepowerDamage() const { return mMobilityFirepowerDamage; }

            void SetKillDamage( float probability ) { mKillDamage = probability; }
            float GetKillDamage() const { return mKillDamage; }

            void Set( float none, float mobility, float firepower,
               float mobilityFirepower, float kill )
            {
               mNoDamage = none;
               mMobilityDamage = mobility;
               mFirepowerDamage = firepower;
               mMobilityFirepowerDamage = mobilityFirepower;
               mKillDamage = kill;
            }

            DamageType& GetDamageType( float damage, bool getGreaterDamage = false ) const;

            const DamageProbability& operator= ( const DamageProbability& probabilities );

            bool operator== ( const DamageProbability& probabilities ) const;
            bool operator!= ( const DamageProbability& probabilities ) const;

         protected:

            // Destructor
            virtual ~DamageProbability() {}

         private:

            // Probabilities for certain types of damage in
            // order of severity; from low to high.
            float mNoDamage;
            float mMobilityDamage;
            float mFirepowerDamage;
            float mMobilityFirepowerDamage;
            float mKillDamage;

            bool mAbsoluteMode; // Determines if the probabilities should be compared by their raw values
      };



      //////////////////////////////////////////////////////////
      // Damage Ranges Code
      //////////////////////////////////////////////////////////
      class SIMCORE_EXPORT DamageRanges : public dtCore::Base
      {
         public:
            DamageRanges( const std::string& name );

            void SetAngleOfFall( float angle ) { mAof = angle; }
            float GetAngleOfFall() const { return mAof; }

            void SetForwardRanges( float mobility, float fire, float mobilityFire, float kill )
            {
               mForwardRanges.set( mobility, fire, mobilityFire, kill );
            }
            void SetForwardRanges( const osg::Vec4& ranges )
            {
               mForwardRanges = ranges;
            }
            osg::Vec4& GetForwardRanges() { return mForwardRanges; }
            const osg::Vec4& GetForwardRanges() const { return mForwardRanges; }

            void SetDeflectRanges( float mobility, float fire, float mobilityFire, float kill )
            {
               mDeflectRanges.set( mobility, fire, mobilityFire, kill );
            }
            void SetDeflectRanges( const osg::Vec4& ranges )
            {
               mDeflectRanges = ranges;
            }
            osg::Vec4& GetDeflectRanges() { return mDeflectRanges; }
            const osg::Vec4& GetDeflectRanges() const { return mDeflectRanges; }

            const DamageRanges& operator= ( const DamageRanges& ranges );

            bool operator== ( const DamageRanges& ranges ) const;
            bool operator!= ( const DamageRanges& ranges ) const;

         protected:
            virtual ~DamageRanges() {}

         private:
            // Angle of trajectory in relation to ground;
            // horizontal trajectory is 0, vertical is 90
            float mAof;

            // Ranges along trajectory
            osg::Vec4 mForwardRanges; // M, F, MF, K

            // Ranges perpendicular to trajectory
            osg::Vec4 mDeflectRanges; // M, F, MF, K
      };



      //////////////////////////////////////////////////////////
      // Munition Damage Code
      //////////////////////////////////////////////////////////
      class SIMCORE_EXPORT MunitionDamage : public dtCore::Base
      {
         public:

            // TODO?: Get fuze type (Zero & Contact are CONTACT, others are PROXIMITY )

            // Constructor
            MunitionDamage( const std::string& name );

            void SetCutoffRange( float range ) { mCutoffRange = range; }
            float GetCutoffRange() const { return mCutoffRange; }

            void SetNewtonForce( float magnitude ) { mNewtonForce = magnitude; }
            float GetNewtonForce() const { return mNewtonForce; }

            void SetAccumulationFactor( float factor ) { mAccumulationFactor = factor; }
            float GetAccumulationFactor() const { return mAccumulationFactor; }

            // The set functions for DamageRanges will copy the supplied range;
            // the values are merely copied, but NOT the name.
            // A references are not used so that the ranges can be set to NULL explicitly.
            void SetDamageRanges1_3( const dtCore::RefPtr<DamageRanges>& ranges );
            void SetDamageRanges2_3( const dtCore::RefPtr<DamageRanges>& ranges );
            void SetDamageRangesMax( const dtCore::RefPtr<DamageRanges>& ranges );

            DamageRanges* GetDamageRanges1_3() { return mRange1_3.get(); }
            DamageRanges* GetDamageRanges2_3() { return mRange2_3.get(); }
            DamageRanges* GetDamageRangesMax() { return mRangeMax.get(); }
            const DamageRanges* GetDamageRanges1_3() const { return mRange1_3.get(); }
            const DamageRanges* GetDamageRanges2_3() const { return mRange2_3.get(); }
            const DamageRanges* GetDamageRangesMax() const { return mRangeMax.get(); }

            // @param trajectory The trajectory of the munition from which to match an Angle-Of-Fall
            // @return damage ranges for the closest match to angle-of-fall for the trajectory;
            //    NOTE: this will return NULL if no ranges exist.
            //    Direct Fire munitions will mostly return NULL for ranges,
            //    especially bullet types.
            const DamageRanges* GetDamageRangesByTrajectory( const osg::Vec3& trajectory ) const;

            // Set damage probabilities for munitions such as BULLETS
            void SetDirectFireProbabilities( float none, float mobility, float firepower, float mobilityFirepower, float kill );
            void SetDirectFireProbabilities( const DamageProbability& probabilities );
            const DamageProbability* GetDirectFireProbabilities() { return mDirectFireProbs.get(); }

            // Set damage probabilities for munitions such as High Explosives (HE),
            // Improved Conventional Munition (ICM), and other PROXIMITY types.
            //
            // NOTE: This will also act as Direct Fire if Direct Fire probabilities have
            // not been set explicitly
            void SetIndirectFireProbabilities( float none, float mobility, float firepower, float mobilityFirepower, float kill );
            void SetIndirectFireProbabilities( const DamageProbability& probabilities );
            const DamageProbability* GetIndirectFireProbabilities() { return mIndirectFireProbs.get(); }

            void GetDamageProbabilities( DamageProbability& outProbabilities,
               float& outDistanceFromImpact, const osg::Vec3& modelDimensions,
               bool directFire,
               const osg::Vec3& munitionTrajectory,
               const osg::Vec3& munitionPosition,
               const osg::Vec3& entityPosition) const;

            // Get the damage probability via the Carleton Method
            // D0 is one of the damage probability coefficients: M, F, MF, K
            //    P = D0 * exp[-D0 * ((X/r1)^2 + (Y/r2)^2)]
            //
            // @param damageTypeCoefficient: The coefficient of a particular
            //       damage type {M,F,MF,K} as declared for High Explosive (HE) weapons
            //       within the ifdam_he.rdr file of the JSAF library "libifdam"
            // @param x: The distance from the detonation along the munition trajectory
            // @param y: The distance from the detonation perpendicular from the munition trajectory
            // @param downRangeRadius: The lethal radius in meters along the munition trajectory
            // @param deflectionRadius: The lethal radius perpendicular from the munition trajectory
            // @return Damage probability
            float GetProbability_CarletonEquation( float damageTypeCoefficient,
               float x, float y, float downRangeRadius, float deflectionRadius ) const;

            // Compute a force from this munition if it detonated.
            // @param targetLocation The location of impact measured in meters
            // @param detonationLocation The location of impact measured in meters
            // @param trajectory The initial trajectory/velocity of this munition just before impact.
            // @return the vector force of this blast (in Newtons) acting on the target point.
            //          The force is mass (1kg) * acceleration (1meter/sec^2)
            // NOTE: The force will be ZERO if the distance between the target and explosion
            // is greater than the munition's cutoff range.
            virtual osg::Vec3 GetForce( const osg::Vec3& targetLocation,
               const osg::Vec3& detonationLocation,
               const osg::Vec3& trajectory ) const;

         protected:

            // Destructor
            virtual ~MunitionDamage();

         private:

            float mCutoffRange;
            float mNewtonForce; // The magnitude of force this munition produces at impact point zero
            float mAccumulationFactor;
            dtCore::RefPtr<DamageProbability> mDirectFireProbs;
            dtCore::RefPtr<DamageProbability> mIndirectFireProbs;
            dtCore::RefPtr<DamageRanges> mRange1_3;
            dtCore::RefPtr<DamageRanges> mRange2_3;
            dtCore::RefPtr<DamageRanges> mRangeMax;
      };

   }
}

#endif
