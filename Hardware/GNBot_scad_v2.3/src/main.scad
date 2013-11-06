// GNBot by Carlosgs (https://github.com/carlosgs/GNBot)
// Based on:
//--------------------------------------------------------------------------
// Miniskybot 2.0
// (c) Juan Gonzalez-Gomez, March-2012
// Robotics and Cybernetics group. Universidad Politecnica de Madrid
//--------------------------------------------------------------------------
//-- CREDITS:
//--  The ball caster have been designed by Sliptonic
//   http://www.thingiverse.com/thing:13782
//--------------------------------------------------------------------------
// This code is under the GPL license
//--------------------------------------------------------------------------
include <configuration.scad>
include <functions.scad>
use <battery_holder.scad>
use <Servo-wheel.scad>
use <fake_ultrasound.scad>

ARDUINO_OFFSET = 20;
THIN_SURFACE_H = 0.3*4;//0.3mm x4 layers

layer_height = 0.35;

top_plate_thickness = 6;//4; // Thicker plate to add strength
robot_height = 48+top_plate_thickness-4;
ball_caster_height = robot_height-(BallSize*BallProtrude);

$fn=60;

//-- Skymega
skymega_lx = 51.4;
skymega_ly = 51.4;
skymega_width=3;
skymega_dx = 15;
skymega_dy = 15;

frame_corner_radius = 3;
top_plate_c1 = battery_c1 + battery_ear_diam+rear_c1;
top_plate_c2 = rear_c2;
front_thickness = 3;


//-- Printing mode
printing_mode = true;

//-- Visualization flags
show_servos = false;
show_battery = false;
show_wheels = false;
show_ultrasound = false;

//-- Rear part body
module rear_part_body()
{
  difference() {

    //-- Main part
    translate([-rear_c1,0,servo_c1-rear_c3])
    rounded_box_half([rear_c1,rear_c2,rear_c3],r=rear_edge_rad);

    //-- Bottom drills for the battery holder
    translate([-battery_ear_diam/2, (rear_c2-battery_c2)/2+battery_ear_diam/2,
          servo_c1-rear_c3+2])
    cylinder(r=drill_M3/2,h=8, $fn=20);

    translate([-battery_ear_diam/2,
          (rear_c2-battery_c2)/2+battery_ear_diam/2+battery_c2-battery_ear_diam,
          servo_c1-rear_c3+2])
    cylinder(r=drill_M3/2,h=8, $fn=20);

   //-- Bottom drills, part 2. For making the part printable, there cannot be
   //-- one cylinder but two. When assembling the robot, these holes should be
   //-- open manually
   translate([-battery_ear_diam/2, (rear_c2-battery_c2)/2+battery_ear_diam/2,
          servo_c1-rear_c3-1])
    cylinder(r=drill_M3/2,h=2, $fn=20);

    translate([-battery_ear_diam/2,
          (rear_c2-battery_c2)/2+battery_ear_diam/2+battery_c2-battery_ear_diam,
          servo_c1-rear_c3-1])
    cylinder(r=drill_M3/2,h=2, $fn=20);

    //-- Space for the captive nuts
    translate([-battery_ear_diam/2, (rear_c2-battery_c2)/2+battery_ear_diam/2,
          servo_c1-rear_c3+nut_h/2+2])
    rotate([0,0,180])
    captive_nut(10);

    translate([-battery_ear_diam/2,
          (rear_c2-battery_c2)/2+battery_ear_diam/2+battery_c2-battery_ear_diam,
          servo_c1-rear_c3+nut_h/2+2])
    rotate([0,0,180])
    captive_nut(10);

    //-- Drills for the servos
    translate([-servo_c8,-1,servo_c9])
    rotate([-90,0,0])
    cylinder(r=drill_M3/2, h=10, $fn=20);

    translate([-servo_c8,rear_c2-9,servo_c9])
    rotate([-90,0,0])
    cylinder(r=drill_M3/2, h=10, $fn=20);

    //-- Captive nuts for the servo's bolts
    color("green")
    translate([-servo_c8,nut_h/2+2,servo_c9])
    rotate([90,0,0])
    rotate([0,0,180])
    captive_nut(10);  

    color("green")
    translate([-servo_c8,-nut_h/2+rear_c2-2,servo_c9])
    rotate([90,0,0])
    rotate([0,0,180])
    captive_nut(10); 

  }

