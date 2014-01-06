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

#include <prefix/SimCorePrefix.h>
#include <SimCore/Components/MunitionDamage.h>
#include <dtUtil/mathdefines.h>

#include <cmath>
namespace SimCore
{
   namespace Components
   {
      //////////////////////////////////////////////////////////////////////////
      // Enumeration Code
      //////////////////////////////////////////////////////////////////////////
      IMPLEMENT_ENUM(DamageType);
      DamageType DamageType::DAMAGE_NONE("NONE"); 
      DamageType DamageType::DAMAGE_MOBILITY("MOBILITY"); 
      DamageType DamageType::DAMAGE_FIREPOWER("FIREPOWER");
      DamageType DamageType::DAMAGE_MOBILITY_FIREPOWER("MOBILITY_FIREPOWER"); 
      DamageType DamageType::DAMAGE_KILL("KILL"); 



      //////////////////////////////////////////////////////////
      // Damage Probability Code
      //////////////////////////////////////////////////////////
      DamageProbability::DamageProbability( const std::string& name )
         : dtCore::Base(name),
         mNoDamage(0.0f),
         mMobilityDamage(0.0f),
         mFirepowerDamage(0.0f),
         mMobilityFirepowerDamage(0.0f),
         mKillDamage(1.0f),
         mAbsoluteMode(false)
      {
      }

      //////////////////////////////////////////////////////////
      DamageType& DamageProbability::GetDamageType( float damage, bool reversed ) const
      {
         float mobilityLevel, firepowerLevel, mobilityFirepowerLevel, noneLevel, killLevel;

         if( ! mAbsoluteMode )
         {
            if( reversed )
            {
               mobilityFirepowerLevel = mKillDamage + mMobilityFirepowerDamage;
               firepowerLevel = mobilityFirepowerLevel + mFirepowerDamage;
               mobilityLevel = firepowerLevel + mMobilityDamage;
               noneLevel = mNoDamage + mobilityLevel;
               killLevel = 1.0 - noneLevel;
               killLevel = mKillDamage > killLevel ? mKillDamage : killLevel;

               if( damage <= killLevel ) { return DamageType::DAMAGE_KILL; }
               if( damage <= mobilityFirepowerLevel ) { return DamageType::DAMAGE_MOBILITY_FIREPOWER; }
               if( damage <= firepowerLevel ) { return DamageType::DAMAGE_FIREPOWER; }
               if( damage <= mobilityLevel ) { return DamageType::DAMAGE_MOBILITY; }
               return DamageType::DAMAGE_NONE;
            }
            else
            {
               mobilityLevel = mNoDamage + mMobilityDamage;
               firepowerLevel = mobilityLevel + mFirepowerDamage;
               mobilityFirepowerLevel = firepowerLevel + mMobilityFirepowerDamage;
               killLevel = mKillDamage + mobilityFirepowerLevel;
               noneLevel = 1.0 - killLevel;
               noneLevel = mNoDamage > noneLevel ? mNoDamage : noneLevel;

               if( damage <= noneLevel ) { return DamageType::DAMAGE_NONE; }
               if( damage <= mobilityLevel ) { return DamageType::DAMAGE_MOBILITY; }
               if( damage <= firepowerLevel ) { return DamageType::DAMAGE_FIREPOWER; }
               if( damage <= mobilityFirepowerLevel ) { return DamageType::DAMAGE_MOBILITY_FIREPOWER; }
               return DamageType::DAMAGE_KILL;
            }

         }
         else
         {
            if( reversed )
            {
               if( damage <= mKillDamage ) { return DamageType::DAMAGE_KILL; }
               if( damage <= mMobilityFirepowerDamage ) { return DamageType::DAMAGE_MOBILITY_FIREPOWER; }
               if( damage <= mFirepowerDamage ) { return DamageType::DAMAGE_FIREPOWER; }
               if( damage <= mMobilityDamage ) { return DamageType::DAMAGE_MOBILITY; }
               return DamageType::DAMAGE_NONE;
            }
            else
            {
               if( damage <= mNoDamage ) { return DamageType::DAMAGE_NONE; }
               if( damage <= mMobilityDamage ) { return DamageType::DAMAGE_MOBILITY; }
               if( damage <= mFirepowerDamage ) { return DamageType::DAMAGE_FIREPOWER; }
               if( damage <= mMobilityFirepowerDamage ) { return DamageType::DAMAGE_MOBILITY_FIREPOWER; }
               return DamageType::DAMAGE_KILL;
            }

         }
      }

