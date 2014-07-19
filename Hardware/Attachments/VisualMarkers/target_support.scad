// This file is part of GNBot (https://github.com/carlosgs/GNBot)
// by Carlosgs (http://carlosgs.es)
// CC-BY-SA license (http://creativecommons.org/licenses/by-sa/3.0/)

module support() {
w = 40;
l = w*2;
t = 2;

res = 60;

cyl_btm_r = 15/2;

h = 30;

hBtm = h*0.8;
hTop = h-hBtm;

nut_d = 7.3; // M3
screw_d = 4.1; // M3

N_drills = 8;
drill_d = 3.1;

intersection() {
	difference() {
		union() {
			cylinder(r1=cyl_btm_r, r2=w/4, h=hBtm, $fn=res);
			translate([0,0,hBtm]) cylinder(r1=w/4, r2=w/2+1, h=hTop, $fn=res);
		}
		translate([0,0,3]) cylinder(r=nut_d/2,h=h,$fn=6,center=false);
		cylinder(r=screw_d/2,h=h,$fn=30,center=true);
		for(i = [1:N_drills])
			rotate([0,0,(i-1)*(360/N_drills)])
				translate([14,0,h-2]) {
					cylinder(r=drill_d/2,h=h*4,$fn=30,center=true);
					scale([1,1,-1]) cylinder(r=drill_d,h=h*4,$fn=30);
					}
	}
	cylinder(r=w/2,h=h,$fn=res);
}
}

