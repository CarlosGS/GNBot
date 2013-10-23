// Chassis

use <obiscad/bcube.scad>

axis_distance = 96;
margin = 5;
height = 10;
servo_width = 20.5;
servo_length = 41.5;
servo_axis_offset = 10;
servo_axis_length = 18+5;

axis_radius = 3.5/2;



rotate([-90,0,0])
difference() {
	translate([(axis_distance+axis_radius*4)/2,0,0]) rotate([90,0,0]) bcube([axis_distance+axis_radius*4,axis_radius*4,axis_radius*2],cr=3,cres=3);

	translate([axis_distance+axis_radius*2,0,0]) rotate([90,0,0]) cylinder(r=axis_radius,h=20*height,center=true,$fn=20);

	translate([axis_radius*2,0,0]) rotate([90,0,0]) cylinder(r=axis_radius,h=20*height,center=true,$fn=20);
}