    *color("red")
    //translate([-battery_ear_diam/2, (rear_c2-battery_c2)/2+battery_ear_diam/2,
          //servo_c1-rear_c3-1])
    translate([-battery_ear_diam/2,(rear_c2-battery_c2)/2+battery_ear_diam/2,
          servo_c1-rear_c3+1])
    cylinder(r=drill_M3/2+0.5,h=1, $fn=20);

}

//-- Final rear part
module rear_part()
{
  total_height = robot_height;
  cyl_height = total_height;
  cylrad = (BallSize/2) + WallThickness + Airgap;

  ball_caster_height = total_height-(BallSize*BallProtrude);

  //-- top_plate_thickness
  //translate([0,0,top_plate_thickness])
  difference() {
    union() {
      rear_part_body();
      translate([-cylrad,rear_c2/2,-ball_caster_height +servo_c1+top_plate_thickness])
      cylinder(r=cylrad, h=ball_caster_height);
    }
   translate([-cylrad,rear_c2/2,-cyl_height/2 + (BallSize/2)+top_plate_thickness])
      cube([cylrad/2, cylrad*2+5, BallSize*1.25],center=true);
    
   translate([-cylrad,rear_c2/2,-cyl_height/2 + (BallSize/2)+top_plate_thickness-3]) 
      #sphere (BallSize/2+Airgap, $fa=5, $fs=0.1);

   translate([-cylrad,rear_c2/2,-cyl_height/2+top_plate_thickness+BallSize/3]) 
   difference() {
        scale([1,1,-1]) cylinder(r=cylrad+5, h=ball_caster_height);
        translate([0,0,9])
        sphere (cylrad+5, $fa=5, $fs=0.1);
   }

   translate([-rear_c1-8,rear_c2/2,servo_c3+5])
   rotate (a = [0,20,0])
   cube ([20,23,80],center=true);

  }

}


module front_part(l=0)
{
  difference() {
    union() {

      //-- Right "leg". It is smalle than the left. Just for having space
      //-- for the battery wires
      translate([servo_c4,0,4])
      cube([battery_c1-servo_c4,5,servo_c1-4]);

      translate([servo_c4,2*servo_c3-5,0])
      cube([battery_c1-servo_c4,5,servo_c1]);

      translate([battery_c1+l,0,-battery_height+3])
      translate([battery_ear_diam,rear_c2,0])
      rotate([0,0,180])
      rounded_box_half([battery_ear_diam+l, rear_c2, rear_c3],r=rear_edge_rad);
    }
    translate([battery_c1-front_thickness,battery_ear_diam-(battery_c2-rear_c2)/2,
               -rear_c3+servo_c1-5])
    cube([battery_ear_diam+l, battery_c2-2*battery_ear_diam, rear_c3+10]);


//  ---------- FRONT TEXT ----------------
rotate([90,0,90])
	translate([3.5+3,-7,61.2])
		scale([1.1,1.2,1])
			linear_extrude (height=10, convexity=2) import (file="Text_GNBot.dxf");

    //-- servo drills
    translate([servo_c4+servo_c8,-1,servo_c9+servo_c10])
    rotate([-90,0,0])
    cylinder(r=drill_M3/2, h=2*servo_c3+2, $fn=20);

    //-- Servo Captive nuts
    translate([servo_c4+servo_c8,2,servo_c9+servo_c10])
    rotate([-90,0,0])
    cylinder(r=nut_radius, h=2*servo_c3-4, $fn=6);

    //-- Bottom drills for the battery holder
    translate([battery_c1+battery_ear_diam/2,
              (rear_c2-battery_c2)/2+battery_ear_diam/2,servo_c1-rear_c3-1])
    cylinder(r=drill_M3/2,h=10, $fn=20);

    translate([battery_c1+battery_ear_diam/2,
              (rear_c2-battery_c2)/2+battery_ear_diam/2+battery_c2-battery_ear_diam,
              servo_c1-rear_c3-1])
    cylinder(r=drill_M3/2,h=10, $fn=20);

    //-- Space for the captive nuts
    translate([battery_c1+battery_ear_diam/2,
               (rear_c2-battery_c2)/2+battery_ear_diam/2,
               nut_h/2+servo_c1-rear_c3+2])
    captive_nut(10); 

    translate([battery_c1+battery_ear_diam/2,
               (rear_c2-battery_c2)/2+battery_ear_diam/2+battery_c2-battery_ear_diam,
               nut_h/2+servo_c1-rear_c3+2])
    captive_nut(10); 
/*
    //-- Ultrasound hole
    translate([battery_c1+battery_ear_diam+l,
               rear_c2/2,servo_c1-ultrasound_hole/2-5])
    rotate([0,90,0])
    rotate([0,0,-90])
    translate([0,0,-5])
    teardrop(r=ultrasound_hole/2, h=10);

    //-- Ultrasound drills
    translate([battery_c1+battery_ear_diam+l,
               rear_c2/2 -8,
               servo_c1-ultrasound_hole/2-5-9])
    rotate([0,90,0])
    cylinder(r=drill_M3/2, h=20,center=true, $fn=20);

    translate([battery_c1+battery_ear_diam+l,
               rear_c2/2 +7,
               servo_c1-ultrasound_hole/2-5+9])
    rotate([0,90,0])
    cylinder(r=drill_M3/2, h=20,center=true, $fn=20);
*/
  }

