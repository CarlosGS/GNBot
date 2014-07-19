// This file is part of GNBot (https://github.com/carlosgs/GNBot)
// by Carlosgs (http://carlosgs.es)
// CC-BY-SA license (http://creativecommons.org/licenses/by-sa/3.0/)

use <target_support.scad>

w = 40;
l = w*2;
t = 2;
tolerance = 0.5;

res = 60;

angleTag_len = 15;


//linear_extrude(height=t/2)
//	circle(r=w/4);

module firstTagEar() {
w = 40;
l = w*2;
t = 2;
tolerance = 0.5;

res = 60;

//linear_extrude(height=t/2)
//	circle(r=w/4);

difference() {
	linear_extrude(height=t)
		union() {
			translate([-w/2,tolerance]) square([w,l/4-tolerance]);
			translate([0,l/4]) circle(r=w/2, $fn=res);
		}
	cylinder(h=t*4, r=w/8, $fn=res, center=true);
}
}

module tag() {
difference() {
	linear_extrude(height=t)
		union() {
			square([w,l/2],center=true);
			translate([0,l/4]) circle(r=w/2, $fn=res);
			translate([0,-l/4]) circle(r=w/2, $fn=res);
		}
	cylinder(h=t*4, r=w/8+tolerance, $fn=res, center=true);
	translate([w/2,0,0]) cylinder(h=t*4, r=w/4+tolerance, $fn=res, center=true);
}
}

module angleTag() {
	translate([w/2,0,0])
	linear_extrude(height=t)
		union() {
			translate([0,-w/4]) square([angleTag_len,w/2]);
			circle(r=w/4, $fn=res);
			translate([angleTag_len,0]) circle(r=w/4, $fn=res);
		}
}



translate([0,0,0]) {


//union() {
color("lightgreen") {
	tag();
	cylinder(h=t, r=w/8, $fn=res);
}
color("yellow") angleTag();//translate([w/2,0,0]) cylinder(h=t, r=w/4, $fn=res);
//translate([0,0.01,0]) rotate([0,0,180]) tag();
//}

translate([-50/2,-50/2,-25]) rotate([-60,0,0]) color("lightgrey") translate([0,0,-30]) support();

color("lightgrey") translate([0,0,-30]) support();

translate([-50,-50,0]) {

color("lightgrey") cylinder(h=t+0.1, r=w/8-0.5, $fn=res);
color("lightgreen") firstTagEar();
color("yellow") mirror([0,1,0]) firstTagEar();
color("lightgrey") translate([0,0,-30]) support();
}
}
