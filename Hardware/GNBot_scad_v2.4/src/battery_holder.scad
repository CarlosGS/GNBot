//---------------------------------------------------------
//-- Battery holder for Miniskybot 2. It is based on
//-- the parameterized batery_holder of Miniskybot 1, but
//-- with 4 "ears" for screwing to the chassis
//---------------------------------------------------------
include <configuration.scad>

//-------------------------------
//-- Battery holder ears
//-- Parameters:
//--   do: outer diameter
//--   h : ear height
//-------------------------------
module ear(do, h)
{
  translate([-do/2,do/2,0])
  difference() {
    union() {
      cylinder(r=do/2, h=h,$fn=40);

      translate([do/4,0,h/2])
      cube([do/2,do,h],center=true);
    }
    cylinder(r=drill_M3/2, h=h+10,$fn=30,center=true);
  }
}

len = 6;

translate([0,-battery_c2/2,0]) {
  ear(do=battery_ear_diam, h=battery_ear_h);
  difference() {
  cube([len,battery_c2,battery_ear_h+3]);
  translate([len,0,0])
    rotate([0,-10,0])
      cube([len,battery_c2,battery_ear_h+10]);
  }
}