   //-- Bottom drills, part 2. For making the part printable, there cannot be
   //-- one cylinder but two. When assembling the robot, these holes should be
   //-- open manually
   translate([battery_c1+battery_ear_diam/2,
              (rear_c2-battery_c2)/2+battery_ear_diam/2,
              servo_c1-rear_c3+2-layer_height])
    cylinder(r=drill_M3/2,h=layer_height, $fn=20);

   translate([battery_c1+battery_ear_diam/2,
              (rear_c2-battery_c2)/2+battery_ear_diam/2+battery_c2-battery_ear_diam,
              servo_c1-rear_c3+2-layer_height])
    cylinder(r=drill_M3/2,h=layer_height, $fn=20);
    
}

module skymega()
{
  color([0,0,1])
  difference() {
     roundedBox([skymega_lx, skymega_ly, 3], r=frame_corner_radius);
    translate([skymega_dx, skymega_dy,0])
      cylinder(r=drill_M3/2, h=20, center=true, $fn=20);

    translate([-skymega_dx, skymega_dy,0])
      cylinder(r=drill_M3/2, h=20, center=true, $fn=20);

    translate([skymega_dx, -skymega_dy,0])
      cylinder(r=drill_M3/2, h=20, center=true, $fn=20);

    translate([-skymega_dx, -skymega_dy,0])
      cylinder(r=drill_M3/2, h=20, center=true, $fn=20);
   }
}

