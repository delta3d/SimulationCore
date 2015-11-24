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
* @author Allen Danklefsen
*/
#include <prefix/SimCorePrefix.h>
#include <SimCore/MatrixManipulations.h>

#include <dtCore/camera.h>
#include <dtCore/deltawin.h>
#include <dtCore/transform.h>
#include <dtUtil/log.h>
#include <dtUtil/matrixutil.h>
#include <dtUtil/mathdefines.h>

namespace SimCore
{
   namespace Components
   {
      ///////////////////////////////////////////////////////////////////////////////////
      void Matrix_Manipulations::ToggleCamera(MATRIX_MANIPULATIONS_MODES camTypeToUse)
      {
         mCurrentMode = camTypeToUse;

         // if outside what we want to use right now, put it back to a default mode
         if(mCurrentMode >= REARVIEW || mCurrentMode <= FLY_MODE)
            mCurrentMode =  ATTACH_TO_UNLOCKED;

         mHPROffset.set(0,0);

         mSoftAttachList.clear();
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void Matrix_Manipulations::UpdateCamera(float deltaTime,Matrix_Manipulations_Matrix& inMatrix, Matrix_Manipulations_Matrix& outMatrix,
         osg::Vec3& inoutOffset, dtCore::Mouse* ourMouse, dtCore::DeltaDrawable* drawableForISectorTests)
      {
         switch(mCurrentMode)
         {
            case ATTACH_TO_UNLOCKED:
            {
               if(ourMouse == NULL)
               {
                  LOG_WARNING("ourMouse passed into Matrix_Manipulations was null, and i need that for my math.");
                  return;
               }

               ////////////////////////////////////////////////////////////////////////////
               //       THIS CODE WILL DO ROTATION FROM ACTOR SENT IN                    //
               ////////////////////////////////////////////////////////////////////////////
               osg::Vec2 clampVec = ourMouse->GetPosition();
               if(std::abs(clampVec[0]) < 0.01f)
                  clampVec[0] = 0.0f;
               if(std::abs(clampVec[1]) < 0.01f)
                  clampVec[1] = 0.0f;

               clampVec[1] *= -1;
               osg::Vec2 mousePos = clampVec * deltaTime * 65.0f;

               osg::Matrix additionRotMatrix(&inMatrix);
               osg::Vec3 hpr, hpr2;
               dtUtil::MatrixUtil::MatrixToHpr(hpr, additionRotMatrix);

               osg::Vec3 originalLoc;
               originalLoc.set(inMatrix[12],inMatrix[13],inMatrix[14]);

               osg::Matrix temporaryRotMatrix(&inMatrix);
               mHPROffset -= mousePos;
               osg::Matrix xMatrix = temporaryRotMatrix.rotate(mousePos[0], 0.0f, 0.0f, -1.0f);
               osg::Matrix yMatrix = temporaryRotMatrix.rotate(mousePos[1], 1.0f, 0.0f, 0.0f);
               osg::Matrix finalRotationMatrix = yMatrix * xMatrix;

               //dtUtil::MatrixUtil::MatrixToHpr(hpr2, finalRotationMatrix);
               osg::Vec2 tempoffset = mHPROffset + osg::Vec2(hpr[0], hpr[1]);
               hpr2[0] = tempoffset[0];
               hpr2[1] = tempoffset[1];
               hpr2[2] = 0;

               //hpr2 = mHPROffset + osg::Vec2(hpr[0], hpr[1]);
               dtUtil::MatrixUtil::HprToMatrix(finalRotationMatrix, hpr2);

               for(int i = 0 ; i < 16; ++i)
               {
                  outMatrix[i] = finalRotationMatrix.ptr()[i];
               }

               outMatrix[12] = originalLoc[0];
               outMatrix[13] = originalLoc[1];
               outMatrix[14] = originalLoc[2];

               ////////////////////////////////////////////////////////////////////////////
               //       THIS CODE REPOSITIONS THE CAMERA UP DOWN LEFT RIGHT OFFSET       //
               ////////////////////////////////////////////////////////////////////////////
               osg::Vec3 position;
               position[0]= inMatrix[12];
               position[1]= inMatrix[13];
               position[2]= inMatrix[14];

               //// Move in the X vector
               osg::Vec3 moveX;
               moveX[0] = inMatrix[0];
               moveX[1] = inMatrix[1];
               moveX[2] = inMatrix[2];
               moveX *= inoutOffset[0];
               position += moveX;

               //// Move in the Y vector
               osg::Vec3 movey;
               movey[0] = inMatrix[4];
               movey[1] = inMatrix[5];
               movey[2] = inMatrix[6];
               movey *= inoutOffset[1];
               position += movey;

               // Move in the z Vector
               osg::Vec3 movez;
               movez[0] = inMatrix[8];
               movez[1] = inMatrix[9];
               movez[2] = inMatrix[10];
               movez *= inoutOffset[2];
               position += movez;

               outMatrix[12] = position[0];
               outMatrix[13] = position[1];
               outMatrix[14] = position[2];

               // very last
               ourMouse->SetPosition(osg::Vec2(0.0f, 0.0f));
            }
            break;

            case HARDATTACH_MODE:
            {
               dtCore::Transform camera2;

               osg::Vec3 position;
               position[0]= inMatrix[12];
               position[1]= inMatrix[13];
               position[2]= inMatrix[14];

               //// Move in the X vector
               osg::Vec3 moveX;
               moveX[0] = inMatrix[0];
               moveX[1] = inMatrix[1];
               moveX[2] = inMatrix[2];
               moveX *= inoutOffset[0];
               position += moveX;

               //// Move in the Y vector
               osg::Vec3 movey;
               movey[0] = inMatrix[4];
               movey[1] = inMatrix[5];
               movey[2] = inMatrix[6];
               movey *= inoutOffset[1];
               position += movey;

               // Move in the z Vector
               osg::Vec3 movez;
               movez[0] = inMatrix[8];
               movez[1] = inMatrix[9];
               movez[2] = inMatrix[10];
               movez *= inoutOffset[2];
               position += movez;

               inMatrix[12] = position[0];
               inMatrix[13] = position[1];
               inMatrix[14] = position[2];

               outMatrix = inMatrix;
            }
            break;

            case SOFTATTACH_MODE:
            {
               dtCore::Transform camera2;

               osg::Vec3 position;
               position[0]= inMatrix[12];
               position[1]= inMatrix[13];
               position[2]= inMatrix[14];

               //// Move in the X vector
               osg::Vec3 moveX;
               moveX[0] = inMatrix[0];
               moveX[1] = inMatrix[1];
               moveX[2] = inMatrix[2];
               moveX *= inoutOffset[0];
               position += moveX;

               //// Move in the Y vector
               osg::Vec3 movey;
               movey[0] = inMatrix[4];
               movey[1] = inMatrix[5];
               movey[2] = inMatrix[6];
               movey *= inoutOffset[1];
               position += movey;

               // Move in the z Vector
               osg::Vec3 movez;
               movez[0] = inMatrix[8];
               movez[1] = inMatrix[9];
               movez[2] = inMatrix[10];
               movez *= inoutOffset[2];
               position += movez;

               inMatrix[12] = position[0];
               inMatrix[13] = position[1];
               inMatrix[14] = position[2];

               osg::Matrix currentMatrix(&inMatrix);

               mSoftAttachList.push_back(currentMatrix);

               if(mSoftAttachList.size() > mAmountOfSoftAttach)
               {
                  osg::Matrix matrixLoving = mSoftAttachList.front();
                  mSoftAttachList.pop_front();

                  for(int i = 0 ; i < 16; ++i)
                     outMatrix[i] = matrixLoving.ptr()[i];
               }
            }
            break;

            case VELOCITY_FOLLOW_MODE:
            {
               if(drawableForISectorTests == NULL)
               {
                  LOG_WARNING("DrawableforISectorTests for Matrix_Manipulations was passed in NULL, and i needed that to do my math");
                  return;
               }
               dtCore::Transform camera2;

               osg::Vec3 originalPosition;
               originalPosition.set(inMatrix[12], inMatrix[13], inMatrix[14]);

               osg::Vec3 position;
               position[0]= inMatrix[12];
               position[1]= inMatrix[13];
               position[2]= inMatrix[14];

               //// Move in the X vector
               osg::Vec3 moveX;
               moveX[0] = inMatrix[0];
               moveX[1] = inMatrix[1];
               moveX[2] = inMatrix[2];
               moveX *= inoutOffset[0];
               position += moveX;

               //// Move in the Y vector
               osg::Vec3 movey;
               movey[0] = inMatrix[4];
               movey[1] = inMatrix[5];
               movey[2] = inMatrix[6];
               movey *= inoutOffset[1];
               position += movey;

               // Move in the z Vector
               osg::Vec3 movez;
               movez[0] = inMatrix[8];
               movez[1] = inMatrix[9];
               movez[2] = inMatrix[10];
               movez *= inoutOffset[2];
               position += movez;

               inMatrix[12] = position[0];
               inMatrix[13] = position[1];
               inMatrix[14] = position[2];

               osg::Matrix currentMatrix(&inMatrix);
               camera2.Set(inMatrix[12], inMatrix[13], inMatrix[14], originalPosition[0],
                                                                  originalPosition[1],
                                                                  originalPosition[2],0,0,1);

               mIsector->SetGeometry(drawableForISectorTests);

               // Obtain the camera position
               osg::Vec3 xyz, hpr;
               camera2.GetTranslation(xyz);
               // Ensure isector ray is in the correct
               // position, orientation and length.
               osg::Vec3 vec( 0.0f, 0.0f, 1.0f );
               float isectorSpan = 20;
               mIsector->Reset();
               mIsector->SetStartPosition( xyz + osg::Vec3(0.0f,0.0f,-isectorSpan) );
               mIsector->SetDirection( vec );
               mIsector->SetLength( isectorSpan * 2.0f );

               // If there was a collision...
               if( mIsector->Update() )
               {
                  // Get the collision point
                  osg::Vec3 hitPt( xyz[0], xyz[1], 0.0f );
                  mIsector->GetHitPoint( hitPt, 0 );

                  // Account for clearance from the ground
                  hitPt[2] += 3;

                  // Correct camera Z position if
                  // camera is inside the terrain.
                  if( hitPt[2] >= xyz[2] )
                  {
                     //set our new position/rotation
                     xyz[2] = hitPt[2];
                     camera2.SetTranslation(xyz);
                     camera2.Set(xyz, originalPosition, osg::Vec3(0,0,1));
                  }
               }

               osg::Matrix outOsgMat;
               camera2.Get(outOsgMat);
               for(int i = 0 ; i < 16; ++i)
                  outMatrix[i] = outOsgMat.ptr()[i];
            }
            break;

            case FLY_MODE:
            case SIDE_VIEW_LEFT:
            case SIDE_VIEW_RIGHT:
            case REARVIEW:
            case MAX_MANIPULATIONS_MODES:
            default:
            {
               LOG_WARNING("Case not handles in MatrixManipulation class!");
               return;
            }
            break;
         }
      }

      void Matrix_Manipulations::HPRClamp(osg::Vec3& clampValues, osg::Vec3& originalOrientateClamp,
                                          osg::Vec3& modifiedOrientateClamp, osg::Vec3& outGoingOrientateClamp)
      {
         float CapArray[3][2];
         CapArray[0][0] = originalOrientateClamp[0] + clampValues[0];
         CapArray[0][1] = originalOrientateClamp[0] - clampValues[0];
         CapArray[1][0] = originalOrientateClamp[1] + clampValues[1];
         CapArray[1][1] = originalOrientateClamp[1] - clampValues[1];

         float subTractAmount[] = {0.0f, 0.0f, 0.0f};
         for(int i = 0 ; i < 2; i++)
         {
            if(CapArray[i][1] < 0)
            {
               CapArray[i][1] = 180 + (180 + CapArray[i][1]);
            }

            if(CapArray[i][1] < CapArray[i][0])
            {
               subTractAmount[i] = CapArray[i][1];
               CapArray[i][0] -= subTractAmount[i];
               CapArray[i][1] = 360.0f - subTractAmount[i];
            }
         }

         osg::Vec3 newOrientateClamp = modifiedOrientateClamp;
         bool hadToNegate[] = {false, false, false};

         for(int i = 0 ; i < 2; i++)
         {
            if(modifiedOrientateClamp[i] < 0)
            {
               hadToNegate[i] = true;
               newOrientateClamp[i] = 180 + (180 + newOrientateClamp[i]);
               newOrientateClamp[i] -= subTractAmount[i];
            }

            if(hadToNegate[i])
            {
               if(newOrientateClamp[i] < CapArray[i][1] && newOrientateClamp[i] > CapArray[i][0])
               {
                  float aValue = sqrt((newOrientateClamp[i] - CapArray[i][1]) * (newOrientateClamp[i] - CapArray[i][1]));
                  float bValue = sqrt((newOrientateClamp[i] - CapArray[i][0]) * (newOrientateClamp[i] - CapArray[i][0]));

                  if(aValue < bValue)
                  {
                     newOrientateClamp[i] = CapArray[i][1];
                  }
                  else
                  {
                     newOrientateClamp[i] = CapArray[i][0];
                  }

                  newOrientateClamp[i] += subTractAmount[i];
                  newOrientateClamp[i] = 180 + (180 - newOrientateClamp[i]);
                  newOrientateClamp[i] *= -1;
                  modifiedOrientateClamp[i] = newOrientateClamp[i];
               }
            }
            else
            {
               if(modifiedOrientateClamp[i] > CapArray[i][0] && modifiedOrientateClamp[i] < CapArray[i][1] )
               {
                  float aValue = sqrt((modifiedOrientateClamp[i] - CapArray[i][1]) * (modifiedOrientateClamp[i] - CapArray[i][1]));
                  float bValue = sqrt((modifiedOrientateClamp[i] - CapArray[i][0]) * (modifiedOrientateClamp[i] - CapArray[i][0]));

                  if(aValue < bValue)
                  {
                     modifiedOrientateClamp[i] = CapArray[i][1];
                  }
                  else
                  {
                     modifiedOrientateClamp[i] = CapArray[i][0];
                  }
               }
            }
         }

         outGoingOrientateClamp[0] = modifiedOrientateClamp[0];
         outGoingOrientateClamp[1] = modifiedOrientateClamp[1];
         outGoingOrientateClamp[2] = modifiedOrientateClamp[2];

      } // function
   } // namespace
} // namespace