      //////////////////////////////////////////////////////////
      const DamageProbability& DamageProbability::operator= ( const DamageProbability& probabilities )
      {
         if( &probabilities != this )
         {
            mNoDamage                = probabilities.mNoDamage;
            mMobilityDamage          = probabilities.mMobilityDamage;
            mFirepowerDamage         = probabilities.mFirepowerDamage;
            mMobilityFirepowerDamage = probabilities.mMobilityFirepowerDamage;
            mKillDamage              = probabilities.mKillDamage;
         }
         return *this;
      }

      //////////////////////////////////////////////////////////
      bool DamageProbability::operator== ( const DamageProbability& probabilities ) const
      {
         return &probabilities == this
            || mNoDamage == probabilities.mNoDamage
            && mMobilityDamage == probabilities.mMobilityDamage
            && mFirepowerDamage == probabilities.mFirepowerDamage
            && mMobilityFirepowerDamage == probabilities.mMobilityFirepowerDamage
            && mKillDamage == probabilities.mKillDamage;
      }

      //////////////////////////////////////////////////////////
      bool DamageProbability::operator!= ( const DamageProbability& probabilities ) const
      {
         return ! ( probabilities == *this );
      }



      //////////////////////////////////////////////////////////
      // Damage Ranges Code
      //////////////////////////////////////////////////////////
      DamageRanges::DamageRanges( const std::string& name ) 
         : dtCore::Base(name),
         mAof(0.0f)
      {}

      //////////////////////////////////////////////////////////
      const DamageRanges& DamageRanges::operator= ( const DamageRanges& ranges )
      {
         if( &ranges != this )
         {
            mAof = ranges.mAof;
            mForwardRanges = ranges.mForwardRanges;
            mDeflectRanges = ranges.mDeflectRanges;
         }
         return *this;
      }

      //////////////////////////////////////////////////////////
      bool DamageRanges::operator== ( const DamageRanges& ranges ) const
      {
         return &ranges == this
            || mAof == ranges.mAof
            && mForwardRanges == ranges.mForwardRanges
            && mDeflectRanges == ranges.mDeflectRanges;
      }

      //////////////////////////////////////////////////////////
      bool DamageRanges::operator!= ( const DamageRanges& ranges ) const
      {
         return ! ( ranges == *this );
      }



      //////////////////////////////////////////////////////////
      // Munition Damage Code
      //////////////////////////////////////////////////////////
      MunitionDamage::MunitionDamage( const std::string& name )
         : dtCore::Base(name),
         mCutoffRange(0.0f),
         mNewtonForce(0.0f),
         mAccumulationFactor(0.0f)
      {
      }

      //////////////////////////////////////////////////////////
      MunitionDamage::~MunitionDamage()
      {
      }

      //////////////////////////////////////////////////////////
      void MunitionDamage::SetDamageRanges1_3( const std::shared_ptr<DamageRanges>&  ranges )
      { 
         if( ! ranges.valid() ) { mRange1_3 = nullptr; return; }
         if( ! mRange1_3.valid() ) { mRange1_3 = new DamageRanges("Range1/3"); }

         (*mRange1_3) = *ranges; 
      }

      //////////////////////////////////////////////////////////
      void MunitionDamage::SetDamageRanges2_3( const std::shared_ptr<DamageRanges>&  ranges )
      { 
         if( ! ranges.valid() ) { mRange2_3 = nullptr; return; }
         if( ! mRange2_3.valid() ) { mRange2_3 = new DamageRanges("Range2/3"); }

         (*mRange2_3) = *ranges;
      }

      //////////////////////////////////////////////////////////
      void MunitionDamage::SetDamageRangesMax( const std::shared_ptr<DamageRanges>&  ranges )
      { 
         if( ! ranges.valid() ) { mRangeMax = nullptr; return; }
         if( ! mRangeMax.valid() ) { mRangeMax = new DamageRanges("RangeMax"); }

         (*mRangeMax) = *ranges; 
      }