module top_plate_skymega(l=0)
{
  translate([(top_plate_c1+l)/2 - rear_c1,
           rear_c2/2,
           top_plate_thickness/2+servo_c1])
  difference() {
    roundedBox([top_plate_c1+l,top_plate_c2,top_plate_thickness],
                r=frame_corner_radius);

     translate([-(battery_ear_diam+l+5)/2+(top_plate_c1+l)/2-front_thickness-2,0,0])
    roundedBox([battery_ear_diam+l+5, battery_c2-2*battery_ear_diam, rear_c3+10],
               r=frame_corner_radius);

    translate([-(top_plate_c1+l-skymega_lx)/2 + 3,0,0])
    union() {
    translate([skymega_dx, skymega_dy,0])
      cylinder(r=drill_M3/2, h=20, center=true, $fn=20);

    translate([-skymega_dx, skymega_dy,0])
      cylinder(r=drill_M3/2, h=20, center=true, $fn=20);

    translate([skymega_dx, -skymega_dy,0])
      cylinder(r=drill_M3/2, h=20, center=true, $fn=20);

    translate([-skymega_dx, -skymega_dy,0])
      cylinder(r=drill_M3/2, h=20, center=true, $fn=20);

    roundedBox([2*skymega_dx-10, top_plate_c2-20,top_plate_thickness+10],frame_corner_radius,true,$fn=20);
    }

    //-- Captive nuts for the skymega
    translate([-(top_plate_c1+l-skymega_lx)/2 + 3,0,0])
    union() {
      translate([skymega_dx, skymega_dy,-top_plate_thickness+nut_h])
      cylinder(r=nut_radius, h=top_plate_thickness,center=true,$fn=6);

      translate([-skymega_dx, skymega_dy,-top_plate_thickness+nut_h])
      cylinder(r=nut_radius, h=top_plate_thickness,center=true,$fn=6);

      translate([skymega_dx, -skymega_dy,-top_plate_thickness+nut_h])
      cylinder(r=nut_radius, h=top_plate_thickness,center=true,$fn=6);

      translate([-skymega_dx, -skymega_dy,-top_plate_thickness+nut_h])
      cylinder(r=nut_radius, h=top_plate_thickness,center=true,$fn=6);
    }

  }
  
}


module top_plate_arduino_uno(l=2)
{
  top_plate_c2 = rear_c2+5;

  trans1 = [(top_plate_c1+l)/2-rear_c1-nut_radius-1,
             -top_plate_c2/2+nut_radius+1
             ,0];

  trans2 = [(top_plate_c1+l)/2 - rear_c1,
           rear_c2/2,
           top_plate_thickness/2+servo_c1];

  
  translate([-ARDUINO_OFFSET,0,0])
  translate(trans2)
  translate(-trans1)
  difference() {
    translate(trans1)
    difference() {
      translate([ARDUINO_OFFSET,0,0])
      roundedBox([top_plate_c1+l,top_plate_c2,top_plate_thickness],
                  r=frame_corner_radius);
      translate([ARDUINO_OFFSET,0,0])
      translate([-(battery_ear_diam+l+5)/2+(top_plate_c1+l)/2-front_thickness-2-4+3,0,0])
      roundedBox([battery_ear_diam+l+5-4-10, battery_c2-2*battery_ear_diam-10, rear_c3+10],
               r=frame_corner_radius);

      translate([-(top_plate_c1+l-skymega_lx)/2 + 3,0,0])
/*      union() {
      translate([skymega_dx, skymega_dy,0])
        cylinder(r=drill_M3/2, h=20, center=true, $fn=20);

      translate([-skymega_dx, skymega_dy,0])
        cylinder(r=drill_M3/2, h=20, center=true, $fn=20);
  
      translate([skymega_dx, -skymega_dy,0])
        cylinder(r=drill_M3/2, h=20, center=true, $fn=20);

      translate([-skymega_dx, -skymega_dy,0])
        cylinder(r=drill_M3/2, h=20, center=true, $fn=20);*/

// CHASIS ROUNDED HOLE
	  translate([10+20/2,0,-THIN_SURFACE_H])
      	roundedBox([2*skymega_dx+18-14, top_plate_c2-20,top_plate_thickness],frame_corner_radius,true,$fn=20);
      //}

      //-- Captive nuts for the skymega
      /*translate([-(top_plate_c1+l-skymega_lx)/2 + 3,0,0])
      union() {
        translate([skymega_dx, skymega_dy,-top_plate_thickness+nut_h])
        cylinder(r=nut_radius, h=top_plate_thickness,center=true,$fn=6);

        translate([-skymega_dx, skymega_dy,-top_plate_thickness+nut_h])
        cylinder(r=nut_radius, h=top_plate_thickness,center=true,$fn=6);

        translate([skymega_dx, -skymega_dy,-top_plate_thickness+nut_h])
        cylinder(r=nut_radius, h=top_plate_thickness,center=true,$fn=6);

        translate([-skymega_dx, -skymega_dy,-top_plate_thickness+nut_h])
        cylinder(r=nut_radius, h=top_plate_thickness,center=true,$fn=6);
      }*/
    }
   translate([ARDUINO_OFFSET,0,0]) {
    //-- Arduino drill 1 (top-left)
    cylinder(r=drill_M3/2, h=top_plate_thickness+10,center=true, $fn=20);

    //-- Captive nut for drill 1
    translate([0, 0,-THIN_SURFACE_H])
    cylinder(r=nut_radius, h=top_plate_thickness,center=true,$fn=6);

    //-- Arduino drill 2 (bottom-left)
    translate([-1.1,-48.4,0])
    cylinder(r=drill_M3/2, h=top_plate_thickness+10,center=true, $fn=20);

    //-- Captive nut for drill 2
    translate([-1.1,-48.4,-THIN_SURFACE_H])
    cylinder(r=nut_radius, h=top_plate_thickness,center=true,$fn=6);

    //-- Arduino drill 3 (top-right)
    translate([51,-15.3,0])
    cylinder(r=drill_M3/2, h=top_plate_thickness+10,center=true, $fn=20);

    //-- Captive nut for drill 3
    translate([51,-15.3,-THIN_SURFACE_H])
    cylinder(r=nut_radius, h=top_plate_thickness,center=true,$fn=6);
   }

    //-- Arduino drill 3 (top-right)
    translate([51,-15.3,0])
    cylinder(r=drill_M3/2, h=top_plate_thickness+10,center=true, $fn=20);

    //-- Captive nut for drill 3
    translate([51,-15.3,-THIN_SURFACE_H])
    cylinder(r=nut_radius, h=top_plate_thickness,center=true,$fn=6);

    //-- Arduino drill 4 (bottom-right)
    translate([51,-43.3,0])
    cylinder(r=drill_M3/2, h=top_plate_thickness+10,center=true, $fn=20);

    //-- Captive nut for drill 4
    translate([51,-43.3,-THIN_SURFACE_H])
    cylinder(r=nut_radius, h=top_plate_thickness,center=true,$fn=6);

  }
    
}


