---------------------------------------------------------------------------
DOF and Model Orientation and Origin
---------------------------------------------------------------------------

Models and DOFs should be oriented so that their front is along the world's
Y-axis.

X-axis = Right
Y-axis = Forward
Z-axis = Up

Complete models assume the world origin to be ground level. In the case of
vehciles, the whole vehicle with wheels should be placed so that the wheels
rest on the modeling grid at 0 on the Z-axis.


---------------------------------------------------------------------------
Wheel DOFs
---------------------------------------------------------------------------

Wheel DOFs Naming Convention:
DOF_Wheel_[L|R|C]_##

[L|R|C] evaluates to one of the letters contained in the [] brackets.
L = Left
R = Right
C = Center

## evaluates to a two digit number.
Numbers start at 01 and begin at the front of the vehicle.
Numbers increment when moving towards the back of the vehicle.

Example for a 4-wheeled vehicle with two left wheels and two right wheels:
DOF_Wheel_L_01
DOF_Wheel_L_02
DOF_Wheel_R_01
DOF_Wheel_R_02



---------------------------------------------------------------------------
View DOFs
---------------------------------------------------------------------------

View DOF Naming Convention:
DOF_View_##

## evaluates to a two digit number, starting at 01.