      //////////////////////////////////////////////////////////
      const DamageRanges* MunitionDamage::GetDamageRangesByTrajectory( const osg::Vec3& trajectory ) const
      {
         osg::Vec3 trajectoryNormal(trajectory);
         float angle = 0.0f;

         if( trajectoryNormal.length2() > 0.0f )
         {
            trajectoryNormal.normalize();

            angle = trajectoryNormal * osg::Vec3(0.0f,0.0f,1.0f);
            // Turn dot product to an angle measured in degrees;
            // the constant 57.29578 converts radians to degrees.
            angle = asinf( std::abs( angle ) ) * 57.29578f;
         }
         else // Use max range since this might be a mine munition
         {
            angle = 90.0f; // force code to choose max range, if it exists
         }

         float diff1, diff2, diff3;
         diff1 = diff2 = diff3 = 360.0f; // default to a failing value
         if(mRange1_3.valid()) { diff1 = std::abs( mRange1_3->GetAngleOfFall() - angle ); }
         if(mRange2_3.valid()) { diff2 = std::abs( mRange2_3->GetAngleOfFall() - angle ); }
         if(mRangeMax.valid()) { diff3 = std::abs( mRangeMax->GetAngleOfFall() - angle ); }

         int range = (diff1 < diff2 && diff1 < diff3) ? 1 :
                     (diff2 < diff1 && diff2 < diff3) ? 2 : 3;

         if( range == 1 ) { return mRange1_3.get(); }
         else if( range == 2 ) { return mRange2_3.get(); }
         return mRangeMax.get();
      }

      //////////////////////////////////////////////////////////
      void MunitionDamage::SetDirectFireProbabilities( float none, float mobility, 
         float firepower, float mobilityFirepower, float kill )
      {
         if( ! mDirectFireProbs.valid() ) { mDirectFireProbs = new DamageProbability("DirectFire"); }

         mDirectFireProbs->Set(none,mobility,firepower,mobilityFirepower,kill);
      }

      //////////////////////////////////////////////////////////
      void MunitionDamage::SetDirectFireProbabilities( const DamageProbability& probabilities )
      {
         if( ! mDirectFireProbs.valid() ) { mDirectFireProbs = new DamageProbability("DirectFire"); }
         *mDirectFireProbs = probabilities;
      }

      //////////////////////////////////////////////////////////
      void MunitionDamage::SetIndirectFireProbabilities( float none, float mobility, 
         float firepower, float mobilityFirepower, float kill )
      {
         if( ! mIndirectFireProbs.valid() ) { mIndirectFireProbs = new DamageProbability("IndirectFire"); }

         mIndirectFireProbs->Set(none,mobility,firepower,mobilityFirepower,kill);
      }

      //////////////////////////////////////////////////////////
      void MunitionDamage::SetIndirectFireProbabilities( const DamageProbability& probabilities )
      {
         if( ! mIndirectFireProbs.valid() ) { mIndirectFireProbs = new DamageProbability("IndirectFire"); }
         *mIndirectFireProbs = probabilities;
      }

      //////////////////////////////////////////////////////////
      void MunitionDamage::GetDamageProbabilities( DamageProbability& outProbabilities, 
         float& outDistanceFromImpact, const osg::Vec3& modelDimensions,
         bool directFire,
         const osg::Vec3& munitionTrajectory, 
         const osg::Vec3& munitionPosition, 
         const osg::Vec3& entityPosition) const
      {
         if( directFire )
         {
            if( mDirectFireProbs.valid() )
            {
               outProbabilities = *mDirectFireProbs;
            }
            else
            {
               if( mIndirectFireProbs.valid() )
               {
                  outProbabilities = *mIndirectFireProbs;
               }
               else
               {
                  // Invulnerable
                  outProbabilities.Set(1.0f,0.0f,0.0f,0.0f,0.0f);
               }
            }
            return;
         }

         // Continue with Indirect Fire...
         osg::Vec3 offset = entityPosition - munitionPosition;

         // Distance is the offset, minus half the width of the model.
         outDistanceFromImpact = offset.length() - modelDimensions.length()/2.0;
         outDistanceFromImpact = dtUtil::Max(outDistanceFromImpact, 0.0f);
         if( outDistanceFromImpact >= mCutoffRange )
         {
            outProbabilities.SetAbsoluteMode( true );
            outProbabilities.Set(1.0f,0.0f,0.0f,0.0f,0.0f);
            return;
         }

         osg::Vec3 trajectoryNormal( munitionTrajectory );

         if( trajectoryNormal.length2() != 0.0f )
         {
            trajectoryNormal.normalize();
         }

         float x = trajectoryNormal * offset; // distance along trajectory
         float y = (offset+(trajectoryNormal*-x)).length(); // distance perpendicular to trajectory
         const DamageRanges* ranges = GetDamageRangesByTrajectory( trajectoryNormal );

         if( ranges != nullptr )
         {
            const osg::Vec4& forwardRanges = ranges->GetForwardRanges();
            const osg::Vec4& deflectRanges = ranges->GetDeflectRanges();

            outProbabilities.SetAbsoluteMode( true );
            outProbabilities.Set(
               mIndirectFireProbs->GetNoDamage(), // there is no range specified for NONE damage 
               GetProbability_CarletonEquation( mIndirectFireProbs->GetMobilityDamage(), 
                  x, y, forwardRanges[0], deflectRanges[0] ),
               GetProbability_CarletonEquation( mIndirectFireProbs->GetFirepowerDamage(), 
                  x, y, forwardRanges[1], deflectRanges[1] ),
               GetProbability_CarletonEquation( mIndirectFireProbs->GetMobilityFirepowerDamage(), 
                  x, y, forwardRanges[2], deflectRanges[2] ),
               GetProbability_CarletonEquation( mIndirectFireProbs->GetKillDamage(), 
                  x, y, forwardRanges[3], deflectRanges[3] )
            );
         }
      }