*translate([-(top_plate_c1-skymega_lx)/2 + 3,0,top_plate_thickness/2+3/2])
skymega();

module miniskybot_frame(l=0)
{
  union() {
    //top_plate_skymega(l);
    top_plate_arduino_uno(4);
    front_part(4);
    rear_part();
  }
}

module show_robot()
{
  miniskybot_frame();
 
  //-- Servos
  if (show_servos) {
    //-- Right servo
    color([0.2,0.2,0.2])
    translate([servo_c4/2,servo_c3,servo_c1/2])
    rotate([90,0,0])
    import("futaba3003/futaba.stl");

    //-- Left servo
    color([0.2,0.2,0.2])
    translate([servo_c4/2,servo_c3+0.1,servo_c1/2])
    rotate([-90,0,0])
    import("futaba3003/futaba.stl");
  }

  //-- Battery holder
  if (show_battery) {
    translate([battery_c1/2,rear_c2/2,-battery_c3/2-battery_top_gap])
    color("blue") battery();
  }

  //-- Wheels  
  if (show_wheels) {
    color("green")
    translate([servo_c5,-servo_c7-wheel_height/2-wheel_gap,servo_c6])
    rotate([90,0,0])
    Servo_wheel();

    color("green")
    translate([servo_c5,wheel_height/2+2*servo_c3+servo_c7+wheel_gap,servo_c6])
    rotate([90,0,0])
    Servo_wheel();
  }

  l=4;

  if (show_ultrasound) {
    translate([battery_c1+battery_ear_diam+l-ultrasound_base_lz-front_thickness,
               rear_c2/2,servo_c1-ultrasound_hole/2-5])
    rotate([0,90,0])
    fake_ultrasound_srf02();
  }

}

if (printing_mode) {
  //-- Rotate and translate the frame so that is is printable
  rotate([0,180,0]) 
  translate([0,0,-servo_c1-top_plate_thickness])
  miniskybot_frame();
}
else {
  show_robot();
}


