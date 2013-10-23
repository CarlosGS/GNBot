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
	union() {
	translate([(axis_distance+servo_axis_offset+2*margin)/2,0,0]) rotate([90,0,0]) bcube([axis_distance+servo_axis_offset+2*margin,servo_width+2*margin,height],cr=3,cres=3);

	translate([axis_distance+servo_axis_offset+margin,0,0]) rotate([90,0,0]) cylinder(r=axis_radius*2.5,h=servo_axis_length,$fn=20);
	}

	translate([(servo_length/2)+margin,0,0]) rotate([90,0,0]) bcube([servo_length,servo_width,height+1],cr=2,cres=2);

	translate([(servo_length/2)+margin*2+servo_length,0,0]) rotate([90,0,0]) bcube([servo_length,servo_width,height+1],cr=2,cres=2);

	translate([axis_distance+servo_axis_offset+margin,0,0]) rotate([90,0,0]) cylinder(r=axis_radius,h=20*height,center=true,$fn=20);

	cube([margin*4,height*2,10],center=true);
}