      //////////////////////////////////////////////////////////
      float MunitionDamage::GetProbability_CarletonEquation( float damageTypeCoefficient, 
         float x, float y, float forwardRadius, float deflectionRadius ) const
      {
         if( forwardRadius == 0.0f ) // prevent division by zero
         {
            forwardRadius = 1.0;
            x = 0.0f;
         }
         if( deflectionRadius == 0.0f ) // prevent division by zero
         {
            deflectionRadius = 1.0;
            y = 0.0f;
         }

         // This is the carleton damage function - search on google...
         //    P = D0 * exp[-D0 * ((X/r1)^2 + (Y/r2)^2)]
         return damageTypeCoefficient * 
            exp( -damageTypeCoefficient * (pow(x/forwardRadius,2.0f) + pow(y/deflectionRadius,2.0f)) );
      }

      //////////////////////////////////////////////////////////
      osg::Vec3 MunitionDamage::GetForce( const osg::Vec3& targetLocation,
         const osg::Vec3& detonationLocation,
         const osg::Vec3& trajectory ) const
      {
         osg::Vec3 trajectoryNormal( trajectory );
         if( trajectoryNormal.length2() != 0.0f ) { trajectoryNormal.normalize(); }

         osg::Vec3 force = targetLocation - detonationLocation;

         // distance along trajectory
         float x = trajectoryNormal * force;
         // distance perpendicular to trajectory
         // NOTE: x is negated to reverse the trajectory and create an
         // intermediate vector at the same x offset along the trajectory
         // as that of the target position; this vector will be perpendicular
         // to the trajectory, and thus y can be calculated by simply obtaining
         // the vecor's length.
         float y = (force+(trajectoryNormal*-x)).length();
         const DamageRanges* ranges = GetDamageRangesByTrajectory( trajectoryNormal );

         if( force.length2() != 0.0f )
         {
            force.normalize();
         }
         else // this is a dead-on hit --- this should rarely, if ever, happen
         {
            // Force straight up if no trajectory
            if( trajectoryNormal.length2() != 0.0f ) { trajectoryNormal.set(0.0f,0.0f,1.0f); }
            // Use the munition trajectory for the force direction
            force = trajectoryNormal;
         }

         if( ranges == nullptr ) { return force; }

         // Obtain the max area
         const osg::Vec4& forwardRanges = ranges->GetForwardRanges();
         const osg::Vec4& deflectRanges = ranges->GetDeflectRanges();
         float forwardRadius = 0.25f;
         float deflectRadius = 0.25f;

         // Find the largest ranges
         // NOTE: largest range usually is that defined for MOBILITY damage,
         // however, it should not be assumed.
         for( int i = 0; i < 4; ++i )
         {
            if( forwardRadius < forwardRanges[i] ) { forwardRadius = forwardRanges[i]; }
            if( deflectRadius < deflectRanges[i] ) { deflectRadius = deflectRanges[i]; }
         }

         // Use the Carleton Equation to determine the force magnitude.
         float damagePercent = GetProbability_CarletonEquation(1.0, x, y, forwardRadius, deflectRadius );
         force *= (mNewtonForce * damagePercent); 

         return force;
      }

   }
}
